/*******************************************************************************
  HAL MAC Isr Source File

  Company:
    Microchip Technology Inc.

  File Name:
    halMacIsr.c

  Summary:
    This file contains the Implementation of hal timer specifically used by
    the MAC layer for its functionality.

  Description:
    This file provides the interface to access the HAL timer API's required by the BitCloud MAC
    layer.
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
                        Includes section.
******************************************************************************/
#include "device.h"
#include <hal/cortexm4/pic32cx/include/halRfCtrl.h>
#include <hal/cortexm4/pic32cx/include/halMacIsr.h>
#if defined(_STATS_ENABLED_)
#include <statStack.h>
#endif
#include <configserver/include/configserver.h>
#include <systemenvironment/include/sysAssert.h>
#include <hal/cortexm4/pic32cx/include/halRfCtrl.h>
#include <mac_phy/mac_hwd_phy/RF231_RF212/PHY/include/phyRtimerIrqDispatcher.h>

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define HAL_NUMBER_OF_TICKS_IN_USEC     (APB_CLK_HZ/RTIMER_FREQUENCY_PRESCALER/1000000ul)

/******************************************************************************
                   Global variables section
******************************************************************************/
static RTimerDescr_t __rtimer;

bool rTimerIntervalComplete = true;
static uint16_t nextCycleCnt;
void setRTimerNextCycleCnt(uint16_t count);
void rTimerLoadCnt(HAL_RTimerMode_t mode, uint16_t per);
/******************************************************************************
                    Prototypes section
******************************************************************************/
/******************************************************************************
  Initializes Rtimer and RF ext. interrupts.
******************************************************************************/
void ZB_HAL_InitMacIsr(void);

/******************************************************************************
  MAC timer handler.
******************************************************************************/

/******************************************************************************
  Redirect interrupt event depending on the TrxState.
  Parameters: none.
  Returns: none.
******************************************************************************/
void phyDispatcheRTimerEvent(void);

/******************************************************************************
                    Implementations section
******************************************************************************/
/******************************************************************************
 Polling the Sync. flag for register access
 Parameters:
   none
 Returns:
   none
 *****************************************************************************/
INLINE void rRtimerSync(void)
{
  //while (TCC2_16_STATUS_s.syncbusy);
}
/******************************************************************************
 Polling the Sync. flag for CTRLB SyncBusy register access
 Parameters:
   none
 Returns:
   none
 *****************************************************************************/
INLINE void halrTimerCTRLBSync(void)
{
  while (TCC2_SYNCBUSY & TCC_SYNCBUSY_CTRLB_Msk);
}
/******************************************************************************
 Polling the Sync. flag for CC1 register access
 Parameters:
   none
 Returns:
   none
 *****************************************************************************/
INLINE void halrTimerCC1Sync(void)
{
  while (TCC2_SYNCBUSY & TCC_SYNCBUSY_CC1_Msk);
}

/******************************************************************************
 Polling the Sync. flag for CNT register access
 Parameters:
   none
 Returns:
   none
 *****************************************************************************/
INLINE void halrTimerCountSync(void)
{
  while (TCC2_SYNCBUSY & TCC_SYNCBUSY_COUNT_Msk );
}
/******************************************************************************
  Initializes Rtimer.
******************************************************************************/
void ZB_HAL_InitMacIsr(void)
{

  __rtimer.mode = HAL_RTIMER_STOPPED_MODE;

  /* Configure the generic clk settings for enabling TC4 and TC5 */
  //GCLK_CLKCTRL = GCLK_CLKCTRL_ID((uint32_t)GCLK_TCC2_GCLK_TC5) | GCLK_CLKCTRL_GEN(GCLK_GEN_3) | GCLK_CLKCTRL_CLKEN;
    //GCLK_PCHCTRL(TCC2_GCLK_ID) = GCLK_PCHCTRL_GEN(GCLK_GEN_3) | GCLK_PCHCTRL_CHEN;

}
/******************************************************************************
 Sets the Sequential CNT variable to be loaded in CC1 when the
 totalCNT > TOP_TIMER_COUNTER_VALUE
 Parameters:
   none
 Returns:
   none
 *****************************************************************************/
void setRTimerNextCycleCnt(uint16_t remCnt)
{
  TCC2_CTRLBSET |= TCC_CTRLBCLR_CMD_READSYNC;
  halrTimerCTRLBSync();
    
  uint16_t tcCnt = TCC2_16COUNT;

  if(remCnt <  tcCnt)
  {
    rTimerIntervalComplete = true;
    phyDispatcheRTimerEvent();
    return;
  }
 uint16_t loadCnt = tcCnt + remCnt;
 if(loadCnt > TOP_TIMER_COUNTER_VALUE)
 {
   TCC2_CC1 = TCC_CC_CC(TOP_TIMER_COUNTER_VALUE);
   halrTimerCC1Sync();
   loadCnt = loadCnt - TOP_TIMER_COUNTER_VALUE;
   nextCycleCnt = loadCnt;
 }
 else
 {
   rTimerIntervalComplete = true;
   TCC2_CC1 = TCC_CC_CC(loadCnt);
   halrTimerCC1Sync();
 }
  TCC2_INTENSET = TCC_INTENSET_MC1(1);
  TCC2_INTFLAG = TCC_INTFLAG_MC1(1);
}

void rTimerLoadCnt(HAL_RTimerMode_t mode, uint16_t per)
{
  __rtimer.period    = (uint16_t)((uint32_t)per * HAL_NUMBER_OF_TICKS_IN_USEC);
  __rtimer.mode      = mode;
  
  TCC2_CTRLBSET |= TCC_CTRLBCLR_CMD_READSYNC;
  halrTimerCTRLBSync();
    
  uint16_t tcCount = TCC2_16COUNT;
  uint32_t totalCC1 = tcCount + __rtimer.period;

  if(totalCC1 > TOP_TIMER_COUNTER_VALUE )
   {
    rTimerIntervalComplete = false;
    uint16_t firstCycleCC1 = TOP_TIMER_COUNTER_VALUE;
    TCC2_CC1 = TCC_CC_CC(firstCycleCC1);
    halrTimerCC1Sync();
    uint16_t countInFistCycle = TOP_TIMER_COUNTER_VALUE - tcCount;
    nextCycleCnt = (__rtimer.period - countInFistCycle);
  }
  else
  {
    TCC2_CC1 = TCC_CC_CC(totalCC1);
    halrTimerCC1Sync();
  }
}
/******************************************************************************
  Starts RTimer. Function should be invoked in critical section.
  Parameters:
    mode    - RTimer mode.
    period  - RTimer period.
******************************************************************************/
bool HAL_StartRtimer(HAL_RTimerMode_t mode, uint16_t period)
{
  
  if (HAL_RTIMER_STOPPED_MODE != __rtimer.mode)
    return false;

  rTimerLoadCnt(mode, period);
  
  TCC2_INTENSET = TCC_INTENSET_MC1(1);
  TCC2_INTFLAG = TCC_INTFLAG_MC1(1);

  return true;
}

/******************************************************************************
  Stops RTimer. Function should be invoked in critical section.
******************************************************************************/
void HAL_StopRtimer(void)
{

  TCC2_INTENCLR = TCC_INTENSET_MC1(1);  // disabling interrupt MC0
  rRtimerSync();

  __rtimer.mode = HAL_RTIMER_STOPPED_MODE;
}

/******************************************************************************
  Returns status of RTimer
******************************************************************************/
bool HAL_IsRtimerStopped(void)
{
  if (__rtimer.mode == HAL_RTIMER_STOPPED_MODE)
    return true;
  else
    return false;
}

/******************************************************************************
  MAC timer handler.
******************************************************************************/
void TCC2_halMacTimerHandler(void)
{
#if defined(_STATS_ENABLED_)
  uint16_t stack_threshold;
  uint16_t stack_left = Stat_GetCurrentStackLeft();
  CS_ReadParameter(CS_STACK_LEFT_THRESHOLD_ID,&stack_threshold);
  SYS_E_ASSERT_FATAL((stack_left>stack_threshold),MAC_TIMER_ISR_STACK_OVERFLOW);
#endif //(_STATS_ENABLED_)

    if (HAL_RTIMER_ONE_SHOT_MODE == __rtimer.mode)
    {
     __rtimer.mode = HAL_RTIMER_STOPPED_MODE;
    }
    TCC2_INTENCLR = TCC_INTENSET_MC1(1);  // disabling interrupt MC1
    rRtimerSync();
    if(rTimerIntervalComplete)
    {
      phyDispatcheRTimerEvent();
    }
    else
    {
      setRTimerNextCycleCnt(nextCycleCnt);
    }
    if (HAL_RTIMER_REPEAT_MODE == __rtimer.mode)
    {
      rTimerLoadCnt(HAL_RTIMER_REPEAT_MODE, __rtimer.period);
      TCC2_INTENSET = TCC_INTENSET_MC1(1);
      TCC2_INTFLAG = TCC_INTFLAG_MC1(1);
    }
}
// eof halMacIsr.c

