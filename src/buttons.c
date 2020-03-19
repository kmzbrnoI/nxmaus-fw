#include <stddef.h>

#include "buttons.h"
#include "hardware.h"

#define BTN_THRESHOLD 20  // 20 ms

uint8_t btn_pressed[BUTTONS_COUNT] = {false, };
uint8_t btn_counter[BUTTONS_COUNT] = {0, };

void (*btn_on_pressed)(uint8_t button) = NULL;
void (*btn_on_depressed)(uint8_t button) = NULL;

void btn_update() {
	uint8_t state = buttons_raw_state();
	for (uint8_t i = 0; i < BUTTONS_COUNT; i++) {
		if (state & 0x01) {
			if (btn_counter[i] > 0) {
				btn_counter[i]--;
				if (btn_counter[i] == 0) {
					btn_pressed[i] = false;
					if (btn_on_depressed != NULL)
						btn_on_depressed(i);
				}
			}
		} else {
			if (btn_counter[i] < BTN_THRESHOLD) {
				btn_counter[i]++;
				if (btn_counter[i] == BTN_THRESHOLD) {
					btn_pressed[i] = true;
					if (btn_on_pressed != NULL)
						btn_on_pressed(i);
				}
			}
		}

		state >>= 1;
	}

}
