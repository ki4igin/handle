#include "tim.h"

/* TIM2 init function */
void MX_TIM2_Init(void)
{
    LL_TIM_InitTypeDef TIM_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

    TIM_InitStruct.Prescaler = 48000 - 1;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = 4294967295;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(TIM2, &TIM_InitStruct);
    LL_TIM_DisableARRPreload(TIM2);
    LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
    LL_TIM_DisableMasterSlaveMode(TIM2);
}

/* TIM14 init function */
void MX_TIM14_Init(void)
{
    LL_TIM_InitTypeDef TIM_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM14);

    TIM_InitStruct.Prescaler = 48000 - 1;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = 1000 - 1;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(TIM14, &TIM_InitStruct);
    LL_TIM_EnableARRPreload(TIM14);
}
