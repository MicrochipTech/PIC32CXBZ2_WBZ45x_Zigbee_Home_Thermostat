/*******************************************************************************
  HAL Sleep  Header File

  Company:
    Microchip Technology Inc.

  File Name:
    halSleep.h

  Summary:
    This file contains the Interface to control sleep mode.

  Description:
    This file contains the Interface to control sleep mode.
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

#ifndef _HALSLEEP_H
#define _HALSLEEP_H

/******************************************************************************
                   Includes section
******************************************************************************/
#include <hal/include/sleep.h>
#include <hal/include/sleepTimer.h>
#include <hal/cortexm4/pic32cx/include/halRfCtrl.h>

/******************************************************************************
                   Defines section
******************************************************************************/
#define HAL_ACTIVE_MODE                    0
#define HAL_SLEEP_MODE                     1
#define HAL_SLEEP_TIMER_IS_STOPPED         0
#define HAL_SLEEP_TIMER_IS_STARTED         1
#define HAL_SLEEP_TIMER_IS_WAKEUP_SOURCE   0
#define HAL_EXT_IRQ_IS_WAKEUP_SOURCE       1
#define HAL_SLEEP_STATE_BEGIN              0
#define HAL_SLEEP_STATE_CONTINUE_SLEEP     1

/******************************************************************************
                   Types section
******************************************************************************/
typedef struct _HalSleepControl_t
{
  HAL_WakeUpCallback_t callback;
  HAL_SleepTimer_t sleepTimer;
  HAL_SleepMode_t sleepMode;
  uint8_t wakeupStation    : 1;
  uint8_t wakeupSource     : 1;
  uint8_t sleepTimerState  : 1;
  uint8_t sleepState       : 1;
  uint8_t startPoll        : 1;
} HAL_SleepControl_t;

typedef enum 
{
  SLEEPMODE_IDLE_0,
  SLEEPMODE_IDLE_1,
  SLEEPMODE_IDLE_2,
  SLEEPMODE_STANDBY
} HALSleepModes_t;
    
/******************************************************************************
                   Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Switch on system power.

\param[in]
  wakeupSource - wake up source
******************************************************************************/
void halPowerOn(const uint8_t wakeupSource);

/*******************************************************************************
  Shuts down the following Peripherials to save power
*******************************************************************************/
void HAL_DisablePeripherials(void);
/*******************************************************************************
  Enables Timer/counter 2
*******************************************************************************/
void HAL_EnableSleepTimer(void);
/*******************************************************************************
  Enables USART1
*******************************************************************************/
void HAL_EnableUSART1(void);
/*******************************************************************************
  Enables Pull up
*******************************************************************************/
void HAL_EnablePUD(void);
/*******************************************************************************
  Disables Pull up
*******************************************************************************/
void HAL_DisablePUD(void);
/*******************************************************************************
  Enables Transceiver
*******************************************************************************/
void HAL_EnableTransceiver(void);
/***************************************************************************//**
\brief Shutdown system.
  NOTES:
  the application should be sure the poweroff will not be
  interrupted after the execution of the sleep().
*******************************************************************************/
void halPowerOff(void);
/*******************************************************************************
  To Enter into MCU Sleep 
*******************************************************************************/
void HAL_Pic32cxSleep();
/*******************************************************************************
  TO Exit from MCU Sleep
*******************************************************************************/
void HAL_Pic32cxWakeup();
/*******************************************************************************
  To Execute Sleep
*******************************************************************************/
void HAL_ExecuteSleep();
#endif /* _HALSLEEP_H */
// eof halSleep.h
