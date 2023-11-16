#include "gpio.h"
#include "gpio_ex.h"

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/

/** Configure pins as
 * Analog
 * Input
 * Output
 * EVENT_OUT
 * EXTI
 */
void MX_GPIO_Init(void)
{
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

    /**/
    LL_GPIO_ResetOutputPin(LEDS_GPIO_Port, LED_B_Pin);

    /**/
    LL_GPIO_ResetOutputPin(LEDS_GPIO_Port, LED_R_Pin);

    /**/
    LL_GPIO_ResetOutputPin(LEDS_GPIO_Port, LED_G_Pin);

    /**/
    LL_GPIO_ResetOutputPin(LOCK_GPIO_Port, LOCK_Pin);

    /**/
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE2);

    /**/
    LL_GPIO_SetPinPull(IRQ_3911_GPIO_Port, IRQ_3911_Pin, LL_GPIO_PULL_NO);

    /**/
    LL_GPIO_SetPinMode(IRQ_3911_GPIO_Port, IRQ_3911_Pin, LL_GPIO_MODE_INPUT);

    /**/
    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_2;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LED_B_Pin | LED_R_Pin | LED_G_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(LEDS_GPIO_Port, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = BTN2_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    LL_GPIO_Init(BTN2_GPIO_Port, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = LOCK_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(LOCK_GPIO_Port, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = BTN1_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    LL_GPIO_Init(BTN1_GPIO_Port, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    NVIC_SetPriority(EXTI2_3_IRQn, 0);
    NVIC_EnableIRQ(EXTI2_3_IRQn);
}
