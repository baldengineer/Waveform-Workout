/* SPDX-License-Identifier: MIT */
#ifndef WORKOUT_USB_CONSOLE_H
#define WORKOUT_USB_CONSOLE_H

/* Arm VBUS detection. The power worker brings up CDC ACM when cable power and VDD allow. */
int usb_console_init(void);

#endif
