#include "spi_ex.h"
#include "stm32f0xx_ll_spi.h"
#include "string.h"
#include "stddef.h"

#define SPI_BUF_LEN 128

static uint8_t txBuf[SPI_BUF_LEN];
static uint8_t rxBuf[SPI_BUF_LEN];

void spi_txrx(const uint8_t *tx_buf, uint8_t *rx_buf, uint32_t size)
{
    if (tx_buf != NULL) {
        memcpy(txBuf, tx_buf, size);
    } else {
        memset(txBuf, 0x00, size);
    }

    const uint8_t *ptx = txBuf;
    uint8_t *prx = (rx_buf != NULL) ? rx_buf : rxBuf;

    do {
        while (LL_SPI_IsActiveFlag_TXE(SPI1) == 0) {
        }
        LL_SPI_TransmitData8(SPI1, *ptx++);
        while (LL_SPI_IsActiveFlag_RXNE(SPI1) == 0) {
        }
        *prx++ = LL_SPI_ReceiveData8(SPI1);
    } while (--size);
}
