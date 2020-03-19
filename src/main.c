#include <stdbool.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "hardware.h"
#include "buttons.h"
#include "lib/uart.h"

///////////////////////////////////////////////////////////////////////////////

int main();
void init();

void button_pressed(uint8_t button);
void uart_received(uint8_t recipient, uint8_t *data, uint8_t size);

///////////////////////////////////////////////////////////////////////////////

int main() {
	init();

	while (true) {
		led_gr_left_toggle();
		_delay_ms(250);
	}
}

void init() {
	leds_init();
	buttons_init();
	btn_on_pressed = button_pressed;
	uart_init();
	uart_on_receive = uart_received;

	sei(); // enable interrupts globally
}

void button_pressed(uint8_t button) {
	led_gr_right_toggle();
}

void uart_received(uint8_t recipient, uint8_t *data, uint8_t size) {
}
