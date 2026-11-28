/* SPDX-License-Identifier: MIT */
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

static int cmd_i2c_scan(const struct shell *sh, size_t argc, char **argv)
{
	const struct device *bus = DEVICE_DT_GET(DT_NODELABEL(zephyr_i2c));
	const struct device *rail = DEVICE_DT_GET(DT_NODELABEL(qt_vdd));
	if (!device_is_ready(bus) || !device_is_ready(rail)) {
		shell_error(sh, "J6 bus or supply unavailable");
		return -ENODEV;
	}
	int rc = regulator_enable(rail);
	if (rc) {
		shell_error(sh, "Could not power J6 (%d)", rc);
		return rc;
	}
	k_sleep(K_MSEC(DT_PROP(DT_NODELABEL(trainer_oled), ready_time_ms)));
	unsigned int found = 0;
	uint8_t unused;
	struct i2c_msg probe = { .buf = &unused, .len = 0, .flags = I2C_MSG_WRITE | I2C_MSG_STOP };
	for (unsigned int address = 0x08; address <= 0x77; ++address) {
		rc = i2c_transfer(bus, &probe, 1, address);
		if (rc == 0) {
			shell_print(sh, "0x%02X", address);
			found++;
		} else if (rc != -EIO && rc != -ENXIO) {
			shell_error(sh, "Scan stopped at 0x%02X (%d)", address, rc);
			break;
		}
		rc = 0;
	}
	int power_rc = regulator_disable(rail);
	if (power_rc) {
		shell_error(sh, "Could not release J6 power (%d)", power_rc);
	}
	if (!rc) {
		shell_print(sh, "%u device(s) found on J6", found);
	}
	return rc ? rc : power_rc;
}

SHELL_CMD_ARG_REGISTER(i2c_scan, NULL, "Scan J6 for 7-bit I2C addresses.", cmd_i2c_scan, 1, 0);
