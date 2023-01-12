// DOM-IGNORE-BEGIN
/*******************************************************************************
  Zigbee green power infradevice Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpInfraDevice.h

  Summary:
    This file contains the proxy/sink specific APIs from high zgp.

  Description:
    This file contains the proxy/sink specific APIs from high zgp.
 *******************************************************************************/


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

// DOM-IGNORE-BEGIN
#ifndef _ZGPINFRADEVICE_H
#define _ZGPINFRADEVICE_H

#ifdef _GREENPOWER_SUPPORT_
// DOM-IGNORE-END

/****************************************************************
                             INCLUDE FILES
 *****************************************************************/ 
 
#include <zcl/include/zcl.h>
#include <zgp/include/zgpCommon.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLow.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowGpdf.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpClusterStructure.h>
#if MICROCHIP_APPLICATION_SUPPORT == 1
#include <app_zigbee/zigbee_console/console.h>
#endif

/******************************************************************************
                    Defines section
 ******************************************************************************/
 
#ifndef COMM_REQ_COMMAND_COUNT
#define COMM_REQ_COMMAND_COUNT         10
#endif
#ifndef COMM_REQ_SERVER_CLUSTER_COUNT
#define COMM_REQ_SERVER_CLUSTER_COUNT  4
#endif
#ifndef COMM_REQ_CLIENT_CLUSTER_COUNT
#define COMM_REQ_CLIENT_CLUSTER_COUNT  4
#endif

// Multi-sensor specific structures - TX pairing Configuration
#define PRECOMMISSIONED_PAIRED_ENDPOINT 0xFE
#define ZGP_APPLICATION_DESCR_PAIRING_CONFIG_TIMEOUT 1000
#define MAX_SINGLE_REPORT_DESC_LEN  25
#define INVALID_LENGTH 0xFF
// Multi-sensor specific structures
#define ZGP_REPORT_DESCRIPTOR_TIMEOUT_BIT_MASK (1u << 0)
#define IS_ZGP_REPORT_DESCRIPTOR_TIMEOUT_PRESENT(a) (a & ZGP_REPORT_DESCRIPTOR_TIMEOUT_BIT_MASK)

#define ZGP_ATTR_RECORD_REMAINING_LENGTH(a) (a & 0xF)
#define ZGP_ATTR_VALUE_PRESENT_BIT_POS 5
#define IS_ZGP_ATTR_VALUE_PRESENT(a) ((a >> ZGP_ATTR_VALUE_PRESENT_BIT_POS) & 0x1)

#define ZGP_DATAPOINT_DESC_NO_OF_ATTRIBUTE_RECORDS_MASK 0x7
#define ZGP_DATAPOINT_DESC_NO_OF_ATTRIBUTE_RECORDS(a) (a & ZGP_DATAPOINT_DESC_NO_OF_ATTRIBUTE_RECORDS_MASK)

#define ZGP_DATAPOINT_DESC_CLIENT_SERVER_BIT_POS 3
#define ZGP_DATAPOINT_DESC_CLIENT_SERVER(a) ((a >> ZGP_DATAPOINT_DESC_CLIENT_SERVER_BIT_POS) & 0x1)

#define ZGP_DATAPOINT_DESC_MANUFAC_ID_PRESENT_BIT_POS 4
#define IS_ZGP_DATAPOINT_DESC_MANUFAC_ID_PRESENT(a) ((a >> ZGP_DATAPOINT_DESC_MANUFAC_ID_PRESENT_BIT_POS) & 0x1)

#define ZGP_ATTR_REPORTED_BIT_POS 4
#define IS_ZGP_ATTR_REPORTED(a)    ((a >> ZGP_ATTR_REPORTED_BIT_POS) & 0x1)

#define ZGP_GENERIC_SWITCH_MIN_LENGTH 0x02
#define INVALID_CONTACT_VALUE 0x00

//#if MICROCHIP_APPLICATION_SUPPORT == 1
//  #ifdef  BOARD_PC
//    #define appSnprintf(...) fprintf(stdout, __VA_ARGS__)
//  #else
//    #define appSnprintf(...) appSnprintf(__VA_ARGS__)
//  #endif
//#else
//#define appSnprintf(...) do{}while(0)
//#endif

/******************************************************************************
                    Types section
 ******************************************************************************/
 /** status codes for ZGP infrastrucutre devices*/
typedef enum _ZGP_InfraDeviceStatus_t
{
  ZGP_SUCCESS                         = 0x00,
  ZGP_ALREADY_IN_COMMISSIONING_MODE   = 0x01,
  ZGP_ENDPOINT_NOT_REGISTERED         = 0x02,
  ZGP_INVALID_ACTION                  = 0x03,
  ZGP_NO_FREE_ENTRY                   = 0x04,
  ZGP_ENTRY_NOT_AVAILABLE             = 0x05,
  ZGP_INSUFFICIENT_SPACE_STATUS       = 0x06,
  ZGP_NOT_IN_EXPECTED_STATE           = 0x07
} ZGP_InfraDeviceStatus_t;

typedef struct
{
  uint8_t macSeqNoCapability      : 1;
  uint8_t rxOnCapability          : 1;
  uint8_t applicationInfoPresent  : 1;
  uint8_t reserved                : 1;
  uint8_t panidRequest            : 1;
  uint8_t gpSecurityKeyRequest    : 1;
  uint8_t fixedLocation           : 1;
  uint8_t extOptionsField         : 1;
} zgpCommCmdReqOptions_t;

typedef struct
{
  uint8_t securityLevelCapabilities  : 2;
  uint8_t keyType                    : 3;
  uint8_t gpdKeyPresent              : 1;
  uint8_t gpdKeyEncryption           : 1;
  uint8_t gpdOutgoingCounterPresent  : 1;
} zgpCommCmdExtOptions_t;

typedef struct PACK
{
  uint8_t manufacturerIdPresent        : 1;
  uint8_t modelIdPresent               : 1;
  uint8_t gpdCommandPresent            : 1;
  uint8_t clusterListPresent           : 1;
  uint8_t switchInformationPresent     : 1;
  uint8_t appDescriptionCommandFollows : 1;
  uint8_t reserved                     : 2;
} zgpGpdAppInfoOptions_t;

typedef struct PACK
{
  uint8_t numOfServerCluster      : 4;
  uint8_t numOfClientCluster      : 4;
} zgpLengthOfClusterList_t;

//GP pairing Configuration Actions command
typedef struct PACK
{
  uint8_t action        :3;
  uint8_t sendGpPairing :1;
  uint8_t reserved      :4;
}zgpPairingConfigActions_t;

//GP pairing Configuration Options command
typedef struct PACK
{
  uint16_t appId                         :3;
  uint16_t communicationMode             :2;
  uint16_t seqNumCapabilities            :1;
  uint16_t rxOnCapability                :1;
  uint16_t fixedLocation                 :1;
  uint16_t assignedAlias                 :1;
  uint16_t securityUse                   :1;
  uint16_t applicationInfoPresent        :1;
  uint16_t reserved                      :5;
}zgpPairingConfigOptions_t;

typedef struct PACK
{
  uint8_t noOfContacts                   :4;
  uint8_t switchType                     :2;
  uint8_t reserved                       :2;
}zgpGenericSwitchConfig_t;

typedef struct PACK
{
  uint8_t switchInfoLength;
  zgpGenericSwitchConfig_t genericSwitchConfig;
  uint8_t currContactStatus;
  uint8_t contactBitMask;
}zgpSwitchInfo_t; 

typedef struct
{
  zgpGpdAppInfoOptions_t appInfoOptions;
  uint16_t manfacId;
  uint16_t modelId;
  uint8_t noOfGpdCmds;
  uint8_t gpdCommandList[COMM_REQ_COMMAND_COUNT];
  zgpLengthOfClusterList_t noOfClusters;
  uint16_t clusterListServer[COMM_REQ_SERVER_CLUSTER_COUNT];
  uint16_t clusterListClient[COMM_REQ_CLIENT_CLUSTER_COUNT];
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
  uint8_t totalNoofReports;
  uint8_t noOfReportsPendingInGpPairingConfig;
  ZGP_ReportDescriptor_t *reportDescriptor; // array of data point descriptors
#endif
#ifdef ZGP_ENABLE_GENERIC_8_CONTACT_SWITCH_SUPPORT
  zgpSwitchInfo_t switchInfo;
#endif
} ZGP_GpdAppInfo_t;

typedef struct PACK
{
  uint8_t noOfPairedEndPoints;
  uint8_t *pairedEndpoints;
}ZGP_EndpointInfo_t;

typedef struct
{
  uint8_t gpdDeviceId;
  zgpCommCmdReqOptions_t options;
  zgpCommCmdExtOptions_t extOptions;
  uint8_t gpdKey[ZGP_SECURITY_KEY_LENGTH];
  uint32_t gpdKeyMic;
  uint32_t gpdOutgoingCounter;
  ZGP_GpdAppInfo_t *appInfo;
} zgpCommReq_t;

typedef enum _ZGP_IndicationType_t
{
  SUCCESSFUL_COMMISSIONING = 0x00,
  GPD_COMMAND_RECEIVED,
  PAIRING_CONFIG_CMD_RECEIVED,
  SINK_COMM_MODE_REQ_CMD_RECEIVED,
  GP_PAIRING_SEARCH_IND,
  GP_TUNNELING_STOP_IND,
  GP_TRANSLATION_TABLE_UPDATE_COMMAND_IND,
  GP_TRANSLATION_TABLE_REQUEST_COMMAND_IND,
  GP_PROXY_TABLE_RESPONSE_IND,
  GP_NOTIFICATION_RESP_IND,
  GP_TRANSLATION_TABLE_RSP_IND,
  GP_SINK_TABLE_RESP_IND,
  ZGP_MAX_EVENTS
} ZGP_IndicationType_t;

typedef struct PACK _ZGP_CommissionedDeviceInfo_t
{
  ZGP_ApplicationId_t appId;
  ZGP_GpdId_t gpdId;
  uint8_t endPoint;
  uint8_t deviceId;
  bool commReqInfoPresent;
  zgpCommReq_t commReq;
} ZGP_CommissionedDeviceInfo_t;

typedef struct PACK _ZGP_PairingConfigCmdInfo_t
{
  ZGP_GpdAppInfo_t *appInfo;
  ZGP_ApplicationId_t appId;
  ZGP_GpdId_t gpdId;
  uint8_t gpdEndPoint;
  zgpPairingConfigActions_t action;
  zgpCommunicationMode_t commMode;
  bool functionalityMatching;
  uint8_t deviceId;
} ZGP_PairingConfigCmdInfo_t;

typedef struct
{
  ZGP_GpdAppInfo_t pairingConfigAppInfo;
  APS_Address_t dstAddr;
  ZGP_EndpointInfo_t endPointInfo;
  uint8_t deviceId;
  uint8_t payloadLength;
  uint8_t maxPayloadSize;
  uint8_t commSessionEntryIndex;
  ZGP_PairingConfigCmdInfo_t pairingConfigCmdInfo;
} ZGP_GpPairingConfigPendingInfo_t;

typedef struct PACK _ZGP_GpdCommand_t
{
  ZGP_ApplicationId_t appId;
  ZGP_GpdId_t gpdId;
  uint8_t endPoint;
  uint8_t cmdId;
  uint8_t cmdPayloadLength;
  uint8_t *cmdPayload;
} ZGP_GpdCommand_t;

typedef struct PACK _ZGP_SinkCommModeReqInfo_t
{
  ZGP_SinkCommissioningModeOptions_t options;
  uint8_t endPoint;
  bool unicastCommMode; // Need to be set by the application
} ZGP_SinkCommModeReqInfo_t;

typedef struct PACK _ZGP_ClusterCmdInd_t
{
  ZCL_Addressing_t *srcAddress;
  uint8_t payloadlength;
  uint8_t *payload;
  ZCL_Status_t retStatus;
} ZGP_ClusterCmdInd_t;

typedef struct _ZGP_IndicationInfo_t
{
  ZGP_IndicationType_t indicationType;
  union
  {
    ZGP_CommissionedDeviceInfo_t commssionedDeviceInfo;
    ZGP_PairingConfigCmdInfo_t pairingConfigInfo;
    ZGP_GpdCommand_t gpdCommand;
    ZGP_SinkCommModeReqInfo_t sinkCommModeReqInfo;
    ZGP_ClusterCmdInd_t clusterCmdInd;
  } indicationData;
} ZGP_IndicationInfo_t;

typedef enum _ZGP_TransTableIndicationType_t
{
  APP_FUNCTIONALITY_CHECK = 0x00,
  ADD_TRANS_TABLE_ENTRY = 0x01,
  REMOVE_TRANS_TABLE_ENTRY = 0x02,
  ZGP_TRANS_TABLE_MAX_EVENTS
} ZGP_TransTableIndicationType_t;

typedef struct PACK _ZGP_AppFuncCheckInfo_t
{
  uint8_t *deviceId; // May be updated by translation table handler
  ZGP_GpdAppInfo_t *appInfo;
  uint8_t *endPointList;
  uint8_t noOfEndPoints;
  bool isMatching;
} ZGP_AppFuncCheckInfo_t;

typedef enum _zgpPairingProcType_t
{
  NONE = 0x00,
  PAIRING_CONFIG_TYPE = 0x01,
  COMM_PROC_TYPE = 0x02
} zgpPairingProcType_t;

typedef struct PACK _ZGP_AddTransTableEntry_t
{
  ZGP_SinkTableEntry_t *sinkEntry;
  uint8_t noOfEndPoints;
  uint8_t *endPointList;
  ZGP_GpdAppInfo_t *appInfo;
  zgpPairingProcType_t pairingType;
} ZGP_AddTransTableEntry_t;

typedef struct PACK _ZGP_RemoveTransTableEntry_t
{
  ZGP_GpdId_t *gpdId;
  uint8_t endPoint;
  ZGP_ApplicationId_t appId;
} ZGP_RemoveTransTableEntry_t;

typedef struct _ZGP_TransTableIndicationInfo_t
{
  ZGP_TransTableIndicationType_t transTableIndType;
  union
  {
    ZGP_AppFuncCheckInfo_t appFuncCheckInfo;
    ZGP_AddTransTableEntry_t addTransTableEntry;
    ZGP_RemoveTransTableEntry_t removeTransTableEntry;
  } indicationData;
} ZGP_TransTableIndicationInfo_t;

/******************************************************************************
                    Prototypes section
 ******************************************************************************/

/**************************************************************************//**
  \brief To initialize high proxy

  \param None

  \return None.
 ******************************************************************************/
void ZGPH_ProxyBasicInit(void);


#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
/**************************************************************************//**
  \brief To initialize high sink

  \param None

  \return None.
 ******************************************************************************/
void ZGPH_SinkBasicInit(void);

/**************************************************************************//**
  \brief To put local sink in commissioning mode.

  \param sinkCommModeOptions - commissioning mode options
             endPoint - Endpoint to be put in commissioning mode
             unicastComm - unicast proxy comm. mode

  \return status.
 ******************************************************************************/
ZGP_InfraDeviceStatus_t ZGPH_PutLocalSinkInCommissioningMode(ZGP_SinkCommissioningModeOptions_t *sinkCommModeOptions, uint8_t endPoint,    bool unicastComm);

/**************************************************************************//**
  \brief To update sink entry locally on NVM based on the given action

  \param sinkEntry - entry fields(if proper security key(non-zero) is not provided,
                         this will be derived based on gpsSharedKeyType
         action - EXTEND_SINKTABLE_ENTRY/REMOVE_GPD supported

  \return status
 ******************************************************************************/
ZGP_InfraDeviceStatus_t ZGPH_UpdateLocalSinkEntry(ZGP_SinkTableEntry_t *sinkEntry, ZGP_SinkTableActions_t action);

/**************************************************************************//**
  \brief : To set sink group id used for the GPD to be commissioned

  \param sinkGroupId - sink group Id to be set

  \return Status.
 ******************************************************************************/
ZGP_InfraDeviceStatus_t ZGPH_SetSinkGroupEntry(GroupAddr_t sinkGroupId);

/**************************************************************************//**
  \brief To set assigned alias used for the GPD to be commissioned

  \param gpdAssignedAlias - The commissioned 16-bit ID to be used as alias for this GPD

  \return Status.
 ******************************************************************************/
ZGP_InfraDeviceStatus_t ZGPH_SetGPDAssignedAlias(ShortAddr_t gpdAssignedAlias);

/**************************************************************************//**
  \brief To read the sink entry based on the index

  \param entry - entry to be populated
             index -  entry index

  \return   true - entry found
            false - entry not found
 ******************************************************************************/
bool ZGPH_GetSinkTableEntryByIndex(ZGP_SinkTableEntry_t *entry, uint8_t index);


#endif // (APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC)

/**************************************************************************//**
  \brief To build and send ZGP pairing config command

  \param addrMode - address mode
         dstAddr - destination address
         pairingConfigCmdInfo - pairing config inputs
         commReqAppInfo - Application information
         endpointInfo - paired endpoints information

  \return zcl status
 ******************************************************************************/
ZGP_InfraDeviceStatus_t ZGPH_SendGpPairingConfigCmd (APS_AddrMode_t addrMode, APS_Address_t *dstAddr, ZGP_PairingConfigCmdInfo_t pairingConfigCmdInfo,
                                                       ZGP_GpdAppInfo_t *commReqAppInfo, ZGP_EndpointInfo_t endPointInfo);

/**************************************************************************//**
  \brief Sending GP pairing

  \param sinkTableEntry - Entry for which pairing is to be sent
             action - Action to Add/Update/remove
             index - Entry Index as per the groupList

  \return None
 ******************************************************************************/
void ZGPH_SendGpPairing(ZGP_SinkTableEntry_t *sinkTableEntry,ZGP_SinkTableActions_t action, uint8_t groupIndex);

#if MICROCHIP_APPLICATION_SUPPORT != 1
/**************************************************************************//**
  \brief Send ZGP cluster command in raw mode

  \param sourceEndpoint - Endpoint requesting the command
             clusterId - Cluster Id
             commandId - cluster cmd id
             frameType - Universal or Cluster specific command
             direction - Client to Server or vice versa
             disableDefaultResponse - is the default response is enabled
             manufacturerCode - Manufacture code to be used
             sequenceNumber - ZCL transaction sequence number
             zclPayloadLength - length of payload to be sent
             pZclPayload - Payload to be sent
             dstAddr - dst addr
             pNotificationCallback - function to be called when a response is received
  \return zcl status
******************************************************************************/
ZCL_Status_t ZGPH_SendCmdWithCallback(uint8_t sourceEndpoint,
                                           uint16_t clusterId,
                                           uint8_t commandId,
                                           uint8_t frameType,
                                           uint8_t direction,
                                           uint8_t disableDefaultResponse,
                                           uint16_t manufacturerCode,
                                           uint8_t sequenceNumber,
                                           uint8_t zclPayloadLength,
                                           const uint8_t* pZclPayload,
                                           ZCL_Addressing_t *dstAddr,
                                           Notify pNotificationCallback);
#endif

#endif // _GREENPOWER_SUPPORT_
#endif // _ZGPINFRADEVICE_H
// eof zgpInfraDevice.h
