/* SPDX-License-Identifier: MIT */
#include "preset_store.h"
#include "trainer.h"
#include <string.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_string_conv.h>

static int cmd_show(const struct shell *sh, size_t argc, char **argv)
{
	struct lesson saved = LESSON_DEFAULT;
	uint8_t oled_address = OLED_ADDRESS_DEFAULT;
	int rc = preset_load(&saved, &oled_address);
	if (rc == -ENOENT) {
		shell_print(sh, "No valid EEPROM preset. Boot defaults:");
	} else if (rc) {
		shell_error(sh, "EEPROM read failed (%d)", rc);
		return rc;
	}
	shell_print(sh, "mode %s", lesson_mode_names[saved.mode]);
	shell_print(sh, "oled_addr 0x%02X", (unsigned int)oled_address);
	for (int i = 0; i < MODE_COUNT; ++i) {
		shell_print(sh, "%s %s", lesson_mode_names[i], lesson_value_names[i][saved.value[i]]);
	}
	return 0;
}

static int lookup(const char *name, const char *const names[], int count)
{
	for (int i = 0; i < count; ++i) {
		if (strcmp(name, names[i]) == 0) {
			return i;
		}
	}
	return -1;
}

static int cmd_set(const struct shell *sh, size_t argc, char **argv)
{
	if (strcmp(argv[1], "oled_addr") == 0) {
		int err = 0;
		unsigned long address = shell_strtoul(argv[2], 0, &err);
		if (err || address > UINT8_MAX || !oled_address_valid(address)) {
			shell_error(sh, "Use 7-bit SSD1306 address 0x3C or 0x3D");
			return -EINVAL;
		}
		err = trainer_set_oled_address(address);
		if (err) {
			shell_error(sh, "Update failed (%d). Use trainer settings show to read EEPROM.", err);
		} else {
			shell_print(sh, "Saved oled_addr 0x%02X for next boot", (unsigned int)address);
		}
		return err;
	}
	int field = MODE_COUNT;
	int count = MODE_COUNT;
	const char *const *names = lesson_mode_names;
	if (strcmp(argv[1], "mode") != 0) {
		field = lookup(argv[1], lesson_mode_names, MODE_COUNT);
		if (field < 0) {
			shell_error(sh, "Use mode, shape, amp, freq, dc, xy, trig or oled_addr");
			return -EINVAL;
		}
		names = lesson_value_names[field];
		count = VALUE_COUNT;
	}
	int value = lookup(argv[2], names, count);
	if (value < 0) {
		shell_error(sh, "Valid values for %s:", argv[1]);
		for (int i = 0; i < count; ++i) {
			shell_print(sh, "  %s", names[i]);
		}
		return -EINVAL;
	}
	int rc = trainer_set(field, value);
	if (rc) {
		shell_error(sh, "Update failed (%d). Use trainer settings show to read EEPROM.", rc);
	} else {
		shell_print(sh, "Saved %s %s", argv[1], names[value]);
	}
	return rc;
}

SHELL_STATIC_SUBCMD_SET_CREATE(settings_commands,
	SHELL_CMD_ARG(show, NULL, "Read the saved EEPROM preset.", cmd_show, 1, 0),
	SHELL_CMD_ARG(set, NULL, "Apply and save: set <field> <value>", cmd_set, 3, 0),
	SHELL_SUBCMD_SET_END);
SHELL_STATIC_SUBCMD_SET_CREATE(trainer_commands,
	SHELL_CMD(settings, &settings_commands, "Persistent trainer settings.", NULL),
	SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(trainer, &trainer_commands, "Waveform Workout controls.", NULL);
