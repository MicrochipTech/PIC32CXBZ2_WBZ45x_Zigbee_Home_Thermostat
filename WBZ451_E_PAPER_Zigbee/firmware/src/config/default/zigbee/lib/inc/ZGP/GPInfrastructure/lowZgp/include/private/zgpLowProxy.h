/*******************************************************************************
  Zigbee green power low proxy Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpLowProxy.h

  Summary:
    This file contains the Low Proxy Interface API.

  Description:
    This file contains the Low Proxy Interface API.
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

#ifndef _ZGLOWPROXY_H
#define _ZGPLOWSINK_H

#ifdef _GREENPOWER_SUPPORT_
/******************************************************************************
                        Includes  section.
******************************************************************************/
#include <systemenvironment/include/sysQueue.h>
#include <hal/include/appTimer.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpDstub.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpPacket.h>
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
                        prototypes
******************************************************************************/
/**************************************************************************//**
\brief checks if Complete SinkInfo is empty

\param[in]gpEntry - Ptr to ProxyTableEntry

\return   true  - if entry is empty
          false - otherwise
******************************************************************************/
bool zgpCheckSinkInfoEmpty(ZGP_ProxyTableEntry_t *gpEntry);

/**************************************************************************//**
\brief Removes proxy table entry pairing info

\param[in] removeEntry - actual entry pairing info to be removed from
           rxdEntryInfo - reference entry(rxd in gp pairing cmd)

\return   true - after removing pairing info, entry becomes empty
          false - after removing pairing info, entry doesn't become empty
******************************************************************************/
bool zgpRemoveProxyPairingInfo(ZGP_ProxyTableEntry_t *removeEntry, ZGP_ProxyTableEntry_t *rxdEntyrInfo);

/**************************************************************************//**
\brief To update proxy entry

\param[in] - currentEntry - entry to be updated
             entryInfo - reference entry

\return   true -If Entry is Updated Successfully
          false -If Entry is not updated Successfully(Can be due to Insufficinet Space)
******************************************************************************/
bool zgpUpdateProxyEntry(ZGP_ProxyTableEntry_t *currentEntry, ZGP_ProxyTableEntry_t *entryInfo);

#endif // _GREENPOWER_SUPPORT_
#endif //_ZGPLOWPROXY_H

// eof zgpLowProxy.h
