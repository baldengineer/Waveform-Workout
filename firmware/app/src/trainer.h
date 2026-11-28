/* SPDX-License-Identifier: MIT */
#ifndef WORKOUT_TRAINER_H
#define WORKOUT_TRAINER_H

#include "lesson.h"

/* MODE_COUNT selects the active editor, otherwise field selects its preset. */
int trainer_set(enum mode field, uint8_t value);
int trainer_set_oled_address(uint8_t address);

#endif
