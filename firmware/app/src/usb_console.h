/* SPDX-License-Identifier: MIT */
#ifndef WORKOUT_USB_CONSOLE_H
#define WORKOUT_USB_CONSOLE_H

/* Initialize CDC ACM and start monitoring cable power and VDD. Never wait for a host. */
int usb_console_init(void);

#endif
