#include <stdbool.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "hardware.h"
#include "buttons.h"
#include "encoder.h"
#include "lib/uart.h"

///////////////////////////////////////////////////////////////////////////////

int main();
void init();

void button_pressed(uint8_t button);
void uart_received(uint8_t recipient, uint8_t *data, uint8_t size);
void encoder_changed(uint8_t val);

///////////////////////////////////////////////////////////////////////////////

int main() {
	init();

	while (true) {
	}
}

void init() {
	leds_init();
	buttons_init();
	btn_on_pressed = button_pressed;
	uart_init(25);
	uart_on_receive = uart_received;
	encoder_init();
	encoder_on_change = encoder_changed;

	// Setup main timer0 on 1 ms
	TCCR0A |= 1 << WGM01; // CTC mode
	TCCR0B |= 0x03; // prescaler 64×
	TIMSK0 |= 1 << OCIE0A; // enable interrupt on compare match A
	OCR0A = 114; // set compare match A to match 1 ms

	led_gr_left_on();
	led_gr_right_on();
	led_red_on();

	_delay_ms(250);

	led_gr_left_off();
	led_gr_right_off();
	led_red_off();

	sei(); // enable interrupts globally
}

ISR(TIMER0_COMPA_vect) {
	// Timer0 on 1 ms
	btn_update();
	encoder_update();
}


void button_pressed(uint8_t button) {
	if (button == BTN_TL1) {
		if (uart_can_fill_output_buf()) {
			led_gr_left_toggle();
			uart_output_buf[0] = 0x21;
			uart_output_buf[1] = 0x81;
			uart_output_buf[2] = 0xA0;
			uart_send_buf_autolen();
		}
	} else if (button == BTN_TL2) {
		if (uart_can_fill_output_buf()) {
			led_gr_left_toggle();
			uart_output_buf[0] = 0x21;
			uart_output_buf[1] = 0x80;
			uart_output_buf[2] = 0xA1;
			uart_send_buf_autolen();
		}
	}
}

void uart_received(uint8_t recipient, uint8_t *data, uint8_t size) {
	if ((size == 3) && (data[0] == 0x61) && (data[1] == 0x01))
		led_red_off();
	if ((size == 3) && (data[0] == 0x61) && (data[1] == 0x00))
		led_red_on();
}

void encoder_changed(uint8_t val) {
}
