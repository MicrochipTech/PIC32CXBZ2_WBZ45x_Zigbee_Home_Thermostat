/*******************************************************************************
  HAL Sleep Timer Clock File

  Company:
    Microchip Technology Inc.

  File Name:
    halSleepTimer.c

  Summary:
    This file contains the Module for count out requested sleep interval.

  Description:
    This file contains the Application advertising functions for this projectModule for count out requested sleep interval.
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
#include <hal/include/sleepTimer.h>
#include <hal/cortexm4/pic32cx/include/halSleepTimerClock.h>
#include <hal/cortexm4/pic32cx/include/halSleep.h>
#include <hal/cortexm4/pic32cx/include/halDbg.h>
#include <hal/cortexm4/pic32cx/include/halDiagnostic.h>
#if defined(_STATS_ENABLED_)
#include <statStack.h>
#endif
#include <configserver/include/configserver.h>
#include <systemenvironment/include/sysAssert.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/

#define SLEEP_TIMER_ITERATOR  (1000ul * 256ul * SLEEPTIMER_DIVIDER / SLEEPTIMER_CLOCK)
#define NVMCTRL_OSC32K_FUSE_CALIB_ADDR  (NVMCTRL_OTP4 + 4)
#define NVMCTRL_OSC32K_FUSE_CALIB_MASK  (0x1fc0)
#define NVMCTRL_OSC32K_FUSE_CALIB_Pos   (6)

/******************************************************************************
                   Types section
******************************************************************************/
#ifndef _PIC32CX_

/******************************************************************************
                   Prototypes section
******************************************************************************/
void rtcHandler(void);
static void halRtcWaitForSync(uint32_t);

/******************************************************************************
                   External global variables section
******************************************************************************/
extern HAL_SleepControl_t halSleepControl;

/******************************************************************************
                   Global variables section
******************************************************************************/
volatile uint32_t halSleepRemainder;
 //upper byte of sleep time
uint8_t halSleepTimerOvfw = 0;
 //interrupt counter
static volatile uint8_t halIrqOvfwCount = 0;
// time of sleep timer in ms.
static uint32_t halSleepTime = 0ul;




/******************************************************************************
                   Implementations section
******************************************************************************/
void halStartSleepTimerClock(void)
{
#if defined(RC_32K)
  // Configure Internal Osc32K as input source
  SYSCTRL_OSC32K_s.runstdby = 1;
  SYSCTRL_OSC32K_s.ondemand = 0;
  SYSCTRL_OSC32K_s.en32k = 1;
  SYSCTRL_OSC32K_s.enable = 1;
  SYSCTRL_OSC32K_s.calib = (((*(uint32_t *)NVMCTRL_OSC32K_FUSE_CALIB_ADDR) \
                             & NVMCTRL_OSC32K_FUSE_CALIB_MASK) >> NVMCTRL_OSC32K_FUSE_CALIB_Pos);

  while(SYSCTRL_PCLKSR_s.osc32krdy == 0);

  GCLK_GENDIV = GCLK_GENDIV_ID(GCLK_GEN_2) | GCLK_GENDIV_DIV(0);
  halGclkSync();

  GCLK_GENCTRL = GCLK_GENCTRL_ID(GCLK_GEN_2) | GCLK_GENCTRL_SRC(OSC32K) | GCLK_GENCTRL_GENEN;
  halGclkSync();

#elif defined (CRYSTAL_32K)
#if defined(HAL_4MHz) || defined(HAL_8MHz) || defined(CLKM)
#ifndef EXOSC32K
/*
The EXOSC32K is defined in halClkCtrl.c
We call halXOSC32KInit() here because as of now to generate 4MHz and 8MHz
we use the internal OSC8M and we don't initialize the EXOSC32K
*/
  halXOSC32KInit();
#endif
#endif

  GCLK_GENDIV = GCLK_GENDIV_ID((uint32_t)GCLK_GEN_2) | GCLK_GENDIV_DIV(0);
  halGclkSync();

  GCLK_GENCTRL = GCLK_GENCTRL_ID((uint32_t)GCLK_GEN_2) | GCLK_GENCTRL_SRC((uint32_t)EXOSC32K) | GCLK_GENCTRL_GENEN;
  halGclkSync();

#endif

 /* Enable RTC APB CLK in the power manager */ 

  MCLK_APBAMASK |= MCLK_APBAMASK_RTC;



  RTC_MODE0_CTRLA = RTC_MODE1_CTRLA_SWRST;
  halRtcWaitForSync(RTC_MODE0_SYNCBUSY_SWRST);


  RTC_MODE0_CTRLA = RTC_MODE0_CTRLA_PRESCALER(SLEEPTIMER_PRESCALER) | RTC_MODE0_CTRLA_ENABLE;
  halRtcWaitForSync(RTC_MODE0_SYNCBUSY_ENABLE);


}

void halStopSleepTimerClock(void)
{
  RTC_MODE0_CTRLA &=~RTC_MODE0_CTRLA_ENABLE;
}

/******************************************************************************
Sets interval.
Parameters:
  value - contains number of ticks which the timer must count out.
Returns:
  none.
******************************************************************************/
void halSetSleepTimerInterval(uint32_t value)
{
  uint32_t currCounter = 0;
  uint32_t remainingTimeBeforeOvf;

  /* Disable and clear the interrupt flags */
  RTC_M0_INTENCLR = RTC_MODE0_INTENCLR_OVF;
  RTC_M0_INTENCLR = RTC_MODE0_INTENCLR_CMP0;
  RTC_M0_INTFLAG = RTC_MODE0_INTFLAG_CMP0;

  currCounter = RTC_M0_COUNT;
  
  remainingTimeBeforeOvf = ~currCounter;

  if (remainingTimeBeforeOvf > value)
  {
    value += currCounter;
    RTC_M0_COMP = RTC_MODE0_COMP_COMP(value);
    halRtcWaitForSync(RTC_MODE0_SYNCBUSY_COMP0);

    /* Enable compare match interrupt */
    RTC_M0_INTENSET = RTC_MODE0_INTENSET_CMP0;
    halSleepRemainder = 0;
  }
  else // overflow is going to be occured
  {
    value -= remainingTimeBeforeOvf;
    halSleepRemainder = value;
    /* Enable the ovf interrupt */ 
    RTC_M0_INTENSET = RTC_MODE0_INTENSET_OVF;
   }

  //TODO: Enable RTC IRQ in vector table
  //(void) HAL_InstallInterruptVector((int32_t)RTC_IRQn, rtcHandler);

  /* Clear & disable RTC interrupt on NVIC */
  NVIC_DisableIRQ(RTC_IRQn);
  NVIC_ClearPendingIRQ(RTC_IRQn); 

  /* set priority & enable RTC interrupt on NVIC */
  NVIC_EnableIRQ(RTC_IRQn);

}

/******************************************************************
\brief wait for RTC module to sync
********************************************************************/
static void halRtcWaitForSync(uint32_t bit)
{
  while (RTC_MODE0_SYNCBUSY & bit);
}

/******************************************************************
\brief RTC interrupt handler
********************************************************************/
void rtcHandler(void)
{

  uint16_t intFlags = 0U;
#if defined(_STATS_ENABLED_)
  uint16_t stack_threshold;
  uint16_t stack_left = Stat_GetCurrentStackLeft();
  CS_ReadParameter(CS_STACK_LEFT_THRESHOLD_ID,&stack_threshold);
  SYS_E_ASSERT_FATAL((stack_left>stack_threshold),RTC_ISR_STACK_OVERFLOW);
#endif //(_STATS_ENABLED_)

  intFlags = RTC_M0_INTFLAG & RTC_M0_INTENSET;

  if (intFlags & RTC_MODE0_INTFLAG_OVF) //ovf occured
  {
    RTC_M0_INTFLAG = RTC_MODE0_INTFLAG_OVF;
    if (0 == halSleepRemainder)
    {
      if (HAL_SLEEP_MODE == halSleepControl.wakeupStation)
        halPowerOn(HAL_SLEEP_TIMER_IS_WAKEUP_SOURCE);
      // post task for task manager
      if (HAL_SLEEP_TIMER_IS_STARTED == halSleepControl.sleepTimerState)
        halInterruptSleepClock();
    }
    else
    {
      // load compared value
      RTC_M0_COMP = RTC_MODE0_COMP_COMP(halSleepRemainder);
      halRtcWaitForSync(RTC_MODE0_SYNCBUSY_COMP0);
      
      // clear compare interrupt flag
      RTC_M0_INTENCLR = RTC_MODE0_INTENCLR_CMP0;
      // enable compare match interrupt
      RTC_M0_INTENSET = RTC_MODE0_INTENSET_CMP0;
      if (HAL_SLEEP_MODE == halSleepControl.wakeupStation)
         (void) HAL_Sleep();
    }
    halIrqOvfwCount++;
    halSynchronizeSleepTime();
  }

  if (intFlags & RTC_MODE0_INTFLAG_CMP0) //comp match int ocuured
  {
    RTC_M0_INTFLAG = RTC_MODE0_INTFLAG_CMP0;
    /* Disable the interrupt flags */
    RTC_M0_INTENCLR = RTC_MODE0_INTENCLR_CMP0;
    // nulling for adjusting
    halSleepRemainder = 0;
    
    if (HAL_SLEEP_MODE == halSleepControl.wakeupStation)
      halPowerOn(HAL_SLEEP_TIMER_IS_WAKEUP_SOURCE);
    // post task for task manager
    if (HAL_SLEEP_TIMER_IS_STARTED == halSleepControl.sleepTimerState)
      halInterruptSleepClock();
  }

}

/**************************************************************************//**
\brief Wake up procedure for all external interrupts
******************************************************************************/
void halWakeupFromIrq(void)
{
  if (HAL_SLEEP_MODE == halSleepControl.wakeupStation)
  {
    halPowerOn(HAL_EXT_IRQ_IS_WAKEUP_SOURCE);

    /* clear the remainder value */
    halSleepRemainder = 0;

    /* Disable the ovf interrupt */
    RTC_M0_INTENCLR = RTC_MODE0_INTENCLR_CMP0;

    // stop high sleep timer logic
    halSleepControl.sleepTimerState = HAL_SLEEP_TIMER_IS_STOPPED;
  }

}

/******************************************************************************
Return time of sleep timer.

Returns:
  time in ms.
******************************************************************************/
uint64_t halGetTimeOfSleepTimer(void)
{

  uint32_t currCounter = 0;
  uint32_t tmpCounter = 0;
  uint64_t  countValue = 0;

  ATOMIC_SECTION_ENTER
  // read interrupt counter
  tmpCounter = halIrqOvfwCount;
  ATOMIC_SECTION_LEAVE

  countValue = (uint64_t)tmpCounter << 32;

  // read asynchronous counter 
  halRtcWaitForSync(RTC_MODE0_SYNCBUSY_COMP0);
  currCounter = RTC_M0_COUNT;
  countValue += currCounter;

  return (halSleepTime + countValue);
  
}

/******************************************************************************
Returns the sleep timer frequency in Hz.
Parameters:
  none.
Returns:
  frequency.
******************************************************************************/
uint32_t halSleepTimerFrequency(void)
{
  return (SLEEPTIMER_CLOCK / SLEEPTIMER_DIVIDER);
}

/**************************************************************************//**
Synchronization system time which based on sleep timer.
******************************************************************************/
void halSleepSystemTimeSynchronize(void)
{
  uint8_t tmpCounter = 0;
  uint32_t tmpValue = 0;

  ATOMIC_SECTION_ENTER
  tmpCounter = halIrqOvfwCount;
  halIrqOvfwCount = 0;  
  ATOMIC_SECTION_LEAVE

  tmpValue = tmpCounter * SLEEP_TIMER_ITERATOR;
  halSleepTime += tmpValue;
  if (halSleepTime < tmpValue)
    halSleepTimerOvfw++;
}
#else
void halStartSleepTimerClock(void)
{
}

void halSleepSystemTimeSynchronize(void)
{
}

uint64_t halGetTimeOfSleepTimer(void)
{
  return 1;

}

void halStopSleepTimerClock(void)
{

}
void rtcHandler(void)
{
}

void halWakeupFromIrq(void)
{
}


#endif

//eof halSleepTimerClock.c

