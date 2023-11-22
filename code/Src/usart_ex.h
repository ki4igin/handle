#ifndef __USART_EX_H__
#define __USART_EX_H__

#include "stm32f0xx.h"

void uart_send_dma(void *buf, uint32_t size);
void uart_recv_dma(void *buf, uint32_t size);
void uart_stop_recv(void);

void uart_send_dma_callback(void);
void uart_recv_dma_callback(void);
void uart_recv_timeout_callback(void);

#endif /* __USART_H__ */
