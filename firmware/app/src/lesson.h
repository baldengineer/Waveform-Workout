/* SPDX-License-Identifier: MIT */
#ifndef WORKOUT_LESSON_H
#define WORKOUT_LESSON_H

#include <stdint.h>

enum mode { SHAPE, AMP, FREQ, DC, XY, TRIG, MODE_COUNT };
enum { CYCLE_SAMPLES = 48, TRIGGER_PERIOD_US = 100, RARE_PERIODS = 1000 };

struct lesson {
	enum mode mode;
	uint8_t value[MODE_COUNT]; /* zero-based front-panel positions */
};

#define LESSON_DEFAULT { .mode = SHAPE, .value = { [AMP] = 2, [FREQ] = 1 } }

void lesson_next_mode(struct lesson *lesson);
void lesson_next_value(struct lesson *lesson);
unsigned int lesson_mode_pixel(const struct lesson *lesson);
unsigned int lesson_value_pixel(const struct lesson *lesson);
unsigned int lesson_sample_rate(const struct lesson *lesson);
uint32_t lesson_sample(const struct lesson *lesson, unsigned int phase, unsigned int vref_mv);
unsigned int trigger_sequence(uint8_t value, uint16_t *pulse_us, uint16_t *marker_us);

#endif
