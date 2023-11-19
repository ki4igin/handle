#include "main.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"
#include "stm32f0xx_ll_tim.h"
#include "rfid.h"
#include "rfal_rf.h"
#include "rfal_analogConfig.h"
#include "st25r3911_com.h"
#include "st25r3911.h"
#include "aura.h"
#include "keys.h"

void SystemClock_Config(void);

uint32_t globalCommProtectCnt = 0;

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    /* SysTick_IRQn interrupt configuration */
    NVIC_SetPriority(SysTick_IRQn, 3);

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_SPI1_Init();
    MX_USART1_UART_Init();
    MX_TIM2_Init();
    MX_TIM14_Init();

    LL_TIM_EnableCounter(TIM14);
    LL_TIM_ClearFlag_UPDATE(TIM14);

    platformLog("Welcome to aura\n");
    LL_mDelay(100);

    // st25r3911SetRegisterBits(0x00, 0b101);
    // st25r3911SetRegisterBits(0x02, 0b10000000);
    LL_mDelay(1000);

    /* Initialize RFAL */
    rfalAnalogConfigInitialize();
    if (rfalInitialize() != ERR_NONE) {
        /* Initialization failed - indicate on LEDs */
        while (1) {
            gpio_ledb_toggle();
            gpio_ledr_toggle();
            gpio_ledg_toggle();

            platformDelay(500);
        }
    }

    for (uint32_t i = 0; i < 10; i++) {
        uint8_t tmp;
        st25r3911ReadRegister(i, &tmp);
        platformLog("reg %d: %s\n", i, hex2Str(&tmp, 1));
        LL_mDelay(100);
    }

    st25r3911ExecuteCommand(ST25R3911_CMD_CALIBRATE_C_SENSOR);
    LL_mDelay(100);

    platformLogReg(0x2F);
    platformLogReg(0x2E);

    keys_init();
    aura_init();

    while (1) {
        if (LL_TIM_IsActiveFlag_UPDATE(TIM14)) {
            LL_TIM_ClearFlag_UPDATE(TIM14);
            gpio_ledb_toggle();
            // aura_send_whoami();
        }
        /* Run RFAL Worker */
        rfalWorker();

        /* Run Demo Application */
        uint32_t card_found = rfid_cycle();
        if (card_found) {
            platformLog("ISO14443A/NFC-A, UID: %s\n",
                        hex2Str(rfid_card_uid.val, sizeof(rfid_card_uid.val)));
        }
        aura_cmd_process();
    }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1) {
    }
    LL_RCC_HSE_Enable();

    /* Wait till HSE is ready */
    while (LL_RCC_HSE_IsReady() != 1) {
    }
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_5, LL_RCC_PLL_MUL_8);
    LL_RCC_PLL_Enable();

    /* Wait till PLL is ready */
    while (LL_RCC_PLL_IsReady() != 1) {
    }
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
    }
    LL_Init1msTick(43392000);
    LL_SetSystemCoreClock(43392000);
    LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
// void Error_Handler(void)
// {
//     /* User can add his own implementation to report the HAL error return state */
//     __disable_irq();
//     while (1) {
//     }
// }

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
