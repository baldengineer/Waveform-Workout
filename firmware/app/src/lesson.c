/* SPDX-License-Identifier: MIT */
#include "lesson.h"

const char *const lesson_mode_names[MODE_COUNT] = {
	[SHAPE] = "shape", [AMP] = "amp", [FREQ] = "freq",
	[DC] = "dc", [XY] = "xy", [TRIG] = "trig",
};

const char *const lesson_value_names[MODE_COUNT][VALUE_COUNT] = {
	[SHAPE] = {"sine", "square", "triangle"},
	[AMP] = {"200mVpp", "500mVpp", "1Vpp"},
	[FREQ] = {"100Hz", "1kHz", "10kHz"},
	[DC] = {"250mV", "500mV", "1V"},
	[XY] = {"0deg", "90deg", "180deg"},
	[TRIG] = {"rare-pulse", "pulse-width", "missing"},
};

void lesson_next_mode(struct lesson *lesson)
{
	lesson->mode = (lesson->mode + 1) % MODE_COUNT;
}

void lesson_next_value(struct lesson *lesson)
{
	uint8_t *value = &lesson->value[lesson->mode];
	*value = (*value + 1) % VALUE_COUNT;
}

unsigned int lesson_mode_pixel(const struct lesson *lesson)
{
	static const uint8_t pixels[] = {7, 8, 6, 5, 4, 3};
	return pixels[lesson->mode];
}

bool lesson_output_changed(const struct lesson *previous, const struct lesson *next)
{
	/* SHAPE, AMP and FREQ are three editors for the same running waveform. */
	if (previous->mode < DC && next->mode < DC) {
		return previous->value[SHAPE] != next->value[SHAPE] ||
		       previous->value[AMP] != next->value[AMP] ||
		       previous->value[FREQ] != next->value[FREQ];
	}
	return previous->mode != next->mode ||
	       previous->value[next->mode] != next->value[next->mode] ||
	       (next->mode == XY && previous->value[AMP] != next->value[AMP]);
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

unsigned int trigger_sequence(uint8_t value, uint8_t *pulse_us, uint8_t *marker_us)
{
	if (value == 1) {
		static const uint8_t widths[] = {10, 40, 80};
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
