/*******************************************************************************
  System sleep Source File

  Company:
    Microchip Technology Inc.

  File Name:
    sysSleep.c

  Summary:
    This file contains the System sleep service.

  Description:
    This file contains the System sleep service.
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

/*********************************************************************************
                          Includes section.
**********************************************************************************/
#include <systemenvironment/include/sysDbg.h>
#include <systemenvironment/include/sysSleep.h>
#include <systemenvironment/include/sysAssert.h>
#include <hal/include/appTimer.h>
#include <mac_phy/mac_hwd_phy/RF231_RF212/PHY/include/phyDeepSleep.h>
#include <hal/cortexm4/pic32cx/include/halSleep.h>
#include <hal/cortexm4/pic32cx/include/halAppClock.h>
#include "task.h"
/*********************************************************************************
                     External variables section
**********************************************************************************/
#if defined(_SYS_LOG_ON_)
  #if defined(_HAL_LOG_INTERFACE_UART0_) || defined(_HAL_LOG_INTERFACE_UART1_)
    extern HAL_UsartDescriptor_t sysUsartDescriptor;
  #endif
#endif

/*********************************************************************************
                     Global variables section
**********************************************************************************/
#if defined(_SYS_LOG_ON_)
  #if defined(_HAL_LOG_INTERFACE_UART0_) || defined(_HAL_LOG_INTERFACE_UART1_)
    HAL_Sleep_t localSleepParam;
  #endif
#endif
static HAL_AppTimer_t simulateSleepTimer;
HAL_WakeUpCallback_t wakeupCallback;

static void simulateSleepTimerFired(void)
{
    wakeupCallback(true);
}

/*********************************************************************************
                            Implementation section.
**********************************************************************************/
#if defined(_SYS_LOG_ON_)
  #if defined(_HAL_LOG_INTERFACE_UART0_) || defined(_HAL_LOG_INTERFACE_UART1_)
    /**************************************************************************//**
    \brief Confirmation about the end of work log system.
    ******************************************************************************/
    static void sysLogTmtConfirm(void)
    {
      sysUsartDescriptor.txCallback = NULL;
      HAL_StartSystemSleep(&localSleepParam);
    }
  #endif
#endif


/**************************************************************************//**
\brief Prepares system for sleep.

\param[in]
  sleepParam - pointer to sleep structure.
******************************************************************************/
void SYS_Sleep(HAL_Sleep_t *sleepParam)
{
  #if defined(_SYS_LOG_ON_)
    #if defined(_HAL_LOG_INTERFACE_UART0_) || defined(_HAL_LOG_INTERFACE_UART1_)
    if (1 == HAL_IsTxEmpty(&sysUsartDescriptor))
    {
      status = HAL_StartSystemSleep(sleepParam);
    }
    else
    {
      localSleepParam = *sleepParam;
      sysUsartDescriptor.txCallback = sysLogTmtConfirm;
    }
    #else
    status = HAL_StartSystemSleep(sleepParam);
    #endif
  #else
    vTaskDelay(sleepParam->sleepTime);
  #endif
}


/**************************************************************************//**
\brief Prepares system(MCU + BB) for sleep.

\param[in]
  none
******************************************************************************/
void SYS_EnterSleep(bool sysSleep)
{
  PHY_PrepareSleep(sysSleep);
}

/**************************************************************************//**
\brief Wakes Up the system(MCU + BB) from sleep.

\param[in]
  none
******************************************************************************/
void SYS_WakeUpSleep(bool sysSleep)
{
  PHY_RestoreFromSleep(sysSleep);
}
/**************************************************************************//**
\brief To Stop Stack Timer before Sleep.

\param[in]
  none
******************************************************************************/
void SYS_StopStackTimerBeforeSleep()
{
  halStopAppClock();
}

/**************************************************************************//**
\brief Restart Stack timer after sleep.

\param[in]
  uint32_t time
******************************************************************************/
void SYS_RestartStackTimerAfterSleep(uint32_t sleepTime)
{
  halAdjustSleepInterval(sleepTime);
  halStartAppClock();
}
