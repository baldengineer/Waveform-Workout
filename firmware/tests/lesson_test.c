/* SPDX-License-Identifier: MIT */
#include "lesson.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void test_panel(void)
{
	struct lesson s = LESSON_DEFAULT;
	static const int pixels[] = {7, 8, 6, 5, 4, 3};
	static const int initial[] = {0, 2, 1, 0, 0, 0};
	for (int m = 0; m < MODE_COUNT; ++m) {
		assert((int)s.mode == m);
		assert(lesson_mode_pixel(&s) == (unsigned)pixels[m]);
		for (int i = 0; i < 3; ++i) {
			unsigned int v = (initial[m] + i) % 3;
			assert(s.value[m] == v);
			assert(lesson_value_pixel(&s) == 2 - v);
			lesson_next_value(&s);
		}
		lesson_next_value(&s);
		lesson_next_mode(&s);
	}
	assert(s.mode == SHAPE);
	for (int m = 0; m < MODE_COUNT; ++m) {
		assert(s.value[m] == (initial[m] + 1) % 3);
	}
}

static int expected_code(double volts)
{
	return (int)lround(volts * 4096.0 / 3.447);
}

static void test_analog(void)
{
	struct lesson s = LESSON_DEFAULT;
	static const double swing[] = {0.20, 0.50, 1.00};
	static const unsigned int hz[] = {100, 1000, 10000};
	for (int shape = 0; shape < 3; ++shape) {
		s.value[SHAPE] = shape;
		for (int amp = 0; amp < 3; ++amp) {
			s.value[AMP] = amp;
			for (int f = 0; f < 3; ++f) {
				s.value[FREQ] = f;
				assert(lesson_sample_rate(&s) == hz[f] * CYCLE_SAMPLES);
				for (int i = 0; i < CYCLE_SAMPLES; ++i) {
					double angle = 2 * acos(-1) * i / CYCLE_SAMPLES;
					double wave[] = {sin(angle), i < CYCLE_SAMPLES / 2 ? 1 : -1,
							 2 * asin(sin(angle)) / acos(-1)};
					uint32_t pair = lesson_sample(&s, i, 3447);
					assert((pair >> 16) == (pair & 0xffff));
					int target = expected_code(0.75 + swing[amp] * wave[shape] / 2);
					assert(abs((int)(pair >> 16) - target) <= 1);
				}
			}
		}
	}
	for (int v = 0; v < 3; ++v) {
		static const double dc[] = {0.25, 0.50, 1.00};
		s.mode = DC;
		s.value[DC] = v;
		uint32_t target = expected_code(dc[v]);
		assert(lesson_sample(&s, 0, 3447) == ((target << 16) | target));
		s.mode = XY; /* Shape and frequency were left at triangle and 10 kHz. */
		s.value[XY] = v;
		assert(lesson_sample_rate(&s) == 1000 * CYCLE_SAMPLES);
		for (int i = 0; i < CYCLE_SAMPLES; ++i) {
			uint32_t pair = lesson_sample(&s, i, 3447);
			double angle = 2 * acos(-1) * i / CYCLE_SAMPLES;
			int a = expected_code(0.75 + 0.5 * sin(angle));
			int b = expected_code(0.75 + 0.5 * sin(angle + v * acos(-1) / 2));
			assert(abs((int)(pair >> 16) - a) <= 1);
			assert(abs((int)(pair & 0xffff) - b) <= 1);
		}
	}
}

static void test_triggers(void)
{
	uint16_t a[RARE_PERIODS], b[RARE_PERIODS];
	for (int v = 0; v < 3; ++v) {
		unsigned int count = trigger_sequence(v, a, b);
		if (v == 1) {
			assert(count == 3 && a[0] == 10 && a[1] == 40 && a[2] == 80);
			assert(b[0] == 10 && b[1] == 0 && b[2] == 0);
		} else {
			assert(count == 1000);
			for (unsigned int i = 0; i < 999; ++i) {
				assert(a[i] == 50 && b[i] == 0);
			}
			assert(a[999] == (v == 0 ? 2 : 0) && b[999] == 10);
		}
	}
}

int main(void)
{
	test_panel();
	test_analog();
	test_triggers();
	puts("Front panel, analog presets, XY phase and trigger sequences passed.");
	return 0;
}
