#include "state.h"
#include "hardware.h"
#include "common.h"

uint8_t state = ST_XN_UNADDRESSED;

///////////////////////////////////////////////////////////////////////////////

static inline void _show_blinking(uint16_t counter);
static inline void _show_loco_direction(uint16_t counter);
static inline void _show_loco_direction_blinking(uint16_t counter);

///////////////////////////////////////////////////////////////////////////////

void state_show(uint16_t counter) {
	if (state == ST_XN_UNADDRESSED /*|| state == ST_CS_STATUS_ASKING || state == ST_LOCO_STATUS_ASKING*/) {
		_show_blinking(counter);
	}
}

static inline void _show_blinking(uint16_t counter) {
	if ((counter%300) == 0) {
		led_gr_right_off();
		led_red_on();
	} else if ((counter%300) == 100) {
		led_red_off();
		led_gr_left_on();
	} else if ((counter%300) == 200) {
		led_gr_left_off();
		led_gr_right_on();
	}
}

static inline void _show_loco_direction(uint16_t counter) {
}

static inline void _show_loco_direction_blinking(uint16_t counter) {
}
