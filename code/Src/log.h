/*
 *      PROJECT:   ST25R391x firmware
 *      $Revision: $
 *      LANGUAGE:  ANSI C
 */

/*! \file log.h
 *
 *  \brief Log
 *
 *  \author Gustavo Patricio
 *
 *
 */

#ifndef __LOG_H
#define __LOG_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
#include "platform.h"

/*
******************************************************************************
* DEFINES
******************************************************************************
*/
#define LOG_MAX_HEX_STR_LENGTH 16                                  /*<! Max length of a hex string             */
#define LOG_MAX_HEX_STR_NUM    2                                   /*<! Max number of simultaneous hex steings */
#define LOG_MAX_LENGTH         (50 + (2 * LOG_MAX_HEX_STR_LENGTH)) /*<! Max length of the buffer to be printed */

/*!
*****************************************************************************
* \brief  SPI initialize
*
* Initializes the UART peripheral
*
*****************************************************************************
*/
void logInitialize(void);

/*!
*****************************************************************************
* \brief  log print
*
* Logs the given string over UART
*
* \warning this method makes use of vsprintf which does not check for buffer
* sizes and a buffer overflow may happen. Do not use this with messages
* bigger than LOG_MAX_LENGTH
*
* \param[in]  format : string containing the message to be printed
* \param[in]  ... : variable arguments for the message
*
*****************************************************************************
*/
void logPrint(const char *format, ...);

/*!
*****************************************************************************
* \brief  Hex to String
*
* Converts the given buffer to a string containing the data in a ASCII hex
*
* \warning buffers bigger than LOG_MAX_HEX_STR_LENGTH shall not be used
*
* \param[in]  data    : buffer with data to be converted
* \param[in]  dataLen : number of bytes in the data buffer
*
* \returns a tring containing the data in a ASCII hex
*****************************************************************************
*/
char *hex2Str(unsigned char *data, uint16_t dataLen);

#endif /* __LOG_H */
