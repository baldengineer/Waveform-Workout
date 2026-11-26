/* SPDX-License-Identifier: MIT */
#include "usb_console.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/usb/usbd.h>

/* STM32L072 USB requires VDD >= 3.0 V. Hysteresis avoids repeated enumeration. */
#define USB_ATTACH_MV 3100
#define USB_DETACH_MV 3000

static const struct gpio_dt_spec vbus =
	GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), vbus_present_gpios);
static const struct device *const supply = DEVICE_DT_GET(DT_ALIAS(volt_sensor0));
static bool attached;

USBD_DEVICE_DEFINE(trainer_usb, DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)),
		   CONFIG_WORKOUT_USB_VID, CONFIG_WORKOUT_USB_PID);
USBD_DESC_LANG_DEFINE(usb_lang);
USBD_DESC_MANUFACTURER_DEFINE(usb_mfr, "BaldEE");
USBD_DESC_PRODUCT_DEFINE(usb_product, "Waveform Workout");
USBD_DESC_SERIAL_NUMBER_DEFINE(usb_serial);
USBD_DESC_CONFIG_DEFINE(usb_config_desc, "CDC ACM shell");
/* bMaxPower is in 2 mA units: 100 mA maximum from VBUS. */
USBD_CONFIGURATION_DEFINE(usb_config, 0, 50, &usb_config_desc);

static void check_power(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(power_work, check_power);

static void check_power(struct k_work *work)
{
	ARG_UNUSED(work);
	struct sensor_value voltage;
	bool enable = false;

	/* GPIO_ACTIVE_LOW in the board definition accounts for the VBUS inverter. */
	if (gpio_pin_get_dt(&vbus) > 0 &&
	    sensor_sample_fetch_chan(supply, SENSOR_CHAN_VOLTAGE) == 0 &&
	    sensor_channel_get(supply, SENSOR_CHAN_VOLTAGE, &voltage) == 0) {
		int mv = voltage.val1 * 1000 + voltage.val2 / 1000;
		enable = attached ? mv >= USB_DETACH_MV : mv >= USB_ATTACH_MV;
	}
	/* Failed power readings also detach. Retry a failed transition on the next poll. */
	if (enable != attached) {
		int rc = enable ? usbd_enable(&trainer_usb) : usbd_disable(&trainer_usb);
		if (!rc || rc == -EALREADY) {
			attached = enable;
		}
	}
	k_work_reschedule(&power_work, K_MSEC(100));
}

int usb_console_init(void)
{
	if (!gpio_is_ready_dt(&vbus) || !device_is_ready(supply)) {
		return -ENODEV;
	}
	int rc = gpio_pin_configure_dt(&vbus, GPIO_INPUT);
	if (rc) {
		return rc;
	}
	struct usbd_desc_node *const descriptors[] = {
		&usb_lang, &usb_mfr, &usb_product, &usb_serial,
	};
	for (size_t i = 0; i < ARRAY_SIZE(descriptors); ++i) {
		rc = usbd_add_descriptor(&trainer_usb, descriptors[i]);
		if (rc) {
			return rc;
		}
	}
	rc = usbd_add_configuration(&trainer_usb, USBD_SPEED_FS, &usb_config);
	if (rc) {
		return rc;
	}
	rc = usbd_register_class(&trainer_usb, "cdc_acm_0", USBD_SPEED_FS, 1);
	if (rc) {
		return rc;
	}
	/* CDC ACM has two interfaces joined by an Interface Association Descriptor. */
	rc = usbd_device_set_code_triple(&trainer_usb, USBD_SPEED_FS,
					 USB_BCC_MISCELLANEOUS, 0x02, 0x01);
	if (rc) {
		return rc;
	}
	rc = usbd_init(&trainer_usb);
	if (rc) {
		return rc;
	}
	k_work_reschedule(&power_work, K_NO_WAIT);
	return 0;
}
