/*******************************************************************************
  Zigbee sink table Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpSinkTable.h

  Summary:
    This file contains the green power cluster sink table implementation.

  Description:
    This file contains the green power cluster sink table implementation.
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

#ifndef _ZGPSINKTABLE_H
#define _ZGPSINKTABLE_H

#ifdef _GREENPOWER_SUPPORT_
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcl.h>
#include <zcl/include/zclGreenPowerCluster.h>
#include <zgp/include/zgpCommon.h>
#include <zcl/include/zclGreenPowerCluster.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowNvmTable.h>
#include <configserver/include/configserver.h>

/******************************************************************************
                    Types section
******************************************************************************/
typedef enum _ZgpSinkTableStatus_t
{
  ZGP_SINK_TABLE_SUCCESS             = 0xF0,
  ZGP_SINK_TABLE_ENTRY_NOT_AVAILABLE = 0xF1,
  ZGP_SINK_TABLE_ENTRY_ADDED         = 0xF2,
  ZGP_SINK_TABLE_ENTRY_REMOVED       = 0xF3,
  ZGP_SINK_TABLE_ENTRY_UPDATED       = 0xF4,
  ZGP_SINK_TABLE_FULL                = 0xF5,
  ZGP_SINK_TABLE_ENTRY_EXISTS        = 0xF6,
  ZGP_SINK_TABLE_INVALID_INDEX       = 0xF7,
  ZGP_SINK_TABLE_INVALID_ACTION      = 0xF8
} zgpSinkTableStatus_t;
/******************************************************************************
                    Externals
******************************************************************************/


/******************************************************************************
                    Prototypes
******************************************************************************/
#endif // APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
#endif // _GREENPOWER_SUPPORT_
#endif // _ZGPSINKTABLE_H

// eof zgpSinkTable.h
