/* SPDX-License-Identifier: MIT */
#include "oled.h"
#include <zephyr/display/cfb.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/kernel.h>

#define PANEL DT_CHOSEN(zephyr_display)
static const struct device *const panel = DEVICE_DT_GET(PANEL);
static const struct device *const rail = DEVICE_DT_GET(DT_NODELABEL(qt_vdd));
static bool available;
static uint8_t font_height;

void oled_init(void)
{
	if (!device_is_ready(rail) || regulator_enable(rail)) {
		return;
	}
	k_sleep(K_MSEC(DT_PROP(PANEL, ready_time_ms)));
	if (device_init(panel) || cfb_framebuffer_init(panel)) {
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
		available = false;
		cfb_framebuffer_deinit(panel);
		regulator_disable(rail);
	}
}
