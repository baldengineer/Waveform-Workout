/* SPDX-License-Identifier: MIT */
#ifndef WORKOUT_OUTPUTS_H
#define WORKOUT_OUTPUTS_H

#include "lesson.h"

int outputs_init(void);
int outputs_apply(const struct lesson *lesson);
void outputs_idle(void);

#endif
