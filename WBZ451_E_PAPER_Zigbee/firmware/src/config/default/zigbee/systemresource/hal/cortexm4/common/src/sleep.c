/*******************************************************************************
  HAL Sleep Source File

  Company:
    Microchip Technology Inc.

  File Name:
    sleep.c

  Summary:
    This file provides interface to the HAL module to enable the power off mode.

  Description:
    This file provides interface to the HAL module to enable the power off mode in a Zigbee
    Device when the device is Idle.
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
#include <hal/cortexm4/pic32cx/include/halSleep.h>
#include <hal/include/appTimer.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define HAL_NULL_POINTER                      -1
#define HAL_SLEEP_TIMER_HAS_ALREADY_STARTED   -3
#define HAL_SLEEP_TIMER_IS_BUSY               -2
#define HAL_SLEEP_SYSTEM_HAS_ALREADY_STARTED  -3

/******************************************************************************
                   Global variables section
******************************************************************************/
HAL_SleepControl_t halSleepControl =
{
  .wakeupStation = HAL_ACTIVE_MODE,
  .sleepTimerState = HAL_SLEEP_TIMER_IS_STOPPED
};

/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
\brief Starts sleep timer and HAL sleep. When system is wake up send callback
\param[in]
    sleepParam - pointer to sleep structure.
\return
    -1 - bad pointer,   \n
    -2 - sleep timer is busy, \n
    -3 - sleep system has been started.
     0 - success.
******************************************************************************/
int HAL_StartSystemSleep(HAL_Sleep_t *sleepParam)
{
  HAL_SleepTimer_t sleepTimer;
  int sleepTimerStatus;

  if (!sleepParam)
    return HAL_NULL_POINTER;

  halSleepControl.callback = sleepParam->callback;
  halSleepControl.startPoll = sleepParam->startPoll;
  sleepTimer.interval = sleepParam->sleepTime;
  sleepTimer.mode = TIMER_ONE_SHOT_MODE;
  sleepTimer.callback = NULL;

  sleepTimerStatus = HAL_StartSleepTimer(&sleepTimer);
  if ((HAL_NULL_POINTER == sleepTimerStatus) || (HAL_SLEEP_TIMER_HAS_ALREADY_STARTED == sleepTimerStatus))
    return HAL_SLEEP_TIMER_IS_BUSY;

  if (-1 == HAL_Sleep())
    return HAL_SLEEP_SYSTEM_HAS_ALREADY_STARTED;

  return 0;
}

// eof sleep.c
