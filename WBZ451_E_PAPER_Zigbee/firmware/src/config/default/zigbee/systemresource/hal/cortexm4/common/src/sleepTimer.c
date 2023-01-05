/*******************************************************************************
  HAL Sleep Timer Source File

  Company:
    Microchip Technology Inc.

  File Name:
    sleepTimer.c

  Summary:
    This file contains the implementation of the sleep timer.

  Description:
    This file contains the implementation of the sleep timer.
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

/******************************************************************************
                   Includes section
******************************************************************************/
#include <hal/include/sleepTimer.h>
#include <hal/include/appTimer.h>
#include <hal/cortexm4/pic32cx/include/halSleepTimerClock.h>
#include <hal/cortexm4/pic32cx/include/halSleep.h>
#include <hal/cortexm4/pic32cx/include/halDbg.h>
/******************************************************************************
                   Define(s) section
******************************************************************************/
#define HAL_NULL_POINTER                      -1
#define HAL_TIME_CAN_NOT_BE_COUNTED           -2
#define HAL_SLEEP_TIMER_HAS_ALREADY_STARTED   -3
#define HAL_SLEEP_TIMER_HAS_ALREADY_STOPPED   -1

/******************************************************************************
                   External global variables section
******************************************************************************/
extern HAL_SleepControl_t halSleepControl;

/******************************************************************************
                   Prototypes section
******************************************************************************/
void sleepTimerFired(void);

/******************************************************************************
                   Implementations section
******************************************************************************/

/******************************************************************************
Starts sleep timer. Interval must be greater one tick time.
  Parameters:
    sleepTimer - address of the HAL_SleepTimer_t.
  Returns:
    -1 - NULL pointer, \n
    -2 - interval can not be counted out, \n
    -3 - sleep timer has already started. \n
     0 - otherwise.
******************************************************************************/
int HAL_StartSleepTimer(HAL_SleepTimer_t *sleepTimer)
{


  if (!sleepTimer)
    return HAL_NULL_POINTER;

  // Convert millisecond interval to the sleep timer ticks.
#ifndef _PIC32CX_
  uint32_t tempValue = 0;

  tempValue = (halSleepTimerFrequency() * sleepTimer->interval) / 1000ul;

  if (!tempValue)
    return HAL_TIME_CAN_NOT_BE_COUNTED;// Can't count out interval
#endif

  if (HAL_SLEEP_TIMER_IS_STARTED == halSleepControl.sleepTimerState) // there is active timer
    return HAL_SLEEP_TIMER_HAS_ALREADY_STARTED;

  // Start asynchronous timer2.
  halSleepControl.sleepTimerState = HAL_SLEEP_TIMER_IS_STARTED;
  halSleepControl.sleepTimer = *sleepTimer;
#ifndef _PIC32CX_

  halSetSleepTimerInterval(tempValue);
#endif
  return 0;
}// end sleepTimer_start

#if 0
/******************************************************************************
Removes timer.
Parameters:
  sleepTimer - is not used now. For capabilities for old version.
Returns:
  -1 - there is no active sleep timer.
   0 - otherwise.
******************************************************************************/
int HAL_StopSleepTimer(HAL_SleepTimer_t *sleepTimer)
{
  (void)sleepTimer;

  // there is no active timer
  if (HAL_SLEEP_TIMER_IS_STOPPED == halSleepControl.sleepTimerState)
    return HAL_SLEEP_TIMER_HAS_ALREADY_STOPPED;
  halClearTimeControl();
  halSleepControl.sleepTimerState = HAL_SLEEP_TIMER_IS_STOPPED;

  return 0;
}
#endif
/******************************************************************************
Interrupt handler about sleep interval was completed.
******************************************************************************/
void halAsyncTimerHandler(void)
{
  // there isn't work timer
  if (HAL_SLEEP_TIMER_IS_STOPPED == halSleepControl.sleepTimerState)
    return;

  if (TIMER_REPEAT_MODE == halSleepControl.sleepTimer.mode)
  {
    sysAssert(halSleepControl.sleepTimer.callback, SLEEPTIMER_NULLCALLBACK_0);
    halSleepControl.sleepTimer.callback();

    // user can stop timer in callback
    if (HAL_SLEEP_TIMER_IS_STOPPED == halSleepControl.sleepTimerState)
      return;
#ifndef _PIC32CX_

    halSetSleepTimerInterval(halSleepControl.sleepTimer.interval);
#endif
  }
  else
  {
    if (HAL_SLEEP_MODE == halSleepControl.wakeupStation)
      halPowerOn(HAL_SLEEP_TIMER_IS_WAKEUP_SOURCE);
    halSleepControl.sleepTimerState = HAL_SLEEP_TIMER_IS_STOPPED;
    if (halSleepControl.sleepTimer.callback)
      halSleepControl.sleepTimer.callback();
  }
}

//eof sleepTimer.c
