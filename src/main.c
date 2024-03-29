#include <stdbool.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>

#include "hardware.h"
#include "buttons.h"
#include "encoder.h"
#include "lib/xpressnet.h"
#include "state.h"
#include "common.h"

///////////////////////////////////////////////////////////////////////////////
// Vars from common.h

uint8_t cs_status = CS_STATUS_UNKNOWN;
LocoInfo loco;
uint16_t last_loco = 0xFFFF;
uint16_t loco_release_start = 0;

///////////////////////////////////////////////////////////////////////////////

int main();
void init();
void eeprom_init();

void button_pressed(uint8_t button);
void encoder_changed(int8_t val);
void state_update(uint16_t counter);
void steps_send_update(uint16_t counter);

void uart_received(uint8_t recipient, uint8_t *data, uint8_t size);
void uart_sniffed(uint8_t sender, uint8_t *data, uint8_t size);
void uart_broadcast_received(uint8_t *data, uint8_t size);
void uart_for_me_received(uint8_t *data, uint8_t size);
void uart_addressed();
void uart_addressed_stopped();
void uart_addr_changed(uint8_t new_addr);
void uart_send_cs_status_ask();
void uart_send_loco_status_ask();

///////////////////////////////////////////////////////////////////////////////

int main() {
	init();
	while (true) {}
}

void init() {
	leds_init();

	buttons_init();
	btn_on_pressed = button_pressed;

	uart_init(XN_DEFAULT_ADDR);
	uart_on_receive = uart_received;
	uart_on_addressed = uart_addressed;
	uart_on_addressed_stopped = uart_addressed_stopped;
	uart_on_addr_changed = uart_addr_changed;
	uart_on_sniff = uart_sniffed;

	encoder_init();
	encoder_on_change = encoder_changed;

	// Setup main timer0 on 1 ms
	TCCR0A |= 1 << WGM01; // CTC mode
	TCCR0B |= 0x03; // prescaler 64×
	TIMSK0 |= 1 << OCIE0A; // enable interrupt on compare match A
	OCR0A = 114; // set compare match A to match 1 ms

	// Setup timer1 on 100 us
	TCCR1A = 0;
	TCCR1B = (1 << WGM12) | (1 << CS10); // CTC mode, prescaler 1×
	TIMSK1 = (1 << OCIE1A); // enable interrupt on compare match A
	OCR1A = 1474; // set compare match A to match 100 us

	led_gr_left_on();
	led_gr_right_on();
	led_red_on();

	eeprom_init();

	_delay_ms(250);

	led_gr_left_off();
	led_gr_right_off();
	led_red_off();

	sei(); // enable interrupts globally
}

void eeprom_init() {
	loco.addr = eeprom_read_word((uint16_t*)EEPROM_LOC_LOCO_ADDR);
	if (loco.addr == 0xFFFF) { // default EEPROM value
		loco.addr = 3;
		eeprom_write_word((uint16_t*)EEPROM_LOC_LOCO_ADDR, loco.addr);
	}

	uint16_t version = eeprom_read_word((uint16_t*)EEPROM_LOC_SW_VERSION);
	if (version != SW_VERSION)
		eeprom_write_word((uint16_t*)EEPROM_LOC_SW_VERSION, SW_VERSION);

	xpressnet_addr = eeprom_read_byte((uint8_t*)EEPROM_LOC_XN_ADDR);
	if (xpressnet_addr == 0xFF) {
		xpressnet_addr = XN_DEFAULT_ADDR;
		eeprom_write_byte((uint8_t*)EEPROM_LOC_XN_ADDR, xpressnet_addr);
	}
}

///////////////////////////////////////////////////////////////////////////////

ISR(TIMER0_COMPA_vect) {
	// Timer0 on 1 ms
	static uint16_t counter = 0;

	btn_update();
	uart_update();
	state_show(counter);
	state_update(counter);
	steps_send_update(counter);

	counter++;
}

ISR(TIMER1_COMPA_vect) {
	// Timer1 on 100 us
	encoder_update();
}

void button_pressed(uint8_t button) {
	if ((button == BTN_STOP) && (uart_can_fill_output_buf()) && (!btn_pressed[BTN_SHIFT])) {
		// TL3 for DCC on/off

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
		if (button == BTN_INC && !btn_pressed[BTN_SHIFT]) {
			if (loco.steps > 0) {
				loco.steps = 0;
				loco.steps_buf = 0;
			} else {
				loco.forward = !loco.forward;
			}

			if (state == ST_LOCO_STOLEN)
				state = ST_LOCO_MINE;

			loco_send_seedir();
		} else if (button == BTN_F0) {
			if (btn_pressed[BTN_SHIFT])
				loco.fa ^= 0x08;
			else
				loco.fa ^= 0x10;
			loco_send_fa();
		} else if (button == BTN_F1) {
			if (btn_pressed[BTN_SHIFT]) {
				loco.fb ^= 0x01;
				loco_send_fb_58();
			} else {
				loco.fa ^= 0x01;
				loco_send_fa();
			}
		} else if (button == BTN_F2) {
			if (btn_pressed[BTN_SHIFT]) {
				loco.fb ^= 0x02;
				loco_send_fb_58();
			} else {
				loco.fa ^= 0x02;
				loco_send_fa();
			}
		} else if (button == BTN_F3) {
			if (btn_pressed[BTN_SHIFT]) {
				loco.fb ^= 0x04;
				loco_send_fb_58();
			} else {
				loco.fa ^= 0x04;
				loco_send_fa();
			}
		} else if (button == BTN_STOP && btn_pressed[BTN_SHIFT] && last_loco_defined() && last_loco != loco.addr) {
			// Loco acquire
			state = ST_LOCO_RELEASED;
			loco.addr = last_loco;
			last_loco = 0xFFFF;
		} else if (button == BTN_INC && btn_pressed[BTN_SHIFT]) {
			loco.steps = 0;
			loco.steps_buf = 0;
			loco_send_emergency_stop();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void uart_addressed() {
	if (state == ST_XN_UNADDRESSED) {
		state = ST_CS_STATUS_ASKING;
		uart_send_cs_status_ask();
	}
}

void uart_addressed_stopped() {
	state = ST_XN_UNADDRESSED;
}

void uart_addr_changed(uint8_t new_addr) {
	eeprom_write_byte((uint8_t*)EEPROM_LOC_XN_ADDR, new_addr);
}

void uart_received(uint8_t recipient, uint8_t *data, uint8_t size) {
	if (recipient == 0)
		uart_broadcast_received(data, size);
	else if (recipient == xpressnet_addr)
		uart_for_me_received(data, size);
}

void uart_broadcast_received(uint8_t *data, uint8_t size) {
	if ((size == 3) && (data[0] == 0x61)) {
		// DCC on/off/service
		if (data[1] == 0x01) {
			cs_status = CS_STATUS_ON;
		} else if (data[1] == 0x00) {
			cs_status = CS_STATUS_OFF;
		} else if (data[1] == 0x02) {
			cs_status = CS_STATUS_SERVICE;
		}
	}
}

void uart_for_me_received(uint8_t *data, uint8_t size) {
	if (size == 4 && data[0] == 0x62 && data[1] == 0x22) {
		// Command station status indication response
		if (data[2] & 0x03)
			cs_status = CS_STATUS_OFF;
		else if (data[2] & 0x08)
			cs_status = CS_STATUS_SERVICE;
		else
			cs_status = CS_STATUS_ON;

		if (state == ST_CS_STATUS_ASKING) {
			state = ST_LOCO_STATUS_ASKING;
			uart_send_loco_status_ask();
		}
	} else if (size == 6 && data[0] == 0xE4) {
		// Locomotive information normal locomotive
		loco.free = (data[1] >> 3) & 0x01;
		uint8_t step_mode = data[1] & 0x07;
		loco.step_mode = STEPS_28; // always 28 steps
		loco.forward = data[2] >> 7;
		loco.fa = data[3];
		loco.fb = data[4];

		uint8_t steps = data[2] & 0x7F;
		if (step_mode == STEPS_14) {
			steps = steps & 0x0F;
			if (steps > 0)
				steps--;
			loco.steps = 2*steps;
		} else if (step_mode == STEPS_27 || step_mode == STEPS_28) {
			steps = ((steps & 0x0F) << 1) | ((steps >> 4) & 0x01);
			if (steps < 4)
				steps = 0;
			else
				steps -= 3;
			loco.steps = steps;
		} else if (step_mode == STEPS_128) {
			if (steps < 2)
				steps = 0;
			else
				steps--;
			if (steps > 0)
				loco.steps = (steps / 4.57)+1;
			else
				loco.steps = 0;
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
		// Our locomotive is being operated by another device
		if (state == ST_LOCO_MINE)
			state = ST_LOCO_STOLEN;
	}
}

void uart_sniffed(uint8_t sender, uint8_t *data, uint8_t size) {
	if (size == 6 && data[0] == 0xE4 && (data[1] == 0x10 || data[1] == 0x11 || data[1] == 0x12 || data[1] == 0x13)) {
		// Loco set speed sniffed
		last_loco = (data[2] << 8) | data[3];
	}
}

void uart_send_cs_status_ask() {
	if (!uart_can_fill_output_buf())
		return;

	uart_output_buf[0] = 0x21;
	uart_output_buf[1] = 0x24;
	uart_send_buf_autolen();
}

void uart_send_loco_status_ask() {
	if (!uart_can_fill_output_buf())
		return;

	uart_output_buf[0] = 0xE3;
	uart_output_buf[1] = 0x00;
	uart_output_buf[2] = loco_addr_hi();
	uart_output_buf[3] = loco_addr_lo();
	uart_send_buf_autolen();
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
	if ((state == ST_CS_STATUS_ASKING) && ((counter%500) == 0)) {
		uart_send_cs_status_ask();
	} else if ((state == ST_LOCO_STATUS_ASKING || state == ST_LOCO_STOLEN) && ((counter%500) == 0)) {
		uart_send_loco_status_ask();
	} else if (state == ST_LOCO_RELEASED) {
		if (loco_release_start == 0) {
			loco_release_start = counter;
		} else if (counter == loco_release_start+1000) {
			state = ST_LOCO_STATUS_ASKING;
			loco_release_start = 0;
			eeprom_write_word((uint16_t*)EEPROM_LOC_LOCO_ADDR, loco.addr);
			uart_send_loco_status_ask();
		}
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
