/*******************************************************************************
  HAL Application Clock Source File

  Company:
    Microchip Technology Inc.

  File Name:
    halAppClock.c

  Summary:
    This file contains the Implementation of appTimer hardware-dependent module.

  Description:
    This file contains the Implementation of appTimer hardware-dependent module.
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
#include <hal/cortexm4/pic32cx/include/halAppClock.h>
#include <hal/cortexm4/pic32cx/include/halMacIsr.h>

#include <systemenvironment/include/sysEvents.h>
#include <hal/cortexm4/pic32cx/include/halSleepTimerClock.h>
#if defined(_STATS_ENABLED_)
#include <statStack.h>
#endif
#include <configserver/include/configserver.h>
#include <systemenvironment/include/sysAssert.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/


/******************************************************************************
                   Prototypes section
******************************************************************************/
/******************************************************************************
  \brief Returns the time left value for the smallest app timer.
  \return time left for the smallest application timer.
******************************************************************************/
uint32_t halGetTimeToNextAppTimer(void);
static void smallestTimerRequestHandler(SYS_EventId_t eventId, SYS_EventData_t data);

/******************************************************************************
                     Global variables section
******************************************************************************/
static uint32_t halAppTime = 0ul;     // time of application timer
uint8_t halAppTimeOvfw = 0;
static volatile uint32_t halAppIrqCount = 0;

static SYS_EventReceiver_t appTimerEventListener = {.func = smallestTimerRequestHandler};



/******************************************************************************
                   Implementations section
******************************************************************************/
  /******************************************************************************
   Polling the Sync. flag for Software Reset 
   Parameters:
     none
   Returns:
     none
   *****************************************************************************/
INLINE void halTimerSWRSTSync(void)
{
  while (TCC2_SYNCBUSY & TCC_SYNCBUSY_SWRST_Msk);
}

/******************************************************************************
 Polling the Sync. flag for Enable 
 Parameters:
   none
 Returns:
   none
 *****************************************************************************/
INLINE void halTimerEnableSync(void)
{
  while (TCC2_SYNCBUSY & TCC_SYNCBUSY_ENABLE_Msk);
}

/******************************************************************************
 Polling the Sync. flag for Clock Domain
 Parameters:
   none
 Returns:
   none
 *****************************************************************************/
INLINE void halTimerClockSync(void)
{
  while (TCC2_SYNCBUSY & TCC_SYNCBUSY_STATUS_Msk);
}

/******************************************************************************
 Polling the Sync. flag for Count register 
 Parameters:
   none
 Returns:
   none
 *****************************************************************************/
INLINE void halTimerCountSync(void)
{
  while (TCC2_SYNCBUSY & TCC_SYNCBUSY_COUNT_Msk );
}

/******************************************************************************
 Polling the Sync. flag for CC register 
 Parameters:
   none
 Returns:
   none
 *****************************************************************************/
INLINE void halTimerCCSync(void)
{
  while (TCC2_SYNCBUSY & TCC_SYNCBUSY_CC0_Msk);
}
/******************************************************************************
 Polling the Sync. flag for CC1 register 
 Parameters:
   none
 Returns:
   none
 *****************************************************************************/
INLINE void halTimerCC1Sync(void)
{
  while (TCC2_SYNCBUSY & TCC_SYNCBUSY_CC1_Msk);
}
/******************************************************************************
 Polling the Sync. flag for CTRLB register 
 Parameters:
   none
 Returns:
   none
 *****************************************************************************/
INLINE void halTimerCTRLBSync(void)
{
  while (TCC2_SYNCBUSY & TCC_SYNCBUSY_CTRLB_Msk);
}

/******************************************************************************
 Interrupt handler signal implementation
 Parameters:
   none
 Returns:
   none
 *****************************************************************************/
INLINE void halInterruptAppClock(void)
{
  halAppIrqCount++;
  halPostTaskFromISR(HAL_APPTIMER);
  // infinity loop spy
  SYS_InfinityLoopMonitoring();
}

/******************************************************************************
 To get app clock elapsed time since last ISR
 Parameters:
   none
 Returns:
   elapsed time in us
 *****************************************************************************/
uint32_t halGetElapsedTimeFromAppClock(void)
{
  TCC2_CTRLBSET |= TCC_CTRLBCLR_CMD_READSYNC;
  halTimerCTRLBSync();
   
  uint16_t tcCount = TCC2_16COUNT;   
  
  return (tcCount * AMOUNT_NSEC_FOR_TIMER_CLOCK)/1000;

}

/**************************************************************************//**
\brief Takes account of the sleep interval.

\param[in]
  interval - time of sleep
******************************************************************************/
void halAdjustSleepInterval(uint32_t interval)
{
  halAppTime += interval;
  halPostTask(HAL_APPTIMER);
}

/**************************************************************************//**
Synchronization system time which based on application timer.
******************************************************************************/
void halAppSystemTimeSynchronize(void)
{
  uint32_t tmpCounter;
  uint32_t tmpValue = 0;

  ATOMIC_SECTION_ENTER
    tmpCounter = halAppIrqCount;
    halAppIrqCount = 0;
  ATOMIC_SECTION_LEAVE

  tmpValue = tmpCounter * HAL_APPTIMERINTERVAL;
  halAppTime += tmpValue;
  if (halAppTime < tmpValue)
    halAppTimeOvfw++;
}

// Only For testing
//#define PORTA_BASE 0x41008000
//#define PORTB_BASE 0x41008000 + 0x80
//#define PORTC_BASE PORTB_BASE + 0x80
//
//#define PORTA_OUTTGL	MMIO_REG(PORTA_BASE + 0x0C, uint32_t)
//#define PORTC_OUTTGL	MMIO_REG(PORTC_BASE + 0x0C, uint32_t)
/******************************************************************
time counter interrupt handler
 Parameters:
   none
 Returns:
   none
******************************************************************/
void TCC2_Handler(void)
{
#if defined(_STATS_ENABLED_)
  uint16_t stack_threshold;
  uint16_t stack_left = Stat_GetCurrentStackLeft();
  CS_ReadParameter(CS_STACK_LEFT_THRESHOLD_ID,&stack_threshold);
  SYS_E_ASSERT_FATAL((stack_left>stack_threshold),TIMER_ISR_STACK_OVERFLOW);
#endif //(_STATS_ENABLED_)

 TCC2_CTRLBSET |= TCC_CTRLBCLR_CMD_READSYNC;
 halTimerCTRLBSync();
 uint16_t tcCount = TCC2_16COUNT; 
 uint8_t intFlag= TCC2_INTFLAG;

  if ((TCC2_INTFLAG & TCC_INTFLAG_MC0(1)) && (TCC2_INTENSET & TCC_INTENSET_MC0(1)))
  {
    // appTimer handling
   TCC2_INTFLAG = TCC_INTFLAG_MC0(1) ;//| TCC_INTFLAG_MC1(1) ; Clearing done in CC1 Handler
   TCC2_16COUNT = 0;
   halTimerCountSync();
   halInterruptAppClock();
  }

  if ((TCC2_INTFLAG & TCC_INTFLAG_MC1(1)) && (!(TCC2_INTENSET & TCC_INTENSET_MC1(1))))
      TCC2_INTFLAG = TCC_INTFLAG_MC1(1); 
  if ((TCC2_INTFLAG & TCC_INTFLAG_MC1(1)) && (TCC2_INTENSET & TCC_INTENSET_MC1(1)))
  {
   TCC2_INTFLAG = TCC_INTFLAG_MC1(1);//rTimerHandling
   TCC2_CC1 = TCC_CC_CC(0);
   halTimerCC1Sync();
   TCC2_halMacTimerHandler();
  }
}

/******************************************************************
configure and enable timer counter channel 0
 Parameters:
   none
 Returns:
   none
******************************************************************/
void halStartAppClock(void)
{
  //MCLK_APBBMASK |= MCLK_APBBMASK_TC3;
  
  /*Configuring Peripheral - 26 TCC2, TC3 clock for GEN3 as source*/
  //GCLK_PCHCTRL(TCC2_GCLK_ID) = GCLK_PCHCTRL_GEN(GCLK_GEN_3) | GCLK_PCHCTRL_CHEN;

  TCC2_CTRLA = 0;
  halTimerClockSync ();
  
  TCC2_CTRLA = TCC_CTRLA_SWRST_Msk;
  halTimerSWRSTSync();

  /* Prescaler DIV2 8Mhz / 8 = 1 Mhz
     Reload or reset the counter on next prescaler clock
    */

  uint32_t mode = TCC_CTRLA_PRESCALER_DIV16  //    TCC_CTRLA_PRESCALER_DIV256
              | TCC_CTRLA_PRESCSYNC_PRESC | TCC_CTRLA_RUNSTDBY(1);
  TCC2_CTRLA =  mode;  //    TCC_CTRLA_PRESCALER_DIV256
  halTimerClockSync();

  //TCC2_WAVE = TCC_WAVE_WAVEGEN_MFRQ;

  /*The TC will wrap around and run on the next underflow/overflow condition, counter count up*/
  TCC2_CTRLBCLR = TCC_CTRLBCLR_ONESHOT(1) | TCC_CTRLBCLR_DIR(1);
  halTimerCTRLBSync();

  //TCC2_DBGCTRL = TCC_DBGCTRL_DBGRUN ;
  

  TCC2_16COUNT = 0;
  halTimerCountSync();

  // ((F_PERI/1000ul) / TIMER_FREQUENCY_PRESCALER(DIV8)) * HAL_APPTIMERINTERVAL (10 ms)
  TCC2_CC0 = TCC_CC_CC( TOP_TIMER_COUNTER_VALUE);
  halTimerCCSync();

  NVIC_DisableIRQ(TCC2_IRQn);
  NVIC_ClearPendingIRQ(TCC2_IRQn);
  NVIC_SetPriority(TCC2_IRQn, 7);
  
  TCC2_INTENSET = TCC_INTENSET_MC0(1);

  NVIC_EnableIRQ(TCC2_IRQn);

  TCC2_CTRLA |= TCC_CTRLA_ENABLE(1);
  halTimerEnableSync();
  
  TCC2_INTFLAG = TCC_INTFLAG_CNT(1);
  TCC2_INTFLAG = TCC_INTFLAG_MC1(1);
  TCC2_STATUS = TCC_STATUS_CMP1(1);
  
  SYS_SubscribeToEvent((SYS_EventId_t)BC_EVENT_MAX_SLEEP_TIME_REQUEST, &appTimerEventListener);
}

/******************************************************************
disable timer
 Parameters:
   none
 Returns:
   none
******************************************************************/
void halStopAppClock(void)
{
  /* Disable the clock */
  NVIC_DisableIRQ(TCC2_IRQn);
  TCC2_INTENCLR = TCC_INTENCLR_MC0(1); // disabling interrupt MC0
  TCC2_CTRLA &= ~TCC_CTRLA_ENABLE(1); // Stop timer
  halTimerEnableSync();
}

/******************************************************************************
Return time of sleep timer.

Returns:
  time in ms.
******************************************************************************/
uint32_t halGetTimeOfAppTimer(void)
{
  halAppSystemTimeSynchronize();
  return halAppTime;
}

/**************************************************************************//**
\brief System clock.

\return
  system clock in Hz.
******************************************************************************/
uint32_t HAL_ReadFreq(void)
{
  return (uint32_t)CPU_CLK_HZ;
}

/******************************************************************************
 Delay in microseconds.
 Parameters:
   us - delay time in microseconds
******************************************************************************/
void halDelayUs(uint16_t us)
{
  uint32_t startCounter = 0;
  uint32_t delta = 0;

  us *= AMOUNT_TIMER_CLOCK_IN_ONE_USEC;
  // begin counter meaning

  TCC2_CTRLBSET |= TCC_CTRLBCLR_CMD_READSYNC;
  halTimerCTRLBSync();
  startCounter = TCC2_16COUNT;
  // different between compare regitser and current counter
  delta = TCC2_CC0 - startCounter;

  if (delta > us)
  {
    TCC2_CTRLBSET |= TCC_CTRLBCLR_CMD_READSYNC;
    halTimerCTRLBSync();
    while ((TCC2_16COUNT - startCounter) < us)
   {
     TCC2_CTRLBSET |= TCC_CTRLBCLR_CMD_READSYNC;
     halTimerCTRLBSync();
   }
  }
  else
  {
    us -= delta;
	TCC2_CTRLBSET |= TCC_CTRLBCLR_CMD_READSYNC;
    halTimerCTRLBSync();
    while ((TCC2_16COUNT > startCounter) || (TCC2_16COUNT < us))
   {
     TCC2_CTRLBSET |= TCC_CTRLBCLR_CMD_READSYNC;
     halTimerCTRLBSync();
   }
  }

}
/**************************************************************************//**
  \brief Processes BC_EVENT_MAX_SLEEP_TIME_REQUEST request

  \param[in] eventId - id of raised event;
  \param[in] data    - event's data
 ******************************************************************************/
static void smallestTimerRequestHandler(SYS_EventId_t eventId, SYS_EventData_t data)
{
  uint32_t *minValue = (uint32_t *)data;
  uint32_t timeToFire = halGetTimeToNextAppTimer();

  if (*minValue <= HAL_MIN_SLEEP_TIME_ALLOWED)
  {
    *minValue = 0;
    return;
  }

  if (*minValue > timeToFire)
  {
    if (timeToFire > HAL_MIN_SLEEP_TIME_ALLOWED)
      *minValue = timeToFire;
    else
      *minValue = 0;
  }
  (void)eventId;
}


// eof halAppClock.c
