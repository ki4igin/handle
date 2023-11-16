/*
 *      PROJECT:   ST25R391x firmware
 *      $Revision: $
 *      LANGUAGE:  ANSI C
 */

/*! \file tick.h
 *
 *  \brief Tick implmentation
 *
 *  This modules conts the number of ticks in 1ms
 *  The increments are triggered by a Timer IRQ
 *
 *
 *  \author Gustavo Patricio
 *
 *
 */

#ifndef __TICK_H
#define __TICK_H

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/

#include "platform.h"

/*!
*****************************************************************************
* \brief  Ticke Get
*
* Gets the current value of the tick counter
*
* \return the current tick counter value (in ms)
*****************************************************************************
*/
uint32_t tickGet(void);

/*!
*****************************************************************************
* \brief  Ticke Increment
*
* Increments the tick counter. To be triggered by the timer IRQ
*
*****************************************************************************
*/
void tickInc(void);

#endif /* __TICK_H */
