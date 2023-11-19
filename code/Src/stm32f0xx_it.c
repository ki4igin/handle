#include "main.h"
#include "stm32f0xx_it.h"
#include "stm32f0xx_ll_tim.h"
#include "st25R3911_interrupt.h"
#include "tick.h"
#include "locker.h"

/* External variables --------------------------------------------------------*/

/******************************************************************************/
/*           Cortex-M0 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void)
{
    while (1) {
    }
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void)
{
    while (1) {
    }
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void)
{
}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void)
{
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void)
{
    tickInc();
}

/******************************************************************************/
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles EXTI line 2 and 3 interrupts.
 */
void EXTI2_3_IRQHandler(void)
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_2) != RESET) {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_2);
        st25r3911Isr();
    }
}

/**
 * @brief This function handles TIM14 global interrupt.
 */
void TIM14_IRQHandler(void)
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM14)) {
        LL_TIM_ClearFlag_UPDATE(TIM14);
        tim14_update_callback();
    }
}
