/*******************************************************************************
  Zigbee green power proxy table Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpProxyTable.h

  Summary:
    This file contains the green power cluster proxy table implementation.

  Description:
    This file contains the green power cluster proxy table implementation.
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

#ifndef _ZGPPROXYTABLE_H
#define _ZGPPROXYTABLE_H

#ifdef _GREENPOWER_SUPPORT_
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcl.h>
#include <zcl/include/zclGreenPowerCluster.h>
#include <zgp/include/zgpCommon.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowNvmTable.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpClusterStructure.h>

/******************************************************************************
                    Types section
******************************************************************************/
typedef enum _ZgpProxyTableStatus_t
{
  ZGP_PROXY_TABLE_SUCCESS             = 0xF0,
  ZGP_PROXY_TABLE_ENTRY_NOT_AVAILABLE = 0xF1,
  ZGP_PROXY_TABLE_ENTRY_ADDED         = 0xF2,
  ZGP_PROXY_TABLE_ENTRY_REMOVED       = 0xF3,
  ZGP_PROXY_TABLE_ENTRY_UPDATED       = 0xF4,
  ZGP_PROXY_TABLE_FULL                = 0xF5,
  ZGP_PROXY_TABLE_ENTRY_EXISTS        = 0xF6,
  ZGP_PROXY_TABLE_INVALID_INDEX       = 0xFF
} zgpProxyTableStatus_t;

/******************************************************************************
                    Externals
******************************************************************************/


/******************************************************************************
                    Prototypes
******************************************************************************/
/**************************************************************************//**
\brief Creates New  proxy table entry

\param[in] Structure for the Proxy Table entry with Necessary Parameters filled.

\return  ZGP_ENTRY_ADDED - if Entry added Successfully
         ZGP_PROXY_TABLE_FULL - if Proxy Table is full
******************************************************************************/
zgpProxyTableStatus_t zgpProxyTableCreateOrUpdateEntry(ZGP_GpPairing_t *gpPairingCmd, bool isUpdateRequested);

#endif // _GREENPOWER_SUPPORT_

#endif // _ZGPPROXYTABLE_H

// eof zgpProxyTable.h
