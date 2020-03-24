#include "common.h"
#include "lib/uart.h"

void loco_send_seedir() {
	if (!uart_can_fill_output_buf())
		return;

	uart_output_buf[0] = 0xE4;
	uart_output_buf[1] = 0x12;
	uart_output_buf[2] = loco_addr_hi();
	uart_output_buf[3] = loco_addr_lo();

	uint8_t xn_steps = loco.steps;
	if (xn_steps > 0)
		xn_steps += 3;

	uart_output_buf[4] = loco.forward << 7 | ((xn_steps >> 1) & 0x0F) |
	                     ((xn_steps & 0x01) << 4);
	uart_send_buf_autolen();
}

void loco_send_fa() {
	if (!uart_can_fill_output_buf())
		return;

	uart_output_buf[0] = 0xE4;
	uart_output_buf[1] = 0x20;
	uart_output_buf[2] = loco_addr_hi();
	uart_output_buf[3] = loco_addr_lo();
	uart_output_buf[4] = loco.fa;
	uart_send_buf_autolen();
}

void loco_send_fb_58() {
	if (!uart_can_fill_output_buf())
		return;

	uart_output_buf[0] = 0xE4;
	uart_output_buf[1] = 0x21;
	uart_output_buf[2] = loco_addr_hi();
	uart_output_buf[3] = loco_addr_lo();
	uart_output_buf[4] = loco.fb & 0x0F;
	uart_send_buf_autolen();
}

void loco_send_fb_912() {
	if (!uart_can_fill_output_buf())
		return;

	uart_output_buf[0] = 0xE4;
	uart_output_buf[1] = 0x22;
	uart_output_buf[2] = loco_addr_hi();
	uart_output_buf[3] = loco_addr_lo();
	uart_output_buf[4] = (loco.fb >> 4);
	uart_send_buf_autolen();
}
