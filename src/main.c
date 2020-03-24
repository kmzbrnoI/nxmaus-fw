#include <stdbool.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "hardware.h"
#include "buttons.h"
#include "encoder.h"
#include "lib/uart.h"
#include "state.h"
#include "common.h"

///////////////////////////////////////////////////////////////////////////////
// Vars from common.h

uint8_t cs_status = CS_STATUS_UNKNOWN;
LocoInfo loco;

///////////////////////////////////////////////////////////////////////////////

int main();
void init();

void button_pressed(uint8_t button);
void uart_received(uint8_t recipient, uint8_t *data, uint8_t size);
void encoder_changed(int8_t val);
void state_update(uint16_t counter);
void dcc_led_update(uint16_t counter);
void steps_send_update(uint16_t counter);

void uart_broadcast_received(uint8_t *data, uint8_t size);
void uart_for_me_received(uint8_t *data, uint8_t size);
void uart_addressed();
void uart_addressed_stopped();

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
	uart_on_addressed = uart_addressed;
	uart_on_addressed_stopped = uart_addressed_stopped;
	encoder_init();
	encoder_on_change = encoder_changed;

	// Setup main timer0 on 1 ms
	TCCR0A |= 1 << WGM01; // CTC mode
	TCCR0B |= 0x03; // prescaler 64×
	TIMSK0 |= 1 << OCIE0A; // enable interrupt on compare match A
	OCR0A = 114; // set compare match A to match 1 ms

	loco.addr = 3;  // TODO

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
	static uint16_t counter = 0;

	// Timer0 on 1 ms
	btn_update();
	encoder_update();
	uart_update();
	state_show(counter);
	state_update(counter);
	dcc_led_update(counter);
	steps_send_update(counter);

	counter++;
}

void button_pressed(uint8_t button) {
	if ((button == BTN_TL1) && (uart_can_fill_output_buf()) && (!btn_pressed[BTN_TL4])) {
		// TL1 for DCC on/off

		if (cs_status == CS_STATUS_OFF) {
			led_gr_left_toggle();
			uart_output_buf[0] = 0x21;
			uart_output_buf[1] = 0x81;
			uart_send_buf_autolen();
		} else if (cs_status == CS_STATUS_ON) {
			uart_output_buf[0] = 0x21;
			uart_output_buf[1] = 0x80;
			uart_send_buf_autolen();
		}
	}

	if ((state == ST_LOCO_MINE) || (state == ST_LOCO_STOLEN)) {
		if (button == BTN_INC) {
			if (loco.steps > 0) {
				loco.steps = 0;
				loco.steps_buf = 0;
			} else {
				loco.forward = !loco.forward;
			}

			if (state == ST_LOCO_STOLEN)
				state = ST_LOCO_MINE;

			loco_send_seedir();
		} else if (button == BTN_TL2) {
			if (btn_pressed[BTN_TL4])
				loco.fa ^= 0x08;
			else
				loco.fa ^= 0x10;
			loco_send_fa();
		} else if (button == BTN_TL3) {
			if (btn_pressed[BTN_TL4]) {
				loco.fb ^= 0x01;
				loco_send_fb_58();
			} else {
				loco.fa ^= 0x01;
				loco_send_fa();
			}
		} else if (button == BTN_TL5) {
			if (btn_pressed[BTN_TL4]) {
				loco.fb ^= 0x02;
				loco_send_fb_58();
			} else {
				loco.fa ^= 0x02;
				loco_send_fa();
			}
		} else if (button == BTN_TL6) {
			if (btn_pressed[BTN_TL4]) {
				loco.fb ^= 0x04;
				loco_send_fb_58();
			} else {
				loco.fa ^= 0x04;
				loco_send_fa();
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void uart_addressed() {
	if (state == ST_XN_UNADDRESSED)
		state = ST_CS_STATUS_ASKING;
}

void uart_addressed_stopped() {
	state = ST_XN_UNADDRESSED;
	// TODO
}

void uart_received(uint8_t recipient, uint8_t *data, uint8_t size) {
	if (recipient == 0)
		uart_broadcast_received(data, size);
	else if (recipient == xpressnet_addr)
		uart_for_me_received(data, size);
}

void uart_broadcast_received(uint8_t *data, uint8_t size) {
	if ((size == 3) && (data[0] == 0x61)) {
		if (data[1] == 0x01) {
			cs_status = CS_STATUS_ON;
			led_red_off();
		} else if (data[1] == 0x00) {
			cs_status = CS_STATUS_OFF;
			led_red_on();
		} else if (data[1] == 0x02) {
			cs_status = CS_STATUS_SERVICE;
			led_red_on();
		}
	}
}

void uart_for_me_received(uint8_t *data, uint8_t size) {
	if (size == 4 && data[0] == 0x62 && data[1] == 0x22) {
		// Command station status indication response
		if (data[2] & 0x01)
			cs_status = CS_STATUS_OFF;
		else if (data[2] & 0x08)
			cs_status = CS_STATUS_SERVICE;
		else
			cs_status = CS_STATUS_ON;

		if (state == ST_CS_STATUS_ASKING)
			state = ST_LOCO_STATUS_ASKING;
	} else if (size == 6 && data[0] == 0xE4) {
		// Locomotive information normal locomotive
		loco.free = (data[1] >> 3) & 0x01;
		uint8_t step_mode = data[1] & 0x03;
		loco.step_mode = STEPS_28;
		loco.forward = data[2] >> 7;
		loco.fa = data[3];
		loco.fb = data[4];

		uint8_t steps = data[2] & 0x7F;
		if (step_mode == STEPS_14) {
			steps--;
			loco.steps = 2*steps;
		} else if (step_mode == STEPS_27 || step_mode == STEPS_28) {
			steps = (steps << 1) | ((steps >> 4) & 0x01);
			if (steps < 4)
				steps = 0;
			else
				steps -= 3;
			loco.steps = steps;
		} else if (step_mode = STEPS_128) {
			if (steps < 2)
				steps = 0;
			else
				steps--;
			loco.steps = (steps / 4.57);
		} else
			loco.steps = 0;

		loco.steps_buf = loco.steps;

		if (state == ST_LOCO_STATUS_ASKING) {
			if (loco.free)
				state = ST_LOCO_MINE;
			else
				state = ST_LOCO_STOLEN;

			led_red_off();
			led_gr_left_off();
			led_gr_right_off();
		}
	} else if ((size == 5) && (data[0] == 0xE3) && (data[1] == 0x40) &&
	           (data[2] == loco_addr_hi()) && (data[3] == loco_addr_lo())) {
		// Out locomotive is being operated by another device
		if (state == ST_LOCO_MINE)
			state = ST_LOCO_STOLEN;
	}
}

///////////////////////////////////////////////////////////////////////////////

void encoder_changed(int8_t val) {
	if (state == ST_LOCO_MINE || state == ST_LOCO_STOLEN) {
		if (val > 0 && loco.steps_buf < MAX_STEP)
			loco.steps_buf++;
		else if (val < 0 && loco.steps_buf > 0)
			loco.steps_buf--;
	}
}

///////////////////////////////////////////////////////////////////////////////

void state_update(uint16_t counter) {
	if ((state == ST_CS_STATUS_ASKING) && ((counter%1000) == 0)) {
		if (uart_can_fill_output_buf()) {
			uart_output_buf[0] = 0x21;
			uart_output_buf[1] = 0x24;
			uart_send_buf_autolen();
		}
	} else if ((state == ST_LOCO_STATUS_ASKING || state == ST_LOCO_STOLEN) && ((counter%500) == 0)) {
		if (uart_can_fill_output_buf()) {
			// Ask for loco status
			uart_output_buf[0] = 0xE3;
			uart_output_buf[1] = 0x00;
			uart_output_buf[2] = loco_addr_hi();
			uart_output_buf[3] = loco_addr_lo();
			uart_send_buf_autolen();
		}
	}
}

void dcc_led_update(uint16_t counter) {
	if (cs_status == CS_STATUS_OFF || cs_status == CS_STATUS_SERVICE) {
		if ((counter%500) == 0)
			led_red_on();
		else if ((counter%500) == 250)
			led_red_off();
	}
}

void steps_send_update(uint16_t counter) {
	if ((counter%50) == 0 && loco.steps != loco.steps_buf) {
		loco.steps = loco.steps_buf;
		if (state == ST_LOCO_STOLEN)
			state = ST_LOCO_MINE;
		loco_send_seedir();
	}
}

///////////////////////////////////////////////////////////////////////////////
