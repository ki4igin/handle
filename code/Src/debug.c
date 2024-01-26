#include "debug.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "stm32f0xx.h"
#include "gpio_ex.h"

#define DEBUG_BUF_SIZE 100

#ifdef DEBUG

__WEAK void debug_send(void *p, uint32_t size)
{
    (void)p;
    (void)size;
}

void debug_printf(const char *fmt, ...)
{
    static char debug_buf[DEBUG_BUF_SIZE];
    va_list argp;
    va_start(argp, fmt);

    uint32_t len = vsprintf(debug_buf, fmt, argp);
    debug_send(debug_buf, len);

    va_end(argp);
}

#else
void debug_printf(const char *fmt, ...)
{
}
#endif

void delay_ms(uint64_t delay)
{
    delay++;
    while (delay) {
        if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
            delay--;
        }
    }
}

void _debug_error_handler(const char *file, uint32_t line)
{
    __disable_irq();
    debug_printf("DEV: error: %s, %d", file, line);
    for (uint32_t i = 0; i < 50; i++) {
        delay_ms(200);
        gpio_led_all_toggle();
    }
    NVIC_SystemReset();
}
