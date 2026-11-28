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

#define PIXELS DT_ALIAS(led_strip)
BUILD_ASSERT(DT_PROP(PIXELS, chain_length) == 9);
static const struct device *const pixels = DEVICE_DT_GET(PIXELS);
static const struct gpio_dt_spec diagnostic = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

struct reply {
	struct k_sem done;
	int rc;
};

enum panel_action { NEXT_MODE, NEXT_VALUE, SET_MODE, SET_VALUE };
struct panel_event {
	enum panel_action action;
	uint8_t field;
	uint8_t value;
	struct reply *reply;
};
K_MSGQ_DEFINE(events, sizeof(struct panel_event), 8, 4);
static atomic_t accepting_edits;

int trainer_set(enum mode field, uint8_t value)
{
	if ((unsigned int)field > MODE_COUNT ||
	    value >= (field == MODE_COUNT ? MODE_COUNT : VALUE_COUNT)) {
		return -EINVAL;
	}
	if (!atomic_get(&accepting_edits)) {
		return -EAGAIN;
	}
	struct reply reply;
	struct panel_event event = {
		.action = field == MODE_COUNT ? SET_MODE : SET_VALUE,
		.field = field, .value = value, .reply = &reply,
	};
	k_sem_init(&reply.done, 0, 1);
	k_msgq_put(&events, &event, K_FOREVER);
	k_sem_take(&reply.done, K_FOREVER);
	return reply.rc;
}

static void on_key(struct input_event *event, void *context)
{
	ARG_UNUSED(context);
	/* gpio-keys debounces both edges. Only a press advances the panel. */
	if (event->type == INPUT_EV_KEY && event->value == 1 &&
	    (event->code == INPUT_KEY_0 || event->code == INPUT_KEY_1)) {
		struct panel_event press = {
			.action = event->code == INPUT_KEY_0 ? NEXT_MODE : NEXT_VALUE,
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
	return led_strip_update_rgb(pixels, frame, ARRAY_SIZE(frame));
}

static int run_trainer(void)
{
	struct lesson lesson = LESSON_DEFAULT;
	if (IS_ENABLED(CONFIG_WORKOUT_FILTER_TEST)) {
		return outputs_apply(&lesson);
	}
	int load_rc = preset_load(&lesson);
	if (load_rc && load_rc != -ENOENT) {
		gpio_pin_set_dt(&diagnostic, 1);
	}
	int rc = outputs_apply(&lesson);
	if (rc) {
		return rc;
	}
	if (IS_ENABLED(CONFIG_WORKOUT_OLED)) {
		oled_init();
		oled_show(&lesson);
	}
	rc = display(&lesson);
	if (rc) {
		return rc;
	}
	k_timeout_t timeout = K_SECONDS(2);
	bool outputs_running = true;
	atomic_set(&accepting_edits, 1);
	for (;;) {
		struct panel_event event;
		if (k_msgq_get(&events, &event, timeout) != 0) {
			if (display(NULL)) {
				gpio_pin_set_dt(&diagnostic, 1);
			}
			timeout = K_FOREVER;
			continue;
		}
		struct lesson next = lesson;
		switch (event.action) {
		case NEXT_MODE: lesson_next_mode(&next); break;
		case NEXT_VALUE: lesson_next_value(&next); break;
		case SET_MODE: next.mode = event.value; break;
		case SET_VALUE: next.value[event.field] = event.value; break;
		}
		rc = 0;
		if (!outputs_running || lesson_output_changed(&lesson, &next)) {
			rc = outputs_apply(&next);
			outputs_running = rc == 0;
		}
		if (!rc) {
			lesson = next;
			rc = preset_save(&lesson);
			int led_rc = display(&lesson);
			if (!rc) {
				rc = led_rc;
			}
			if (IS_ENABLED(CONFIG_WORKOUT_OLED)) {
				oled_show(&lesson);
			}
		}
		if (rc) {
			gpio_pin_set_dt(&diagnostic, 1);
		}
		if (event.reply) {
			event.reply->rc = rc;
			k_sem_give(&event.reply->done);
		}
		timeout = K_SECONDS(2);
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
		gpio_pin_set_dt(&diagnostic, 1);
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
