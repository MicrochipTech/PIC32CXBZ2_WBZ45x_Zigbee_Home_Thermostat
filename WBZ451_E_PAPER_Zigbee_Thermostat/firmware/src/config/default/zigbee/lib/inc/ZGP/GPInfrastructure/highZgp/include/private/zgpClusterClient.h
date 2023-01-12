/*******************************************************************************
  Zigbee green power cluster client Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpClusterClient.h

  Summary:
    This file contains the green power cluster client interface.

  Description:
    This file contains the green power cluster client interface.
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

#ifndef _ZGPCLUSTERCLIENT_H
#define _ZGPCLUSTERCLIENT_H

#ifdef _GREENPOWER_SUPPORT_
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcl.h>
#include <zcl/include/zclGreenPowerCluster.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpProxyTable.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpProxyBasic.h>
#include <zcl/include/zclCommandManager.h>

/******************************************************************************
                    Defines section
******************************************************************************/
#define GP_PAIRING_OPTIONS_LENGTH           0x03
//proxy comm mode command related
#ifndef TUNNELING_SMALL_BUFFER_COUNT
#define TUNNELING_SMALL_BUFFER_COUNT        2
#endif

#ifndef TUNNELING_LARGE_BUFFER_COUNT
#define TUNNELING_LARGE_BUFFER_COUNT        1
#endif
#define TOTAL_NO_OF_TUNNELING_ENTRIES      (TUNNELING_SMALL_BUFFER_COUNT + TUNNELING_LARGE_BUFFER_COUNT)
#define TUNNELING_BUFFER_MAX_SIZE           90u
#ifndef TUNNELING_SMALL_BUFFER_SIZE
// The following can be changed based on the use case
#define TUNNELING_SMALL_BUFFER_SIZE        (TUNNELING_BUFFER_MAX_SIZE / 2)
#endif
// The following is required as per test spec.
#define TUNNELING_LARGE_BUFFER_SIZE         TUNNELING_BUFFER_MAX_SIZE
#define TOTAL_TUNNELING_BUFFER_SIZE         (TUNNELING_SMALL_BUFFER_COUNT * TUNNELING_SMALL_BUFFER_SIZE + \
                                             TUNNELING_LARGE_BUFFER_COUNT * TUNNELING_LARGE_BUFFER_SIZE)

/******************************************************************************
                    Type section
******************************************************************************/
typedef struct PACK
{
  uint8_t bufferLength;
  uint8_t *buffer;
} gpNotificationBuffer_t; 

typedef struct _ZgpEntryInfo_t
{
  ZGP_TableOperationField_t filterField;
  ZGP_GpdId_t gpdId;
  uint8_t endPoint;
} zgpEntryInfo_t;

typedef struct PACK
{
  gpNotificationBuffer_t notificationBuffer;
  zgpEntryInfo_t entryInfo;
  uint16_t groupModeTime;
  uint16_t lightweightUnicastModeTime;
  uint8_t cmdPayloadLength;
  uint8_t macSeqNo;
  bool commNotification;
} zgpClientTunnelingEntry_t;

typedef union PACK
{
  zgpGpNotificationOptions_t gpNotificationOptions;
  zgpGpCommNotifyOptions_t gpCommNotificationOptions;
} gpClientNotificationOptions_t;

/******************************************************************************
                    Externals
******************************************************************************/

/******************************************************************************
                    Prototypes
******************************************************************************/
/**************************************************************************//**
  \brief Initializing zgp cluster client
******************************************************************************/
void zgpClusterClientInit(void);

/**************************************************************************//**
  \brief Sending notification in commissioning/operational mode
******************************************************************************/
ZCL_Status_t zgpClientSendNotification(bool isCommNotification, void *additionalInfo, \
                                       ZGP_LowDataInd_t *dstubInd);

/**************************************************************************//**
  \brief Calucalte time delay(tunneling delay) for the given link quality
******************************************************************************/
uint8_t zgpClientGetTimeDelayFromLqi(uint8_t linkQuality);

/**************************************************************************//**
\brief Stop the on-going tunneling if proxy entry is removed in-between
******************************************************************************/
void zgpClientStopTunneling(ZGP_ApplicationId_t appId, ZGP_GpdId_t *gpdId, uint8_t endPoint);

/**************************************************************************//**
  \brief Build the proxyTableEntry in OTA format

  \param[in] proxyTableOTA - payload to be populated
             proxyTableEntry - proxy table entry

  \return payload length
******************************************************************************/
uint16_t zgppBuildOTAProxyTableEntry( uint8_t *proxyTableOTA, ZGP_ProxyTableEntry_t *proxyTableEntry);

#endif
#endif // _ZGPCLUSTERCLIENT_H

// eof zgpClusterClient.h
