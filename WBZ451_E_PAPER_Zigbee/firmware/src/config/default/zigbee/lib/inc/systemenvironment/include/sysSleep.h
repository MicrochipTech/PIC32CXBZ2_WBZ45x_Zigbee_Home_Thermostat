/*******************************************************************************
  System sleep Header File

  Company:
    Microchip Technology Inc.

  File Name:
    sysSleep.h

  Summary:
    Implementation of the system sleep service.

  Description:
    Implementation of the system sleep service.
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

#ifndef _SYSSLEEP_H
#define _SYSSLEEP_H
/*********************************************************************************
                          Includes section.
**********************************************************************************/
#include <hal/include/sleep.h>
#include <hal/cortexm4/pic32cx/include/halSleep.h>
#include <mac_phy/mac_hwd_phy/RF231_RF212/PHY/include/phyPic32cx.h>
/*********************************************************************************
                   Define(s) section
**********************************************************************************/
#define SYS_IDLE                       HAL_IdleMode()
#define ZB_EnterSleep                  SYS_EnterSleep
#define ZB_WakeUpFromSleep             SYS_WakeUpSleep
#define ZB_StopStackTimerBeforeSleep   SYS_StopStackTimerBeforeSleep
#define ZB_RestartStackTimerAfterSleep SYS_RestartStackTimerAfterSleep

/*********************************************************************************
                          Function prototypes section.
**********************************************************************************/
/******************************************************************************//**
\brief Prepares system for sleep.

\param[in]
  sleepParam - pointer to sleep structure.
**********************************************************************************/
void SYS_Sleep(HAL_Sleep_t *sleepParam);
/**************************************************************************//**
\brief Prepares system(MCU + BB) for sleep.

\param[in]
  none
******************************************************************************/
void SYS_EnterSleep(bool sysSleep);
/**************************************************************************//**
\brief Wakes Up the system(MCU + BB) from sleep.

\param[in]
  none
******************************************************************************/
void SYS_WakeUpSleep(bool sysSleep);
/**************************************************************************//**
\brief To Stop Stack Timer before Sleep.

\param[in]
  none
******************************************************************************/
void SYS_StopStackTimerBeforeSleep();
/**************************************************************************//**
\brief Restart Stack timer after sleep.

\param[in]
  uint32_t time
******************************************************************************/
void SYS_RestartStackTimerAfterSleep(uint32_t sleepTime);
#endif /* _SYSSLEEP_H */
