#ifndef _UART_H_
#define _UART_H_

/* XpressNET communication via UART.
 * Automatic collision solving relies on running timer0.
 */

#include <stdio.h>
#include <stdbool.h>

#define UART_OUTPUT_BUF_MAX_SIZE 32
#define UART_INPUT_BUF_MAX_SIZE 42
extern uint8_t uart_output_buf[UART_OUTPUT_BUF_MAX_SIZE];
extern uint8_t uart_output_buf_size;

extern uint8_t uart_input_buf[UART_INPUT_BUF_MAX_SIZE];
extern uint8_t uart_input_buf_size;

#define XN_MAX_ADDR 31
extern uint8_t xpressnet_addr;

extern void (*uart_on_receive)(uint8_t recipient, uint8_t *data, uint8_t size);

void uart_init(uint8_t xn_addr);

bool uart_can_fill_output_buf();
int uart_send(uint8_t *data, uint8_t size);
int uart_send_buf_autolen();
int uart_send_buf();

#endif
