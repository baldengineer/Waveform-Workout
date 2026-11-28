/* SPDX-License-Identifier: MIT */
#include "preset_store.h"
#include <string.h>
#include <zephyr/drivers/eeprom.h>
#include <zephyr/sys/crc.h>

/* Two independent 16-byte slots at EEPROM offset zero. Byte fields keep the
 * format independent of compiler enum size, struct padding and endianness.
 * The commit byte lives in its own STM32 EEPROM word.
 */
struct preset_record {
	uint8_t version;
	uint8_t sequence;
	uint8_t mode;
	uint8_t value[6];
	uint8_t oled_address; /* Occupies the original v1 CRC byte. */
	uint8_t crc;
	uint8_t reserved;
	uint8_t committed;
	uint8_t padding[3];
};

enum { FORMAT_VERSION = 2, COMMITTED = 0xa5 };
#define EEPROM_NODE DT_NODELABEL(eeprom)
static const struct device *const storage = DEVICE_DT_GET(EEPROM_NODE);
BUILD_ASSERT(MODE_COUNT == 6 && sizeof(struct preset_record) == 16);
BUILD_ASSERT(DT_REG_SIZE(EEPROM_NODE) >= 2 * sizeof(struct preset_record));

static uint8_t checksum(const struct preset_record *record)
{
	return crc8_ccitt(0, (const uint8_t *)record, offsetof(struct preset_record, crc));
}

static bool decode(struct preset_record *record)
{
	if (record->committed != COMMITTED || record->mode >= MODE_COUNT) {
		return false;
	}
	for (size_t i = 0; i < MODE_COUNT; ++i) {
		if (record->value[i] >= VALUE_COUNT) {
			return false;
		}
	}
	if (record->version == 1) {
		uint8_t old_crc = crc8_ccitt(0, record, offsetof(struct preset_record, oled_address));
		if (old_crc != record->oled_address) {
			return false;
		}
		record->oled_address = OLED_ADDRESS_DEFAULT;
		return true;
	}
	return record->version == FORMAT_VERSION && oled_address_valid(record->oled_address) &&
	       record->crc == checksum(record);
}

static int latest(struct preset_record *result)
{
	struct preset_record slots[2];
	if (!device_is_ready(storage)) {
		return -ENODEV;
	}
	int rc = eeprom_read(storage, 0, slots, sizeof(slots));
	if (rc) {
		return rc;
	}
	int slot = 0;
	if (!decode(&slots[0])) {
		if (!decode(&slots[1])) {
			return -ENOENT;
		}
		slot = 1;
	} else if (decode(&slots[1]) && slots[1].sequence == (uint8_t)(slots[0].sequence + 1)) {
		slot = 1;
	}
	*result = slots[slot];
	return slot;
}

int preset_load(struct lesson *lesson, uint8_t *oled_address)
{
	struct preset_record record;
	int slot = latest(&record);
	if (slot < 0) {
		return slot;
	}
	lesson->mode = record.mode;
	*oled_address = record.oled_address;
	memcpy(lesson->value, record.value, sizeof(record.value));
	return 0;
}

int preset_save(const struct lesson *lesson, uint8_t oled_address)
{
	if (!oled_address_valid(oled_address)) {
		return -EINVAL;
	}
	struct preset_record record = {0};
	int slot = latest(&record);
	if (slot < 0 && slot != -ENOENT) {
		return slot;
	}
	if (slot >= 0 && record.version == FORMAT_VERSION && record.mode == lesson->mode &&
	    record.oled_address == oled_address &&
	    memcmp(record.value, lesson->value, sizeof(record.value)) == 0) {
		return 0;
	}
	int offset = (slot == 0) ? sizeof(record) : 0;
	record.sequence++;
	record.version = FORMAT_VERSION;
	record.oled_address = oled_address;
	record.mode = lesson->mode;
	memcpy(record.value, lesson->value, sizeof(record.value));
	record.crc = checksum(&record);

	const int marker = offset + offsetof(struct preset_record, committed);
	uint8_t flag = 0;
	int rc = eeprom_write(storage, marker, &flag, 1);
	if (!rc) {
		rc = eeprom_write(storage, offset, &record, offsetof(struct preset_record, reserved));
	}
	if (!rc) {
		flag = COMMITTED;
		rc = eeprom_write(storage, marker, &flag, 1);
	}
	return rc;
}
