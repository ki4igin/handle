#ifndef __TIM_EX_H__
#define __TIM_EX_H__

#include "tim.h"
#include "stm32f0xx.h"

inline static void tim_update_autoreload(TIM_TypeDef *tim)
{
    tim->DIER &= ~TIM_DIER_UIE;
    tim->EGR |= TIM_EGR_UG;
    tim->SR &= ~TIM_SR_UIF;
    tim->DIER |= TIM_DIER_UIE;
}

static void tim14_one_pulse_en(uint32_t time_ms)
{
    LL_TIM_SetAutoReload(TIM14, time_ms);
    tim_update_autoreload(TIM14);
    LL_TIM_EnableCounter(TIM14);
}

#endif
