#ifndef __STM32F0xx_IT_H
#define __STM32F0xx_IT_H

void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void EXTI2_3_IRQHandler(void);
void DMA1_Channel2_3_IRQHandler(void);

#endif /* __STM32F0xx_IT_H */
