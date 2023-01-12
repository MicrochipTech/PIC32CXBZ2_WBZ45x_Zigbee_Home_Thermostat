/*******************************************************************************
  Zigbee green power task manager Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpTaskManager.h

  Summary:
    This file contains the Header file of zgp task manager.

  Description:
    This file contains the Header file of zgp task manager.
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

#ifndef _ZGP_TASK_MANAGER_H
#define  _ZGP_TASK_MANAGER_H

#ifdef _GREENPOWER_SUPPORT_

/******************************************************************************
                               Includes section
 ******************************************************************************/
#include <systemenvironment/include/sysTypes.h>

/******************************************************************************
                                Types section
 ******************************************************************************/
/** Identifiers of APS task handlers. */
typedef enum  _ZgpTaskID_t
{
  ZGP_DSTUB_TX_GPDF,
  ZGP_DSTUB_DATA_INDICATION,
  ZGP_TASKS_SIZE
} ZgpTaskID_t;

/******************************************************************************
                               Prototypes section
 ******************************************************************************/
/******************************************************************************
  \brief post ZGP DSTUB task.

  \param[in] taskID - identifier of ZGP DSTUB task.
  \return None.
 ******************************************************************************/
void zgpPostTask(const ZgpTaskID_t taskID);

/**************************************************************************//**
  \brief GP-Data-handler for scheduling Tx

  \param[in] None
  \return None.
 ******************************************************************************/
void zgpDstubTxTaskHandler(void);

/**************************************************************************//**
  \brief GP-Data-handler for indication

  \param[in] None
  \return None.
 ******************************************************************************/
void zgpDstubRxIndicationHandler(void);
#endif  //GREENPOWER_SUPPORT
#endif /* _ZGP_DSTUB_TASK_MANAGER_H */
/** eof zgpDstubTaskManager.h */

