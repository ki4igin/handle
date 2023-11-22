/*
 *      PROJECT:   ST25R391x firmware
 *      $Revision: $
 *      LANGUAGE:  ANSI C
 */

/*! \file log.c
 *
 *  \brief Log
 *
 *  \author Gustavo Patricio
 *
 *
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/

#include <stdarg.h>
#include "log.h"
#include "usart_ex.h"
#include <stdio.h>

/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/
static char gHexStr[LOG_MAX_HEX_STR_NUM][LOG_MAX_HEX_STR_LENGTH];
static char gLogMsg[LOG_MAX_LENGTH];
static uint8_t gHexStrIdx;

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/

/*******************************************************************************/
void logPrint(const char *format, ...)
{
    char *pStr;

    /* vsnprintf is not available, buffer overflow may happen */
    va_list argptr;
    va_start(argptr, format);
    int32_t len = vsprintf(gLogMsg, format, argptr);
    va_end(argptr);

    pStr = gLogMsg;

    uart_send_dma(pStr, len);
}

/*******************************************************************************/
char *hex2Str(uint8_t *data, uint16_t dataLen)
{
    const char *hex = "0123456789ABCDEF";
    unsigned char *pin;
    char *pout;
    char *ret;
    uint8_t i;

    pin = data;
    pout = gHexStr[gHexStrIdx];
    ret = pout;

    if (dataLen == 0) {
        *pout = 0;
    } else {
        if (dataLen > LOG_MAX_HEX_STR_LENGTH) {
            dataLen = LOG_MAX_HEX_STR_LENGTH;
        }

        for (i = 0; i < (dataLen - 1); i++) {
            *pout++ = hex[(*pin >> 4) & 0x0F];
            *pout++ = hex[(*pin++) & 0x0F];
        }
        *pout++ = hex[(*pin >> 4) & 0x0F];
        *pout++ = hex[(*pin) & 0x0F];
        *pout = 0;
    }

    gHexStrIdx++;
    gHexStrIdx %= LOG_MAX_HEX_STR_NUM;

    return ret;
}
