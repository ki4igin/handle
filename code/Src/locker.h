#ifndef __LOCKER_H__
#define __LOCKER_H__

#include "gpio_ex.h"
#include "stm32f0xx_ll_gpio.h"

inline static void locker_open(void)
{
    LL_GPIO_SetOutputPin(LOCK_GPIO_Port, LOCK_Pin);
}

inline static void locker_close(void)
{
    LL_GPIO_ResetOutputPin(LOCK_GPIO_Port, LOCK_Pin);
}

inline static uint32_t locker_is_open(void)
{
    return LL_GPIO_IsOutputPinSet(LOCK_GPIO_Port, LOCK_Pin);
}

#endif