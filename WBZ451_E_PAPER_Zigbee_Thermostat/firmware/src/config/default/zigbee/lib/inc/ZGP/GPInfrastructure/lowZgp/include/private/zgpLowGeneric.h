/*******************************************************************************
  Zigbee green power low generic Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpLowGeneric.h

  Summary:
    This file contains the green power cluster Low generic feature interface.

  Description:
    This file contains the green power cluster Low generic feature interface.
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

#ifndef _ZGPLOWGENERIC_H
#define _ZGPLOWGENERIC_H

#ifdef _GREENPOWER_SUPPORT_
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zgp/include/zgpCommon.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpDstub.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLow.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowGpdf.h>
#include <zdo/include/zdo.h>

/******************************************************************************
                           Definitions section
******************************************************************************/
#ifndef ZGP_DUPLICATE_TABLE_SIZE
  #define ZGP_DUPLICATE_TABLE_SIZE  5
#endif
#define ZGP_DEFAULT_IEEE_ADDR                    0xFFFFFFFFFFFFFFFF
                             
/******************************************************************************
                    Types section
******************************************************************************/
typedef struct _ZgpZdpReq_t
{
  ZDO_ZdpReq_t zdpReq;
  bool busy;
} zgpZdpReq_t;

typedef enum _zgp_DuplicateCheckStatus_t
{
  DUPLICATE_ENTRY = 0x00,
  ENTRY_ADDED     = 0x01,
  NO_ENTRY_AVAILABLE =0x02
} zgpDuplicateCheckStatus_t;

typedef struct PACK
{
  ZGP_GpdId_t gpdId;
  uint8_t endpoint;
  ZGP_ApplicationId_t applicationId;
  ZGP_SecLevel_t gpdfSecurityLevel;
  uint32_t seqNoSecurityFrameCounter;
} zgpDuplicateEntryParameters_t;

typedef struct PACK
{
  zgpDuplicateEntryParameters_t zgpDuplicateEntryParameters;
  int16_t entryTtl;
  bool isActive;
} zgpDuplicateEntry_t;

typedef struct PACK
{
  zgpDuplicateEntry_t zgpDuplicateEntry[ZGP_DUPLICATE_TABLE_SIZE];
  BcTime_t tableLastTimeStamp; 
} zgpDuplicateTable_t;

/******************************************************************************
                    Externals
******************************************************************************/


/******************************************************************************
                    Prototypes
******************************************************************************/
/**************************************************************************//**
  \brief Initializing zgp generic
******************************************************************************/
void zgpGenericInit(void);

/**************************************************************************//**
  \brief Sec request processing from dstub
******************************************************************************/
void zgpGenericSecRequest(ZGP_LowDataInd_t *dstubDataInd, zgpDstubSecResponse_t *secResp);

#endif // _GREENPOWER_SUPPORT_

#endif // _ZGPLOWGENERIC_H

// eof zgpLowGeneric.h
