/* SPDX-License-Identifier: MIT */
#include <zephyr/drivers/sensor.h>
#include <zephyr/usb/usbd.h>

/* Robot reads/writes these ELF symbols while the emulated CPU is paused. */
volatile uint32_t test_vdd_mv = 3447;
volatile uint32_t test_sensor_fault; /* 1: fetch fails, 2: channel read fails */
volatile uint32_t test_fail_next_transition;
volatile uint32_t test_enable_calls;
volatile uint32_t test_disable_calls;
volatile uint32_t test_stack_enabled;

static int fetch(const struct device *dev, enum sensor_channel channel)
{
	return test_sensor_fault == 1 ? -EIO : 0;
}

static int get_voltage(const struct device *dev, enum sensor_channel channel,
		       struct sensor_value *value)
{
	if (channel != SENSOR_CHAN_VOLTAGE) {
		return -ENOTSUP;
	}
	if (test_sensor_fault == 2) {
		return -EIO;
	}
	return sensor_value_from_milli(value, test_vdd_mv);
}

static DEVICE_API(sensor, supply_api) = {
	.sample_fetch = fetch,
	.channel_get = get_voltage,
};
DEVICE_DT_DEFINE(DT_NODELABEL(test_supply), NULL, NULL, NULL, NULL,
		 POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, &supply_api);

int __real_usbd_enable(struct usbd_context *ctx);
int __real_usbd_disable(struct usbd_context *ctx);

static int transition(struct usbd_context *ctx, int (*operation)(struct usbd_context *))
{
	int rc;
	if (test_fail_next_transition) {
		test_fail_next_transition = 0;
		rc = -EIO;
	} else {
		rc = operation(ctx);
	}
	test_stack_enabled = ctx->status.enabled;
	return rc;
}

int __wrap_usbd_enable(struct usbd_context *ctx)
{
	test_enable_calls++;
	return transition(ctx, __real_usbd_enable);
}

int __wrap_usbd_disable(struct usbd_context *ctx)
{
	test_disable_calls++;
	return transition(ctx, __real_usbd_disable);
}
