#ifndef _UART_H_
#define _UART_H_

#include <stdio.h>
#include <stdbool.h>

#define UART_OUTPUT_BUF_MAX_SIZE 32
extern uint8_t uart_output_buf[UART_OUTPUT_BUF_MAX_SIZE];
extern uint8_t uart_output_buf_size;
extern bool sending;

void uart_init(void);

int uart_send(uint8_t *data, uint8_t size);
int uart_send_buf();

char uart_getchar();

#endif
