#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#include <avr/io.h>

static inline void leds_init() {
	DDRB |= 0x07; // output
	PORTB &= 0xF8; // all LED outputs low
}

static inline void buttons_init() {
	DDRC &= 0b11001110; // buttons as inputs
	DDRB &= 0b11000111;
	// no pull ups (hardware-based)
}

static inline void led_gr_left_on() { PORTB |= (1 << PORTB2); }
static inline void led_gr_left_off() { PORTB &= ~(1 << PORTB2); }
static inline void led_gr_left_toggle() { PORTB ^= (1 << PORTB2); }

static inline void led_gr_right_on() { PORTB |= (1 << PORTB1); }
static inline void led_gr_right_off() { PORTB &= ~(1 << PORTB1); }
static inline void led_gr_right_toggle() { PORTB ^= (1 << PORTB1); }

static inline void led_red_on() { PORTB |= (1 << PORTB0); }
static inline void led_red_off() { PORTB &= ~(1 << PORTB0); }
static inline void led_red__toggle() { PORTB ^= (1 << PORTB0); }

static inline void uart_out() { PORTD |= (1 << PORTD2); }
static inline void uart_in() { PORTD &= ~(1 << PORTD2); }

#define BUTTONS_COUNT 6
#define BTN_TL1 0
#define BTN_TL2 1
#define BTN_TL3 2
#define BTN_TL4 3
#define BTN_TL5 4
#define BTN_TL6 5

static inline uint8_t buttons_raw_state() {
	// bits: 0 0 B5 B4 B3 B2 B1 B0
	return ((PINC >> PINC3) & 0x01) |
	       ((PINC & 0x01) << 1) |
	       (((PINC >> PINC5) & 0x01) << 2) |
	       (((PINB >> PINB4) & 0x03) << 3) |
	       (((PINC >> PINC4) & 0x01) << 5);
}

#endif
