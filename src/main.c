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
	uart_init(25); // TODO

	sei(); // enable interrupts globally
}

void button_pressed(uint8_t button) {
	led_gr_right_toggle();
}
