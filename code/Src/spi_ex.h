#ifndef __SPI_EX_H__
#define __SPI_EX_H__

#include "stm32f0xx.h"

void spi_txrx(const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t size);

#endif /* __SPI_H__ */
