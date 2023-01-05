/*******************************************************************************
  Zigbee green power cluster server Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpClusterServer.c

  Summary:
    This file contains the green power cluster server side implementation.

  Description:
    This file contains the green power cluster server side implementation.
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

#ifdef _GREENPOWER_SUPPORT_
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC

/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcl.h>
#include <zcl/include/zclParser.h>
#include <zgp/include/zgpCommon.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLow.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighGeneric.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighMem.h>
#include <zcl/include/zclGreenPowerCluster.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterGeneric.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterServer.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpCluster.h>
#include <zgp/include/zgpDbg.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpSinkBasic.h>
#include <zcl/include/zclAttributes.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpSinkTable.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighGeneric.h>
#include <systemenvironment/include/sysTypes.h>
#include <pds/include/wlPdsMemIds.h>

/******************************************************************************
                    Define section
******************************************************************************/
#define MINIMUM_GP_NOTIFICATION_LENGTH 12 // 2 - Options, 4 - GPD Id, 4 - SecurityFrameCounter,
                                          // 1 - cmdId, 1 - cmdPayloadlength(octet string)
#define GP_LINK_KEY_DEFAULT_VALUE "\x5a\x69\x67\x42\x65\x65\x41\x6c\x6c\x69\x61\x6e\x63\x65\x30\x39"
#define GP_SHARED_KEY_INIT_VALUE  "\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
#define MINIMUM_SINK_TABLE_REQ_LENGTH   2// 1 - options 1 -index

#define VALID_GPM_ADDRESS 0xffff
#define ZGP_INVALID_REPORT_COUNT 0xff

#if MICROCHIP_APPLICATION_SUPPORT == 1
#define GP_PAIRING_SEARCH_COMMAND_IND NULL
#define GP_TUNNELING_STOP_COMMAND_IND NULL
#else
#define GP_PAIRING_SEARCH_COMMAND_IND gpPairingSearchCommandInd
#define GP_TUNNELING_STOP_COMMAND_IND gpTunnelingStopCommandInd
#endif
/******************************************************************************
                    Prototypes section
******************************************************************************/
static uint16_t formGpdfFrameFromNotification(uint8_t cmdId, uint8_t payloadLength, uint8_t *payload, \
                                            ZGP_LowDataInd_t *gpdfDataInd);
static ZCL_Status_t gpNotificationHandling(uint8_t cmdId, ZCL_Addressing_t *addressing, uint8_t payloadLength, \
                                           uint8_t *payload);
static ZCL_Status_t gpNotificationInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
static ZCL_Status_t gpCommissioningNotificationInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
static ZCL_Status_t gpSinkCommissioningModeInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);

static ZCL_Status_t gpPairingConfigurationInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
static ZCL_Status_t gpSinkTableRequestInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
static ZCL_Status_t gpProxyTableResponseInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
static uint8_t gpParsingGpNotificationInd(uint8_t cmdId, uint8_t payloadLength, uint8_t *payload, \
                                          ZGP_GpNotification_t *gpNotification);
static ZCL_Status_t gpPairingConfigHandler(uint8_t payloadLength, uint8_t *payload, ZCL_Addressing_t *addressing);
static void zgpSinkAttrEventInd(ZCL_Addressing_t *addressing, ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event);
static bool triggerPairingConfigRxdEvent(zgpPairingConfigContextInfo_t *pairingConfigContextInfo);
static bool triggerSinkCommissioningModeReqRxdEvent(ZGP_SinkCommissioningModeOptions_t *options, uint8_t endPoint);
static void stackEventObserver(SYS_EventId_t id, SYS_EventData_t data);
static ZCL_Status_t gpTranslationTableUpdateCommandInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
static ZCL_Status_t gpTranslationTableRequestCommandInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
#if MICROCHIP_APPLICATION_SUPPORT != 1
static ZCL_Status_t gpPairingSearchCommandInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
static ZCL_Status_t gpTunnelingStopCommandInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
#endif
static ZCL_Status_t parseBasicInfoFromPayload(zgpPairingConfigContextInfo_t *pairingConfigContextInfo, uint8_t *payload,\
                                              uint8_t rxdlength, uint16_t *fieldLength);
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
static void pairingConfigSessionTimeoutCallback(void);
static ZCL_Status_t parseNonAppDescriptionActionCmd(zgpPairingConfigContextInfo_t *pairingConfigContextInfo, uint8_t payloadLength, \
                                       uint8_t *payload, uint8_t payloadIndex);
static ZCL_Status_t parseAppDescriptionActionCmd(zgpPairingConfigContextInfo_t *pairingConfigContextInfo, uint8_t payloadLength, \
                                       uint8_t *payload, uint8_t payloadIndex);
static void processForPairingCompletion(zgpPairingConfigContextInfo_t *pairingConfigContextInfo);
#endif
static ZCL_Status_t filterRxdActionAndGetSessionTableIndex(zgpPairingConfigContextInfo_t *pairingConfigContextInfo, \
                                                   ZCL_Addressing_t *addressing);
static ZCL_Status_t processActionOnCompletePairing(zgpPairingConfigContextInfo_t *pairingConfigContextInfo);

/******************************************************************************
                    Global variables
******************************************************************************/
const ZCL_GreenPowerClusterCommands_t zgpClusterServerCommands =
{
  ZCL_DEFINE_GP_CLUSTER_COMMANDS(gpNotificationInd, GP_PAIRING_SEARCH_COMMAND_IND, GP_TUNNELING_STOP_COMMAND_IND, gpCommissioningNotificationInd, \
      gpSinkCommissioningModeInd, gpTranslationTableUpdateCommandInd, gpTranslationTableRequestCommandInd, gpPairingConfigurationInd, \
      gpSinkTableRequestInd, gpProxyTableResponseInd, NULL, NULL, NULL, NULL, NULL, NULL, NULL)
};

ZCL_GreenPowerClusterServerAttributes_t zgpClusterServerAttributes =
{
  ZCL_DEFINE_GP_CLUSTER_SERVER_ATTRIBUTES()
};

ClusterId_t zgpServerClusterIds[1] =
{
  GREEN_POWER_CLUSTER_ID
};

ZCL_Cluster_t zgpServerClusters[1] =
{
  DEFINE_GP_CLUSTER(ZCL_SERVER_CLUSTER_TYPE,
      &zgpClusterServerAttributes, &zgpClusterServerCommands)
};
static SYS_EventReceiver_t stackEventReceiver = { .func = stackEventObserver};
// buffer shared between cluster server and sink for processing rxd gpdf
// It runs wihtout context break so no flag is required to sync. up
zgpSinkRxdGpdfIndBuffer_t zgpRxdGpdfIndBuffer;
zgpCommReqPairingConfigSessionEntry_t commSessionEntries[ZGP_TOTAL_NO_COMM_SESSIONS];
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
static ZGP_ReportDescriptor_t reportDescBuffer[ZGP_MAX_NO_OF_REPORTS];
static ZGP_DataPointDescriptor_t dataPointBufffer[ZGP_MAX_NO_OF_DATA_POINT_DESCRIPTORS];
static HAL_AppTimer_t pairingConfigSessionTimer = {.callback = pairingConfigSessionTimeoutCallback,\
                                                   .mode = TIMER_ONE_SHOT_MODE
                                                   };
#endif

/******************************************************************************
                    Implementations section
******************************************************************************/

/**************************************************************************//**
\brief Initialize Green power cluster.
******************************************************************************/
void zgpClusterServerInit(void)
{
  uint32_t gpsFunctionality = GPS_FEATURE | GPS_STUB | GPS_DERIVED_GROUP_COMM | GPS_PRE_COMM_GROUP_COMM | \
                              GPS_COMPACT_ATTRIBUTE_REPORTING | \
                             GPS_LIGHTWEIGHT_UNICAST_COMM | GPS_PROXIMITY_COMMISSIONING | GPS_CT_BASED_COMMISSIONING | \
                             GPS_MULTIHOP_COMMISSIONING | GPS_SECURITY_LEVEL_0 | GPS_SECURITY_LEVEL_2 | GPS_SECURITY_LEVEL_3 | GPS_IEEE_ADDR;
  uint32_t gpsActiveFunctionality = GPS_GP_FUNCTIONALITY | GPS_ACTIVE_FUNCTIONALITY_FIXED_VALUE;
  ZCL_Cluster_t *cluster = ZCL_GetCluster(GREEN_POWER_ENDPOINT, GREEN_POWER_CLUSTER_ID, ZCL_CLUSTER_SIDE_SERVER);

  if (cluster)
  {
    cluster->ZCL_AttributeEventInd = zgpSinkAttrEventInd;
  }
  memcpy(&zgpClusterServerAttributes.gpsFunctionality.value[0], (void *)&gpsFunctionality,
         sizeof(zgpClusterServerAttributes.gpsFunctionality.value));
  memcpy(&zgpClusterServerAttributes.gpsActiveFunctionality.value[0],
         (void *)&gpsActiveFunctionality, sizeof(zgpClusterServerAttributes.gpsActiveFunctionality.value));
  zgpClusterServerAttributes.gpsMaxSinkTableEntries.value = ZGP_SINK_TABLE_SIZE;
  zgpClusterServerAttributes.clusterRevision.value = GP_CLUSTER_REVISION;
  // Initialize long octet string length fields for sink table and blocked gpdId
  memset(&zgpClusterServerAttributes.sinkTable.value[0], 0x00,
         sizeof(zgpClusterServerAttributes.sinkTable.value));
  zgpClusterServerAttributes.gpSharedSecurityKeyType.value = ZGP_KEY_TYPE_NO_KEY;
  memcpy(&zgpClusterServerAttributes.gpLinkKey.value[0], (uint8_t *)GP_LINK_KEY_DEFAULT_VALUE ,
         sizeof(zgpClusterServerAttributes.gpLinkKey.value));
  memcpy(&zgpClusterServerAttributes.gpSharedSecuritykey.value[0], (uint8_t *)GP_SHARED_KEY_INIT_VALUE ,
         sizeof(zgpClusterServerAttributes.gpSharedSecuritykey.value));

  // Setting minGpdSecurityLevel to (0x02), protectionWithGpLinkKey enabled
  zgpClusterServerAttributes.gpsSecurityLevel.value = ZGP_SINK_DEFAULT_SECURITY_LEVEL_ATTR_VALUE;
  zgpClusterServerAttributes.gpsCommunicationMode.value = DERIVED_GROUPCAST;
  zgpClusterServerAttributes.gpsCommissioningExitMode.value = GPS_EXIT_ON_FIRST_PAIRING_SUCCESS;
  zgpClusterServerAttributes.gpsCommissioningWindow.value = COMMISSIONING_WINDOW_DEFAULT_VALUE_IN_MSEC/1000;

  if (PDS_IsAbleToRestore(ZGP_SINK_ATTR_MEM_ID))
  {
    // For NFN device, we are able to restore sink attr id
    PDS_Restore(ZGP_SINK_ATTR_MEM_ID);
  }
  else
  {
    // Subcribe to stack event on forming/entering network for FN device
    SYS_SubscribeToEvent(BC_EVENT_NETWORK_STARTED, &stackEventReceiver);
    SYS_SubscribeToEvent(BC_EVENT_NETWORK_ENTERED, &stackEventReceiver);
  }

  memset(&commSessionEntries[0], 0x00, sizeof(commSessionEntries));
  memset(&zgpRxdGpdfIndBuffer, 0x00, sizeof(zgpRxdGpdfIndBuffer));
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT  
  // since the report id - 0xFF means free entry
  memset(&reportDescBuffer[0], 0xFF, sizeof(reportDescBuffer));
  memset(&dataPointBufffer[0], 0x00, sizeof(dataPointBufffer));
#endif
}

/**************************************************************************//**
\brief To fetch shared secuirty key
\param[in] none
\return value of shared security key
******************************************************************************/
uint8_t *zgpServerGetSharedSecurityKey(void)
{
  return &zgpClusterServerAttributes.gpSharedSecuritykey.value[0];
}

/**************************************************************************//**
\brief To fetch gpSharedSecurityTYpe
\param[in] none
\return type of shared security key
******************************************************************************/
ZGP_SecKeyType_t zgpServerGetSharedSecurityKeyType(void)
{
  return (ZGP_SecKeyType_t)zgpClusterServerAttributes.gpSharedSecurityKeyType.value;
}

/**************************************************************************//**
  \brief Handler of stack events(joining)

  \param[in] id - event id
  \param[in] data - event data

  \return None.
******************************************************************************/
static void stackEventObserver(SYS_EventId_t id, SYS_EventData_t data)
{
  ExtAddr_t trustCenterAddress;
  ZGP_GpsSecurityLevelAttribute_t *secLevel = (ZGP_GpsSecurityLevelAttribute_t *)&zgpClusterServerAttributes.gpsSecurityLevel.value;
  bool icBasedLinkKeyType;

  CS_ReadParameter(CS_APS_TRUST_CENTER_ADDRESS_ID, &trustCenterAddress);
  CS_ReadParameter(CS_INSTALL_CODE_BASED_JOIN_LINK_KEY_TYPE_ID, &icBasedLinkKeyType);

  secLevel->involveTc = 0;
  // This is called for NETWORK_ENTERED/NETWORK_STARTED events
  // check the secuirty type(distributed/centralized) along with IC code then
  // update the attribute
  if (!IS_EQ_EXT_ADDR(APS_BROADCAST_ALL_EXT_ADDRESS, trustCenterAddress) && \
      icBasedLinkKeyType)
  {
    secLevel->involveTc = 1;
  }
  // Need to store the attribute
  PDS_Store(ZGP_SINK_ATTR_MEM_ID);
}

/**************************************************************************//**
 \brief Build the sinkTableEntry in OTA format

  \param[in] sinkTableOTA - payload to be populated
             sinkTableEntry - sink table entry

  \return payload length
******************************************************************************/
uint16_t zgpsBuildOTASinkTableEntry( uint8_t *sinkTableOTA, ZGP_SinkTableEntry_t *sinkTableEntry)
{
  // for OTA format of sink table entry refer A.3.3.2.2.1 Over the air transmission of Sink Table
  uint16_t otaEntryIndex = 0;
  zgpSinkTableEntryOptions_t options;
  //copy options field
  options = sinkTableEntry->options;
  memcpy(&sinkTableOTA[otaEntryIndex], &options, sizeof(zgpSinkTableEntryOptions_t));
  otaEntryIndex += sizeof(zgpSinkTableEntryOptions_t);

  // copy GPD address
  otaEntryIndex += zgpAddGpdSourceAddrToOtaPayload((ZGP_ApplicationId_t)options.appId, &sinkTableEntry->tableGenericInfo,\
                         &sinkTableOTA[otaEntryIndex]);
  //Adding Device ID
   sinkTableOTA[otaEntryIndex++] = sinkTableEntry->deviceId;

   //  sink group address list
  if(options.communicationMode == PRECOMMISSIONED_GROUPCAST)
  {
    otaEntryIndex += zgpAddPreCommissionedGroupList(&sinkTableEntry->tableGenericInfo, &sinkTableOTA[otaEntryIndex]);
  }

  // Assigned Alias
  if(options.assignedAlias)
  {
    uint16_t assignedAlias;
    assignedAlias = sinkTableEntry->tableGenericInfo.gpdAssignedAlias;
    memcpy(&sinkTableOTA[otaEntryIndex], &assignedAlias, sizeof(assignedAlias));
    otaEntryIndex += sizeof(assignedAlias);
  }

  // Adding groupcast radius
  sinkTableOTA[otaEntryIndex++] = sinkTableEntry->tableGenericInfo.groupCastRadius;

  otaEntryIndex += zgpGenericAddSecurityFields(&sinkTableOTA[otaEntryIndex], &sinkTableEntry->tableGenericInfo, \
                                             options.securityUse, options.seqNumCapabilities);

  return otaEntryIndex;
}

/**************************************************************************//**
  \brief Form the GPDF from the received GP commissioning notification/GP notification

  \param[in] cmdId - cluster command id
             payloadLength - rxd payload length
             payload - payload
             gpdfDataInd - dstub data indication

  \return proxy addr
******************************************************************************/
static uint16_t formGpdfFrameFromNotification(uint8_t cmdId, uint8_t payloadLength, uint8_t *payload, \
                                            ZGP_LowDataInd_t *gpdfDataInd)
{
  uint8_t maxPayloadLength = ZGP_MAX_GSDU_SIZE;
  ZGP_GpNotification_t gpNotification;
  uint8_t cmdPayloadLength = 0;

  memset(&gpNotification, 0x00, sizeof(ZGP_GpNotification_t));
  cmdPayloadLength = gpParsingGpNotificationInd(cmdId, payloadLength, payload, &gpNotification);

  if (INVALID_CMD_LENGTH == cmdPayloadLength)
    return NWK_NO_SHORT_ADDR;

  gpdfDataInd->status = ZGP_DSTUB_SECURITY_SUCCESS;
  gpdfDataInd->applicationId = (ZGP_ApplicationId_t)gpNotification.options.commNotifyOptions.appId;
  gpdfDataInd->autoCommissioning = false;
  gpdfDataInd->endPoint = gpNotification.endpoint;

  gpdfDataInd->gpdSecurityFrameCounter = gpNotification.gpdSecurityFrameCounter;
  gpdfDataInd->seqNumber = gpNotification.gpdSecurityFrameCounter;
  gpdfDataInd->linkQuality = gpNotification.gppGpdLink;
  gpdfDataInd->srcPanId = 0xffff;
  gpdfDataInd->status = ZGP_DSTUB_SECURITY_SUCCESS;
  gpdfDataInd->frameType = ZGP_FRAME_DATA;

  if (ZGP_SRC_APPID == gpdfDataInd->applicationId)
  {
    gpdfDataInd->srcAddrMode = MAC_NO_ADDR_MODE;
    gpdfDataInd->srcId = gpNotification.gpdId.gpdSrcId;
    if (gpdfDataInd->srcId >= ZGP_FIRST_RESERVED_SRC_ID)
      return NWK_NO_SHORT_ADDR;
    if (ZGP_MAINTENANCE_FRAME_SRC_ID == gpdfDataInd->srcId)
      gpdfDataInd->frameType = ZGP_FRAME_MAINTENANCE;
  }
  else
  {
    maxPayloadLength = maxPayloadLength - sizeof(gpdfDataInd->srcId) + sizeof(gpdfDataInd->srcAddress.ext) \
                       + sizeof(gpdfDataInd->endPoint);
    gpdfDataInd->srcAddrMode = MAC_EXT_ADDR_MODE;
    gpdfDataInd->srcAddress.ext = gpNotification.gpdId.gpdIeeeAddr;
  }

  if (cmdPayloadLength > maxPayloadLength)
    return NWK_NO_SHORT_ADDR;

  gpdfDataInd->gpdCommandId = gpNotification.gpdCmdId;


  if (ZCL_GP_CLUSTER_SERVER_GP_COMMISSIONING_NOTIFICATION_COMMAND_ID == cmdId)
  {
    if (gpNotification.options.commNotifyOptions.securityProcFailed)
    {
      gpdfDataInd->status = ZGP_DSTUB_UNPROCESSED;
      gpdfDataInd->mic = gpNotification.mic;

    }
    gpdfDataInd->gpdfSecurityLevel = (ZGP_SecLevel_t)gpNotification.options.commNotifyOptions.securityLevel;
    gpdfDataInd->gpdfKeyType = (ZGP_SecKeyType_t)gpNotification.options.commNotifyOptions.securityKeyType;
    gpdfDataInd->rxAfterTx = gpNotification.options.commNotifyOptions.rxAfterTx;
  }
  else
  {
    gpdfDataInd->gpdfSecurityLevel = (ZGP_SecLevel_t)gpNotification.options.notifyOptions.securityLevel;
    gpdfDataInd->gpdfKeyType = (ZGP_SecKeyType_t)gpNotification.options.notifyOptions.securityKeyType;
    gpdfDataInd->rxAfterTx = gpNotification.options.notifyOptions.rxAfterTx;
  }

  if ((ZGP_SECURITY_LEVEL_1 == gpdfDataInd->gpdfSecurityLevel) || \
      ((ZGP_KEY_TYPE_RESERVED1 == gpdfDataInd->gpdfKeyType) || (ZGP_KEY_TYPE_RESERVED2 == gpdfDataInd->gpdfKeyType)))
    return NWK_NO_SHORT_ADDR;

  memcpy(gpdfDataInd->gpdAsdu, gpNotification.gpdCmdPayload, cmdPayloadLength);
  gpdfDataInd->gpdAsduLength = cmdPayloadLength;

  return gpNotification.gppShortAddr;
}

/**************************************************************************//**
  \brief GP notification/commissioning notification handling

  \param[in] cmdId - COMMISSIONING NOTIFICATION/ NOTIFICATION
             addressing - addressing info
             payloadlength & payload - payload info

  \return zcl status
******************************************************************************/
static ZCL_Status_t gpNotificationHandling(uint8_t cmdId, ZCL_Addressing_t *addressing, uint8_t payloadLength, \
                                           uint8_t *payload)
{
  uint16_t proxyAddr = 0;

  memset(&zgpRxdGpdfIndBuffer.gpdPayload[0], 0x00, sizeof(zgpRxdGpdfIndBuffer.gpdPayload));
  // Init. dstub and its payload pointer  
  memset(&zgpRxdGpdfIndBuffer.dstubDataInd, 0x00, sizeof(zgpRxdGpdfIndBuffer.dstubDataInd));
  zgpRxdGpdfIndBuffer.dstubDataInd.gpdAsdu = &zgpRxdGpdfIndBuffer.gpdPayload[0];

  proxyAddr = formGpdfFrameFromNotification(cmdId, payloadLength, payload,\
                                               &zgpRxdGpdfIndBuffer.dstubDataInd);

  if (NWK_NO_SHORT_ADDR == proxyAddr)
    return ZCL_MALFORMED_COMMAND_STATUS;

  if (ZCL_GP_CLUSTER_SERVER_GP_NOTIFICATION_COMMAND_ID == cmdId)
  {
    ZGP_TableOperationField_t  tableOperationField = {.entryType = ZGP_SINK_ENTRY, .commMode = ALL_COMMUNICATION_MODE, \
                               .appId = zgpRxdGpdfIndBuffer.dstubDataInd.applicationId, .nonEmptyIndexForRead = ZGP_ENTRY_INVALID_INDEX};
    ZGP_SinkTableEntry_t *sinkEntry = (ZGP_SinkTableEntry_t *)zgpGetMemReqBuffer();
    uint8_t validCommModeMask = (1u <<  DERIVED_GROUPCAST) | (1u << PRECOMMISSIONED_GROUPCAST);
    bool frameInValidCommMode = true;
    ZGP_GpdId_t gpdId;

    gpdId.gpdIeeeAddr = zgpRxdGpdfIndBuffer.dstubDataInd.srcAddress.ext;
    if (ZGP_SRC_APPID == zgpRxdGpdfIndBuffer.dstubDataInd.applicationId)
      gpdId.gpdSrcId = zgpRxdGpdfIndBuffer.dstubDataInd.srcId;

    if (APS_SHORT_ADDRESS == addressing->indDstAddrMode)
    {
      if (BROADCAST_ADDR_RX_ON_WHEN_IDLE == addressing->indDstAddr.shortAddress)
      {
        zgpFreeMemReqBuffer();
        return ZCL_INVALID_FIELD_STATUS;
      }
      validCommModeMask = (1u << LIGHTWEIGHT_UNICAST) | (1u << FULL_UNICAST);
    }

    if (ACTIVE_ENTRY_AVAILABLE != ZGPL_ReadTableEntryFromNvm((void *)sinkEntry, tableOperationField, &gpdId, zgpRxdGpdfIndBuffer.dstubDataInd.endPoint))
    {
#if (MICROCHIP_APPLICATION_SUPPORT == 1)
      appSnprintf("ZGP - SinkEntryNotAvailable\r\n");
#endif
      frameInValidCommMode = false;
    }
    else if (!(validCommModeMask & sinkEntry->commModeMask))
    {
      frameInValidCommMode = false;
      if (APS_SHORT_ADDRESS == addressing->indDstAddrMode)
      {
        // As per the spec. A.3.5.2.4 - Gp notification handling based on comm. mode
        // Need to send gpPairing commands
        // addSink -> 1 with supported communication mode
        zgpServerSendGpPairingForEntry(sinkEntry, EXTEND_SINKTABLE_ENTRY);
        // addSink -> 0 with unsupported communication mode
        // Currently we support only lightweight so sending the same for REMOVE_PAIRING
        sinkEntry->options.communicationMode = LIGHTWEIGHT_UNICAST;
        zgpServerSendGpPairingForEntry(sinkEntry, REMOVE_PAIRING);
      }
    }

    zgpFreeMemReqBuffer();
    if (!frameInValidCommMode)
      return ZCL_SUCCESS_STATUS;
  }
  if (!ZGPL_CheckForDuplicate(&zgpRxdGpdfIndBuffer.dstubDataInd))
    return ZCL_SUCCESS_STATUS;

  /* We should not drop GP Notifications in the commissioning mode as there may be other devices
  in the network trying to interact with the device. For example, ON/OFF commands coming in while other
  switch is being commissioned */
  if ((OPERATIONAL_MODE == ZGPL_GetDeviceMode(false)) && (cmdId == ZCL_GP_CLUSTER_SERVER_GP_COMMISSIONING_NOTIFICATION_COMMAND_ID))
    return ZCL_FAILURE_STATUS;

  zgpSinkGpdfHandling(&zgpRxdGpdfIndBuffer.dstubDataInd, false, proxyAddr);
  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
  \brief GP notification indication

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return zcl status
******************************************************************************/
static ZCL_Status_t gpNotificationInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  ZCL_Status_t retStatus = gpNotificationHandling(ZCL_GP_CLUSTER_SERVER_GP_NOTIFICATION_COMMAND_ID, addressing, payloadLength, payload);

  if (addressing->nonUnicast)
    return ZCL_SUCCESS_STATUS;
  else
    return retStatus;
}

/**************************************************************************//**
  \brief GP commissioning notification indication

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return zcl status
******************************************************************************/
static ZCL_Status_t gpCommissioningNotificationInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  ZCL_Status_t retStatus = gpNotificationHandling(ZCL_GP_CLUSTER_SERVER_GP_COMMISSIONING_NOTIFICATION_COMMAND_ID, addressing, payloadLength, payload);

  if (addressing->nonUnicast)
    return ZCL_SUCCESS_STATUS;
  else
    return retStatus;
}

/**************************************************************************//**
  \brief Parsing the received GP commissioning notification/GP notification

  \param[in] cmdId - commissioning notification/GP notification
             payloadlength & payload - payload info
             gpNotification - notification structure to be populated

  \return commandLength - INVALID_CMD_LENGTH - invalid command frame
                          otherwise valid commandLength
******************************************************************************/
static uint8_t gpParsingGpNotificationInd(uint8_t cmdId, uint8_t payloadLength, uint8_t *payload, \
                                          ZGP_GpNotification_t *gpNotification)
{
  bool micPresent = false;
  bool proxyInfoPresent = false;
  uint32_t gpdSecurityFrameCounter;
  uint8_t cmdLength;
  uint8_t payloadIndex = 0;

  memcpy(&gpNotification->options.commNotifyOptions, &payload[payloadIndex], sizeof(gpNotification->options.commNotifyOptions));
  payloadIndex += sizeof(gpNotification->options.commNotifyOptions);

  if (ZCL_GP_CLUSTER_SERVER_GP_COMMISSIONING_NOTIFICATION_COMMAND_ID == cmdId)
  {
    if (gpNotification->options.commNotifyOptions.securityProcFailed)
    {
      micPresent = true;
    }
    if (gpNotification->options.commNotifyOptions.proxyInfoPresent)
    {
      proxyInfoPresent = true;
    }
  }
  else
  {
    if (gpNotification->options.notifyOptions.proxyInfoPresent)
    {
      proxyInfoPresent = true;
    }
  }

  {
    uint8_t gpdIdLength = ZGPH_ParseGpdIdFromPayload((ZGP_ApplicationId_t)gpNotification->options.commNotifyOptions.appId, &gpNotification->gpdId, \
                                                                &gpNotification->endpoint, &payload[payloadIndex]);
    if (INVALID_CMD_LENGTH == gpdIdLength)
      return INVALID_CMD_LENGTH;
    payloadIndex += gpdIdLength;
  }

  memcpy(&gpdSecurityFrameCounter, &payload[payloadIndex], sizeof(gpdSecurityFrameCounter));
  gpNotification->gpdSecurityFrameCounter = gpdSecurityFrameCounter;
  payloadIndex += sizeof(gpdSecurityFrameCounter);

  gpNotification->gpdCmdId = payload[payloadIndex++];

  cmdLength = payload[payloadIndex++];

  if (payloadIndex > payloadLength)
    return INVALID_CMD_LENGTH;

  // Parsing octet string
  if (cmdLength && (cmdLength != INVALID_CMD_LENGTH))
  {
    if ((payloadLength - payloadIndex) < cmdLength)
      return INVALID_CMD_LENGTH;
    memcpy(gpNotification->gpdCmdPayload, &payload[payloadIndex], cmdLength);

    payloadIndex += cmdLength;
  }
  else
    cmdLength = 0x00;

  if (proxyInfoPresent)
  {
    uint16_t proxyShortAddr;

    memcpy(&proxyShortAddr, &payload[payloadIndex], sizeof(proxyShortAddr));
    gpNotification->gppShortAddr = proxyShortAddr;

    if (!IS_CORRECT_SHORT_ADDR(proxyShortAddr))
      return INVALID_CMD_LENGTH;

    payloadIndex += sizeof(proxyShortAddr);

    gpNotification->gppGpdLink = payload[payloadIndex++];
  }

  if (micPresent)
  {
    uint32_t mic;

    memcpy(&mic, &payload[payloadIndex], sizeof(mic));
    gpNotification->mic = mic;
    payloadIndex += sizeof(mic);
  }

  if (payloadIndex > payloadLength)
    return INVALID_CMD_LENGTH;
  else
    return cmdLength;
}

/**************************************************************************//**
  \brief GP sink commissioning mode

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return success\failure.
******************************************************************************/
static ZCL_Status_t gpSinkCommissioningModeInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  ZGP_SinkCommissioningMode_t sinkCommModeReq;
  bool proxyUnicastCommMode = false;
  ZGP_InfraDeviceStatus_t retStatus;

  if (sizeof(ZGP_SinkCommissioningMode_t) > payloadLength)
    return ZCL_MALFORMED_COMMAND_STATUS;

  memcpy((void *)&sinkCommModeReq, payload, sizeof(ZGP_SinkCommissioningMode_t));

  if ((VALID_GPM_ADDRESS != sinkCommModeReq.gpmAddrForPairing) || (VALID_GPM_ADDRESS != sinkCommModeReq.gpmAddrForSec) || \
      (sinkCommModeReq.options.involveGPMinPairing) || (sinkCommModeReq.options.involveGPMinSecurity) || (GREEN_POWER_ENDPOINT == sinkCommModeReq.sinkEndPoint))
    return ZCL_INVALID_FIELD_STATUS;

  if (triggerSinkCommissioningModeReqRxdEvent(&sinkCommModeReq.options, sinkCommModeReq.sinkEndPoint))
    proxyUnicastCommMode = true;

  retStatus =  ZGPH_PutLocalSinkInCommissioningMode(&sinkCommModeReq.options, sinkCommModeReq.sinkEndPoint, \
                         proxyUnicastCommMode);

  if (ZGP_SUCCESS == retStatus)
  {
    return ZCL_SUCCESS_STATUS;
  }
  else
  {
    return ZCL_FAILURE_STATUS;
  }
}

/**************************************************************************//**
  \brief GP translation table update indication

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return success or failure.
******************************************************************************/
static ZCL_Status_t gpTranslationTableUpdateCommandInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  return zgpClusterCommandHandler(GP_TRANSLATION_TABLE_UPDATE_COMMAND_IND, addressing, payloadLength, payload);
}

/**************************************************************************//**
  \brief GP translation table request indication

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return success or failure.
******************************************************************************/
static ZCL_Status_t gpTranslationTableRequestCommandInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  return zgpClusterCommandHandler(GP_TRANSLATION_TABLE_REQUEST_COMMAND_IND, addressing, payloadLength, payload);
}

/**************************************************************************//**
  \brief GP pairing configuration indication

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return status.
******************************************************************************/
static ZCL_Status_t gpPairingConfigurationInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  ZCL_Status_t status;

  status = gpPairingConfigHandler(payloadLength,payload, addressing);

  if (addressing->nonUnicast)
    return ZCL_SUCCESS_STATUS;
  else
    return status;
}

/**************************************************************************//**
  \brief GP sink table request

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return status - status of the request.
******************************************************************************/
static ZCL_Status_t gpSinkTableRequestInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  return zgpClusterGenericTableReqIndHandling(addressing, payloadLength, payload, false);
}

/**************************************************************************//**
  \brief GP proxy table response indication

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return status - status of the request..
******************************************************************************/
static ZCL_Status_t gpProxyTableResponseInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  return zgpClusterCommandHandler(GP_PROXY_TABLE_RESPONSE_IND, addressing, payloadLength, payload);
}

#if MICROCHIP_APPLICATION_SUPPORT != 1
/**************************************************************************//**
  \brief GP pairing search indication

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return status - status of the request..
******************************************************************************/
static ZCL_Status_t gpPairingSearchCommandInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  return zgpClusterCommandHandler(GP_PAIRING_SEARCH_IND, addressing, payloadLength, payload);
}

/**************************************************************************//**
  \brief GP pairing search indication

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return status - status of the request..
******************************************************************************/
static ZCL_Status_t gpTunnelingStopCommandInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  return zgpClusterCommandHandler(GP_TUNNELING_STOP_IND, addressing, payloadLength, payload);
}
#endif

/**************************************************************************//**
  \brief parse basic information from the payload

  \param[in] pairingConfigContextInfo - pairing config context information
             payload - payload info
             rxdlength - length of rxd payload
             fieldLength - length of the field to be parsed

  \return status after processing
******************************************************************************/
static ZCL_Status_t parseBasicInfoFromPayload(zgpPairingConfigContextInfo_t *pairingConfigContextInfo, uint8_t *payload,\
                                              uint8_t rxdlength, uint16_t *fieldLength)
{
  uint8_t payloadIndex = 0;
  zgpPairingContextBasicInfo_t *basicContextInfo = &pairingConfigContextInfo->basicContextInfo;

  memcpy(&pairingConfigContextInfo->actions, &payload[payloadIndex], sizeof(pairingConfigContextInfo->actions));
  payloadIndex += sizeof(pairingConfigContextInfo->actions);

  memcpy(&pairingConfigContextInfo->options, &payload[payloadIndex], sizeof(pairingConfigContextInfo->options));
  payloadIndex += sizeof(pairingConfigContextInfo->options);

  if((pairingConfigContextInfo->actions.action > APPLICATION_DESCRIPTION) || \
     (((EXTEND_SINKTABLE_ENTRY == pairingConfigContextInfo->actions.action) || (REPLACE_SINKTABLE_ENTRY == pairingConfigContextInfo->actions.action) || \
       (REMOVE_PAIRING == pairingConfigContextInfo->actions.action)) && (FULL_UNICAST == pairingConfigContextInfo->options.communicationMode)) || \
     ((ZGP_SRC_APPID != pairingConfigContextInfo->options.appId) && (ZGP_IEEE_ADDR_APPID != pairingConfigContextInfo->options.appId)))
    return ZCL_INVALID_FIELD_STATUS;

  if(ZGP_SRC_APPID == pairingConfigContextInfo->options.appId)
  {
    uint32_t srcId;
    memcpy(&srcId, &payload[payloadIndex], sizeof(srcId));
    basicContextInfo->gpdId.gpdSrcId = srcId;
    //if Src ID = 0x00000000 don't update sinkTable
    if((!ZGPL_IsValidSrcId(basicContextInfo->gpdId.gpdSrcId, ZGP_FRAME_DATA, true))
#if MICROCHIP_APPLICATION_SUPPORT != 1
     || (basicContextInfo->gpdId.gpdSrcId == ZGP_ALL_SRC_ID)
#endif
      )
    {
      return ZCL_INVALID_FIELD_STATUS;
    }

    payloadIndex += sizeof(srcId);
  }
  else if(ZGP_IEEE_ADDR_APPID == pairingConfigContextInfo->options.appId)
  {
    ExtAddr_t extAddr;
    memcpy(&extAddr, &payload[payloadIndex], sizeof(extAddr));
    basicContextInfo->gpdId.gpdIeeeAddr = extAddr;
    // if  IeeeAddr = 0x0000000000000000 drop the frame don't update sinkTable
    if((basicContextInfo->gpdId.gpdIeeeAddr == 0x0000000000000000)
#if MICROCHIP_APPLICATION_SUPPORT != 1
        || (basicContextInfo->gpdId.gpdIeeeAddr == ZGP_ALL_IEEE_ADDR)
#endif
      )
    {
      return ZCL_INVALID_FIELD_STATUS;
    }
    payloadIndex +=  sizeof(extAddr);
    basicContextInfo->endPoint = payload[payloadIndex];
    payloadIndex++;
  }

  *fieldLength = payloadIndex;
  if (rxdlength < payloadIndex)
    return ZCL_MALFORMED_COMMAND_STATUS;
  else
  {
    pairingConfigContextInfo->pairingConfigState = PAIRING_CONFIG_NON_APP_DESC_ACTION_FIELD_PARSING;
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
    if (APPLICATION_DESCRIPTION == pairingConfigContextInfo->actions.action)
      pairingConfigContextInfo->pairingConfigState = PAIRING_CONFIG_APP_DESC_ACTION_FIELD_PARSING;
#endif
    return ZCL_SUCCESS_STATUS;
  }
}

#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
/**************************************************************************//**
  \brief callback of session timer

  \param[in] None

  \return None
******************************************************************************/
static void pairingConfigSessionTimeoutCallback(void)
{
  uint32_t currentTimestamp = halGetTimeOfAppTimer();
  uint32_t minTimeout = ~0L;

  for (uint8_t index = 0; index < ZGP_TOTAL_NO_COMM_SESSIONS; index++)
  {
    if (PAIRING_CONFIG_ENTRY == commSessionEntries[index].entryStatus)
    {
      uint32_t elapsedTime = currentTimestamp - commSessionEntries[index].entryTimestamp;
      if (elapsedTime >= ZGP_PAIRING_CONFIG_SESSION_TIMEOUT)
      {
        zgpServerFreeSessionEntry(index);
      }
      else
      {
        minTimeout = MIN(minTimeout, (ZGP_PAIRING_CONFIG_SESSION_TIMEOUT - elapsedTime));
      }
    }
  }

  if (~0L != minTimeout)
  {
    pairingConfigSessionTimer.interval = minTimeout;
    HAL_StartAppTimer(&pairingConfigSessionTimer);
  }
}
#endif

/**************************************************************************//**
  \brief get the session entry index if available otherwise free entry index

  \param[in] isPairingConfig - the entry is specific to pairing config or not
             appId - application id
             basicContextInfo - basic context information
             sessionIndex - entry index and free index to be populated

  \return true - any valid entry available
          false - otherwise
******************************************************************************/
bool zgpServerGetCommReqPairingConfigSessionTableIndex(ZGP_ApplicationId_t appId, ZGP_GpdId_t *gpdId, uint8_t endPoint, \
                                                     zgpSessionIndex_t *sessionIndex)
{
  bool isAnyValidEntry = false;

  for (uint8_t index = 0; index < ZGP_TOTAL_NO_COMM_SESSIONS; index++)
  {
    if (ENTRY_AVAILABLE != commSessionEntries[index].entryStatus)
    {
      if (PAIRING_CONFIG_ENTRY == commSessionEntries[index].entryStatus)
        isAnyValidEntry = true;
      if (commSessionEntries[index].appId == appId)
      {
        if (((ZGP_SRC_APPID == appId) && (commSessionEntries[index].gpdId.gpdSrcId == gpdId->gpdSrcId)) || \
            ((ZGP_IEEE_ADDR_APPID == appId) && (commSessionEntries[index].gpdId.gpdIeeeAddr == gpdId->gpdIeeeAddr) && \
             (commSessionEntries[index].endPoint == endPoint)))
        {
          sessionIndex->entryIndex = index;
          sessionIndex->entryStatus = commSessionEntries[index].entryStatus;
          //break;
        }
      }
    }
    else
      sessionIndex->freeIndex = index;
  }
  return isAnyValidEntry;
}

/**************************************************************************//**
  \brief get the session entry of COMM_REQ type

  \param[in] sessionIndex - index of the entry to be populated

  \return true - any valid entry available
          false - otherwise
******************************************************************************/
bool zgpServerGetCommReqEntry(zgpSessionIndex_t *sessionIndex)
{
  for (uint8_t index = 0; index < ZGP_TOTAL_NO_COMM_SESSIONS; index++)
  {
    if (commSessionEntries[index].entryStatus == COMM_REQ_ENTRY)
    {
      sessionIndex->entryIndex = index;
      return true;
    }
  }
  return false;
}

#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
/**************************************************************************//**
  \brief initialize the session entry of the given index

  \param[in] index - session entry index

  \return None
******************************************************************************/
void zgpServerInitReportDescMask(uint8_t index)
{
  uint8_t reportMaskIndex = 0;
  uint8_t reportCount = commSessionEntries[index].gpdAppInfo.totalNoofReports;

  memset(&commSessionEntries[index].rxdReportDescriptorMask[0], 0xFF, sizeof(commSessionEntries[index].rxdReportDescriptorMask));
  while (reportCount)
  {
    uint8_t mask;
    if (reportCount > 8)
    {
      mask = 0x00;
      reportCount -= 8;
    }
    else
    {
      mask = ~((1u << reportCount) - 1);
      reportCount = 0;
    }
    commSessionEntries[index].rxdReportDescriptorMask[reportMaskIndex] &= mask;
    reportMaskIndex++;
  }
}

/**************************************************************************//**
  \brief Handler of new session entry - initialization of entry fields

  \param[in] freeIndex - index of new entry
             addressing - addressing info
             pairingConfigContextInfo - rxd pairing config field information
             isTimerRunning - session timer is running

  \return true - new entry initialized properly
          false - otherwise
******************************************************************************/
static bool newPairingSessionEntryHandling(uint8_t freeIndex, ZCL_Addressing_t *addressing, \
                                           zgpPairingConfigContextInfo_t *pairingConfigContextInfo, bool isTimerRunning)
{
  if (ZGP_INVALID_SESSION_ENTRY_INDEX == freeIndex)
  {
    return false;
  }
  else
  {
    // Need to initialize the entry
    commSessionEntries[freeIndex].entryStatus = PAIRING_CONFIG_ENTRY;
    commSessionEntries[freeIndex].appId = (ZGP_ApplicationId_t)pairingConfigContextInfo->options.appId;
    commSessionEntries[freeIndex].addrMode = addressing->indDstAddrMode;
    commSessionEntries[freeIndex].commissioningToolNwkAddr = addressing->addr.shortAddress;
    commSessionEntries[freeIndex].endPoint = pairingConfigContextInfo->basicContextInfo.endPoint;
    memcpy(&commSessionEntries[freeIndex].gpdId, &pairingConfigContextInfo->basicContextInfo.gpdId, sizeof(commSessionEntries[freeIndex].gpdId));
    commSessionEntries[freeIndex].entryTimestamp = halGetTimeOfAppTimer();
    commSessionEntries[freeIndex].validActionRxd = false;
    // will be initialized during processing
    commSessionEntries[freeIndex].gpdAppInfo.totalNoofReports = 0x00;
    //commSessionEntries[freeIndex].gpdAppInfo.totalNoofReports = pairingConfigFieldInfo->totalNoOfReports;
    commSessionEntries[freeIndex].gpdAppInfo.reportDescriptor = NULL;
    ZGPL_ResetTableEntry((void *) &commSessionEntries[freeIndex].sinkEntry, ZGP_SINK_ENTRY);
    memset(&commSessionEntries[freeIndex].rxdReportDescriptorMask[0], 0x00, sizeof(commSessionEntries[freeIndex].rxdReportDescriptorMask));

    if (!isTimerRunning)
    {
      pairingConfigSessionTimer.interval = ZGP_PAIRING_CONFIG_SESSION_TIMEOUT;
      HAL_StopAppTimer(&pairingConfigSessionTimer);
      HAL_StartAppTimer(&pairingConfigSessionTimer);
    }
    return true;
  }
}
#endif

/**************************************************************************//**
  \brief filter the received action and create the entry for new session

  \param[in] pairingConfigContextInfo - Rxd pairing config information
             addressing - addressing information

  \return status after filtering
******************************************************************************/
static ZCL_Status_t filterRxdActionAndGetSessionTableIndex(zgpPairingConfigContextInfo_t *pairingConfigContextInfo, \
                                                   ZCL_Addressing_t *addressing)
{
  bool isTimerRunning = false;
  zgpSessionIndex_t sessionIndex = {.entryIndex = ZGP_INVALID_SESSION_ENTRY_INDEX, .freeIndex = ZGP_INVALID_SESSION_ENTRY_INDEX};

  pairingConfigContextInfo->pairingConfigState = PAIRING_CONFIG_COMPLETE_PAIRING_INFO_RXD;
  isTimerRunning = zgpServerGetCommReqPairingConfigSessionTableIndex((ZGP_ApplicationId_t)pairingConfigContextInfo->options.appId, &pairingConfigContextInfo->basicContextInfo.gpdId, \
                                                                     pairingConfigContextInfo->basicContextInfo.endPoint, &sessionIndex);
  pairingConfigContextInfo->basicContextInfo.sessionEntryIndex = sessionIndex.entryIndex;
  if (ZGP_INVALID_SESSION_ENTRY_INDEX == sessionIndex.entryIndex)
  {
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
    if ((REMOVE_PAIRING == pairingConfigContextInfo->actions.action) || (REMOVE_GPD == pairingConfigContextInfo->actions.action) || (NO_ACTION == pairingConfigContextInfo->actions.action))
    {
      return ZCL_SUCCESS_STATUS;
    }
    else if ((EXTEND_SINKTABLE_ENTRY == pairingConfigContextInfo->actions.action) || (REPLACE_SINKTABLE_ENTRY == pairingConfigContextInfo->actions.action))
    {
      if (!pairingConfigContextInfo->basicContextInfo.appDescriptionSet)
      {
        return ZCL_SUCCESS_STATUS;
      }
      else
      {
        if (!newPairingSessionEntryHandling(sessionIndex.freeIndex, addressing, pairingConfigContextInfo, isTimerRunning))
          return ZCL_INSUFFICIENT_SPACE_STATUS;
        pairingConfigContextInfo->basicContextInfo.sessionEntryIndex = sessionIndex.freeIndex;
        pairingConfigContextInfo->pairingConfigState = PAIRING_CONFIG_NON_APP_DESC_ACTION_PROCESSING;
      }
    }
    else
    {
      // application description
      if (!newPairingSessionEntryHandling(sessionIndex.freeIndex, addressing, pairingConfigContextInfo, isTimerRunning))
        return ZCL_INSUFFICIENT_SPACE_STATUS;
      pairingConfigContextInfo->basicContextInfo.sessionEntryIndex = sessionIndex.freeIndex;
      pairingConfigContextInfo->pairingConfigState = PAIRING_CONFIG_APP_DESC_ACTION_PROCESSING;
    }
#else
    (void)isTimerRunning;
#endif
  }
  else
  {
    if ((PAIRING_CONFIG_ENTRY != sessionIndex.entryStatus) || (REMOVE_PAIRING == pairingConfigContextInfo->actions.action) || \
        (REMOVE_GPD == pairingConfigContextInfo->actions.action) || (NO_ACTION == pairingConfigContextInfo->actions.action))
    {
      return ZCL_FAILURE_STATUS;
    }
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
    else
    {
      uint8_t sessionIndex = pairingConfigContextInfo->basicContextInfo.sessionEntryIndex;
      //if CT address is different then drop the frame
      if ((commSessionEntries[sessionIndex].addrMode != addressing->indDstAddrMode) || \
          (commSessionEntries[sessionIndex].commissioningToolNwkAddr != addressing->addr.shortAddress))
        return ZCL_FAILURE_STATUS;

      if ((EXTEND_SINKTABLE_ENTRY == pairingConfigContextInfo->actions.action) || (REPLACE_SINKTABLE_ENTRY == pairingConfigContextInfo->actions.action))
      {
        if (commSessionEntries[sessionIndex].validActionRxd)
          // valid action already received so drop this frame
          return ZCL_INVALID_FIELD_STATUS;

        pairingConfigContextInfo->pairingConfigState = PAIRING_CONFIG_NON_APP_DESC_ACTION_PROCESSING;
      }
      else
      {
        pairingConfigContextInfo->pairingConfigState = PAIRING_CONFIG_APP_DESC_ACTION_PROCESSING;
        if (commSessionEntries[sessionIndex].gpdAppInfo.totalNoofReports && \
            (pairingConfigContextInfo->basicContextInfo.rxdReportInfo.totalNoOfReports != commSessionEntries[sessionIndex].gpdAppInfo.totalNoofReports))
          return ZCL_INVALID_FIELD_STATUS;
        if (pairingConfigContextInfo->basicContextInfo.rxdReportInfo.totalNoOfReports > ZGP_MAX_NO_OF_REPORTS)
          return ZCL_INSUFFICIENT_SPACE_STATUS;
      }
    }
#endif
  }
  return ZCL_SUCCESS_STATUS;
}

/**************************************************************************//**
  \brief parse the received end point information

  \param[in] pairingConfigContextInfo - Rxd pairing config information
             payload - payload information
             payloadIndex - index of the payload from which end point info should be parsed
             rxdLength - length of the received payload

  \return status after parsing
******************************************************************************/
static ZCL_Status_t parsePairedEndPointInfo(zgpPairingConfigContextInfo_t *pairingConfigContextInfo, uint8_t *payload, \
                                           uint8_t *payloadIndex, uint8_t rxdLength)
{
  uint8_t index = *payloadIndex;
  uint8_t pairedEndPointCnt = 0;

  pairingConfigContextInfo->basicContextInfo.noOfPairedEndpoints = payload[index++];
  pairedEndPointCnt = pairingConfigContextInfo->basicContextInfo.noOfPairedEndpoints;

  if ((PRECOMMISSIONED_GROUPCAST == pairingConfigContextInfo->options.communicationMode) && ( 0xFE != pairingConfigContextInfo->basicContextInfo.noOfPairedEndpoints ))
  {
    return ZCL_INVALID_FIELD_STATUS;
  }

  if ((pairingConfigContextInfo->basicContextInfo.noOfPairedEndpoints >= 0xFD) || (0x00 == pairingConfigContextInfo->basicContextInfo.noOfPairedEndpoints))
  {
    pairedEndPointCnt = 0;
  }

  // parsing the application information
  if ((index + pairedEndPointCnt/*paired endPoints*/) > rxdLength)
  {
    return ZCL_MALFORMED_COMMAND_STATUS;
  }

  pairingConfigContextInfo->basicContextInfo.pairedEndPointPtr = &payload[index];

  // skipping paired endpoints
  index += pairedEndPointCnt;

  *payloadIndex = index;
  return ZCL_SUCCESS_STATUS;
}

/**************************************************************************//**
  \brief parse the pairing config command for the action not equal to APPLICATION_DESCRIPTION

  \param[in] pairingConfigFieldInfo - pairing config field information
             payloadLength - rxd payload length
             payload - payload info
             payloadIndex - index of payload from which cmd info will be parsed

  \return status after processing the action
******************************************************************************/
static ZCL_Status_t parseNonAppDescriptionActionCmd(zgpPairingConfigContextInfo_t *pairingConfigContextInfo, uint8_t payloadLength, \
                                                    uint8_t *payload, uint8_t payloadIndex)
{
  ZGP_SinkTableEntry_t *sinkTableEntry = pairingConfigContextInfo->basicContextInfo.sinkEntry;
  uint8_t groupListCnt;
  ZCL_Status_t retStatus;

  memcpy(&sinkTableEntry->options, &pairingConfigContextInfo->options, sizeof(sinkTableEntry->options));
 //Since both the structures has the same options field except for the reserved bits,we retain the reserved field value of sinkTableEntry as it is
  sinkTableEntry->options.reserved = 0x00;

  memcpy(&sinkTableEntry->tableGenericInfo.gpdId, &pairingConfigContextInfo->basicContextInfo.gpdId, sizeof(sinkTableEntry->tableGenericInfo.gpdId));
  sinkTableEntry->tableGenericInfo.endPoint = pairingConfigContextInfo->basicContextInfo.endPoint;

  memcpy(&sinkTableEntry->deviceId, &payload[payloadIndex],sizeof(sinkTableEntry->deviceId));
  payloadIndex += sizeof(sinkTableEntry->deviceId);

  // Refer Spec A.3.3.4.6.6 Effect of Recipient on GpConfigurationCommand(Extend a SinkTable Entry)
  if(sinkTableEntry->options.communicationMode == PRECOMMISSIONED_GROUPCAST)
  {
    groupListCnt = payload[payloadIndex];
    payloadIndex ++;

    if(groupListCnt > ZGP_SINK_GROUP_LIST_SIZE)
    {
      return ZCL_INSUFFICIENT_SPACE_STATUS;
    }

    for (uint8_t i = 0; i < groupListCnt; i++)
    {
      uint16_t groupList;
      uint16_t alias;
      memcpy(&groupList,&payload[payloadIndex],sizeof(uint16_t));
      sinkTableEntry->tableGenericInfo.zgpSinkGrouplist[i].sinkGroup = groupList;
      payloadIndex += sizeof(groupList);
      memcpy(&alias,&payload[payloadIndex],sizeof(uint16_t));
      sinkTableEntry->tableGenericInfo.zgpSinkGrouplist[i].alias = alias;
      payloadIndex += sizeof(alias);
    }
  }

  if(sinkTableEntry->options.assignedAlias)
  {
    uint16_t alias = 0;
    memcpy(&alias, &payload[payloadIndex], sizeof(sinkTableEntry->tableGenericInfo.gpdAssignedAlias));
    sinkTableEntry->options.assignedAlias = true;
    sinkTableEntry->tableGenericInfo.gpdAssignedAlias = alias;
    payloadIndex += sizeof(sinkTableEntry->tableGenericInfo.gpdAssignedAlias);
  }

  memcpy(&sinkTableEntry->tableGenericInfo.groupCastRadius, &payload[payloadIndex],sizeof(sinkTableEntry->tableGenericInfo.groupCastRadius));
  payloadIndex += sizeof(sinkTableEntry->tableGenericInfo.groupCastRadius);

  if(sinkTableEntry->options.securityUse)
  {
    memcpy(&sinkTableEntry->tableGenericInfo.securityOptions,&payload[payloadIndex],sizeof(sinkTableEntry->tableGenericInfo.securityOptions));
    payloadIndex += sizeof(sinkTableEntry->tableGenericInfo.securityOptions);
  }

  if(sinkTableEntry->options.securityUse || (!sinkTableEntry->options.securityUse && sinkTableEntry->options.seqNumCapabilities))
  {
    uint32_t gpdSecurityFrameCounter;

    memcpy(&gpdSecurityFrameCounter,&payload[payloadIndex],sizeof(sinkTableEntry->tableGenericInfo.gpdSecurityFrameCounter));
    sinkTableEntry->tableGenericInfo.gpdSecurityFrameCounter = gpdSecurityFrameCounter;
    payloadIndex += sizeof(sinkTableEntry->tableGenericInfo.gpdSecurityFrameCounter);
  }

  if(sinkTableEntry->options.securityUse)
  {
    if ((EXTEND_SINKTABLE_ENTRY == pairingConfigContextInfo->actions.action) || (REPLACE_SINKTABLE_ENTRY == pairingConfigContextInfo->actions.action) || (NO_ACTION == pairingConfigContextInfo->actions.action))
    {
      ZGP_GpsSecurityLevelAttribute_t *gpsSecurity = (ZGP_GpsSecurityLevelAttribute_t *) &zgpClusterServerAttributes.gpsSecurityLevel.value;

      // if the SecurityLevel field of the SecurityUse field is set to 0b01, the sink SHALL NOT update (if exist-ent) nor create a Sink Table entry for this GPD ID
      if(sinkTableEntry->tableGenericInfo.securityOptions.securityLevel == ZGP_SECURITY_LEVEL_1)
      {
        return ZCL_INVALID_FIELD_STATUS;
      }
      if ((pairingConfigContextInfo->actions.action != NO_ACTION) && \
           (sinkTableEntry->tableGenericInfo.securityOptions.securityLevel < gpsSecurity->minGpdSecurityLevel))
      {
        return ZCL_UNSUPPORTED_ATTRIBUTE_STATUS;
      }
    }

    memcpy(&sinkTableEntry->tableGenericInfo.securityKey, &payload[payloadIndex],sizeof(sinkTableEntry->tableGenericInfo.securityKey));
    payloadIndex += sizeof(sinkTableEntry->tableGenericInfo.securityKey);
  }

  retStatus = parsePairedEndPointInfo(pairingConfigContextInfo, payload, \
                                           &payloadIndex, payloadLength);

  if (ZCL_SUCCESS_STATUS != retStatus)
  {
    return retStatus;
  }

  if (pairingConfigContextInfo->options.applicationInfoPresent)
  {
    if (!zgpParsingAppInfoFromPayload(payload, &payloadIndex, &pairingConfigContextInfo->basicContextInfo.gpdAppInfo, payloadLength))
    {
      return ZCL_MALFORMED_COMMAND_STATUS;
    }
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
    pairingConfigContextInfo->basicContextInfo.appDescriptionSet = pairingConfigContextInfo->basicContextInfo.gpdAppInfo.appInfoOptions.appDescriptionCommandFollows;
#endif
  }
  return ZCL_SUCCESS_STATUS;
}

/**************************************************************************//**
  \brief process the pairing config after all packets received

  \param[in] pairingConfigFieldInfo - pairing config field information

  \return status after processing
******************************************************************************/
static ZCL_Status_t processActionOnCompletePairing(zgpPairingConfigContextInfo_t *pairingConfigContextInfo)
{
  ZGP_SinkTableEntry_t *sinkTableEntry = pairingConfigContextInfo->basicContextInfo.sinkEntry;

  if ((EXTEND_SINKTABLE_ENTRY == pairingConfigContextInfo->actions.action) || (REPLACE_SINKTABLE_ENTRY == pairingConfigContextInfo->actions.action) || \
      (REMOVE_PAIRING == pairingConfigContextInfo->actions.action))
  {
    ZGP_TableUpdateAction_t tableAction = UPDATE_ENTRY;

    //Update Sink Table based on the Entry Information
    if (REPLACE_SINKTABLE_ENTRY == pairingConfigContextInfo->actions.action)
      tableAction = REPLACE_ENTRY;
    else if (REMOVE_PAIRING == pairingConfigContextInfo->actions.action)
      tableAction = REMOVE_PAIRING_ENTRY;

    // Check whether the device id is supported or
    // one of the given command in cmd list is supported
    if ((EXTEND_SINKTABLE_ENTRY == pairingConfigContextInfo->actions.action) || (REPLACE_SINKTABLE_ENTRY == pairingConfigContextInfo->actions.action))
    {
      ZGP_TransTableIndicationInfo_t transTableInfo;

      memset(&transTableInfo, 0x00, sizeof(transTableInfo));
      transTableInfo.transTableIndType = APP_FUNCTIONALITY_CHECK;
      transTableInfo.indicationData.appFuncCheckInfo.deviceId = &sinkTableEntry->deviceId;
      transTableInfo.indicationData.appFuncCheckInfo.appInfo = &pairingConfigContextInfo->basicContextInfo.gpdAppInfo;
      transTableInfo.indicationData.appFuncCheckInfo.isMatching = true;
      transTableInfo.indicationData.appFuncCheckInfo.noOfEndPoints = pairingConfigContextInfo->basicContextInfo.noOfPairedEndpoints;
      transTableInfo.indicationData.appFuncCheckInfo.endPointList = pairingConfigContextInfo->basicContextInfo.pairedEndPointPtr;
      SYS_PostEvent(BC_EVENT_ZGPH_TRANS_TABLE_INDICATION, (SYS_EventData_t)&transTableInfo);
      if (!transTableInfo.indicationData.appFuncCheckInfo.isMatching)
      {
        return ZCL_INVALID_FIELD_STATUS;
      }
    }

    if (!ZGPL_AddOrUpdateTableEntryOnNvm((void *)sinkTableEntry, tableAction, ZGP_SINK_ENTRY))
    {
      return ZCL_INSUFFICIENT_SPACE_STATUS;
    }

    if ((EXTEND_SINKTABLE_ENTRY == pairingConfigContextInfo->actions.action) || (REPLACE_SINKTABLE_ENTRY == pairingConfigContextInfo->actions.action))
    {
      zgpTransRemoveAndAddEventHandler(PAIRING_CONFIG_TYPE, sinkTableEntry, &pairingConfigContextInfo->basicContextInfo.gpdAppInfo, \
                                       pairingConfigContextInfo->basicContextInfo.noOfPairedEndpoints, \
                                       pairingConfigContextInfo->basicContextInfo.pairedEndPointPtr);
    }

    // For remove pairing, no need to add the group and send gp pairing(handled as part of zgpAddOrUpdateTableEntryOnNvm
    // prevously)
    if (REMOVE_PAIRING != pairingConfigContextInfo->actions.action)
    {
      zgpServerAddGroup(sinkTableEntry , pairingConfigContextInfo->actions.sendGpPairing);
    }
    if (true == pairingConfigContextInfo->actions.sendGpPairing)
    {
      zgpServerSendGpPairingForEntry(sinkTableEntry, (ZGP_SinkTableActions_t)pairingConfigContextInfo->actions.action);
    }
  }
  if(pairingConfigContextInfo->actions.action == NO_ACTION && pairingConfigContextInfo->actions.sendGpPairing == true)
  {
    ZGP_ReadOperationStatus_t entryStatus;
    ZGP_TableOperationField_t  tableOperationField = {.entryType = ZGP_SINK_ENTRY, .commMode = pairingConfigContextInfo->options.communicationMode, .appId = pairingConfigContextInfo->options.appId, \
    .nonEmptyIndexForRead = ZGP_ENTRY_INVALID_INDEX};
    ZGP_GpdId_t gpdId;

    // If Action field = 0x00 then check GP Pairing field and if SendGpPairing = 1(GPD ID And Endpoint are of Importance)
    //Retrieve the Sink TableEntry by giving App ID and EndPoint corresponding to this Endpoint and SendGpPairing
    memcpy(&gpdId, &sinkTableEntry->tableGenericInfo.gpdId, sizeof(ZGP_GpdId_t));
    entryStatus = ZGPL_ReadTableEntryFromNvm((void *)sinkTableEntry, tableOperationField, &sinkTableEntry->tableGenericInfo.gpdId, sinkTableEntry->tableGenericInfo.endPoint);

    //If no entry available for the GPD then don't Send GP Pairing
    if(ACTIVE_ENTRY_AVAILABLE != entryStatus)
    {
      return ZCL_NOT_FOUND_STATUS;
    }
    else
    {
      zgpServerSendGpPairingForEntry(sinkTableEntry, EXTEND_SINKTABLE_ENTRY);
    }
  }
  else if(pairingConfigContextInfo->actions.action == REMOVE_GPD)
  {
    zgpSinkRemoveGpdEntry(sinkTableEntry, pairingConfigContextInfo->actions.sendGpPairing);
  }
  return ZCL_SUCCESS_STATUS;
}

#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
/**************************************************************************//**
  \brief Get the buffer for data point descriptors

  \param[in] None

  \return allocated buffer address
          NULL otherwise
******************************************************************************/
static ZGP_DataPointDescriptor_t * getDataPointBuffer(void)
{
  for (uint8_t i = 0; i < ZGP_MAX_NO_OF_DATA_POINT_DESCRIPTORS; i++)
  {
    if (!dataPointBufffer[i].busy)
    {
      dataPointBufffer[i].busy = true;
      return &dataPointBufffer[i];
    }
  }
  return NULL;
}

/**************************************************************************//**
  \brief Get the buffer for rxd report descriptors

  \param[in] None

  \return allocated buffer address
          NULL otherwise
******************************************************************************/
static ZGP_ReportDescriptor_t * getReportDescBuffer(void)
{
  for (uint8_t i = 0; i < ZGP_MAX_NO_OF_REPORTS; i++)
  {
    if (ZGP_INVALID_REPORT_ID == reportDescBuffer[i].reportId)
    {
      return &reportDescBuffer[i];
    }
  }
  return NULL;
}

/**************************************************************************//**
  \brief Free the buffer of report descriptor and linked data descriptors

  \param[in] reportDesc - report descriptor address

  \return None
******************************************************************************/
void zgpServerFreeReportDataDescBufer(ZGP_ReportDescriptor_t *reportDesc)
{
  while(reportDesc)
  {
    ZGP_DataPointDescriptor_t *dataPointDesc;

    if ((&reportDescBuffer[ZGP_MAX_NO_OF_REPORTS] <= reportDesc) || (&reportDescBuffer[0] > reportDesc)|| \
        (((uint8_t *)&reportDescBuffer[ZGP_MAX_NO_OF_REPORTS - 1] - (uint8_t *)reportDesc) % sizeof(ZGP_ReportDescriptor_t)) )
    {
      //asserting
      SYS_E_ASSERT_FATAL(false, ZGP_SINK_REPORT_DESC_INVALID_PTR);
    }

    reportDesc->reportId = ZGP_INVALID_REPORT_ID;
    dataPointDesc = reportDesc->dataPointDescriptor;
    while(dataPointDesc)
    {
      if ((&dataPointBufffer[ZGP_MAX_NO_OF_DATA_POINT_DESCRIPTORS] <= dataPointDesc) || (&dataPointBufffer[0] > dataPointDesc)|| \
          (((uint8_t *)&dataPointBufffer[ZGP_MAX_NO_OF_DATA_POINT_DESCRIPTORS - 1] - (uint8_t *)dataPointDesc) % sizeof(ZGP_DataPointDescriptor_t)) )
      {
        // asserting
        SYS_E_ASSERT_FATAL(false, ZGP_SINK_DATA_POINT_DESC_INVALID_PTR);
      }
      dataPointDesc->busy = false;
      dataPointDesc = dataPointDesc->nextDataPointDescriptor;
    }
    reportDesc = reportDesc->nextReportDescriptor;
  }
}

/**************************************************************************//**
  \brief To parse the report descriptor fields from the received payload

  \param[in] rxdReportInfo - report info to be parsed
             payload - payload info
             payloadIndex - starting index of payload to be parsed
             payloadLength - payload length
             entryIndex - session entry index

  \return status after parsing
******************************************************************************/
ZCL_Status_t zgpParseReportDescriptorFields(zgpRxdReportInfo_t *rxdReportInfo, uint8_t *payload, \
                                            uint8_t payloadIndex, uint8_t payloadLength, uint8_t entryIndex)
{
  uint8_t reportIndex = 0;
  ZGP_ReportDescriptor_t *reportDesc = NULL;
  ZGP_ReportDescriptor_t *prevReportDesc = NULL;
  uint8_t noOfReportsInRxdCmd;

  rxdReportInfo->totalNoOfReports = payload[payloadIndex++];
  noOfReportsInRxdCmd = payload[payloadIndex++];
  rxdReportInfo->noOfValidReports = 0;
  rxdReportInfo->rxdReportDescriptors = NULL;

  if (!rxdReportInfo->totalNoOfReports || !noOfReportsInRxdCmd || \
      (noOfReportsInRxdCmd > rxdReportInfo->totalNoOfReports))
  {
    return ZCL_INVALID_VALUE_STATUS;
  }
  while (reportIndex < noOfReportsInRxdCmd)
  {
    uint8_t remainingLength;
    uint8_t noOfDataPointDesc = 0;
    ZGP_DataPointDescriptor_t *dataPointDescr = NULL;
    ZGP_DataPointDescriptor_t *prevDataPointDescr = NULL;
    uint8_t startIndex;
    bool dropReportDesc = false;

    prevReportDesc = reportDesc;
    reportDesc = getReportDescBuffer();

    if (NULL == reportDesc)
    {
      if (rxdReportInfo->rxdReportDescriptors)
      {
        // release all buffers
        zgpServerFreeReportDataDescBufer(rxdReportInfo->rxdReportDescriptors);
      }
      return ZCL_INSUFFICIENT_SPACE_STATUS;
    }
    if (NULL == rxdReportInfo->rxdReportDescriptors)
      rxdReportInfo->rxdReportDescriptors = reportDesc;

    if (prevReportDesc)
      prevReportDesc->nextReportDescriptor = reportDesc;
    reportDesc->nextReportDescriptor = NULL;
    reportDesc->dataPointDescriptor = NULL;

    reportDesc->reportId = payload[payloadIndex++];
    // Need to check the duplicate of report id
    if ((rxdReportInfo->rxdReportDescriptorMask[reportDesc->reportId/8] & (1u << (reportDesc->reportId%8))) || \
        ((ZGP_INVALID_SESSION_ENTRY_INDEX != entryIndex) && \
         (commSessionEntries[entryIndex].rxdReportDescriptorMask[reportDesc->reportId/8] & (1u << (reportDesc->reportId%8)))))
    {
      // need to check the session entry mask whether this report id already rxd

      // duplicate report id
      // so drop the request
      dropReportDesc = true;
    }

    rxdReportInfo->rxdReportDescriptorMask[reportDesc->reportId/8] |= (1u << (reportDesc->reportId%8));
    reportDesc->reportOptions = payload[payloadIndex++];
    reportDesc->timeoutPeriod = 0x00;
    if (IS_ZGP_REPORT_DESCRIPTOR_TIMEOUT_PRESENT(reportDesc->reportOptions))
    {
      uint16_t timeoutPeriod;

      memcpy(&timeoutPeriod, &payload[payloadIndex], sizeof(reportDesc->timeoutPeriod));
      reportDesc->timeoutPeriod = timeoutPeriod;
      payloadIndex += sizeof(reportDesc->timeoutPeriod);
    }

    remainingLength = payload[payloadIndex++];
    if ((payloadIndex + remainingLength) >  payloadLength)
    {
      zgpServerFreeReportDataDescBufer(rxdReportInfo->rxdReportDescriptors);
      return ZCL_MALFORMED_COMMAND_STATUS;
    }

    startIndex = payloadIndex;

    while((startIndex + remainingLength) > payloadIndex)
    {
      uint8_t attrRecordCount = 0;
      uint8_t attrRecordIndex = 0;
      uint16_t tempData = 0;

      prevDataPointDescr = dataPointDescr;
      dataPointDescr = getDataPointBuffer();

      if (NULL == dataPointDescr)
      {
        zgpServerFreeReportDataDescBufer(rxdReportInfo->rxdReportDescriptors);
        // release all buffers
        return ZCL_INSUFFICIENT_SPACE_STATUS;
      }

      if (NULL == reportDesc->dataPointDescriptor)
        reportDesc->dataPointDescriptor = dataPointDescr;
      if (prevDataPointDescr)
        prevDataPointDescr->nextDataPointDescriptor = dataPointDescr;
      dataPointDescr->nextDataPointDescriptor = NULL;

      noOfDataPointDesc++;
      dataPointDescr->dataPointOptions = payload[payloadIndex++];
      memcpy(&tempData, &payload[payloadIndex], sizeof(dataPointDescr->clusterId));
      payloadIndex += sizeof(dataPointDescr->clusterId);
      dataPointDescr->clusterId = tempData;

      if (IS_ZGP_DATAPOINT_DESC_MANUFAC_ID_PRESENT(dataPointDescr->dataPointOptions))
      {
        memcpy(&tempData, &payload[payloadIndex], sizeof(dataPointDescr->manufacturerId));
        dataPointDescr->manufacturerId = tempData;
        payloadIndex += sizeof(dataPointDescr->manufacturerId);
      }

      attrRecordCount = ZGP_DATAPOINT_DESC_NO_OF_ATTRIBUTE_RECORDS(dataPointDescr->dataPointOptions) + 1;
      while(attrRecordIndex < attrRecordCount)
      {
        uint8_t remainingLength;
        uint8_t attrOptionsIndex = 0;
        uint16_t attrId;
        ZGP_AttributeRecord_t tempAttrRecord;

        memcpy(&attrId, &payload[payloadIndex], sizeof(tempAttrRecord.attrId));
        tempAttrRecord.attrId = attrId;
        payloadIndex += sizeof(tempAttrRecord.attrId);
        tempAttrRecord.attrDataType = payload[payloadIndex++];
        tempAttrRecord.attrOptions = payload[payloadIndex++];
        remainingLength = ZGP_ATTR_RECORD_REMAINING_LENGTH(tempAttrRecord.attrOptions) + 1;
        attrOptionsIndex = payloadIndex;
        if ((payloadIndex + remainingLength) > payloadLength)
        {
          zgpServerFreeReportDataDescBufer(rxdReportInfo->rxdReportDescriptors);
          return ZCL_MALFORMED_COMMAND_STATUS;
        }
        tempAttrRecord.attrOffsetWithinReport = 0x00;
        if (IS_ZGP_ATTR_REPORTED(tempAttrRecord.attrOptions))
        {
          tempAttrRecord.attrOffsetWithinReport = payload[payloadIndex++];
        }
        if (IS_ZGP_ATTR_VALUE_PRESENT(tempAttrRecord.attrOptions))
        {
          uint8_t attrLength = ZCL_GetAttributeLength(tempAttrRecord.attrDataType, \
            &payload[payloadIndex]);
          uint8_t attrValueIndex = payloadIndex;

          payloadIndex += attrLength;
          if (attrLength > ZGP_ATTRIBUTE_MAX_SIZE)
            attrLength = ZGP_ATTRIBUTE_MAX_SIZE;

          while(attrLength)
          {
            attrLength--;
            tempAttrRecord.attrValue[attrLength] = payload[attrValueIndex + attrLength];
          }
        }
        if ((attrOptionsIndex+ remainingLength) > payloadIndex)
        {
          zgpServerFreeReportDataDescBufer(rxdReportInfo->rxdReportDescriptors);
          // release the buffer
          return ZCL_MALFORMED_COMMAND_STATUS;
        }
        if (attrRecordIndex < ZGP_MAX_ATTR_RECORD_IN_A_DATA_POINTER)
          memcpy(&dataPointDescr->attrRecord[attrRecordIndex], &tempAttrRecord, sizeof(dataPointDescr->attrRecord[attrRecordIndex]));
        attrRecordIndex++;
      }
    }

    reportDesc->noOfDatapointDescriptor = noOfDataPointDesc;
    if (dropReportDesc)
    {
      // release the current descriptor & data point descriptor
      // make prev as the current one
      zgpServerFreeReportDataDescBufer(reportDesc);
      if (prevReportDesc)
        prevReportDesc->nextReportDescriptor = NULL;
      reportDesc = prevReportDesc;
    }
    else
      rxdReportInfo->noOfValidReports++;
    reportIndex++;
  }

  if (!rxdReportInfo->noOfValidReports)
  {
    // release all the buffers
    // all are duplicate frames
    zgpServerFreeReportDataDescBufer(rxdReportInfo->rxdReportDescriptors);
    return ZCL_INVALID_FIELD_STATUS;
  }
  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
  \brief parse pairing config command for application description action

  \param[in] pairingConfigFieldInfo - pairing config field information
             payloadLength - received payload length
             payload - payload info
             payloadIndex - payload index from which app.des. is parsed

  \return status after processing app description command
******************************************************************************/
static ZCL_Status_t parseAppDescriptionActionCmd(zgpPairingConfigContextInfo_t *pairingConfigContextInfo, uint8_t payloadLength, \
                                       uint8_t *payload, uint8_t payloadIndex)
{
  zgpSessionIndex_t sessionIndex = {.entryIndex = ZGP_INVALID_SESSION_ENTRY_INDEX, .freeIndex = ZGP_INVALID_SESSION_ENTRY_INDEX};
  ZCL_Status_t retStatus;


  zgpServerGetCommReqPairingConfigSessionTableIndex((ZGP_ApplicationId_t)pairingConfigContextInfo->options.appId, &pairingConfigContextInfo->basicContextInfo.gpdId, \
                                                    pairingConfigContextInfo->basicContextInfo.endPoint, &sessionIndex);

  if ((ZGP_INVALID_SESSION_ENTRY_INDEX != sessionIndex.entryIndex) && \
       (PAIRING_CONFIG_ENTRY != sessionIndex.entryStatus))
    return ZCL_FAILURE_STATUS;

  memset(&pairingConfigContextInfo->basicContextInfo.rxdReportInfo.rxdReportDescriptorMask[0], 0x00, sizeof(pairingConfigContextInfo->basicContextInfo.rxdReportInfo.rxdReportDescriptorMask));
  payloadIndex += 2; // skipping device id and groupcastRadius

  // considering 2 bytes for Total no. of reports and number of reports field
  if (payloadLength < (payloadIndex + 2))
  {
    return ZCL_MALFORMED_COMMAND_STATUS;
  }

  retStatus = parsePairedEndPointInfo(pairingConfigContextInfo, payload, \
                                           &payloadIndex, payloadLength);

  if (ZCL_SUCCESS_STATUS != retStatus)
    return retStatus;

  if (pairingConfigContextInfo->options.applicationInfoPresent)
  {
    if (!zgpParsingAppInfoFromPayload(payload, &payloadIndex, &pairingConfigContextInfo->basicContextInfo.gpdAppInfo, payloadLength))
    {
      return ZCL_MALFORMED_COMMAND_STATUS;
    }
  }

  //pairingConfigContextInfo->basicContextInfo.rxdReportInfo.totalNoOfReports = payload[payloadIndex++];
  //noOfReportsInRxdCmd = payload[payloadIndex++];

  memset(&pairingConfigContextInfo->basicContextInfo.rxdReportInfo, 0x00, sizeof(pairingConfigContextInfo->basicContextInfo.rxdReportInfo));
  retStatus = zgpParseReportDescriptorFields(&pairingConfigContextInfo->basicContextInfo.rxdReportInfo, payload, payloadIndex,\
                                             payloadLength,sessionIndex.entryIndex);

  return retStatus;
}

/**************************************************************************//**
  \brief process pairing config command for application description action

  \param[in] pairingConfigFieldInfo - pairing config field information

  \return None
******************************************************************************/
void zgpServerProcessAppDescCommand(zgpRxdReportInfo_t *rxdReportInfo, uint8_t sessionIndex)
{
  uint8_t i = 0;

  ZGP_ReportDescriptor_t *reportDescptr = commSessionEntries[sessionIndex].gpdAppInfo.reportDescriptor;

  if (!commSessionEntries[sessionIndex].gpdAppInfo.totalNoofReports)
  {
    commSessionEntries[sessionIndex].gpdAppInfo.totalNoofReports = rxdReportInfo->totalNoOfReports;
    zgpServerInitReportDescMask(sessionIndex);
  }

  while(i < ZGP_RXD_REPORT_DESC_MASK_SIZE)
  {
    commSessionEntries[sessionIndex].rxdReportDescriptorMask[i] |= rxdReportInfo->rxdReportDescriptorMask[i];
    i++;
  }
  if (reportDescptr)
  {
    while(reportDescptr->nextReportDescriptor)
    {
      reportDescptr = reportDescptr->nextReportDescriptor;
    }
    reportDescptr->nextReportDescriptor = rxdReportInfo->rxdReportDescriptors;
  }
  else
  {
    commSessionEntries[sessionIndex].gpdAppInfo.reportDescriptor = rxdReportInfo->rxdReportDescriptors;
  }
}

/**************************************************************************//**
  \brief process session table entry for pairing completion

  \param[in] pairingConfigFieldInfo - pairing config field information

  \return None
******************************************************************************/
static void processForPairingCompletion(zgpPairingConfigContextInfo_t *pairingConfigContextInfo)
{

  pairingConfigContextInfo->pairingConfigState = PAIRING_CONFIG_COMPLETE;
  if (zgpCommSessionAllPacketsRceived(pairingConfigContextInfo->basicContextInfo.sessionEntryIndex))
  {
    pairingConfigContextInfo->pairingConfigState = PAIRING_CONFIG_COMPLETE_PAIRING_INFO_RXD;
  }
}

/**************************************************************************//**
  \brief To check all the packets received for the given session

  \param[in] sessionIndex - session entry index

  \return true if all packets received
          false otherwise
******************************************************************************/
bool zgpCommSessionAllPacketsRceived(uint8_t sessionIndex)
{
  bool allReportDescRxd = true;
  bool validActionrxd = true;
  uint8_t i = 0;

  while(i < ZGP_RXD_REPORT_DESC_MASK_SIZE)
  {
    if (commSessionEntries[sessionIndex].rxdReportDescriptorMask[i] != 0xFF)
      allReportDescRxd = false;
    i++;
  }

  if (!commSessionEntries[sessionIndex].validActionRxd)
    validActionrxd = false;

  return (allReportDescRxd && validActionrxd);
}
#endif

/**************************************************************************//**
  \brief free the session entry

  \param[in] sessionEntryIndex - session entry index

  \return none
******************************************************************************/
void zgpServerFreeSessionEntry(uint8_t sessionEntryIndex)
{
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
  if (commSessionEntries[sessionEntryIndex].gpdAppInfo.reportDescriptor)
    zgpServerFreeReportDataDescBufer(commSessionEntries[sessionEntryIndex].gpdAppInfo.reportDescriptor);
#endif
  memset(&commSessionEntries[sessionEntryIndex], 0x00, sizeof(commSessionEntries[sessionEntryIndex]));
  commSessionEntries[sessionEntryIndex].entryStatus = ENTRY_AVAILABLE;
}

/**************************************************************************//**
  \brief Process GpPairingConfigurationCommand

  \param[in] payloadLength - payload length
             payload - payload info
             addressing - addressing info

  \return status - status after processing request
******************************************************************************/
static ZCL_Status_t gpPairingConfigHandler(uint8_t payloadLength, uint8_t *payload, ZCL_Addressing_t *addressing)
{
  zgpPairingConfigContextInfo_t pairingConfigContextInfo;
  uint16_t payloadIndex = 0;
  ZCL_Status_t zclRetStatus = ZCL_SUCCESS_STATUS;

  // Initialize options and action field from the received command
  memset(&pairingConfigContextInfo, 0x00, sizeof(zgpPairingConfigContextInfo_t));
  pairingConfigContextInfo.basicContextInfo.sessionEntryIndex = ZGP_INVALID_SESSION_ENTRY_INDEX;

  pairingConfigContextInfo.pairingConfigState = PAIRING_CONFIG_BASIC_INFO_PARSING;

  if (PAIRING_CONFIG_BASIC_INFO_PARSING == pairingConfigContextInfo.pairingConfigState)
  {
    // parse gpd address info from the payload
    zclRetStatus = parseBasicInfoFromPayload(&pairingConfigContextInfo, payload, payloadLength, &payloadIndex);
    if (ZCL_SUCCESS_STATUS != zclRetStatus)
    {
      return zclRetStatus;
    }
    pairingConfigContextInfo.basicContextInfo.sinkEntry = (ZGP_SinkTableEntry_t *)zgpGetMemReqBuffer();
    ZGPL_ResetTableEntry((void *)pairingConfigContextInfo.basicContextInfo.sinkEntry, ZGP_SINK_ENTRY);
  }

  if (PAIRING_CONFIG_NON_APP_DESC_ACTION_FIELD_PARSING == pairingConfigContextInfo.pairingConfigState)
  {
    zclRetStatus = parseNonAppDescriptionActionCmd(&pairingConfigContextInfo, payloadLength, payload, payloadIndex);
    if (ZCL_SUCCESS_STATUS != zclRetStatus)
    {
      zgpFreeMemReqBuffer();
      return zclRetStatus;
    }
    pairingConfigContextInfo.pairingConfigState = PAIRING_CONFIG_FILTERING_BASEC_ON_ACTION_AND_SESSION_ENTRY;
  }

#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
  if (PAIRING_CONFIG_APP_DESC_ACTION_FIELD_PARSING == pairingConfigContextInfo.pairingConfigState)
  {

    pairingConfigContextInfo.basicContextInfo.rxdReportInfo.rxdReportDescriptors = NULL;

    zclRetStatus = parseAppDescriptionActionCmd(&pairingConfigContextInfo, payloadLength, payload, payloadIndex);
    if (ZCL_SUCCESS_STATUS != zclRetStatus)
    {
      zgpFreeMemReqBuffer();
      return zclRetStatus;
    }
    pairingConfigContextInfo.pairingConfigState = PAIRING_CONFIG_FILTERING_BASEC_ON_ACTION_AND_SESSION_ENTRY;
  }
#endif

  if (PAIRING_CONFIG_FILTERING_BASEC_ON_ACTION_AND_SESSION_ENTRY == pairingConfigContextInfo.pairingConfigState)
  {
    zclRetStatus = filterRxdActionAndGetSessionTableIndex(&pairingConfigContextInfo, addressing);
    if (ZCL_SUCCESS_STATUS != zclRetStatus)
    {
      zgpFreeMemReqBuffer();
      return zclRetStatus;
    }
  }

#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
  if (PAIRING_CONFIG_NON_APP_DESC_ACTION_PROCESSING == pairingConfigContextInfo.pairingConfigState)
  {
    uint8_t sessionIndex = pairingConfigContextInfo.basicContextInfo.sessionEntryIndex;
    uint8_t reportCount = commSessionEntries[sessionIndex].gpdAppInfo.totalNoofReports;
    ZGP_ReportDescriptor_t *reportDesc = commSessionEntries[sessionIndex].gpdAppInfo.reportDescriptor;

    commSessionEntries[sessionIndex].commMode = pairingConfigContextInfo.options.communicationMode;
    commSessionEntries[sessionIndex].action = pairingConfigContextInfo.actions;
    commSessionEntries[sessionIndex].validActionRxd = true;
    memcpy(&commSessionEntries[sessionIndex].sinkEntry,  pairingConfigContextInfo.basicContextInfo.sinkEntry, \
                                  sizeof(commSessionEntries[sessionIndex].sinkEntry));
    memcpy(&commSessionEntries[sessionIndex].gpdAppInfo, &pairingConfigContextInfo.basicContextInfo.gpdAppInfo, \
             sizeof(commSessionEntries[sessionIndex].gpdAppInfo));

    commSessionEntries[sessionIndex].gpdAppInfo.totalNoofReports = reportCount;
    commSessionEntries[sessionIndex].gpdAppInfo.reportDescriptor = reportDesc;
    pairingConfigContextInfo.pairingConfigState = PAIRING_CONFIG_CHECK_FOR_COMPLETE_PAIRING_INFO;
  }

  if (PAIRING_CONFIG_APP_DESC_ACTION_PROCESSING == pairingConfigContextInfo.pairingConfigState)
  {
    // application description handling
    zgpServerProcessAppDescCommand(&pairingConfigContextInfo.basicContextInfo.rxdReportInfo, pairingConfigContextInfo.basicContextInfo.sessionEntryIndex);

    memcpy(pairingConfigContextInfo.basicContextInfo.sinkEntry, &commSessionEntries[pairingConfigContextInfo.basicContextInfo.sessionEntryIndex].sinkEntry,\
                                  sizeof(commSessionEntries[pairingConfigContextInfo.basicContextInfo.sessionEntryIndex].sinkEntry));
    pairingConfigContextInfo.pairingConfigState = PAIRING_CONFIG_CHECK_FOR_COMPLETE_PAIRING_INFO;
    memcpy(&pairingConfigContextInfo.actions, &commSessionEntries[pairingConfigContextInfo.basicContextInfo.sessionEntryIndex].action, sizeof(pairingConfigContextInfo.actions));
    pairingConfigContextInfo.options.communicationMode = commSessionEntries[pairingConfigContextInfo.basicContextInfo.sessionEntryIndex].commMode;
  }

  if (PAIRING_CONFIG_CHECK_FOR_COMPLETE_PAIRING_INFO == pairingConfigContextInfo.pairingConfigState)
  {
    processForPairingCompletion(&pairingConfigContextInfo);
    memcpy(&pairingConfigContextInfo.basicContextInfo.gpdAppInfo, &commSessionEntries[pairingConfigContextInfo.basicContextInfo.sessionEntryIndex].gpdAppInfo, \
           sizeof(pairingConfigContextInfo.basicContextInfo.gpdAppInfo));

  }
#endif

  if (PAIRING_CONFIG_COMPLETE_PAIRING_INFO_RXD == pairingConfigContextInfo.pairingConfigState)
  {
    // Provide the indication to the application
    // all the information for this pairing session received, so we can free the entry
    // if index is 0xFF, then no need to free the entry and buffer since this is pairing without app. description set
    // if index is not 0xff, we need to free the memory(session table entry and common buffer)
    bool funcMatching = triggerPairingConfigRxdEvent(&pairingConfigContextInfo);
    if (funcMatching)
    {
      zclRetStatus = processActionOnCompletePairing(&pairingConfigContextInfo);
    }
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
    if (ZGP_INVALID_SESSION_ENTRY_INDEX != pairingConfigContextInfo.basicContextInfo.sessionEntryIndex)
    {
      zgpServerFreeSessionEntry(pairingConfigContextInfo.basicContextInfo.sessionEntryIndex);
    }
#endif
    pairingConfigContextInfo.pairingConfigState = PAIRING_CONFIG_COMPLETE;
  }

  zgpFreeMemReqBuffer();
  return zclRetStatus;
}

/**************************************************************************//**
  \brief Removing sink table entry

  \param[in] sinkEntry - sink entry
  \param[in] sendGpPairing - To send gp pairing over the air

  \return status of remove ZPD entry
******************************************************************************/
zgpSinkTableStatus_t zgpSinkRemoveGpdEntry(ZGP_SinkTableEntry_t *sinkEntry, bool sendGpPairing)
{
  ZGP_GpdId_t gpdId;
  ZGP_TableOperationField_t filterField = {.appId = sinkEntry->options.appId, .entryType = ZGP_SINK_ENTRY, .commMode = ALL_COMMUNICATION_MODE};

  memcpy(&gpdId, &sinkEntry->tableGenericInfo.gpdId, sizeof(ZGP_GpdId_t));

  if (ZGPL_DeleteTableEntryFromNvm(filterField, &gpdId, sinkEntry->tableGenericInfo.endPoint))
  {
    if(sendGpPairing)
    {
      ZGPH_SendGpPairing(sinkEntry,REMOVE_GPD, 0xFF);
    }
    zgpTransRemoveAndAddEventHandler(NONE, sinkEntry, NULL, 0x00, NULL);
    return ZGP_SINK_TABLE_ENTRY_REMOVED;
  }

  return ZGP_SINK_TABLE_ENTRY_NOT_AVAILABLE;

}
/**************************************************************************//**
  \brief Trigger pairing config received event

  \param[in] pairingConfigFieldInfo - pairing config context info

  \return true - functionality is matching
          false - false otherwise
******************************************************************************/
static bool triggerPairingConfigRxdEvent(zgpPairingConfigContextInfo_t *pairingConfigFieldInfo)
{
  ZGP_IndicationInfo_t indicationInfo;
  ZGP_PairingConfigCmdInfo_t *pairingConfigCmdInfo = &indicationInfo.indicationData.pairingConfigInfo;

  memset(&indicationInfo, 0x00, sizeof(indicationInfo));
  pairingConfigCmdInfo->functionalityMatching = true;
  indicationInfo.indicationType = PAIRING_CONFIG_CMD_RECEIVED;
  pairingConfigCmdInfo->appId = (ZGP_ApplicationId_t)pairingConfigFieldInfo->options.appId;
  memcpy((void *)&pairingConfigCmdInfo->gpdId, (void *)&pairingConfigFieldInfo->basicContextInfo.gpdId, \
          sizeof(pairingConfigCmdInfo->gpdId));
  pairingConfigCmdInfo->gpdEndPoint = pairingConfigFieldInfo->basicContextInfo.endPoint;
  pairingConfigCmdInfo->action = pairingConfigFieldInfo->actions;
  pairingConfigCmdInfo->commMode = (zgpCommunicationMode_t)pairingConfigFieldInfo->options.communicationMode;
  pairingConfigCmdInfo->appInfo = &pairingConfigFieldInfo->basicContextInfo.gpdAppInfo;

  SYS_PostEvent(BC_EVENT_ZGPH_INDICATION, (SYS_EventData_t)&indicationInfo);

  return pairingConfigCmdInfo->functionalityMatching;
}

/**************************************************************************//**
  \brief Trigger sink comm. mode request received event

  \param[in] options - comm. mode options
             endPoint - end point

  \return unicastCommMode - by default it is false
                            can be updated by the upper layer on event indication
******************************************************************************/
static bool triggerSinkCommissioningModeReqRxdEvent(ZGP_SinkCommissioningModeOptions_t *options, uint8_t endPoint)
{
  ZGP_IndicationInfo_t indicationInfo;

  memset(&indicationInfo, 0x00, sizeof(indicationInfo));
  indicationInfo.indicationType = SINK_COMM_MODE_REQ_CMD_RECEIVED;
  indicationInfo.indicationData.sinkCommModeReqInfo.unicastCommMode = false;
  memcpy((void *)&indicationInfo.indicationData.sinkCommModeReqInfo.options, (void *)options, \
          sizeof(indicationInfo.indicationData.sinkCommModeReqInfo.options));
  indicationInfo.indicationData.sinkCommModeReqInfo.endPoint = endPoint;

  SYS_PostEvent(BC_EVENT_ZGPH_INDICATION, (SYS_EventData_t)&indicationInfo);

  return indicationInfo.indicationData.sinkCommModeReqInfo.unicastCommMode;
}

/**************************************************************************//**
  \brief Sending gpPairing for the entry

  \param[in] sinkTableEntry - table entry pointer
             action - sinktable action

  \return None
******************************************************************************/
void zgpServerSendGpPairingForEntry(ZGP_SinkTableEntry_t *sinkTableEntry, ZGP_SinkTableActions_t action)
{
  if(sinkTableEntry->options.communicationMode == PRECOMMISSIONED_GROUPCAST)
  {
    //If Comm Mode is GroupCast then Sink (Green Power EndPoint)should be added as part of Group Member at APS level
    //TBD on PairedEndPointUpdate & SinkGroupListUpdate
    for(uint8_t i = 0 ; i < ZGP_SINK_GROUP_LIST_SIZE; i++)
    {
      if (ZGP_NWK_ADDRESS_GROUP_INIT != sinkTableEntry->tableGenericInfo.zgpSinkGrouplist[i].sinkGroup)
        //Trigger GP Pairing after Updating the GpConfiguration in SinkTable Entry..
        ZGPH_SendGpPairing(sinkTableEntry,action,i);
    }
  }
  else //for Derived and LightWeight Unicast
  {
    //Trigger GP Pairing after Updating the GpConfiguration in SinkTable Entry..
    ZGPH_SendGpPairing(sinkTableEntry,action, 0x00/*unused index for non-precommissioned mode*/);
  }
}
/**************************************************************************//**
  \brief Add the group and send device announce for derived/precommissioned groupcast

  \param[in] sinkTableEntry - pointer to sink entry
             sendDeviceAnnounce - to send device announcement

  \return None
******************************************************************************/
void zgpServerAddGroup(ZGP_SinkTableEntry_t *sinkTableEntry, bool sendDeviceAnnounce)
{
  ZGP_SinkGroup_t groupList[ZGP_SINK_GROUP_LIST_SIZE];
  uint8_t indexCount = 0;
  uint16_t derivedGroupId = ZGPL_GetAliasSourceAddr(&sinkTableEntry->tableGenericInfo.gpdId);

  memset(&groupList[0], 0x00, sizeof(groupList));
  if (DERIVED_GROUPCAST == sinkTableEntry->options.communicationMode)
  {
    indexCount = 1;
    groupList[0].sinkGroup = derivedGroupId;
    groupList[0].alias = derivedGroupId;
  }
  else if(PRECOMMISSIONED_GROUPCAST == sinkTableEntry->options.communicationMode)
  {
    indexCount = ZGP_SINK_GROUP_LIST_SIZE;
    memcpy(&groupList[0], &sinkTableEntry->tableGenericInfo.zgpSinkGrouplist[0], sizeof(groupList));
  }

  for (uint8_t i = 0; i < indexCount; i++)
  {
    if (ZGP_NWK_ADDRESS_GROUP_INIT != groupList[i].sinkGroup)
    {
      if (!NWK_IsGroupMember(groupList[i].sinkGroup, GREEN_POWER_ENDPOINT))
      {
        if (!NWK_AddGroup(groupList[i].sinkGroup, GREEN_POWER_ENDPOINT))
        {
          // To be decided on handling this negative scenario
        }
      }

      if (sendDeviceAnnounce)
      {
        if (ZGP_NWK_ADDRESS_GROUP_INIT == groupList[i].alias)
          groupList[i].alias = derivedGroupId;
        ZGPL_SendDeviceAnnounceCmd(groupList[i].alias, ZGP_DEVICE_ANNCE_EXT_ADDR);
      }
    }
  }
}

/**************************************************************************//**
  \brief sending GpPairingCmd

  \param[in] sinkTableEntry - pointer to sink table entry
             action - Add/Remove
             groupIndex - index in the groupList

  \return status - status of the request
******************************************************************************/
void ZGPH_SendGpPairing(ZGP_SinkTableEntry_t *sinkTableEntry,ZGP_SinkTableActions_t action, uint8_t groupIndex)
{
  uint8_t index = 0;
  uint8_t *gpPairingPayload;
  zgpGpPairingOptions_t options;
  ZGP_GpdId_t gpdId;
  uint16_t dDroupIdAddr;
  uint16_t assignedAlias;
  uint16_t groupId;
  ZCL_Request_t *req;

  if(((ZGP_SRC_APPID == sinkTableEntry->options.appId) && ((!ZGPL_IsValidSrcId(sinkTableEntry->tableGenericInfo.gpdId.gpdSrcId, ZGP_FRAME_DATA, true))
#if MICROCHIP_APPLICATION_SUPPORT != 1
     || (sinkTableEntry->tableGenericInfo.gpdId.gpdSrcId == ZGP_ALL_SRC_ID)
#endif
      ))
      || ((ZGP_IEEE_ADDR_APPID == sinkTableEntry->options.appId) && ((!sinkTableEntry->tableGenericInfo.gpdId.gpdIeeeAddr)
#if MICROCHIP_APPLICATION_SUPPORT != 1
     || (sinkTableEntry->tableGenericInfo.gpdId.gpdIeeeAddr == ZGP_ALL_IEEE_ADDR)
#endif
      )))
     return;

  memset(&options, 0x00, sizeof(options));
  options.appId = sinkTableEntry->options.appId;
  options.communicationMode = sinkTableEntry->options.communicationMode;
  options.groupCastRadiusPresent = sinkTableEntry->tableGenericInfo.groupCastRadius? true: false;
  options.gpdFixed = sinkTableEntry->options.fixedLocation;
  options.gpdMacSeqNumCapability = sinkTableEntry->options.seqNumCapabilities;
  options.gpdSecurityKeyPresent = sinkTableEntry->options.securityUse;
  options.removeGpd = false;
  options.securityKeyType = sinkTableEntry->tableGenericInfo.securityOptions.securityKeyType;
  options.securityLevel = sinkTableEntry->tableGenericInfo.securityOptions.securityLevel;
  options.gpdSecurityFrameCounterPresent = false;
  options.assignedAliasPresent = sinkTableEntry->options.assignedAlias;

  if (EXTEND_SINKTABLE_ENTRY == action || REPLACE_SINKTABLE_ENTRY == action)
  {
    options.addSink = true;
    options.gpdSecurityFrameCounterPresent = true;
  }
  else if ((REMOVE_PAIRING == action) || (REMOVE_GPD == action))
  {
    options.addSink = false;
    options.assignedAliasPresent = false;
    options.groupCastRadiusPresent = false;
    options.gpdSecurityKeyPresent = false;
    if (REMOVE_GPD == action)
      options.removeGpd = true;
  }
  else
    return;

  if (!(options.securityLevel || options.gpdMacSeqNumCapability))
    options.gpdSecurityFrameCounterPresent = false;

  // FULL unicast not supported
  if (!options.removeGpd && (FULL_UNICAST == options.communicationMode))
    return;

  if (!(req = ZCL_CommandManagerAllocCommand()))
    return;

  gpPairingPayload = (uint8_t *)req->requestPayload;

  // Not copying the option since it may be updated later in the function
  // so just skipping index
  index += (sizeof(options) - 1);

  index +=  zgpAddGpdSourceAddrToOtaPayload((ZGP_ApplicationId_t)options.appId, &sinkTableEntry->tableGenericInfo,\
                         &gpPairingPayload[index]);

  gpdId.gpdIeeeAddr = sinkTableEntry->tableGenericInfo.gpdId.gpdIeeeAddr;
  dDroupIdAddr = ZGPL_GetAliasSourceAddr(&gpdId);
  assignedAlias = dDroupIdAddr;
  groupId = dDroupIdAddr;

  if (!options.removeGpd)
  {
    if (LIGHTWEIGHT_UNICAST == options.communicationMode)
    {
      uint16_t shortAddr = NWK_GetShortAddr();

      memcpy(&gpPairingPayload[index], MAC_GetExtAddr(), sizeof(ExtAddr_t));
      index += sizeof(ExtAddr_t);
      memcpy(&gpPairingPayload[index], &shortAddr, sizeof(shortAddr));
      index += sizeof(shortAddr);
      options.assignedAliasPresent = false;
    }
    else if (DERIVED_GROUPCAST == options.communicationMode)
    {
      memcpy(&gpPairingPayload[index], &dDroupIdAddr, sizeof(dDroupIdAddr));
      index += sizeof(dDroupIdAddr);
      if (options.assignedAliasPresent)
        assignedAlias = sinkTableEntry->tableGenericInfo.gpdAssignedAlias;
    }
    else
    {
      groupId = sinkTableEntry->tableGenericInfo.zgpSinkGrouplist[groupIndex].sinkGroup;
      memcpy(&gpPairingPayload[index], &groupId, sizeof(groupId));
      index += sizeof(groupId);
      if (options.addSink && (ZGP_NWK_ADDRESS_GROUP_INIT != sinkTableEntry->tableGenericInfo.zgpSinkGrouplist[groupIndex].alias))
      {
        assignedAlias = sinkTableEntry->tableGenericInfo.zgpSinkGrouplist[groupIndex].alias;
        options.assignedAliasPresent = true;
      }
    }

    if (options.addSink)
    {
      gpPairingPayload[index++] = sinkTableEntry->deviceId;
    }

    if (options.gpdSecurityFrameCounterPresent)
    {
      uint32_t frameCounter = 0x00;
      if (options.securityLevel || options.gpdMacSeqNumCapability)
        frameCounter = sinkTableEntry->tableGenericInfo.gpdSecurityFrameCounter;

      memcpy(&gpPairingPayload[index], &frameCounter, sizeof(frameCounter));
      index += sizeof(frameCounter);
    }
    if (options.gpdSecurityKeyPresent)
    {
      memcpy(&gpPairingPayload[index], &sinkTableEntry->tableGenericInfo.securityKey[0], sizeof(sinkTableEntry->tableGenericInfo.securityKey));
      index += sizeof(sinkTableEntry->tableGenericInfo.securityKey);
    }
    if (options.assignedAliasPresent)
    {
      memcpy(&gpPairingPayload[index], &assignedAlias, sizeof(assignedAlias));
      index += sizeof(assignedAlias);
    }
    if (options.groupCastRadiusPresent)
    {
      gpPairingPayload[index++] = sinkTableEntry->tableGenericInfo.groupCastRadius;
    }
  }

  // Updating the option field
  memcpy(&gpPairingPayload[0], &options, (sizeof(options) - 1));

  req->dstAddressing.addrMode = APS_SHORT_ADDRESS;
  req->dstAddressing.addr.shortAddress = BROADCAST_ADDR_RX_ON_WHEN_IDLE;

  zgpClusterGenericSendZgpCommand(req, (uint8_t)ZCL_CLUSTER_SIDE_CLIENT, ZCL_GP_CLUSTER_CLIENT_GP_PAIRING_COMMAND_ID, index);
}
/**************************************************************************//**
  \brief Send proxy commissioning mode command

  \param[in] options - options field
  \param[in] commissioningWindow - commissioningWindow field
  \param[in] channel - channel field

  \return zcl status
******************************************************************************/
ZCL_Status_t ZGPH_SendProxyCommissioningModeCommand(zgpGpProxyCommModeOptions_t options, uint16_t commissioningWindow, uint8_t channel)
{
  ZCL_Request_t *req;
  uint8_t *gpProxyCommModePaylod;
  uint8_t payLoadIndex = 0;

  if (!(req = ZCL_CommandManagerAllocCommand()))
    return ZCL_INSUFFICIENT_SPACE_STATUS;
 
  gpProxyCommModePaylod = (uint8_t *)req->requestPayload; 
  req->dstAddressing.addrMode = APS_SHORT_ADDRESS;
  req->dstAddressing.addr.shortAddress = 0xFFFD;

  memcpy(&gpProxyCommModePaylod[payLoadIndex], (void *)&options, sizeof(options));
  payLoadIndex += sizeof(options);

  if (options.commWindowPresent)
  {
    // Exit mode - On commissioning window expiration
    memcpy((void *)&gpProxyCommModePaylod[payLoadIndex], (void *)&commissioningWindow, sizeof(uint16_t));
    payLoadIndex += sizeof(uint16_t);
  }

  if (options.channelPresent)
  {
    // Channel present
    gpProxyCommModePaylod[payLoadIndex++] =  channel;
  }

  zgpClusterGenericSendZgpCommand(req, ZCL_CLUSTER_SIDE_CLIENT, ZCL_GP_CLUSTER_CLIENT_GP_PROXY_COMM_MODE_COMMAND_ID, \
                                  payLoadIndex);

  return ZCL_SUCCESS_STATUS;
}

/**************************************************************************//**
  \brief Sending proxy table request.

  \param[in] addr - dst addr
  \param[in] options - options field
  \param[in] gpdId_Ieee - gpdId / IEEE addr
  \param[in] ep - ep for IEEE addr gpd
  \param[in] index - index field

  \return zcl status
******************************************************************************/
ZCL_Status_t ZGPH_SendProxyTableRequest(uint16_t addr, uint8_t options, uint64_t gpdId_Ieee, uint8_t ep, uint8_t index)
{
  ZCL_Request_t *req;
  uint8_t *gpProxyTableReqPaylod;
  uint8_t payLoadIndex = 0;
  zgpTableReqOptions_t *proxyTableReqOptions;

  if (!(req = ZCL_CommandManagerAllocCommand()))
    return ZCL_INSUFFICIENT_SPACE_STATUS;

  gpProxyTableReqPaylod = (uint8_t *)req->requestPayload;

  req->dstAddressing.addrMode = APS_SHORT_ADDRESS;
  req->dstAddressing.addr.shortAddress = addr;

  gpProxyTableReqPaylod[payLoadIndex++] = options;

  proxyTableReqOptions = (zgpTableReqOptions_t *)&options;
  if (GPD_ID_REF == proxyTableReqOptions->requestType)
  {
    if (ZGP_SRC_APPID == proxyTableReqOptions->appId)
    {
      memcpy((void *)&gpProxyTableReqPaylod[payLoadIndex], (void *)&gpdId_Ieee, sizeof(uint32_t));
      payLoadIndex += sizeof(uint32_t);
    }
    else
    {
      memcpy((void *)&gpProxyTableReqPaylod[payLoadIndex], (void *)&gpdId_Ieee, sizeof(uint64_t));
      payLoadIndex += sizeof(uint64_t);

      gpProxyTableReqPaylod[payLoadIndex++] = ep;
    }
  }
  else
  {
    gpProxyTableReqPaylod[payLoadIndex++] = index;
  }

  zgpClusterGenericSendZgpCommand(req, ZCL_CLUSTER_SIDE_CLIENT, ZCL_GP_CLUSTER_CLIENT_GP_PROXY_TABLE_REQ_COMMAND_ID, \
                                  payLoadIndex);

  return ZCL_SUCCESS_STATUS;
}

/**************************************************************************//**
  \brief sending GP pairing/GP response cmd

  \param[in] payLoadLength - payload length to be sent
             payLoad - pointer to payload to be sent

  \return status - status of the request
******************************************************************************/
ZCL_Status_t gpSendGpPairingRspCommand(uint16_t cmdId, uint8_t payLoadLength, uint8_t  *payLoad)
{
  ZCL_Request_t *req;
  uint8_t *gpPairingPayload;

  if (!(req = ZCL_CommandManagerAllocCommand()))
    return ZCL_INSUFFICIENT_SPACE_STATUS;

  req->dstAddressing.addrMode = APS_SHORT_ADDRESS;
  req->dstAddressing.addr.shortAddress = BROADCAST_ADDR_RX_ON_WHEN_IDLE;

  gpPairingPayload = (uint8_t *)req->requestPayload;
  memcpy((void *)gpPairingPayload, payLoad, payLoadLength);

  zgpClusterGenericSendZgpCommand(req, ZCL_CLUSTER_SIDE_CLIENT, cmdId, \
                                  payLoadLength);

  return ZCL_SUCCESS_STATUS;

}
/**************************************************************************//**
\brief Attribute Event indication handler(to indicate when attr values have
        read or written)

\param[in] addressing - pointer to addressing information;
\param[in] attributeId - attribute id
\param[in] event - Event triggering this callback
 \return -none
******************************************************************************/
static void zgpSinkAttrEventInd(ZCL_Addressing_t *addressing, ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event)
{
  if(event == ZCL_WRITE_ATTRIBUTE_EVENT)
  {
    PDS_Store(ZGP_SINK_ATTR_MEM_ID);
  }
}

/**************************************************************************//**
  \brief Parsing application info. from the payload

  \param[in] gpdAsdu - payload info
             payloadIndex - index of payload to be parsed
                            after parsing, this will be advanced
             appInfo - application information to be populated
             rxdFrameLength - length of the received payload

  \return true - continue with gpdf processing
          false - not proceeding with gpdf processing
******************************************************************************/
bool zgpParsingAppInfoFromPayload(uint8_t *gpdAsdu, uint8_t *payloadIndex, ZGP_GpdAppInfo_t *appInfo, uint16_t rxdFrameLength)
{
  uint8_t index = *payloadIndex;

  memcpy(&appInfo->appInfoOptions, &gpdAsdu[index], sizeof(appInfo->appInfoOptions));
  index++;

  // Update translation table and lookup table
  if (appInfo->appInfoOptions.manufacturerIdPresent)
  {
    memcpy(&appInfo->manfacId, &gpdAsdu[index], sizeof(appInfo->manfacId));
    index += sizeof(appInfo->manfacId);
  }
  if (appInfo->appInfoOptions.modelIdPresent)
  {
    memcpy(&appInfo->modelId, &gpdAsdu[index], sizeof(appInfo->modelId));
    index += sizeof(appInfo->modelId);
  }
  if (appInfo->appInfoOptions.gpdCommandPresent)
  {
    uint8_t noOfCmds = gpdAsdu[index++];
    appInfo->noOfGpdCmds = noOfCmds;

    if (appInfo->noOfGpdCmds > COMM_REQ_COMMAND_COUNT)
      noOfCmds = COMM_REQ_COMMAND_COUNT;

    for (uint8_t i=0; i < noOfCmds; i++)
      appInfo->gpdCommandList[i] = gpdAsdu[index + i];
    index += appInfo->noOfGpdCmds;
  }
  if (appInfo->appInfoOptions.clusterListPresent)
  {
    memcpy(&appInfo->noOfClusters, &gpdAsdu[index++], sizeof(zgpLengthOfClusterList_t));

    {
      uint8_t serverClusterCnt = appInfo->noOfClusters.numOfServerCluster;
      uint8_t clientClusterCnt = appInfo->noOfClusters.numOfClientCluster;

      if (serverClusterCnt > COMM_REQ_SERVER_CLUSTER_COUNT)
        serverClusterCnt = COMM_REQ_SERVER_CLUSTER_COUNT;

      if (clientClusterCnt > COMM_REQ_CLIENT_CLUSTER_COUNT)
        clientClusterCnt = COMM_REQ_CLIENT_CLUSTER_COUNT;

      for (uint8_t i = 0; i < serverClusterCnt; i++)
        memcpy(&appInfo->clusterListServer[i], &gpdAsdu[index + i*2], sizeof(appInfo->clusterListServer[i]));

      index += (appInfo->noOfClusters.numOfServerCluster * sizeof(uint16_t));

      if (rxdFrameLength < index)
        return false;

      for (uint8_t i = 0; i < clientClusterCnt; i++)
        memcpy(&appInfo->clusterListClient[i], &gpdAsdu[index + i*2], sizeof(appInfo->clusterListClient[i]));

      index += (appInfo->noOfClusters.numOfClientCluster * sizeof(uint16_t));
    }
  }
#ifdef ZGP_ENABLE_GENERIC_8_CONTACT_SWITCH_SUPPORT
  if (appInfo->appInfoOptions.switchInformationPresent)
  {
     appInfo->switchInfo.switchInfoLength = gpdAsdu[index];
     index++;
     memcpy(&appInfo->switchInfo.genericSwitchConfig, &gpdAsdu[index], sizeof(uint8_t));
     index++;
     appInfo->switchInfo.currContactStatus = gpdAsdu[index];
     index++;

     if(appInfo->switchInfo.switchInfoLength != ZGP_GENERIC_SWITCH_MIN_LENGTH) //value 0x02 defined as per spec A.4.2.1.1.10 
        return false;
     // As per spec A.4.2.1.1.10 
     if(appInfo->switchInfo.currContactStatus == INVALID_CONTACT_VALUE || appInfo->switchInfo.genericSwitchConfig.noOfContacts == INVALID_CONTACT_VALUE)
       return false;
  }
#endif
  if (rxdFrameLength < index)
  {
    return false;
  }
  *payloadIndex = index;
  return true;
}
#endif // APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
#endif // _GREENPOWER_SUPPORT_

// eof zgpClusterServer.c
