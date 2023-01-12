/*******************************************************************************
  HAL Sleep Source File

  Company:
    Microchip Technology Inc.

  File Name:
    halSleep.c

  Summary:
    This file contains the Implementation of HAL sleep modes.

  Description:
    This file contains the Implementation of HAL sleep modes.
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
#include "device.h"
#include <hal/cortexm4/pic32cx/include/halRfCtrl.h>
#include <hal/cortexm4/pic32cx/include/halAppClock.h>
#include <hal/cortexm4/pic32cx/include/halSleep.h>
#include <hal/cortexm4/pic32cx/include/halSleepTimerClock.h>
#include <systemenvironment/include/sysEvents.h>
#include <systemenvironment/include/sysIdleHandler.h>
#include <mac_phy/mac_hwd_phy/RF231_RF212/PHY/include/at86rf233.h>
#include <mac_phy/mac_hwd_phy/RF231_RF212/PHY/include/phyDeepSleep.h>
#ifdef _PIC32CX_
#include <mac_phy/mac_hwd_phy/RF231_RF212/PHY/include/phyPic32cx.h>
#else
#include <mac_phy/mac_hwd_phy/RF231_RF212/PHY/include/phyRfSpiProto.h>
#endif

#define Sleep_Check(x)
#define XTAL_TURN_OFF 1
uint32_t g_refoReg[6];

#ifndef _PIC32CX_
/******************************************************************************
                   Define(s) section
******************************************************************************/

/******************************************************************************
                   Prototypes section
******************************************************************************/
void halSwitchToActiveClock(void);


/**************************************************************************//**
\brief  Turn on pin 9 (DIG1) and pin 10 (DIG2) to enable antenna select.
******************************************************************************/
void phyAntennaSwitcherOn(void);

/**************************************************************************//**
\brief  Turn off pin 9 (DIG1) and pin 10 (DIG2) to disable antenna select.
******************************************************************************/
void phyAntennaSwitcherOff(void);

/******************************************************************************
                   External global variables section
******************************************************************************/
extern HAL_SleepControl_t halSleepControl;

/******************************************************************************
                   Global variables section
******************************************************************************/
static uint64_t halTimeStartOfSleep = 0ull;
static bool mcuSleep = false;
static bool trxSleep = false;


/******************************************************************************
                   Implementations section
******************************************************************************/
/**************************************************************************//**
\brief Switch on system power.

\param[in]
  wakeupSource - wake up source
******************************************************************************/
void halPowerOn(const uint8_t wakeupSource)
{
  halSleepControl.wakeupStation = HAL_ACTIVE_MODE;
  halSleepControl.wakeupSource = wakeupSource;

  if (trxSleep)
  {
    HAL_ClearRfSlpTr();
  }

  //halPostTask(HAL_WAKEUP);
}

/*******************************************************************************
 Put the cpu in sleep state
*******************************************************************************/
void sleep(void)
{
  __DSB();
  __WFI();
}

/*******************************************************************************
 set/prepare sleep mode
*******************************************************************************/
void halSetSleepMode(HALSleepModes_t sleepMode)
{
  switch(sleepMode)
  {
    case SLEEPMODE_IDLE_0:
    case SLEEPMODE_IDLE_1:
    case SLEEPMODE_IDLE_2:
    {
      // todo SCB_SCR_s.sleepdeep = 0;//make sure it is not standby
      // todo PM_SLEEP_s.idle = (uint8_t)sleepMode;
    }
    break;
    case SLEEPMODE_STANDBY:
      {
        // todo SCB_SCR_s.sleepdeep = 1; //enable standby
      }
     break;
  }
}

/*******************************************************************************
 \brief prepare for sleep
*******************************************************************************/
void halPrepareForSleep(void)
{
  /* Disable USB, enabled by default */
  // todo PM_APBBMASK_s.usb = 0;
  // todo PM_AHBMASK_s.usb = 0;

  /* Disable BOD33 */
  // todo SYSCTRL_BOD33_s.enable = 0;
}

/*******************************************************************************
 \brief Restoring the state atter wakeup
*******************************************************************************/
void halRestoreAfterWakeup(void)
{
  /* enable USB, a sit is enabled by default */
  // todo PM_APBBMASK_s.usb = 1;
  // todo PM_AHBMASK_s.usb = 1;

  /* Enable BOD33 */
  // todo SYSCTRL_BOD33_s.enable = 1;
}

/******************************************************************************
 \brief Shutdowns system.
  NOTES:
  the application should be sure the poweroff will not be
  interrupted after the execution of the sleep().
******************************************************************************/
void halPowerOff(void)
{
  HAL_SleepMode_t sleepMode = HAL_SLEEP_MODE_POWER_DOWN;

  if (HAL_ACTIVE_MODE == halSleepControl.wakeupStation)
    return;  // it is a too late to sleep.

  // stop application timer clock
  halStopAppClock(); // will be shutdown
  if (0ul == halTimeStartOfSleep)
  { // start of sleep procedure
    // save time of stopping of the application timer
    halTimeStartOfSleep = halGetTimeOfSleepTimer();
  }

  if (HAL_SLEEP_TIMER_IS_STARTED == halSleepControl.sleepTimerState)
    sleepMode = HAL_SLEEP_MODE_POWER_SAVE;

  if (HAL_SLEEP_STATE_CONTINUE_SLEEP != halSleepControl.sleepState)
  {
    HAL_DisableRxTxSwitcher();

    SYS_PostEvent((SYS_EventId_t)HAL_EVENT_FALLING_ASLEEP, (SYS_EventData_t)&sleepMode);
  }
  else
    SYS_PostEvent((SYS_EventId_t)HAL_EVENT_CONTINUING_SLEEP, (SYS_EventData_t)&sleepMode);

#ifdef CLKM
  SYS_PostEvent((SYS_EventId_t)BC_EVENT_PHY_INIT, INITIALIZING_SLEEP_STATE);
#endif

  if (halRadioInTrxOff())
  {
    if (BSP_IsAntDiversityActionsRequired())
      // disable antenna diversity switcher
      phyAntennaSwitcherOff();
    /* Put the transceiver in deep sleep if trx state is TRX_OFF */
    HAL_SetRfSlpTr();
    trxSleep = true;
  }

  halSleepControl.sleepState = HAL_SLEEP_STATE_CONTINUE_SLEEP;
#if defined(_SLEEP_WHEN_IDLE_)
  if (SYS_IsSleepWhenIdleEnabled())
  {
    halPrepareForSleep();
    __enable_irq();
    mcuSleep = true;

    if (SYS_taskFlag || (!trxSleep))
    {
		// During device power off procedure some source has posted a task
		// so the power on procedure shall be called
		if (HAL_ACTIVE_MODE != halSleepControl.wakeupStation)
			halPowerOn(HAL_SLEEP_TIMER_IS_WAKEUP_SOURCE);
		// post task for task manager
		if (HAL_SLEEP_TIMER_IS_STARTED == halSleepControl.sleepTimerState)
		{
			halInterruptSleepClock();
		}
	
	return;
    }

    halSleepControl.sleepMode = sleepMode;
    if (HAL_SLEEP_MODE_IDLE == sleepMode)
    {
      halSetSleepMode(SLEEPMODE_IDLE_0);
      sleep();
    }
    else if (HAL_SLEEP_MODE_POWER_SAVE == sleepMode)
    { // sleep timer is started
      halSetSleepMode(SLEEPMODE_STANDBY);
      sleep();
    }
    else
    {
      halStopSleepTimerClock();//sleep timer is also stopped
      halSetSleepMode(SLEEPMODE_STANDBY);
      sleep();
      halStartSleepTimerClock(); 
    }
  }
#endif /* _SLEEP_WHEN_IDLE_ */
}

/********************** ********************************************************
  Prepares system for power-save, power-down.
  Power-down the mode is possible only when internal RC is used
  Parameters:
  none.
  Returns:
  0 if there is a possibililty to power-down system.
******************************************************************************/
int HAL_Sleep(void)
{
  halSleepControl.wakeupStation = HAL_SLEEP_MODE;  // the reset of sign of entry to the sleep mode.  
  //halPostTask(HAL_SLEEP);
  return 0;
}

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
 Handler for task manager. It is executed when system has waked up.
******************************************************************************/
void halWakeupHandler(void)
{
  uint64_t timeEndOfSleep;
  HAL_SleepControl_t wakeupSource;

  if (mcuSleep)
  {
#ifndef _PIC32CX_
    halSwitchToActiveClock();
#endif
    halRestoreAfterWakeup();

    timeEndOfSleep = halGetTimeOfSleepTimer();

    timeEndOfSleep -= halTimeStartOfSleep;  // time of sleep
    halTimeStartOfSleep = 0ul;
    // adjust application timer interval
    halAdjustSleepInterval((uint32_t)timeEndOfSleep);

    // start application timer clock
    halStartAppClock();
	mcuSleep = false;
  }

  if (trxSleep)
  {
    // wakeup the transceiver by setting SLPTR to '0'
    HAL_ClearRfSlpTr();

    // Wait for radio to wake up.
    halWaitRadio();
    trxSleep = false;
    if (BSP_IsAntDiversityActionsRequired())
      // enable antenna diversity switcher
      phyAntennaSwitcherOn();
  }

#ifdef CLKM
  SYS_PostEvent((SYS_EventId_t)BC_EVENT_PHY_INIT, INITIALIZING_WAKE_UP_STATE);
#endif

  halSleepControl.sleepState = HAL_SLEEP_STATE_BEGIN;

  memcpy(&wakeupSource, &halSleepControl, sizeof(HAL_SleepControl_t));

  SYS_PostEvent((SYS_EventId_t)HAL_EVENT_WAKING_UP, (SYS_EventData_t)&wakeupSource);

  HAL_EnableRxTxSwitcher();
  halSleepControl.sleepMode = HAL_SLEEP_MODE_NONE;
  if (HAL_SLEEP_TIMER_IS_WAKEUP_SOURCE == halSleepControl.wakeupSource)
  {
    if (halSleepControl.callback)
      halSleepControl.callback(halSleepControl.startPoll);
  }
}

/*******************************************************************************
  Makes MCU enter Sleep mode.
*******************************************************************************/
void HAL_IdleMode(void)
{

}
/*******************************************************************************
  Shuts down the following Peripherials to save power
*******************************************************************************/
void HAL_DisablePeripherials(void)
{

}
/*******************************************************************************
  Enables USART1
*******************************************************************************/
void HAL_EnableUSART1(void)
{
  // todo PM_APBCMASK_s.sercom0 = 1;
}
/*******************************************************************************
  Disables Pull up
*******************************************************************************/
void HAL_DisablePUD(void)
{
}
/*******************************************************************************
  Enables Pull up
*******************************************************************************/
void HAL_EnablePUD(void)
{   
}
/*******************************************************************************
  Enables Timer/counter 2
*******************************************************************************/
void HAL_EnableSleepTimer(void)
{
}
/*******************************************************************************
  Enables Transceiver clock
*******************************************************************************/
void HAL_EnableTransceiver(void)
{
  // todo PM_APBCMASK_s.rfctrl = 1;
}

#endif //#infdef _PIC32CX_

/*******************************************************************************
  To Enter into MCU Sleep 
*******************************************************************************/
void HAL_Pic32cxSleep()
{
    
}

/*******************************************************************************
  TO Exit from MCU Sleep
*******************************************************************************/
void HAL_Pic32cxWakeup()
{
  
}

/*******************************************************************************
  To Execute Sleep
*******************************************************************************/
void HAL_ExecuteSleep()
{

}

/*******************************************************************************
  Enables Transceiver clock
*******************************************************************************/
void HAL_EnableTransceiver(void)
{
  // todo
}
// eof sleep.c
