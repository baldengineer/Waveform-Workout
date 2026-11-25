/* SPDX-License-Identifier: MIT */
#include "lesson.h"

void lesson_next_mode(struct lesson *lesson)
{
	lesson->mode = (lesson->mode + 1) % MODE_COUNT;
}

void lesson_next_value(struct lesson *lesson)
{
	uint8_t *value = &lesson->value[lesson->mode];
	*value = (*value + 1) % 3;
}

unsigned int lesson_mode_pixel(const struct lesson *lesson)
{
	static const uint8_t pixels[] = {7, 8, 6, 5, 4, 3};
	return pixels[lesson->mode];
}

unsigned int lesson_value_pixel(const struct lesson *lesson)
{
	return 2 - lesson->value[lesson->mode];
}

unsigned int lesson_sample_rate(const struct lesson *lesson)
{
	static const unsigned int hz[] = {100, 1000, 10000};
	return CYCLE_SAMPLES * (lesson->mode == XY ? 1000 : hz[lesson->value[FREQ]]);
}

/* Signed shape in thousandths of peak amplitude, starting at the rising
 * center crossing for the sine and triangle.
 */
static int shape_level(uint8_t shape, unsigned int phase)
{
	static const int16_t quarter_sine[] = {
		0, 131, 259, 383, 500, 609, 707, 793, 866, 924, 966, 991, 1000
	};
	unsigned int quarter = CYCLE_SAMPLES / 4;
	unsigned int quadrant = phase / quarter;
	unsigned int step = phase % quarter;

	if (shape == 1) {
		return quadrant < 2 ? 1000 : -1000;
	}
	if (quadrant & 1) {
		step = quarter - step;
	}
	int level = shape == 2 ? (int)(1000 * step / quarter) : quarter_sine[step];
	return quadrant < 2 ? level : -level;
}

static uint32_t dac_code(unsigned int microvolts, unsigned int vref_mv)
{
	/* Use 64 bits before scaling the 1.25 V peak by 4096. Only used when
	 * preparing the table, never on the sample clock.
	 */
	unsigned int reference = 1000 * vref_mv;
	return ((uint64_t)microvolts * 4096 + reference / 2) / reference;
}

uint32_t lesson_sample(const struct lesson *lesson, unsigned int phase, unsigned int vref_mv)
{
	static const int vpp_mv[] = {200, 500, 1000};
	static const unsigned int dc_uv[] = {250000, 500000, 1000000};
	uint32_t a, b;

	if (lesson->mode == DC) {
		a = b = dac_code(dc_uv[lesson->value[DC]], vref_mv);
	} else {
		int peak_mv = vpp_mv[lesson->value[AMP]] / 2;
		unsigned int shape = lesson->mode == XY ? 0 : lesson->value[SHAPE];
		unsigned int offset = lesson->mode == XY ? lesson->value[XY] * CYCLE_SAMPLES / 4 : 0;
		a = dac_code(750000 + peak_mv * shape_level(shape, phase), vref_mv);
		b = dac_code(750000 + peak_mv * shape_level(shape, (phase + offset) % CYCLE_SAMPLES),
			     vref_mv);
	}
	return (a << 16) | b; /* DAC2 = J2 (A), DAC1 = TP3 (B). */
}

unsigned int trigger_sequence(uint8_t value, uint16_t *pulse_us, uint16_t *marker_us)
{
	if (value == 1) {
		static const uint16_t widths[] = {10, 40, 80};
		for (unsigned int i = 0; i < 3; ++i) {
			pulse_us[i] = widths[i];
			marker_us[i] = i == 0 ? 10 : 0;
		}
		return 3;
	}
	for (unsigned int i = 0; i < RARE_PERIODS - 1; ++i) {
		pulse_us[i] = 50;
		marker_us[i] = 0;
	}
	pulse_us[RARE_PERIODS - 1] = value == 0 ? 2 : 0;
	marker_us[RARE_PERIODS - 1] = 10;
	return RARE_PERIODS;
}
