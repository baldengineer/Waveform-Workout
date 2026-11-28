/* SPDX-License-Identifier: MIT */
#ifndef WORKOUT_OLED_H
#define WORKOUT_OLED_H

#include "lesson.h"

void oled_init(void);
void oled_show(const struct lesson *lesson);

#endif
