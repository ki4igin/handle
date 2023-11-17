#ifndef __GPIO_EX_H__
#define __GPIO_EX_H__

#include "stm32f0xx.h"
#include "stm32f0xx_ll_gpio.h"

#define IRQ_3911_Pin       LL_GPIO_PIN_2
#define IRQ_3911_GPIO_Port GPIOA
#define IRQ_3911_EXTI_IRQn EXTI2_3_IRQn

#define LED_B_Pin          LL_GPIO_PIN_3
#define LED_R_Pin          LL_GPIO_PIN_4
#define LED_G_Pin          LL_GPIO_PIN_5
#define LEDS_GPIO_Port     GPIOA
#define BTN2_Pin           LL_GPIO_PIN_6
#define BTN2_GPIO_Port     GPIOA
#define LOCK_Pin           LL_GPIO_PIN_14
#define LOCK_GPIO_Port     GPIOB
#define BTN1_Pin           LL_GPIO_PIN_7
#define BTN1_GPIO_Port     GPIOB

#define SPI_NSS_Port       GPIOA
#define SPI_NSS_Pin        LL_GPIO_PIN_15

#define gpio_on_off_toggle_declare(_name, _port, _pin) \
    inline static void gpio_##_name##_on(void)         \
    {                                                  \
        LL_GPIO_SetOutputPin(_port, _pin);             \
    }                                                  \
    inline static void gpio_##_name##_off(void)        \
    {                                                  \
        LL_GPIO_ResetOutputPin(_port, _pin);           \
    }                                                  \
    inline static void gpio_##_name##_toggle(void)     \
    {                                                  \
        LL_GPIO_TogglePin(_port, _pin);                \
    }

// clang-format off
gpio_on_off_toggle_declare(ledr, LEDS_GPIO_Port, LED_R_Pin)
gpio_on_off_toggle_declare(ledb, LEDS_GPIO_Port, LED_G_Pin)
gpio_on_off_toggle_declare(ledg, LEDS_GPIO_Port, LED_B_Pin)
// clang-format on

#endif
