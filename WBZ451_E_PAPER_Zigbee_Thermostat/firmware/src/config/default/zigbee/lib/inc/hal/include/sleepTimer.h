/*******************************************************************************
  HAL sleepTimer Header File

  Company:
    Microchip Technology Inc.

  File Name:
    sleepTimer.h

  Summary:
    This file describes the sleepTimer interface.

  Description:
    This file describes the sleepTimer interface.
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

#ifndef _SLEEPTIMER_H
#define _SLEEPTIMER_H

// \cond
/******************************************************************************
                   Includes section
******************************************************************************/
#include <hal/include/bcTimer.h>
// \endcond

/******************************************************************************
                   Types section
******************************************************************************/
/** \brief fields of structure \n
    \brief uint32_t interval - timer firing interval (set by user) \n
    \brief TimerMode_t mode - timer work mode (set by user). Must be chosen from: \n
           TIMER_REPEAT_MODE \n
           TIMER_ONE_SHOT_MODE \n
    \brief void (*callback)() - pointer to the timer callback function (set by user);
                                can be set to NULL \n */
typedef BC_Timer_t HAL_SleepTimer_t;

/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Starts sleep timer. Interval must be greater one time of sleep timer tick.
\param[in]
    sleepTimer - pointer to sleep timer structure.
\return
  -1 - NULL pointer, \n
  -2 - interval can not be counted out, \n
  -3 - sleep timer has already started, \n
   0 - otherwise.
******************************************************************************/
int HAL_StartSleepTimer(HAL_SleepTimer_t *sleepTimer);

/**************************************************************************//**
\brief Removes timer.
\param[in]
  sleepTimer - address of the timer to be removed from the list
\return
  -1 - there is no active sleep timer, \n
   0 - otherwise.
******************************************************************************/
int HAL_StopSleepTimer(HAL_SleepTimer_t *sleepTimer);

#endif /* _SLEEPTIMER_H */
// eof sleepTimer.h
