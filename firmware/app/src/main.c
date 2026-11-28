/* SPDX-License-Identifier: MIT */
#include "outputs.h"
#include "oled.h"
#include "preset_store.h"
#include "trainer.h"
#include "usb_console.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/device_runtime.h>

#define PIXELS DT_ALIAS(led_strip)
BUILD_ASSERT(DT_PROP(PIXELS, chain_length) == 9);
static const struct device *const pixels = DEVICE_DT_GET(PIXELS);
static const struct device *const buttons = DEVICE_DT_GET(DT_PATH(buttons));
static const struct gpio_dt_spec diagnostic = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct device *const pixel_spi = DEVICE_DT_GET(DT_BUS(PIXELS));

static void heartbeat(struct k_timer *timer)
{
	static bool light;
	light = !light;
	gpio_pin_set_dt(&diagnostic, light);
	k_timer_start(timer, light ? K_MSEC(20) : K_MSEC(4980), K_NO_WAIT);
}
K_TIMER_DEFINE(heartbeat_timer, heartbeat, NULL);

static void indicate_fault(void)
{
	k_timer_stop(&heartbeat_timer);
	gpio_pin_set_dt(&diagnostic, 1);
}

struct reply {
	struct k_sem done;
	int rc;
};

enum panel_action { NEXT_MODE, NEXT_VALUE, SET_MODE, SET_VALUE, SET_OLED_ADDRESS };
struct panel_event {
	enum panel_action action;
	uint8_t field;
	uint8_t value;
	int64_t pressed_at;
	struct reply *reply;
};
K_MSGQ_DEFINE(events, sizeof(struct panel_event), 8, 4);
static atomic_t accepting_edits;

static int submit_edit(struct panel_event event)
{
	if (!atomic_get(&accepting_edits)) {
		return -EAGAIN;
	}
	struct reply reply;
	event.reply = &reply;
	k_sem_init(&reply.done, 0, 1);
	k_msgq_put(&events, &event, K_FOREVER);
	k_sem_take(&reply.done, K_FOREVER);
	return reply.rc;
}

int trainer_set(enum mode field, uint8_t value)
{
	if ((unsigned int)field > MODE_COUNT ||
	    value >= (field == MODE_COUNT ? MODE_COUNT : VALUE_COUNT)) {
		return -EINVAL;
	}
	return submit_edit((struct panel_event){
		.action = field == MODE_COUNT ? SET_MODE : SET_VALUE, .field = field, .value = value,
	});
}

int trainer_set_oled_address(uint8_t address)
{
	if (!oled_address_valid(address)) {
		return -EINVAL;
	}
	return submit_edit((struct panel_event){ .action = SET_OLED_ADDRESS, .value = address });
}

static void on_key(struct input_event *event, void *context)
{
	ARG_UNUSED(context);
	/* gpio-keys debounces both edges. A held key generates one press. */
	if (event->type == INPUT_EV_KEY && event->value == 1 &&
	    (event->code == INPUT_KEY_0 || event->code == INPUT_KEY_1)) {
		struct panel_event press = {
			.action = event->code == INPUT_KEY_0 ? NEXT_MODE : NEXT_VALUE,
			.pressed_at = k_uptime_get(),
		};
		k_msgq_put(&events, &press, K_NO_WAIT);
	}
}
INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_PATH(buttons)), on_key, NULL);

static int display(const struct lesson *lesson)
{
	struct led_rgb frame[DT_PROP(PIXELS, chain_length)] = {0};
	if (lesson) {
		frame[lesson_mode_pixel(lesson)].g = 4;
		frame[lesson_value_pixel(lesson)].g = 4;
	}
	int rc = pm_device_runtime_get(pixel_spi);
	if (rc) {
		return rc;
	}
	rc = led_strip_update_rgb(pixels, frame, ARRAY_SIZE(frame));
	int power_rc = pm_device_runtime_put(pixel_spi);
	return rc ? rc : power_rc;
}

static int run_trainer(void)
{
	struct lesson lesson = LESSON_DEFAULT;
	if (IS_ENABLED(CONFIG_WORKOUT_FILTER_TEST)) {
		return outputs_apply(&lesson);
	}
	uint8_t oled_address = OLED_ADDRESS_DEFAULT;
	int load_rc = preset_load(&lesson, &oled_address);
	if (load_rc && load_rc != -ENOENT) {
		indicate_fault();
	}
	int rc = outputs_apply(&lesson);
	if (rc) {
		return rc;
	}
	const uint8_t boot_oled_address = oled_address;
	int64_t panel_until = 0;
	bool outputs_running = true;
	atomic_set(&accepting_edits, 1);
	for (;;) {
		struct panel_event event;
		k_timeout_t timeout = panel_until ? K_TIMEOUT_ABS_MS(panel_until) : K_FOREVER;
		if (k_msgq_get(&events, &event, timeout) != 0) {
			if (display(NULL)) {
				indicate_fault();
			}
			if (IS_ENABLED(CONFIG_WORKOUT_OLED)) {
				oled_sleep();
			}
			panel_until = 0;
			continue;
		}
		bool press = event.action == NEXT_MODE || event.action == NEXT_VALUE;
		bool wake_only = press && event.pressed_at >= panel_until;
		if (press) {
			panel_until = event.pressed_at + 2000;
		}
		rc = 0;
		if (!wake_only) {
			struct lesson next = lesson;
			uint8_t next_address = oled_address;
			switch (event.action) {
			case NEXT_MODE: lesson_next_mode(&next); break;
			case NEXT_VALUE: lesson_next_value(&next); break;
			case SET_MODE: next.mode = event.value; break;
			case SET_VALUE: next.value[event.field] = event.value; break;
			case SET_OLED_ADDRESS: next_address = event.value; break;
			}
			if (!outputs_running || lesson_output_changed(&lesson, &next)) {
				rc = outputs_apply(&next);
				outputs_running = rc == 0;
			}
			if (!rc) {
				lesson = next;
				rc = preset_save(&lesson, next_address);
				if (!rc) {
					oled_address = next_address;
				}
			}
		}
		if (k_uptime_get() < panel_until) {
			int led_rc = display(&lesson);
			if (!rc) {
				rc = led_rc;
			}
			if (IS_ENABLED(CONFIG_WORKOUT_OLED)) {
				if (wake_only) {
					oled_wake(boot_oled_address);
				}
				oled_show(&lesson);
			}
		}
		if (rc) {
			indicate_fault();
		}
		if (event.reply) {
			event.reply->rc = rc;
			k_sem_give(&event.reply->done);
		}
	}
}

int main(void)
{
	if (!gpio_is_ready_dt(&diagnostic) ||
	    gpio_pin_configure_dt(&diagnostic, GPIO_OUTPUT_INACTIVE)) {
		return -ENODEV;
	}
	if (usb_console_init()) {
		/* A console fault must not prevent standalone waveform playback. */
		indicate_fault();
	} else if (!IS_ENABLED(CONFIG_WORKOUT_FILTER_TEST)) {
		k_timer_start(&heartbeat_timer, K_SECONDS(5), K_NO_WAIT);
	}
	int rc = outputs_init();
	if (!rc) {
		if (!device_is_ready(pixels) || !device_is_ready(buttons)) {
			rc = -ENODEV;
		} else {
			/* Keep both debounced GPIO interrupts armed during CPU Sleep. */
			rc = pm_device_runtime_get(buttons);
			if (!rc) {
				rc = display(NULL);
			}
			if (!rc) {
				rc = run_trainer();
			}
		}
		if (rc) {
			outputs_idle();
		}
	}
	if (rc) {
		indicate_fault();
	}
	return rc;
}
