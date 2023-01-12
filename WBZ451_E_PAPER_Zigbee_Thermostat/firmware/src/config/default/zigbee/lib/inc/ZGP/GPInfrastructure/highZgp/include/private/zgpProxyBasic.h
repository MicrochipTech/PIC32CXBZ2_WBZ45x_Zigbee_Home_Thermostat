/*******************************************************************************
  Zigbee green power proxy basic Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpProxyBasic.h

  Summary:
    This file contains the zgp proxy basic interface.

  Description:
    This file contains the zgp proxy basic interface.
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

#ifndef _ZGPPROXYBASIC_H
#define _ZGPPROXYBASIC_H

#ifdef _GREENPOWER_SUPPORT_
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcl.h>
#include <zcl/include/zclGreenPowerCluster.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowGpdf.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighGeneric.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpInfraDevice.h>

/******************************************************************************
                    Defines
******************************************************************************/
#define PROXY_ADDRESS_CONFLICT_SEQUENCE_NO 0x30

/******************************************************************************
                    Types
******************************************************************************/
typedef struct PACK
{
  zgpCommModeType_t commModeType;
  ShortAddr_t sinkAddr;
  uint8_t transmitChannel;
  bool inUnicastMode;
} zgpProxyBasicModeInfo_t;

/******************************************************************************
                    Externals
******************************************************************************/

/******************************************************************************
                    Prototypes
******************************************************************************/
/**************************************************************************//**
  \brief To fetch the nwk addr of the sink sent proxy commissioning mode req
******************************************************************************/
uint16_t zgpProxyBasicGetSinkNwkAddr(void);

/**************************************************************************//**
  \brief To handle GPDF received from sink(via GP cluster cmds) and GPD(GPDF)
******************************************************************************/
ZCL_Status_t zgpProxyDataHandling(zgpProxyDataFrameType_t dataFrameType, void *dataFrame);

/**************************************************************************//**
  \brief Indication callback from dstub for GPDFs received from GPD
******************************************************************************/
void zgpProxyDstubDataInd(const ZGP_LowDataInd_t *const ind);

#endif // _GREENPOWER_SUPPORT_
#endif // _ZGPPROXYBASIC_H

// eof zgpProxyBasic.h
