#include <avr/io.h>
#include <stdbool.h>

#include "encoder.h"
#include "hardware.h"

#define ENCODER_THRESHOLD 10  // 10 ms

void (*encoder_on_change)(int8_t val) = NULL;

uint8_t a_counter = ENCODER_THRESHOLD, b_counter = ENCODER_THRESHOLD;
bool a_pressed = true, b_pressed = true;

static inline void _a_pressed();
static inline void _a_depressed();
static inline void _callback(int8_t val);

void encoder_update() {
	if (encoder_raw_a()) {
		if (a_counter < ENCODER_THRESHOLD) {
			a_counter++;
			if ((a_counter == ENCODER_THRESHOLD) && (!a_pressed)) {
				a_pressed = true;
				_a_pressed();
			}
		}
	} else {
		if (a_counter > 0) {
			a_counter--;
			if ((a_counter == 0) && (a_pressed)) {
				a_pressed = false;
				_a_depressed();
			}
		}
	}

	if (encoder_raw_b()) {
		if (b_counter < ENCODER_THRESHOLD) {
			b_counter++;
			if (b_counter == ENCODER_THRESHOLD)
				b_pressed = true;
		}
	} else {
		if (b_counter > 0) {
			b_counter--;
			if (b_counter == 0)
				b_pressed = false;
		}
	}
}

static inline void _a_pressed() { if (b_pressed) _callback(1); }
static inline void _a_depressed() { if (b_pressed) _callback(-1); }

static inline void _callback(int8_t val) {
	if (encoder_on_change != NULL)
		encoder_on_change(val);
}
