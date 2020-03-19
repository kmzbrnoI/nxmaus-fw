#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#include <avr/io.h>

static inline void leds_init() {
	DDRB |= 0x07; // output
	PORTB &= 0xF8; // all LED outputs low
}

static inline void buttons_init() {
}

static inline void led_gr_left_on() { PORTB |= (1 << PORTB2); }
static inline void led_gr_left_off() { PORTB &= ~(1 << PORTB2); }
static inline void led_gr_left_toggle() { PORTB ^= (1 << PORTB2); }

static inline void uart_out() { PORTD |= (1 << PORTD2); }
static inline void uart_in() { PORTD &= ~(1 << PORTD2); }

#endif
