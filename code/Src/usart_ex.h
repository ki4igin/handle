#ifndef __USART_EX_H__
#define __USART_EX_H__

#include "stm32f0xx.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_usart.h"

inline static void uart_send_array_dma(void *buf, uint32_t size)
{
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_ClearFlag_GI2(DMA1);

    LL_DMA_ConfigAddresses(
        DMA1,
        LL_DMA_CHANNEL_2,
        (uint32_t)buf,
        LL_USART_DMA_GetRegAddr(USART1, LL_USART_DMA_REG_DATA_TRANSMIT),
        LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_2));
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, size);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
}

#endif /* __USART_H__ */
