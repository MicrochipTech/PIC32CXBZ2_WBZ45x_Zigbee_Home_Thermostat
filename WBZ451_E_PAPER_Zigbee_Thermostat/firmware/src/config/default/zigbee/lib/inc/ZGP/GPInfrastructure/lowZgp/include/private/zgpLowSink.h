/*******************************************************************************
  Zigbee green power Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpLowSink.h

  Summary:
    This file contains the Lower Sink Interface API.

  Description:
    This file contains the Lower Sink Interface API.
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

#ifndef _ZGPLOWSINK_H
#define _ZGPLOWSINK_H

#ifdef _GREENPOWER_SUPPORT_
/******************************************************************************
                        Includes  section.
******************************************************************************/
#include <systemenvironment/include/sysQueue.h>
#include <hal/include/appTimer.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpDstub.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpPacket.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowNvmTable.h>
#include <zgp/include/zgpDbg.h>
#include <systemenvironment/include/sysAssert.h>

/******************************************************************************
                        Define section.
******************************************************************************/


/******************************************************************************
                        Types section.
******************************************************************************/

/******************************************************************************
                        Global varible section.
******************************************************************************/

/******************************************************************************
                        Prototypes
******************************************************************************/
/**************************************************************************//**
  \brief remove the pairing information in sink table(pre-commissioned mode)

  \param[in] currentEntry - pointer to sink table entry to be updated
             groupListToBeRemoved - groupList to be removed
             commMode - communication mode to be depaired
             sendPairing - bool to enabled/disable pairing tx

  \return true - if entry becomes empty
          false otherwise
******************************************************************************/
bool zgpRemoveSinkPairingInfo(ZGP_SinkTableEntry_t *currentEntry, ZGP_SinkGroup_t *groupListToBeRemoved, uint8_t commMode);

/**************************************************************************//**
\brief add or extend the sink information for an entry

\param[in]   currEntry - pointer to the entry to be added (or) extended
            sinkTableEntry  - pointer to the entry to be copied

\return   true -If Entry is Updated Successfully
          false -If Entry is not updated Successfully(Can be due to Insufficinet Space)
******************************************************************************/
bool zgpAddExtendSinkTableEntry(ZGP_SinkTableEntry_t *currEntry, ZGP_SinkTableEntry_t *sinkTableEntry);

/**************************************************************************//**
\brief fill GPD info of sink table entry

\param[in]   currEntry - pointer to the entry to be replaced
          sinkTableEntry  - pointer to the entry to be copied

\return None
******************************************************************************/
void zgpFillGpdInfoOfSinkTableEntry(ZGP_SinkTableEntry_t *currEntry, ZGP_SinkTableEntry_t *sinkTableEntry);

#endif // _GREENPOWER_SUPPORT_
#endif //_ZGPLOWSINK_H

// eof zgpLowSink.h
