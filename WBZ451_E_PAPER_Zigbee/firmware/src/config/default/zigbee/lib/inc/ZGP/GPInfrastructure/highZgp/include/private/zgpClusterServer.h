/*******************************************************************************
  Zigbee green power cluster server Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpClusterServer.h

  Summary:
    This file contains the green power cluster Server interface.

  Description:
    This file contains the green power cluster Server interface.
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

#ifndef _ZGPCLUSTERSERVER_H
#define _ZGPCLUSTERSERVER_H

#ifdef _GREENPOWER_SUPPORT_
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcl.h>
#include <zcl/include/zclGreenPowerCluster.h>
#include <zcl/include/zclCommandManager.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpSinkBasic.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpInfraDevice.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpSinkTable.h>
/******************************************************************************
                    defines section
******************************************************************************/
// Multi-sensor configuration parameters by the application
#ifndef ZGP_TOTAL_NO_COMM_SESSIONS
#define ZGP_TOTAL_NO_COMM_SESSIONS              2
#endif

#ifndef ZGP_MAX_NO_OF_REPORTS
#define ZGP_MAX_NO_OF_REPORTS    12
#endif

#ifndef ZGP_MAX_NO_OF_DATA_POINT_DESCRIPTORS
#define ZGP_MAX_NO_OF_DATA_POINT_DESCRIPTORS        (ZGP_MAX_NO_OF_REPORTS + 6)
#endif

#ifndef ZGP_PAIRING_CONFIG_SESSION_TIMEOUT
#define ZGP_PAIRING_CONFIG_SESSION_TIMEOUT 20000u
#endif

// Implementation specific definitions
#define SECURITY_KEY_LENGTH                 16
#define ZGP_INVALID_REPORT_ID 0xFF

#define ZGP_INVALID_SESSION_ENTRY_INDEX  0xFF
#define ZGP_RXD_REPORT_DESC_MASK_SIZE (ZGP_MAX_NO_OF_REPORTS/8 + \
                                      (ZGP_MAX_NO_OF_REPORTS % 8 ? 1 : 0))

/******************************************************************************
                    Type def section
******************************************************************************/
typedef enum PACK _zgpSessionEntryStatus_t
{
  ENTRY_AVAILABLE = 0x00,
  PAIRING_CONFIG_ENTRY = 0x01,
  COMM_REQ_ENTRY = 0x02
} zgpSessionEntryStatus_t;

typedef struct PACK
{
  uint8_t entryIndex;
  uint8_t freeIndex;
  zgpSessionEntryStatus_t entryStatus;
} zgpSessionIndex_t;

typedef enum PACK
{
  PAIRING_CONFIG_BASIC_INFO_PARSING = 0x01,
  PAIRING_CONFIG_NON_APP_DESC_ACTION_FIELD_PARSING,
  PAIRING_CONFIG_APP_DESC_ACTION_FIELD_PARSING,
  PAIRING_CONFIG_FILTERING_BASEC_ON_ACTION_AND_SESSION_ENTRY,
  PAIRING_CONFIG_NON_APP_DESC_ACTION_PROCESSING,
  PAIRING_CONFIG_APP_DESC_ACTION_PROCESSING,
  PAIRING_CONFIG_CHECK_FOR_COMPLETE_PAIRING_INFO,
  PAIRING_CONFIG_COMPLETE_PAIRING_INFO_RXD,
  PAIRING_CONFIG_COMPLETE,
} zgpPairingConfigFrameState_t;

#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT 
typedef struct PACK
{
  ZGP_ReportDescriptor_t *rxdReportDescriptors;
  uint8_t rxdReportDescriptorMask[ZGP_RXD_REPORT_DESC_MASK_SIZE];
  uint8_t totalNoOfReports;
  uint8_t noOfValidReports;
} zgpRxdReportInfo_t;
#endif

typedef struct PACK
{
  ZGP_GpdAppInfo_t gpdAppInfo;
  ZGP_GpdId_t gpdId;
  uint8_t endPoint;
  uint8_t sessionEntryIndex;
  uint8_t noOfPairedEndpoints;
  uint8_t *pairedEndPointPtr;
  ZGP_SinkTableEntry_t *sinkEntry;
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT   
  zgpRxdReportInfo_t rxdReportInfo;
  bool appDescriptionSet;
#endif  
} zgpPairingContextBasicInfo_t;

typedef struct PACK
{
  zgpPairingConfigActions_t actions;
  zgpPairingConfigOptions_t options;
  zgpPairingContextBasicInfo_t basicContextInfo;
  zgpPairingConfigFrameState_t pairingConfigState;
} zgpPairingConfigContextInfo_t;

typedef struct PACK
{
  ZGP_GpdAppInfo_t gpdAppInfo;
  ZGP_GpdId_t gpdId;
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT  
  uint8_t rxdReportDescriptorMask[ZGP_RXD_REPORT_DESC_MASK_SIZE];
#endif  
  ZGP_ApplicationId_t appId;
  zgpSessionEntryStatus_t entryStatus;
  uint8_t endPoint;
  bool validActionRxd;

  union
  {
    // pairing config specific fields
    struct
    {
      ZGP_SinkTableEntry_t sinkEntry;
      uint32_t entryTimestamp;
      uint16_t commissioningToolNwkAddr;
      APS_AddrMode_t addrMode;
      uint8_t commMode;
      zgpPairingConfigActions_t action;
    };
    // comm. req specific fields
    struct
    {
      bool isBidirectionalCommissioning;
      sinkCommProcState_t commState;
      uint16_t tempMasterAddr;
      bool sendInDirectMode;
      zgpCommReq_t commReq;
      uint8_t secKey[ZGP_SECURITY_KEY_LENGTH];
      bool isKeyUpdated;
    };
  };
} zgpCommReqPairingConfigSessionEntry_t;

typedef struct _RxdGpdfIndBuffer_t
{
  ZGP_LowDataInd_t dstubDataInd;
  uint8_t gpdPayload[MAX_PAYLOAD_BY_GPD];
} zgpSinkRxdGpdfIndBuffer_t;

/******************************************************************************
                    Externals
******************************************************************************/


/******************************************************************************
                    Prototypes
******************************************************************************/
/**************************************************************************//**
  \brief Initializing cluser server
******************************************************************************/
void zgpClusterServerInit(void);

/**************************************************************************//**
  \brief Get shared key atrribute of server
******************************************************************************/
uint8_t *zgpServerGetSharedSecurityKey(void);

/**************************************************************************//**
  \brief Get shared key type atrribute of server
******************************************************************************/
ZGP_SecKeyType_t zgpServerGetSharedSecurityKeyType(void);

/**************************************************************************//**
  \brief Remove sink table entry on matching GPD id
******************************************************************************/
zgpSinkTableStatus_t zgpSinkRemoveGpdEntry(ZGP_SinkTableEntry_t *sinkEntry, bool sendGpPairing);

/**************************************************************************//**
 \brief Build the sinkTableEntry in OTA format
******************************************************************************/
uint16_t zgpsBuildOTASinkTableEntry( uint8_t *sinkTableOTA, ZGP_SinkTableEntry_t *sinkTableEntry);

/**************************************************************************//**
\brief Parsing application info. from the payload
******************************************************************************/
bool zgpParsingAppInfoFromPayload(uint8_t *gpdAsdu, uint8_t *index, ZGP_GpdAppInfo_t *appInfo, uint16_t rxdFrameLength);

/**************************************************************************//**
  \brief Add the group and send device announce for derived/precommissioned groupcast
******************************************************************************/
void zgpServerAddGroup(ZGP_SinkTableEntry_t *sinkTableEntry, bool sendDeviceAnnounce);

/**************************************************************************//**
  \brief Sending gpPairing for the entry
******************************************************************************/
void zgpServerSendGpPairingForEntry(ZGP_SinkTableEntry_t *sinkTableEntry, ZGP_SinkTableActions_t action);

/**************************************************************************//**
  \brief Getting session entry
******************************************************************************/
bool zgpServerGetCommReqPairingConfigSessionTableIndex(ZGP_ApplicationId_t appId, \
                                                       ZGP_GpdId_t *gpdId, uint8_t endPoint, \
                                                       zgpSessionIndex_t *sessionIndex);

/**************************************************************************//**
  \brief To fetch the commissioning session entry of COMM_REQ type
******************************************************************************/
bool zgpServerGetCommReqEntry(zgpSessionIndex_t *sessionIndex);

#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
/**************************************************************************//**
  \brief To check all the packets received for the given session
******************************************************************************/
bool zgpCommSessionAllPacketsRceived(uint8_t sessionIndex);

/**************************************************************************//**
  \brief initialize the session entry of the given index
******************************************************************************/
void zgpServerInitReportDescMask(uint8_t index);

/**************************************************************************//**
  \brief To process the received application descriptor information
******************************************************************************/
void zgpServerProcessAppDescCommand(zgpRxdReportInfo_t *rxdReportInfo, uint8_t sessionIndex);

/**************************************************************************//**
  \brief To parse the report descriptor fields from the received payload
******************************************************************************/
ZCL_Status_t zgpParseReportDescriptorFields(zgpRxdReportInfo_t *rxdReportInfo, uint8_t *payload, \
                                            uint8_t payloadIndex, uint8_t payloadLength, uint8_t entryIndex);

/**************************************************************************//**
  \brief To free report descriptor buffer for the session
******************************************************************************/
void zgpServerFreeReportDataDescBufer(ZGP_ReportDescriptor_t *reportDesc);
#endif // ZGP_ENABLE_MULTI_SENSOR_SUPPORT

/**************************************************************************//**
  \brief free the session entry
******************************************************************************/
void zgpServerFreeSessionEntry(uint8_t sessionEntryIndex);
#endif // APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
#endif // _GREENPOWER_SUPPORT_
#endif // _ZGPCLUSTERSERVER_H

// eof zgpClusterServer.h
