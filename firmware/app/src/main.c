/* SPDX-License-Identifier: MIT */
#include "outputs.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>

#define PIXELS DT_ALIAS(led_strip)
BUILD_ASSERT(DT_PROP(PIXELS, chain_length) == 9);
static const struct device *const pixels = DEVICE_DT_GET(PIXELS);
static const struct gpio_dt_spec diagnostic = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
K_MSGQ_DEFINE(presses, sizeof(uint16_t), 8, 2);

static void on_key(struct input_event *event, void *context)
{
	ARG_UNUSED(context);
	/* gpio-keys debounces both edges. Only a press advances the panel. */
	if (event->type == INPUT_EV_KEY && event->value == 1) {
		k_msgq_put(&presses, &event->code, K_NO_WAIT);
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
	return led_strip_update_rgb(pixels, frame, ARRAY_SIZE(frame));
}

static int run_trainer(void)
{
	struct lesson lesson = LESSON_DEFAULT;
	int rc = outputs_apply(&lesson);
	if (rc || IS_ENABLED(CONFIG_WORKOUT_FILTER_TEST)) {
		return rc; /* In the filter image TIM6 and DMA continue autonomously. */
	}
	k_timeout_t timeout = K_SECONDS(2);
	rc = display(&lesson);
	while (!rc) {
		uint16_t key;
		if (k_msgq_get(&presses, &key, timeout) != 0) {
			rc = display(NULL);
			timeout = K_FOREVER;
			continue;
		}
		enum mode previous = lesson.mode;
		if (key == INPUT_KEY_0) {
			lesson_next_mode(&lesson);
		} else if (key == INPUT_KEY_1) {
			lesson_next_value(&lesson);
		} else {
			continue;
		}
		/* Moving between the three basic editors leaves playback in phase. */
		if (key == INPUT_KEY_1 || previous >= DC || lesson.mode >= DC) {
			rc = outputs_apply(&lesson);
			if (rc) {
				break;
			}
		}
		rc = display(&lesson);
		timeout = K_SECONDS(2);
	}
	return rc;
}

int main(void)
{
	if (!gpio_is_ready_dt(&diagnostic) ||
	    gpio_pin_configure_dt(&diagnostic, GPIO_OUTPUT_INACTIVE)) {
		return -ENODEV;
	}
	int rc = outputs_init();
	if (!rc) {
		if (!device_is_ready(pixels) || !device_is_ready(DEVICE_DT_GET(DT_PATH(buttons)))) {
			rc = -ENODEV;
		} else {
			rc = display(NULL);
			if (!rc) {
				rc = run_trainer();
			}
		}
		if (rc) {
			outputs_idle();
		}
	}
	if (rc) {
		gpio_pin_set_dt(&diagnostic, 1);
	}
	return rc;
}
