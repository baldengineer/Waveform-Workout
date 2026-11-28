/* SPDX-License-Identifier: MIT */
#include "outputs.h"
#include "usb_console.h"
#include <zephyr/sys/printk.h>

int usb_console_init(void)
{
	return 0; /* Zephyr's serial shell starts on the emulated USART. */
}

int outputs_init(void)
{
	return 0;
}

void outputs_idle(void)
{
}

int outputs_apply(const struct lesson *lesson)
{
	printk("Playback request: %s %s\n", lesson_mode_names[lesson->mode],
	       lesson_value_names[lesson->mode][lesson->value[lesson->mode]]);
	return 0;
}
