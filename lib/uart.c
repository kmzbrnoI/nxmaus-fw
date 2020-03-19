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

///////////////////////////////////////////////////////////////////////////////

void send_next_byte();
void _uart_send_buf();

///////////////////////////////////////////////////////////////////////////////
// Init

void uart_init(void) {
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
}

char uart_getchar() {
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}
