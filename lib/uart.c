#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef BAUD
#define BAUD 62500
#endif
#include <util/setbaud.h>

#include "uart.h"
#include "../src/hardware.h"

uint8_t uart_output_buf[UART_OUTPUT_BUF_MAX_SIZE];
uint8_t uart_output_buf_size = 0;
uint8_t uart_next_byte_to_send = 0;
bool sending = false;
bool waiting_for_send = false;

uint8_t uart_input_buf[UART_INPUT_BUF_MAX_SIZE];
uint8_t uart_input_buf_size;
bool receiving = false;
uint8_t received_xor;

uint8_t xpressnet_addr;

///////////////////////////////////////////////////////////////////////////////

void send_next_byte();
void _uart_send_buf();
void _uart_received_ninth(uint8_t data);
void _uart_received_non_ninth(uint8_t data);
bool _parity_ok(uint8_t data);
uint8_t _message_len(uint8_t header_byte);

///////////////////////////////////////////////////////////////////////////////
// Init

void uart_init(uint8_t xn_addr) {
	xpressnet_addr = xn_addr;

	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

#if USE_2X
	UCSR0A |= _BV(U2X0);
#else
	UCSR0A &= ~(_BV(U2X0));
#endif

	// Set RS485 direction bits
	DDRD |= _BV(PORTD2); // output
	uart_in();

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // 9-bit data
	UCSR0B = _BV(RXCIE0) | _BV(TXCIE0) | _BV(UCSZ02) | _BV(RXEN0) | _BV(TXEN0);  // RX, TX enable; RT, TX interrupt enable
}

///////////////////////////////////////////////////////////////////////////////
// Sending

int uart_send(uint8_t *data, uint8_t size) {
	if (!uart_can_fill_output_buf())
		return 1;
	if (size > UART_OUTPUT_BUF_MAX_SIZE)
		return 2;

	for (uint8_t i = 0; i < size; i++)
		uart_output_buf[i] = data[i];
	uart_output_buf_size = size;

	uart_send_buf();
	return 0;
}

int uart_send_buf() {
	if (sending)
		return 1;
	waiting_for_send = true;
	return 0;
}

void _uart_send_buf() {
	sending = true;
	waiting_for_send = false;
	uart_next_byte_to_send = 0;
	uart_out();
}

void send_next_byte() {
	loop_until_bit_is_set(UCSR0A, UDRE0); // wait for mepty transmit buffer

	if (uart_next_byte_to_send == 0)
		UCSR0B |= _BV(TXB80);
	else
		UCSR0B &= ~_BV(TXB80);

	UDR0 = uart_output_buf[uart_next_byte_to_send];
	uart_next_byte_to_send++;
}

ISR(USART_TX_vect) {
	if (uart_next_byte_to_send < uart_output_buf_size) {
		send_next_byte();
	} else {
		uart_in();
		sending = false;
	}
}

bool uart_can_fill_output_buf() {
	return !sending && !waiting_for_send;
}

///////////////////////////////////////////////////////////////////////////////
// Receiving

ISR(USART_RX_vect) {
	uint8_t status = UCSR0A;
	bool ninth = (UCSR0B >> 1) & 0x01;
	uint8_t data = UDR0;

	if (status & (1<<FE0)|(1<<DOR0)|(1<<UPE0))
		return; // return on error

	if (ninth)
		_uart_received_ninth(data);
	else
		_uart_received_non_ninth(data);
}

void _uart_received_ninth(uint8_t data) {
	uint8_t addr;

	addr = data & 0x1F;
	if ((addr != xpressnet_addr) && (addr != 0))
		return;

	if (!_parity_ok(data))
		return;

	if ((data >> 5) & 0x03 == 0x02) {
		// normal inquiry -> send data ASAP
		if (waiting_for_send)
			_uart_send_buf();
	} else if ((data >> 5) & 0x03 == 0) {
		// request acknowledgement
		// TODO
	} else {
		// message for us
		receiving = true;
		received_xor = 0;
		uart_input_buf_size = 0;
	}
}

void _uart_received_non_ninth(uint8_t data) {
	received_xor ^= data;
	uart_input_buf[uart_input_buf_size] = data;
	uart_input_buf_size++;

	if (uart_input_buf_size >= _message_len(uart_input_buf[0])) {
		// whole message received
		receiving = false;
		if (received_xor == 0) {
			// TODO whole message received
		}
	}
}

bool _parity_ok(uint8_t data) {
	bool parity = false;
	for (uint8_t i = 0; i < 8; i++) {
		parity |= data & 0x01;
		data >>= 1;
	}
	return !parity;
}

uint8_t _message_len(uint8_t header_byte) {
	return (header_byte & 0x0F) + 3;
}
