#include <stdbool.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "hardware.h"
#include "lib/uart.h"

///////////////////////////////////////////////////////////////////////////////

int main();
void init();

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
	uart_init(25); // TODO

	sei(); // enable interrupts globally
}
