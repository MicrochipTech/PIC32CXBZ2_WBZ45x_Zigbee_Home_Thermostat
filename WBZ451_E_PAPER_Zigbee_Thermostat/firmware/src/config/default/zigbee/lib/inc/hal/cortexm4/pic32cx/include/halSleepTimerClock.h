/*******************************************************************************
  HAL Sleep TimerClock Header File

  Company:
    Microchip Technology Inc.

  File Name:
    halSleepTimerClock.h

  Summary:
    This file contains the Definition for count out requested sleep interval.

  Description:
    This file contains the Definition for count out requested sleep interval.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

/******************************************************************************
 *   WARNING: CHANGING THIS FILE MAY AFFECT CORE FUNCTIONALITY OF THE STACK.  *
 *   EXPERT USERS SHOULD PROCEED WITH CAUTION.                                *
 ******************************************************************************/

#ifndef _HALSLEEPTIMERCLOCK_H
#define _HALSLEEPTIMERCLOCK_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <systemenvironment/include/sysTypes.h>
#include <hal/include/halTaskManager.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define SLEEPTIMER_CLOCK                    32768ul
#define SLEEPTIMER_PRESCALER                (5)
#define SLEEPTIMER_DIVIDER                  (1 << SLEEPTIMER_PRESCALER)
#define HAL_MIN_SLEEP_TIME_ALLOWED          (1)

#define HAL_WakeUpCPU                        halWakeupFromIrq
/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
Starts the sleep timer clock.
******************************************************************************/
void halStartSleepTimerClock(void);

/******************************************************************************
Stops the sleep timer clock.
******************************************************************************/
void halStopSleepTimerClock(void);

/******************************************************************************
Sets interval.
Parameters:
  value - contains number of ticks which the timer must count out.
Returns:
  none.
******************************************************************************/
void halSetSleepTimerInterval(uint32_t value);

/******************************************************************************
Returns the sleep timer frequency in Hz.
Parameters:
  none.
Returns:
  the sleep timer frequency in Hz.
******************************************************************************/
uint32_t halSleepTimerFrequency(void);

/**************************************************************************//**
\brief Clear timer control structure
******************************************************************************/
void halClearTimeControl(void);

/**************************************************************************//**
\brief Wake up procedure for all external interrupts
******************************************************************************/
void halWakeupFromIrq(void);

/**************************************************************************//**
\brief Get time of sleep timer.

\return
  time in ms.
******************************************************************************/
uint64_t halGetTimeOfSleepTimer(void);

/******************************************************************************
                   Inline static functions section
******************************************************************************/

/******************************************************************************
  Interrupt handler signal implementation
******************************************************************************/
INLINE void halInterruptSleepClock(void)
{
  //halPostTask(HAL_ASYNC_TIMER);
}

/******************************************************************************
  Interrupt handler signal implementation
******************************************************************************/
INLINE void halSynchronizeSleepTime(void)
{
  //halPostTask(HAL_SYNC_SLEEP_TIME);
}


#endif /* _HALSLEEPTIMERCLOCK_H */

// eof halSleepTimerClock.h
