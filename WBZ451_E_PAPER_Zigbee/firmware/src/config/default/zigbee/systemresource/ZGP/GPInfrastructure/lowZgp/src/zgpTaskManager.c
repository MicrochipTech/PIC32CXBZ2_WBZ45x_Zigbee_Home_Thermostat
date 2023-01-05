/*******************************************************************************
  Zigbee green power task manager Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpTaskManager.c

  Summary:
    This file contains the Implementation of ZGP task manager.

  Description:
    This file contains the Implementation of ZGP task manager.
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

#ifdef _GREENPOWER_SUPPORT_

#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
/******************************************************************************
                             Includes section
 ******************************************************************************/
#include <systemenvironment/include/sysTaskManager.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpTaskManager.h>
#include <zgp/include/zgpDbg.h>
#include <systemenvironment/include/sysAssert.h>

/******************************************************************************
                           Implementations section
 ******************************************************************************/
/**************************************************************************//**
  \brief Prepare to run same ZGP task handler.

  \param[in] taskId - identifier of ZGP task.
  \return None.
 ******************************************************************************/
void zgpPostTask(const ZgpTaskID_t taskID)
{
  SYS_E_ASSERT_FATAL(taskID < ZGP_TASKS_SIZE, ZGP_TASKMANAGER_TASKHANDLER_0);

  if (ZGP_DSTUB_DATA_INDICATION == taskID)
  {
    SYS_PostTask(ZGP_TASK_ID);
  }
  else if (ZGP_DSTUB_TX_GPDF == taskID)
  {
    SYS_PostTask(ZGP_DSTUB_TASK_ID);
  }
}

/**************************************************************************//**
  \brief Global task handler of ZGP dstub component(lower priority) .
 ******************************************************************************/
void ZGP_Dstub_TaskHandler(void)
{
  zgpDstubTxTaskHandler();
}

/**************************************************************************//**
  \brief Global task handler of ZGP layer(proxy/sink modules).
 ******************************************************************************/
void ZGP_TaskHandler(void)
{
  zgpDstubRxIndicationHandler();
}
#endif // APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
#endif // _GREENPOWER_SUPPORT_
/** eof zgpTaskManager.c */

