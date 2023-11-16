/*! \file
 *
 *  \author
 *
 *  \brief Platform header file. Defining platform independent functionality.
 *
 */

/*
 *      PROJECT:
 *      $Revision: $
 *      LANGUAGE:  ISO C99
 */

/*! \file platform.h
 *
 *  \author Gustavo Patricio
 *
 *  \brief Platform specific definition layer
 *
 *  This should contain all platform and hardware specifics such as
 *  GPIO assignment, system resources, locks, IRQs, etc
 *
 *  Each distinct platform/system/board must provide this definitions
 *  for all SW layers to use
 *
 */

#ifndef PLATFORM_H
#define PLATFORM_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/

// #include "stdint.h"
// #include "stdbool.h"
#include "limits.h"

#include "stm32f0xx.h"
#include "spi.h"
#include "spi_ex.h"

#include "timer.h"
#include "tick.h"
#include "spi.h"
#include "log.h"
#include "gpio_ex.h"
#include "stm32f0xx_ll_gpio.h"
#include "st_types.h"
#include "st25r3911_com.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/
#define ST25R391X_SS_PIN          SPI_NSS_Pin  /*!< GPIO pin used for ST25R3911 SPI SS                */
#define ST25R391X_SS_PORT         SPI_NSS_Port /*!< GPIO port used for ST25R3911 SPI SS port          */

#define ST25R391X_INT_PIN         IRQ_3911_Pin       /*!< GPIO pin used for ST25R3911 External Interrupt    */
#define ST25R391X_INT_PORT        IRQ_3911_GPIO_Port /*!< GPIO port used for ST25R3911 External Interrupt   */

#define PLATFORM_LED_FIELD_PIN    LED_B_Pin /*GPIO_PIN_1*/ /*!< GPIO pin used as field LED                        */ /* SWIM pin conflict */
#define PLATFORM_LED_FIELD_PORT   LEDS_GPIO_Port /*GPIOD*/ /*!< GPIO port used as field LED                       */ /* SWIM pin conflict */
#define PLATFORM_LED_A_PIN        LED_B_Pin                                                                          /*!< GPIO pin used for LED A    */
#define PLATFORM_LED_A_PORT       LEDS_GPIO_Port                                                                     /*!< GPIO port used for LED A   */
#define PLATFORM_LED_B_PIN        LED_B_Pin                                                                          /*!< GPIO pin used for LED B    */
#define PLATFORM_LED_B_PORT       LEDS_GPIO_Port                                                                     /*!< GPIO port used for LED B   */
#define PLATFORM_LED_F_PIN        LED_B_Pin                                                                          /*!< GPIO pin used for LED F    */
#define PLATFORM_LED_F_PORT       LEDS_GPIO_Port                                                                     /*!< GPIO port used for LED F   */
#define PLATFORM_LED_V_PIN        LED_B_Pin                                                                          /*!< GPIO pin used for LED V    */
#define PLATFORM_LED_V_PORT       LEDS_GPIO_Port                                                                     /*!< GPIO port used for LED V   */
#define PLATFORM_LED_AP2P_PIN     LED_B_Pin                                                                          /*!< GPIO pin used for LED AP2P */
#define PLATFORM_LED_AP2P_PORT    LEDS_GPIO_Port                                                                     /*!< GPIO port used for LED AP2P*/

#define PLATFORM_USER_BUTTON_PIN  BTN1_Pin       /*!< GPIO pin user button       */
#define PLATFORM_USER_BUTTON_PORT BTN1_GPIO_Port /*!< GPIO port user button      */

/*
******************************************************************************
* GLOBAL MACROS
******************************************************************************
*/

#define platformProtectST25R391xComm() \
    do {                               \
        globalCommProtectCnt++;        \
        __DSB();                       \
        NVIC_DisableIRQ(EXTI2_3_IRQn); \
        __DSB();                       \
        __ISB();                       \
    } while (0) /*!< Protect unique access to ST25R391x communication channel - IRQ disable on single thread environment (MCU) ; Mutex lock on a multi thread environment */
#define platformUnprotectST25R391xComm()    \
    do {                                    \
        if (--globalCommProtectCnt == 0U) { \
            NVIC_EnableIRQ(EXTI2_3_IRQn);   \
        }                                   \
    } while (0) /*!< Unprotect unique access to ST25R391x communication channel - IRQ enable on a single thread environment (MCU) ; Mutex unlock on a multi thread environment */

#define platformIrqST25R3911SetCallback(cb)
#define platformIrqST25R3911PinInitialize()

#define platformProtectST25R391xIrqStatus()   platformProtectST25R391xComm()   /*!< Protect unique access to IRQ status var - IRQ disable on single thread environment (MCU) ; Mutex lock on a multi thread environment */
#define platformUnprotectST25R391xIrqStatus() platformUnprotectST25R391xComm() /*!< Unprotect the IRQ status var - IRQ enable on a single thread environment (MCU) ; Mutex unlock on a multi thread environment         */

#define platformLedsInitialize()              /*!< Initializes the pins used as LEDs to outputs*/

#define platformLedOff(port, pin)             platformGpioClear(port, pin)  /*!< Turns the given LED Off                     */
#define platformLedOn(port, pin)              platformGpioSet(port, pin)    /*!< Turns the given LED On                      */
#define platformLedToogle(port, pin)          platformGpioToogle(port, pin) /*!< Toogle the given LED                        */

#define platformGpioSet(port, pin)            LL_GPIO_SetOutputPin(port, pin)
#define platformGpioClear(port, pin)          LL_GPIO_ResetOutputPin(port, pin)
#define platformGpioToogle(port, pin)         LL_GPIO_WriteOutputPort(port, pin)
#define platformGpioIsHigh(port, pin)         ((((uint32_t)((port)->IDR) & (uint32_t)(pin)) != 0U))
#define platformGpioIsLow(port, pin)          (!platformGpioIsHigh(port, pin))

#define platformTimerCreate(t)                timerCalculateTimer(t) /*!< Create a timer with the given time (ms)     */
#define platformTimerIsExpired(timer)         timerIsExpired(timer)  /*!< Checks if the given timer is expired        */
#define platformDelay(t)                      timerDelay(t)          /*!< Performs a delay for the given time (ms)    */

#define platformGetSysTick()                  tickGet() /*!< Get System Tick ( 1 tick = 1 ms)            */

#define platformSpiSelect()                   platformGpioClear(ST25R391X_SS_PORT, ST25R391X_SS_PIN) /*!< SPI SS\CS: Chip|Slave Select                */
#define platformSpiDeselect()                 platformGpioSet(ST25R391X_SS_PORT, ST25R391X_SS_PIN)   /*!< SPI SS\CS: Chip|Slave Deselect              */
#define platformSpiTxRx(txBuf, rxBuf, len)    spi_txrx(txBuf, rxBuf, len)                            /*!< SPI transceive                              */

#define platformLog(...)                      logPrint(__VA_ARGS__) /*!< Log  method                                 */

#define platformLogReg(_reg)                                  \
    do {                                                      \
        uint8_t tmp;                                          \
        st25r3911ReadRegister(_reg, &tmp);                    \
        platformLog("reg " #_reg ": %s\n", hex2Str(&tmp, 1)); \
        LL_mDelay(100);                                       \
    } while (0)
/*
******************************************************************************
* RFAL FEATURES CONFIGURATION
******************************************************************************
*/

#define RFAL_FEATURE_NFCA                   true  /*!< Enable/Disable RFAL support for NFC-A (ISO14443A)                         */
#define RFAL_FEATURE_NFCB                   false /*!< Enable/Disable RFAL support for NFC-B (ISO14443B)                         */
#define RFAL_FEATURE_NFCF                   false /*!< Enable/Disable RFAL support for NFC-F (FeliCa)                            */
#define RFAL_FEATURE_NFCV                   false /*!< Enable/Disable RFAL support for NFC-V (ISO15693)                          */
#define RFAL_FEATURE_T1T                    false /*!< Enable/Disable RFAL support for T1T (Topaz)                               */
#define RFAL_FEATURE_ST25TB                 false /*!< Enable/Disable RFAL support for ST25TB                                    */
#define RFAL_FEATURE_DYNAMIC_ANALOG_CONFIG  false /*!< Enable/Disable Analog Configs to be dynamically updated (RAM)             */
#define RFAL_FEATURE_DPO                    false /*!< Enable/Disable RFAL dynamic power support                                 */
#define RFAL_FEATURE_ISO_DEP                false /*!< Enable/Disable RFAL support for ISO-DEP (ISO14443-4)                      */
#define RFAL_FEATURE_ISO_DEP_POLL           false /*!< Enable/Disable RFAL support for Poller mode (PCD) ISO-DEP (ISO14443-4)    */
#define RFAL_FEATURE_ISO_DEP_LISTEN         false /*!< Enable/Disable RFAL support for Listen mode (PICC) ISO-DEP (ISO14443-4)   */
#define RFAL_FEATURE_NFC_DEP                false /*!< Enable/Disable RFAL support for NFC-DEP (NFCIP1/P2P)                      */

#define RFAL_FEATURE_ISO_DEP_IBLOCK_MAX_LEN 64 /*!< ISO-DEP I-Block max length. Please use values as defined by rfalIsoDepFSx */
#define RFAL_FEATURE_ISO_DEP_APDU_MAX_LEN   64 /*!< ISO-DEP APDU max length. Please use multiples of I-Block max length       */

#endif /* PLATFORM_H */
