/* SPDX-License-Identifier: MIT */
#include "oled.h"
#include "preset_store.h"
#include <zephyr/display/cfb.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/kernel.h>

#define PANEL DT_CHOSEN(zephyr_display)
#define ALT_PANEL DT_NODELABEL(trainer_oled_alt)
BUILD_ASSERT(DT_REG_ADDR(PANEL) == OLED_ADDRESS_DEFAULT);
BUILD_ASSERT(DT_REG_ADDR(ALT_PANEL) == OLED_ADDRESS_LOW);
static const struct device *panel;
static const struct device *const rail = DEVICE_DT_GET(DT_NODELABEL(qt_vdd));
static bool available;
static uint8_t font_height;

void oled_wake(uint8_t address)
{
	if (available || !device_is_ready(rail) || regulator_enable(rail)) {
		return;
	}
	panel = address == OLED_ADDRESS_LOW ? DEVICE_DT_GET(ALT_PANEL) : DEVICE_DT_GET(PANEL);
	k_sleep(K_MSEC(MAX(DT_PROP(PANEL, ready_time_ms), DT_PROP(ALT_PANEL, ready_time_ms))));
	/* Cutting J6 resets the controller. Zephyr 4.5's SSD1306 has no
	 * deinit hook, so replay its driver init on subsequent power cycles.
	 * Only the trainer thread accesses this deferred display instance.
	 */
	int rc = device_is_ready(panel) ? panel->ops.init(panel) : device_init(panel);
	if (rc || cfb_framebuffer_init(panel)) {
		regulator_disable(rail);
		return;
	}
	/* Use the 10x16 font, independent of linker order. */
	for (int i = 0; i < cfb_get_numof_fonts(panel); ++i) {
		uint8_t width;
		if (!cfb_get_font_size(panel, i, &width, &font_height) &&
		    width == 10 && font_height == 16) {
			cfb_framebuffer_set_font(panel, i);
			available = true;
			return;
		}
	}
	cfb_framebuffer_deinit(panel);
	regulator_disable(rail);
}

void oled_sleep(void)
{
	if (available) {
		/* Blank even if a concurrent J6 scan still holds the regulator. */
		display_blanking_on(panel);
		cfb_framebuffer_deinit(panel);
		regulator_disable(rail);
		available = false;
	}
}

void oled_show(const struct lesson *lesson)
{
	if (!available) {
		return;
	}
	int height = cfb_get_display_parameter(panel, CFB_DISPLAY_HEIGHT);
	cfb_framebuffer_clear(panel, false);
	cfb_draw_text(panel, lesson_mode_names[lesson->mode], 0, 0);
	if (height >= 3 * font_height) {
		struct cfb_position start = {0, font_height + 2};
		struct cfb_position end = {cfb_get_display_parameter(panel, CFB_DISPLAY_WIDTH) - 1,
					   start.y};
		cfb_draw_line(panel, &start, &end);
	}
	cfb_draw_text(panel, lesson_value_names[lesson->mode][lesson->value[lesson->mode]],
		      0, height - font_height);
	if (cfb_framebuffer_finalize(panel)) {
		oled_sleep();
	}
}
