/* SPDX-License-Identifier: MIT */
#ifndef WORKOUT_PRESET_STORE_H
#define WORKOUT_PRESET_STORE_H

#include "lesson.h"

/* The trainer loop is the sole writer. Readers take one EEPROM snapshot.
 * A failed read leaves the destination unchanged. -ENOENT means no valid preset.
 */
int preset_load(struct lesson *lesson);
int preset_save(const struct lesson *lesson);

#endif
