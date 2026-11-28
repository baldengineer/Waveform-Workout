/* SPDX-License-Identifier: MIT */
#ifndef WORKOUT_PRESET_STORE_H
#define WORKOUT_PRESET_STORE_H

#include "lesson.h"

enum { OLED_ADDRESS_LOW = 0x3c, OLED_ADDRESS_DEFAULT = 0x3d };

static inline bool oled_address_valid(uint8_t address)
{
	return address == OLED_ADDRESS_LOW || address == OLED_ADDRESS_DEFAULT;
}

/* The trainer loop is the sole writer. Readers take one EEPROM snapshot.
 * A failed read leaves the destination unchanged. -ENOENT means no valid preset.
 */
int preset_load(struct lesson *lesson, uint8_t *oled_address);
int preset_save(const struct lesson *lesson, uint8_t oled_address);

#endif
