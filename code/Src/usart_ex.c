#include "usart_ex.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_usart.h"

void uart_send_dma(void *buf, uint32_t size)
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

void uart_recv_dma(void *buf, uint32_t size)
{
    LL_USART_EnableRxTimeout(USART1);
    LL_DMA_ConfigAddresses(
        DMA1,
        LL_DMA_CHANNEL_3,
        LL_USART_DMA_GetRegAddr(USART1, LL_USART_DMA_REG_DATA_RECEIVE),
        (uint32_t)buf,
        LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3));
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, size);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
}

void uart_stop_recv(void)
{
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
    LL_USART_DisableRxTimeout(USART1);
    LL_DMA_ClearFlag_GI3(DMA1);
}

__WEAK void uart_send_dma_callback(void)
{
}

__WEAK void uart_recv_dma_callback(void)
{
}

__WEAK void uart_recv_timeout_callback(void)
{
}

void DMA1_Channel2_3_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_GI2(DMA1)) {
        LL_DMA_ClearFlag_GI2(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
        uart_send_dma_callback();
    }
    if (LL_DMA_IsActiveFlag_GI3(DMA1)) {
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
        LL_DMA_ClearFlag_GI3(DMA1);
        uart_recv_dma_callback();
    }
}

void USART1_IRQHandler(void)
{
    if (LL_USART_IsActiveFlag_RTO(USART1)) {
        LL_USART_ClearFlag_RTO(USART1);
        uart_recv_timeout_callback();
    }
}
