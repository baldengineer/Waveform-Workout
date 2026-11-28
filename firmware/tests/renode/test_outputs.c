/* SPDX-License-Identifier: MIT */
#include "outputs.h"
#include <zephyr/sys/printk.h>

volatile uint32_t test_playback_requests;
volatile uint32_t test_idle_requests;

int outputs_init(void)
{
	return 0;
}

void outputs_idle(void)
{
	test_idle_requests++;
}

int outputs_apply(const struct lesson *lesson)
{
	test_playback_requests++;
	printk("Playback request: %s %s\n", lesson_mode_names[lesson->mode],
	       lesson_value_names[lesson->mode][lesson->value[lesson->mode]]);
	return 0;
}
