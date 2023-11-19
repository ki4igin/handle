#ifndef __LOCKER_H__
#define __LOCKER_H__

#include "gpio_ex.h"
#include "stm32f0xx_ll_gpio.h"
#include "tim_ex.h"

inline static void locker_open(void)
{
    LL_GPIO_SetOutputPin(LOCK_GPIO_Port, LOCK_Pin);
    gpio_ledr_off();
    gpio_ledg_on();
    tim14_one_pulse_en(5000);
}

inline static void locker_close(void)
{
    LL_GPIO_ResetOutputPin(LOCK_GPIO_Port, LOCK_Pin);
    gpio_ledg_off();
    gpio_ledr_on();
}

inline static uint32_t locker_is_open(void)
{
    return LL_GPIO_IsOutputPinSet(LOCK_GPIO_Port, LOCK_Pin);
}

inline static void tim14_update_callback(void)
{
    locker_close();
}

#endif