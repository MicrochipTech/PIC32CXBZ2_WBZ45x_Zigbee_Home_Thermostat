/*******************************************************************************
  Zigbee green power cluster client Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpClusterClient.c

  Summary:
    This file contains the green power cluster client side implementation.

  Description:
    This file contains the green power cluster client side implementation.
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
#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC

/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcl.h>
#include <zcl/include/zclParser.h>
#include <zcl/include/zclGreenPowerCluster.h>
#include <zcl/include/zclCommandManager.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpInfraDevice.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpProxyBasic.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpProxyTable.h>
#include <nwk/include/nwkAttributes.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLow.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighGeneric.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighMem.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterGeneric.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterClient.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpClusterStructure.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpCluster.h>
#include <zgp/include/zgpDbg.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpSinkTable.h>
#include <systemenvironment/include/sysEvents.h>
#include <systemenvironment/include/sysAssert.h>
#include <pds/include/wlPdsMemIds.h>
/******************************************************************************
                    Define section
******************************************************************************/
#define GP_LINK_KEY_DEFAULT_VALUE "\x5a\x69\x67\x42\x65\x65\x41\x6c\x6c\x69\x61\x6e\x63\x65\x30\x39"
#define GP_SHARED_KEY_INIT_VALUE  "\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
#define TUNNELING_DELAY_STEP_SIZE     32
#define LINK_QUALITY_MAX_VAL          3
#define LENGTH_OF_FREE_ENTRY         0x0
#define INVALID_TUNNELING_TIME       0xFFFF
#define RSSI_LOW_THRESHOLD_VALUE     (-109)
#define RSSI_HIGH_THRESHOLD_VALUE    8
#define RSSI_OFFSET_VALUE            110
#define LQI_STEP_SIZE                64 // For the range 0-255 & 2 bit resolution

#if MICROCHIP_APPLICATION_SUPPORT == 1
#define GP_NOTIFICATION_RESP_COMMAND_IND NULL
#else
#define GP_NOTIFICATION_RESP_COMMAND_IND gpNotificationRespInd
#endif

/******************************************************************************
                    Prototypes section
******************************************************************************/
static ZCL_Status_t gpPairingInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
static ZCL_Status_t gpProxyCommModeInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
static ZCL_Status_t gpRespInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
static ZCL_Status_t gpProxyTableReqInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
static ZCL_Status_t gpSinkTableRespInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
static ZCL_Status_t zgppParseGPPairingCmd(ZGP_GpPairing_t *gpPairingCmd, uint8_t *payload, uint8_t payloadLength);
static bool zgppParseProxyCommModeCmd(ZGP_GpProxyCommMode_t *gpProxyCommModeCmd, uint8_t *payload,\
                                      uint8_t payloadLength);
static bool zgppParseGPRespCmd(ZCL_Addressing_t *addressing, ZGP_GpResp_t *gpRespCmd, uint8_t *payload, uint8_t payloadLength);
static uint8_t gpBuildPayloadAndAddressingHeader(ZGP_LowDataInd_t *dstubInd, uint8_t *gpNotificationPayload, \
                                                 bool includeMic);
static uint8_t getAliasSeqNo(uint8_t macSeqNo, zgpCommunicationMode_t commMode);
static zgpClientTunnelingEntry_t * getFreeTunnelingEntry(uint8_t bufferLength);
static void tunnelingTimerCallback(void);
static void zgpProxyAttrEventInd(ZCL_Addressing_t *addressing, ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event);
static void addToTunnelingTimer(zgpClientTunnelingEntry_t *tunnelingEntry);
static void startGppTunnelingTimer(uint16_t interval);
static void stopGppTunnelingTimer(void);
static bool sendCommNotification(zgpClientTunnelingEntry_t *tunnelingEntry, \
                                 bool inBroadCast);
static zgpClientTunnelingEntry_t *allocAndInitTunnelingEntries(bool isSecProcFailedSet, \
                                                               ZGP_LowDataInd_t *dstubInd);
static bool sendGpNotificationInUnicastOrGroupCastCommMode(zgpClientTunnelingEntry_t *tunnelingEntry, bool inUnicastMode);
static bool allocateAndSendNotification(zgpClientTunnelingEntry_t *tunnelingEntry, ZGP_Addressing_t *dstAddressing);
static uint8_t getNextAvailableShortAddressOrGroupAddressFromList(uint8_t index, ZGP_ProxyTableEntry_t *entry, ZGP_Addressing_t *addressing);
static ZCL_Status_t gpTransTableRespInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
#if MICROCHIP_APPLICATION_SUPPORT != 1
static ZCL_Status_t gpNotificationRespInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
#endif

#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
static uint8_t zgpBuildReportDescPayload(ZGP_GpdAppInfo_t *commReqAppInfo, uint8_t *tempPayload);
static ZGP_InfraDeviceStatus_t zgpBuildApplDescInPairingConfigPayload(ZGP_GpdAppInfo_t *commReqAppInfo, uint8_t *gpPairingTempPayload, uint8_t *payloadLength, uint8_t maxPairingConfigPayloadSize);
#endif
/******************************************************************************
                    Internal variables
******************************************************************************/
static zgpClientTunnelingEntry_t proxyTunnelingEntries[TOTAL_NO_OF_TUNNELING_ENTRIES];
static uint8_t tunnelingBuffer[TOTAL_TUNNELING_BUFFER_SIZE];
static HAL_AppTimer_t gppTunnelingTimer;
static BcTime_t gppTunnelingSysTime = 0;
ZGP_GpPairingConfigPendingInfo_t gpPairingConfigPendingInfo;
/******************************************************************************
                    Global variables
******************************************************************************/
const ZCL_GreenPowerClusterCommands_t zgpClusterClientCommands =
{
  ZCL_DEFINE_GP_CLUSTER_COMMANDS(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, \
                                 NULL, GP_NOTIFICATION_RESP_COMMAND_IND, gpPairingInd, gpProxyCommModeInd, gpRespInd, \
                                 gpTransTableRespInd, gpSinkTableRespInd, gpProxyTableReqInd)
};

ZCL_GreenPowerClusterClientAttributes_t zgpClusterClientAttributes =
{
  ZCL_DEFINE_GP_CLUSTER_CLIENT_ATTRIBUTES()
};

ClusterId_t zgpClientClusterIds[1] =
{
  GREEN_POWER_CLUSTER_ID
};

ZCL_Cluster_t zgpClientClusters[1] =
{
  DEFINE_GP_CLUSTER(ZCL_CLIENT_CLUSTER_TYPE,
      &zgpClusterClientAttributes, &zgpClusterClientCommands)
};

/******************************************************************************
                    Implementations section
******************************************************************************/

/**************************************************************************//**
\brief Initialize Green power client cluster.
 \return None.
******************************************************************************/
void zgpClusterClientInit(void)
{
  uint32_t gppFunctionality = GPP_FEATURE | GPP_STUB | GPP_DERIVED_GROUP_COMM | GPP_PRE_COMM_GROUP_COMM | \
                              GPP_LIGHTWEIGHT_UNICAST_COMM | GPP_COMMISSIONING | GPP_CT_BASED_COMMISSIONING | \
                              GPP_SECURITY_LEVEL_0 | GPP_SECURITY_LEVEL_2 | GPP_SECURITY_LEVEL_3 | GPP_IEEE_ADDR;;
  uint32_t gppActiveFunctionality = GPP_GP_FUNCTIONALITY | GPP_ACTIVE_FUNCTIONALITY_FIXED_VALUE;
  ZCL_Cluster_t *cluster = ZCL_GetCluster(GREEN_POWER_ENDPOINT, GREEN_POWER_CLUSTER_ID, ZCL_CLUSTER_SIDE_CLIENT);
  uint16_t bufferIndex = 0;

  if (cluster)
  {
    cluster->ZCL_AttributeEventInd = zgpProxyAttrEventInd;
  }

  memcpy(&zgpClusterClientAttributes.gppFunctionality.value[0], (void *)&gppFunctionality, sizeof(zgpClusterClientAttributes.gppFunctionality.value));
  memcpy(&zgpClusterClientAttributes.gppActiveFunctionality.value[0], (void *)&gppActiveFunctionality, sizeof(zgpClusterClientAttributes.gppActiveFunctionality.value));
  zgpClusterClientAttributes.gppMaxProxyTableEntries.value = ZGP_PROXY_TABLE_SIZE;
  zgpClusterClientAttributes.clusterRevision.value = GP_CLUSTER_REVISION;
  // Initialize long octet string length fields for proxy table and blocked gpdId
  memset(&zgpClusterClientAttributes.proxyTable.value[0], 0x00, sizeof(zgpClusterClientAttributes.proxyTable.value));
  zgpClusterClientAttributes.gpSharedSecurityKeyType.value = ZGP_KEY_TYPE_NO_KEY;
  memcpy(&zgpClusterClientAttributes.gpLinkKey.value[0], (uint8_t *)GP_LINK_KEY_DEFAULT_VALUE , sizeof(zgpClusterClientAttributes.gpLinkKey.value));
  memcpy(&zgpClusterClientAttributes.gpSharedSecuritykey.value[0], (uint8_t *)GP_SHARED_KEY_INIT_VALUE , sizeof(zgpClusterClientAttributes.gpSharedSecuritykey.value));

  if(PDS_IsAbleToRestore(ZGP_PROXY_SHARED_KEY_TYPE_MEM_ID))
  {
    PDS_Restore(ZGP_PROXY_SHARED_KEY_TYPE_MEM_ID);
  }
  if(PDS_IsAbleToRestore(ZGP_PROXY_SHARED_KEY_MEM_ID))
  {
    PDS_Restore(ZGP_PROXY_SHARED_KEY_MEM_ID);
  }
  if(PDS_IsAbleToRestore(ZGP_PROXY_LINK_KEY_MEM_ID))
  {
    PDS_Restore(ZGP_PROXY_LINK_KEY_MEM_ID);
  }

  for (uint8_t i=0; i < TOTAL_NO_OF_TUNNELING_ENTRIES; i++)
  {
    proxyTunnelingEntries[i].notificationBuffer.bufferLength = TUNNELING_SMALL_BUFFER_SIZE;
    if (i >= TUNNELING_SMALL_BUFFER_COUNT)
      proxyTunnelingEntries[i].notificationBuffer.bufferLength = TUNNELING_LARGE_BUFFER_SIZE;
    proxyTunnelingEntries[i].notificationBuffer.buffer = &tunnelingBuffer[bufferIndex];
    bufferIndex += proxyTunnelingEntries[i].notificationBuffer.bufferLength;
  }
  SYS_E_ASSERT_FATAL((TOTAL_TUNNELING_BUFFER_SIZE == bufferIndex), ZGP_CLIENT_INVALID_GP_BUFFER_LENGTH);
}

/**************************************************************************//**
  \brief GP pairing indication

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return status of gpPairing indication.
******************************************************************************/
static ZCL_Status_t gpPairingInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  ZGP_GpPairing_t gpPairingCmd;
  ZCL_Status_t retStatus = ZCL_SUCCESS_STATUS;

  memset(&gpPairingCmd, 0x00, sizeof(ZGP_GpPairing_t));
  retStatus =  zgppParseGPPairingCmd(&gpPairingCmd, payload, payloadLength);

  if (retStatus == ZCL_SUCCESS_STATUS)
    retStatus = zgpProxyDataHandling(GP_CLUSTER_GP_PAIRING_CMD,&gpPairingCmd);
 
  return retStatus;
}

/**************************************************************************//**
  \brief GP proxy commissioning mode indication

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return status of gpProxyCommMode indication.
******************************************************************************/
static ZCL_Status_t gpProxyCommModeInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  ZGP_GpProxyCommMode_t gpProxyCommModeCmd;

  memset(&gpProxyCommModeCmd, 0x00, sizeof(ZGP_GpProxyCommMode_t));
  gpProxyCommModeCmd.srcSinkNwkAddress = addressing->addr.shortAddress;
  if (zgppParseProxyCommModeCmd(&gpProxyCommModeCmd, payload, payloadLength))
    zgpProxyDataHandling(GP_CLUSTER_GP_PROXY_COMMISSIONING_CMD, &gpProxyCommModeCmd);

  (void)payloadLength;
  return ZCL_SUCCESS_STATUS;
}

/**************************************************************************//**
  \brief GP response indication

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return status of gp Response indication.
******************************************************************************/
static ZCL_Status_t gpRespInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  ZGP_GpResp_t gpResponseCmd;

  memset(&gpResponseCmd, 0x00, sizeof(ZGP_GpResp_t));
  if (zgppParseGPRespCmd(addressing, &gpResponseCmd, payload, payloadLength))
    zgpProxyDataHandling(GP_CLUSTER_GP_RESPONSE_CMD, &gpResponseCmd);

  return ZCL_SUCCESS_STATUS;
}

/**************************************************************************//**
  \brief GP proxy table request indication

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return status of proxy table request indication.
******************************************************************************/
static ZCL_Status_t gpProxyTableReqInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  return zgpClusterGenericTableReqIndHandling(addressing, payloadLength, payload, true);
}
/**************************************************************************//**
  \brief GP sink table response indication

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return status.
******************************************************************************/
static ZCL_Status_t gpSinkTableRespInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  return zgpClusterCommandHandler(GP_SINK_TABLE_RESP_IND, addressing, payloadLength, payload);
}

/**************************************************************************//**
  \brief GP sink table response indication

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return status.
******************************************************************************/
static ZCL_Status_t gpTransTableRespInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  return zgpClusterCommandHandler(GP_TRANSLATION_TABLE_RSP_IND, addressing, payloadLength, payload);
}

/**************************************************************************//**
  \brief Sending sink table request

  \param[in] addr - dst addr
  \param[in] options - options field
  \param[in] gpdId_Ieee - gpdId / IEEE addr
  \param[in] ep - ep for IEEE addr gpd
  \param[in] index - index field

  \return zcl status
******************************************************************************/
ZCL_Status_t ZGPH_SendSinkTableRequest(uint16_t addr, uint8_t options, uint64_t gpdId_Ieee, uint8_t ep, uint8_t index)
{
  ZCL_Request_t *req;
  uint8_t *gpSinkTableReqPaylod;
  uint8_t payLoadIndex = 0;
  zgpTableReqOptions_t *sinkTableReqOptions = (zgpTableReqOptions_t *)&options;

  if (!(req = ZCL_CommandManagerAllocCommand()))
    return ZCL_INSUFFICIENT_SPACE_STATUS;

  gpSinkTableReqPaylod = (uint8_t *)req->requestPayload;

  req->dstAddressing.addrMode = APS_SHORT_ADDRESS;
  req->dstAddressing.addr.shortAddress = addr;

  gpSinkTableReqPaylod[payLoadIndex++] = options;

  if (GPD_ID_REF == sinkTableReqOptions->requestType)
  {
    if (ZGP_SRC_APPID == sinkTableReqOptions->appId)
    {
      memcpy((void *)&gpSinkTableReqPaylod[payLoadIndex], (void *)&gpdId_Ieee, sizeof(uint32_t));
      payLoadIndex += sizeof(uint32_t);
    }
    else
    {
      memcpy((void *)&gpSinkTableReqPaylod[payLoadIndex], (void *)&gpdId_Ieee, sizeof(uint64_t));
      payLoadIndex += sizeof(uint64_t);
      gpSinkTableReqPaylod[payLoadIndex++] = ep;
    }
  }
  else
  {
    gpSinkTableReqPaylod[payLoadIndex++] = index;
  }

  zgpClusterGenericSendZgpCommand(req, ZCL_CLUSTER_SIDE_SERVER, ZCL_GP_CLUSTER_SERVER_GP_SINK_TABLE_REQUEST_COMMAND_ID, \
                                  payLoadIndex);

  return ZCL_SUCCESS_STATUS;
}

#if MICROCHIP_APPLICATION_SUPPORT != 1
/**************************************************************************//**
  \brief GP notification responsesink table response indication

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return status.
******************************************************************************/
static ZCL_Status_t gpNotificationRespInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  return zgpClusterCommandHandler(GP_NOTIFICATION_RESP_IND, addressing, payloadLength, payload);
}
#endif

/**************************************************************************//**
  \brief GP pairing command parsing

  \param[in] gpPairingCmd- pairing command info
             payload- payload to parse
             payloadLength- length of the payload

  \return true - successful parsing
          false - invalid payload
******************************************************************************/
static ZCL_Status_t zgppParseGPPairingCmd(ZGP_GpPairing_t *gpPairingCmd, uint8_t *payload, \
                                                         uint8_t payloadLength)
{
  uint8_t payloadIndex = 0;

  memcpy(&gpPairingCmd->options, &payload[payloadIndex], GP_PAIRING_OPTIONS_LENGTH);
  payloadIndex += GP_PAIRING_OPTIONS_LENGTH;

  if((true == gpPairingCmd->options.addSink && true == gpPairingCmd->options.removeGpd) || \
   (gpPairingCmd->options.securityLevel == ZGP_SECURITY_LEVEL_1) || \
   (gpPairingCmd->options.securityLevel && ((ZGP_KEY_TYPE_RESERVED1 == gpPairingCmd->options.securityKeyType) || \
   (ZGP_KEY_TYPE_RESERVED2 == gpPairingCmd->options.securityKeyType))))
  {
    // drop the frame 
    return ZCL_INVALID_FIELD_STATUS;
  }

  {
    uint8_t gpdIdLength = ZGPH_ParseGpdIdFromPayload((ZGP_ApplicationId_t)gpPairingCmd->options.appId, &gpPairingCmd->gpdId, \
                                                                &gpPairingCmd->endpoint, &payload[payloadIndex]);
    if (INVALID_CMD_LENGTH == gpdIdLength)
      return ZCL_INVALID_FIELD_STATUS;
    payloadIndex += gpdIdLength;
  }

  // Read sink IEEE & NWK address (or) sinkGroupId based on
  // RemoveGPD and Communication mode
  // Refer Table 37 in docs-14-0563-13-batt-errata-for-proxy-basic
  if(gpPairingCmd->options.removeGpd == false)
  {
    if (gpPairingCmd->options.communicationMode == LIGHTWEIGHT_UNICAST)
    {
      ExtAddr_t sinkIeeeAddr;
      uint16_t sinkNwkAddr;

      memcpy(&sinkIeeeAddr, &payload[payloadIndex], sizeof(gpPairingCmd->sinkIeeeAddr));
      gpPairingCmd->sinkIeeeAddr = sinkIeeeAddr;
      payloadIndex += sizeof(gpPairingCmd->sinkIeeeAddr);
      memcpy(&sinkNwkAddr, &payload[payloadIndex], sizeof(gpPairingCmd->sinkNwkAddr));
      gpPairingCmd->sinkNwkAddr = sinkNwkAddr;
      payloadIndex += sizeof(gpPairingCmd->sinkNwkAddr);
    }
    else if((gpPairingCmd->options.communicationMode == DERIVED_GROUPCAST || gpPairingCmd->options.communicationMode == PRECOMMISSIONED_GROUPCAST))
    {
      uint16_t sinkGroupId;
      memcpy(&sinkGroupId, &payload[payloadIndex], sizeof(gpPairingCmd->sinkGroupId));
      gpPairingCmd->sinkGroupId = sinkGroupId;
      payloadIndex += sizeof(gpPairingCmd->sinkGroupId);
    }
    else
      return ZCL_INVALID_FIELD_STATUS;
  }
  // Read DeviceId, GPD SecurityFrameCounter, GPD key, Assigned alais, groupCast radius
  if(gpPairingCmd->options.addSink == true)
  {
    //Device Id
    uint8_t deviceId;
    memcpy(&deviceId, &payload[payloadIndex], sizeof(gpPairingCmd->deviceId));
    gpPairingCmd->deviceId = deviceId;
    payloadIndex += sizeof(gpPairingCmd->deviceId);
    // GPD security frame Counter
    if(gpPairingCmd->options.gpdSecurityFrameCounterPresent)
    {
      uint32_t gpdSecurityFrameCounter;
      memcpy(&gpdSecurityFrameCounter, &payload[payloadIndex], sizeof(gpPairingCmd->gpdSecurityFrameCounter));
      gpPairingCmd->gpdSecurityFrameCounter = gpdSecurityFrameCounter;
      payloadIndex += sizeof(gpPairingCmd->gpdSecurityFrameCounter);
      // If security level is zero & mac seq. num capabilities is false, then security framecounter should be zero
      // As per A.3.3.5.2
      if ((ZGP_SECURITY_LEVEL_0 == gpPairingCmd->options.securityLevel) && (!gpPairingCmd->options.gpdMacSeqNumCapability) && \
          (gpPairingCmd->gpdSecurityFrameCounter))
        return ZCL_INVALID_FIELD_STATUS;
    }
    //GPD key
    if(gpPairingCmd->options.gpdSecurityKeyPresent)
    {
      memcpy(&gpPairingCmd->gpdKey[0], &payload[payloadIndex], ZGP_SECURITY_KEY_LENGTH);
      payloadIndex += ZGP_SECURITY_KEY_LENGTH;
    }
    // Assigned Alias
    if(gpPairingCmd->options.assignedAliasPresent)
    {
      uint16_t assignedAlias;
      memcpy(&assignedAlias, &payload[payloadIndex], sizeof(gpPairingCmd->assignedAlias));
      gpPairingCmd->assignedAlias = assignedAlias;
      payloadIndex += sizeof(gpPairingCmd->assignedAlias);
    }
    // groupCast radius
    if(gpPairingCmd->options.groupCastRadiusPresent)
    {
      gpPairingCmd->groupCastRadius =  payload[payloadIndex++];
    }
  }

  if (payloadIndex <= payloadLength)
  {
    if(((ZGP_SRC_APPID == gpPairingCmd->options.appId) && ((!ZGPL_IsValidSrcId(gpPairingCmd->gpdId.gpdSrcId, ZGP_FRAME_DATA, true))
#if MICROCHIP_APPLICATION_SUPPORT != 1
     || (gpPairingCmd->gpdId.gpdSrcId == ZGP_ALL_SRC_ID)
#endif
      ))
      || ((ZGP_IEEE_ADDR_APPID == gpPairingCmd->options.appId) && ((!gpPairingCmd->gpdId.gpdIeeeAddr) 
#if MICROCHIP_APPLICATION_SUPPORT != 1
     || (gpPairingCmd->gpdId.gpdIeeeAddr == ZGP_ALL_IEEE_ADDR)
#endif
       )))
      return ZCL_INVALID_FIELD_STATUS;
    else
      return ZCL_SUCCESS_STATUS;
  }
  else
    return ZCL_MALFORMED_COMMAND_STATUS;

}

/**************************************************************************//**
  \brief GP proxy commissioning mode command parsing

  \param[in] gpProxyCommModeCmd = cmd to be parsed
             payload -payload to parse
             payloadLength - rxd payload length

  \return true - valid frame
          false - otherwise.
******************************************************************************/
static bool zgppParseProxyCommModeCmd(ZGP_GpProxyCommMode_t *gpProxyCommModeCmd, uint8_t *payload,\
                                      uint8_t payloadLength)
{
  uint8_t payloadIndex = 0;

  //Read options
  memcpy(&gpProxyCommModeCmd->options, &payload[payloadIndex], sizeof(gpProxyCommModeCmd->options));
  payloadIndex += sizeof(gpProxyCommModeCmd->options);
  // Read commissioning window
  if(gpProxyCommModeCmd->options.commWindowPresent) // on commissioning window expiration
  {
    uint16_t commWindow;
    memcpy(&commWindow, &payload[payloadIndex], sizeof(gpProxyCommModeCmd->commWindow));
    gpProxyCommModeCmd->commWindow = commWindow;
    payloadIndex += sizeof(gpProxyCommModeCmd->commWindow);
  }
  //Read channel
  if(gpProxyCommModeCmd->options.channelPresent == true)
  {
    uint8_t channel;
    memcpy(&channel, &payload[payloadIndex], sizeof(gpProxyCommModeCmd->channel));
    gpProxyCommModeCmd->channel = channel;
    payloadIndex += sizeof(gpProxyCommModeCmd->channel);
  }
  if (payloadLength < payloadIndex)
    return false;
  return true;
}

/**************************************************************************//**
  \brief GP response command parsing

  \param[in] addressing - Addressing info
             gpRespCmd - pointer to gpResponse command
             payload - payload to parse
             payloadLength - length of the payload
  \return true - correct GP response
          false - incorrect GP response.
******************************************************************************/
static bool zgppParseGPRespCmd(ZCL_Addressing_t *addressing, ZGP_GpResp_t *gpRespCmd, uint8_t *payload, uint8_t payloadLength)
{
  zgpGpRespOptions_t options;
  uint16_t tempMasterShortAddr;
  uint8_t tempMasterTxChannel;
  uint8_t payloadIndex = 0;

  memcpy(&options, &payload[payloadIndex], sizeof(gpRespCmd->options));
  gpRespCmd->options = options;
  payloadIndex += sizeof(gpRespCmd->options);

  //Read TempMasterShort Address
  memcpy(&tempMasterShortAddr, &payload[payloadIndex], sizeof(gpRespCmd->tempMasterShortAddr));
  gpRespCmd->tempMasterShortAddr = tempMasterShortAddr;
  payloadIndex += sizeof(gpRespCmd->tempMasterShortAddr);

  //Read TempMaster Tx channel
  memcpy(&tempMasterTxChannel, &payload[payloadIndex], sizeof(gpRespCmd->tempMasterTxChannel));
  gpRespCmd->tempMasterTxChannel = tempMasterTxChannel;
  payloadIndex += sizeof(gpRespCmd->tempMasterTxChannel);

  {
    uint8_t gpdIdLength = ZGPH_ParseGpdIdFromPayload((ZGP_ApplicationId_t)gpRespCmd->options.appId, &gpRespCmd->gpdId, \
                                                                &gpRespCmd->endpoint, &payload[payloadIndex]);
    if (INVALID_CMD_LENGTH == gpdIdLength)
      return false;
    payloadIndex += gpdIdLength;
  }

  //Read GPD cmd ID
  gpRespCmd->gpdCmdId = payload[payloadIndex++];
  
  //Remaining bytes are the payload
  gpRespCmd->gpdCmdPayloadLength = payload[payloadIndex++];

  if ((payloadIndex > payloadLength) || \
      (gpRespCmd->gpdCmdPayloadLength != (payloadLength - payloadIndex)))
    return false;

  memcpy(gpRespCmd->gpdCmdPayload, &payload[payloadIndex], gpRespCmd->gpdCmdPayloadLength);

  gpRespCmd->nonUnicast = addressing->nonUnicast;
  return true;
}

/**************************************************************************//**
  \brief Schedule the GP commissioning/notification to tunneling Timer

  \param[in] tunnelingEntry - tunneling entry info
  \return - none
******************************************************************************/
static void addToTunnelingTimer(zgpClientTunnelingEntry_t *tunnelingEntry)
{
  uint16_t timerInterval = MIN(tunnelingEntry->lightweightUnicastModeTime, tunnelingEntry->groupModeTime);

  if (!gppTunnelingTimer.interval)
  {
    startGppTunnelingTimer(timerInterval);
  }
  else
  {
    BcTime_t currentTime = HAL_GetSystemTime();
    uint16_t elapsedTime = currentTime - gppTunnelingSysTime;

    if ((gppTunnelingTimer.interval - elapsedTime) > timerInterval)
    {
      stopGppTunnelingTimer();
      // Need to update all the entry lightweightUnicastTime/groupCast time
      // if it is valid
      for (uint8_t i = 0; i < TOTAL_NO_OF_TUNNELING_ENTRIES; i++)
      {
        if ((LENGTH_OF_FREE_ENTRY != proxyTunnelingEntries[i].cmdPayloadLength) && \
            (&proxyTunnelingEntries[i] != tunnelingEntry))
        {
          if (INVALID_TUNNELING_TIME != proxyTunnelingEntries[i].lightweightUnicastModeTime)
          {
            proxyTunnelingEntries[i].lightweightUnicastModeTime -= elapsedTime;
          }
          if (INVALID_TUNNELING_TIME != proxyTunnelingEntries[i].groupModeTime)
          {
            proxyTunnelingEntries[i].groupModeTime -= elapsedTime;
          }
        }
      }
      startGppTunnelingTimer(timerInterval);
    }
    else
    {
      if (INVALID_TUNNELING_TIME != tunnelingEntry->lightweightUnicastModeTime)
        tunnelingEntry->lightweightUnicastModeTime += elapsedTime;

      if (INVALID_TUNNELING_TIME != tunnelingEntry->groupModeTime)
        tunnelingEntry->groupModeTime += elapsedTime;
    }
  }
}

/**************************************************************************//**
  \brief Start/restart the tunneling timer

  \param[in] interval - timer interval
******************************************************************************/
static void startGppTunnelingTimer(uint16_t interval)
{
  gppTunnelingSysTime = HAL_GetSystemTime();
  gppTunnelingTimer.interval = interval;
  gppTunnelingTimer.mode = TIMER_ONE_SHOT_MODE;
  gppTunnelingTimer.callback = tunnelingTimerCallback;
  HAL_StartAppTimer(&gppTunnelingTimer);
}

/**************************************************************************//**
\brief Stop the tunneling timer
******************************************************************************/
static void stopGppTunnelingTimer(void)
{
  gppTunnelingTimer.interval = 0;
  HAL_StopAppTimer(&gppTunnelingTimer);
}

/**************************************************************************//**
\brief tunneling Timer callback
******************************************************************************/
static void tunnelingTimerCallback(void)
{
  uint16_t minTimeOut = 0xFFFF;

  // Need to subtract gppTunnelingTimer.interval from the gpNotification entries
  for (uint8_t i = 0; i < TOTAL_NO_OF_TUNNELING_ENTRIES; i++)
  {
    if (LENGTH_OF_FREE_ENTRY != proxyTunnelingEntries[i].cmdPayloadLength)
    {
      if (INVALID_TUNNELING_TIME != proxyTunnelingEntries[i].lightweightUnicastModeTime)
      {
        proxyTunnelingEntries[i].lightweightUnicastModeTime -= gppTunnelingTimer.interval;
        if (!proxyTunnelingEntries[i].lightweightUnicastModeTime)
        {
          proxyTunnelingEntries[i].lightweightUnicastModeTime = INVALID_TUNNELING_TIME;
          if (proxyTunnelingEntries[i].commNotification)
            sendCommNotification(&proxyTunnelingEntries[i], false);
          else
            sendGpNotificationInUnicastOrGroupCastCommMode(&proxyTunnelingEntries[i], true);
        }
        else
          minTimeOut = MIN(minTimeOut, proxyTunnelingEntries[i].lightweightUnicastModeTime);
      }
      if (INVALID_TUNNELING_TIME != proxyTunnelingEntries[i].groupModeTime)
      {
        proxyTunnelingEntries[i].groupModeTime -= gppTunnelingTimer.interval;
        if (!proxyTunnelingEntries[i].groupModeTime)
        {
          proxyTunnelingEntries[i].groupModeTime = INVALID_TUNNELING_TIME;
          if (proxyTunnelingEntries[i].commNotification)
            sendCommNotification(&proxyTunnelingEntries[i], true);
          else
            sendGpNotificationInUnicastOrGroupCastCommMode(&proxyTunnelingEntries[i], false);
        }
        else
          minTimeOut = MIN(minTimeOut, proxyTunnelingEntries[i].groupModeTime);
      }

      if ((INVALID_TUNNELING_TIME == proxyTunnelingEntries[i].lightweightUnicastModeTime)&& \
          (INVALID_TUNNELING_TIME == proxyTunnelingEntries[i].groupModeTime))
      {
        // no pending notification for this entry so we can free this
        proxyTunnelingEntries[i].cmdPayloadLength = LENGTH_OF_FREE_ENTRY;
      }
    }
  }

  if (0xFFFF != minTimeOut)
  {
    startGppTunnelingTimer(minTimeOut);
  }
  else
    stopGppTunnelingTimer();
}

/**************************************************************************//**
  \brief send GP commissioning notification

  \param[in] tunnelingEntry - tunneling entry
             inBroadCast - status of inbroadcast bit

  \return true - success(unicast address found)
          false - no unicast address entry found
******************************************************************************/
static bool sendCommNotification(zgpClientTunnelingEntry_t *tunnelingEntry, \
                                 bool inBroadCast)
{
  zgpGpCommNotifyOptions_t options;
  uint8_t index = 0;
  ZGP_Addressing_t dstAddressing;
  memset(&dstAddressing, 0x00, sizeof(ZGP_Addressing_t));
  uint8_t payloadLength;
  ZGP_GpdId_t gpdId;

  if ((OPERATIONAL_MODE == ZGPL_GetDeviceMode(true)))
    return true;

  dstAddressing.addrMode = APS_SHORT_ADDRESS;
  dstAddressing.addr.shortAddress = BROADCAST_ADDR_RX_ON_WHEN_IDLE;
  if (!inBroadCast)
    dstAddressing.addr.shortAddress = zgpProxyBasicGetSinkNwkAddr();
  dstAddressing.radius = 0;

  dstAddressing.cmdDir = ZCL_CLUSTER_SIDE_SERVER;
  dstAddressing.cmdId =  ZCL_GP_CLUSTER_SERVER_GP_COMMISSIONING_NOTIFICATION_COMMAND_ID;

  memcpy(&options, &tunnelingEntry->notificationBuffer.buffer[index], sizeof(zgpGpCommNotifyOptions_t));
  index += sizeof(zgpGpCommNotifyOptions_t);

  memcpy(&gpdId, &tunnelingEntry->notificationBuffer.buffer[index], sizeof(gpdId));

  if (ZGP_SRC_APPID == options.appId)
    index += ZGP_SRC_ID_LENGTH; // src Id - 4 bytes
  else
    index += (8/* IEEE addr */ + 1 /* end point*/);

  index += ZGP_SEC_FRAME_CTR_LENGTH; // For security frame counter - 4 bytes
  if (inBroadCast)
  {
    if (ZGP_CHANNEL_REQUEST_CMD_ID == tunnelingEntry->notificationBuffer.buffer[index])
      dstAddressing.aliasSrcAddr = 0x0007;
    else
      dstAddressing.aliasSrcAddr = ZGPL_GetAliasSourceAddr(&gpdId);
    dstAddressing.aliasSeqNumber = (tunnelingEntry->macSeqNo - 12) % 256; // As per spec A.3.6.3.3.2
  }

  payloadLength = tunnelingEntry->cmdPayloadLength;

  ZGP_sendGpNotification(&dstAddressing,&tunnelingEntry->notificationBuffer.buffer[0],payloadLength);

  return true;
}

/**************************************************************************//**
  \brief allocate and send notification
 
   \param[in] tunnelingEntry - tunneling entry
             dstAddressing - addressing info
 
  \return true - success
          false - otherwise
******************************************************************************/
static bool allocateAndSendNotification(zgpClientTunnelingEntry_t *tunnelingEntry, ZGP_Addressing_t *dstAddressing)
{
  uint8_t payloadLength;

  dstAddressing->cmdDir = ZCL_CLUSTER_SIDE_SERVER;
  dstAddressing->cmdId =  ZCL_GP_CLUSTER_SERVER_GP_NOTIFICATION_COMMAND_ID;
  
  payloadLength = tunnelingEntry->cmdPayloadLength;

  if(!ZGP_sendGpNotification(dstAddressing,&tunnelingEntry->notificationBuffer.buffer[0],payloadLength))
    return false;
  
  return true;
}

/**************************************************************************//**
  \brief send GP notification in unicast mode or groupcast mode


  \param[in] tunnelingEntry - tunneling entry
             inUnicastMode - true for unicast mode
                             false for derived/pre-commissioned groupcast mode
 
  \return true - success
          false - otherwise
******************************************************************************/
static bool sendGpNotificationInUnicastOrGroupCastCommMode(zgpClientTunnelingEntry_t *tunnelingEntry, bool inUnicastMode)
{
  ZGP_Addressing_t  destAddressing;
  ZGP_ProxyTableEntry_t *proxyTableEntry = (ZGP_ProxyTableEntry_t *)zgpGetMemReqBuffer();
  uint16_t derivedAlias = ZGPL_GetAliasSourceAddr(&tunnelingEntry->entryInfo.gpdId);

  if (ENTRY_NOT_AVAILABLE == ZGPL_ReadTableEntryFromNvm((void *)proxyTableEntry, tunnelingEntry->entryInfo.filterField, \
         &tunnelingEntry->entryInfo.gpdId, tunnelingEntry->entryInfo.endPoint))
  {
    tunnelingEntry->cmdPayloadLength = LENGTH_OF_FREE_ENTRY;
    zgpFreeMemReqBuffer();
    return false;
  }

  memset(&destAddressing, 0x00, sizeof(destAddressing));
  if (inUnicastMode || (!inUnicastMode && proxyTableEntry->options.commissionedGroupGps))
  {
    uint8_t listIndex = 0;

    destAddressing.aliasSeqNumber = getAliasSeqNo(tunnelingEntry->macSeqNo, PRECOMMISSIONED_GROUPCAST);
    destAddressing.addrMode = APS_SHORT_ADDRESS;
    if (!inUnicastMode)
      destAddressing.addrMode = APS_GROUP_ADDRESS;
    do
    {
      listIndex = getNextAvailableShortAddressOrGroupAddressFromList(listIndex, proxyTableEntry, &destAddressing);

      if (ZGP_ENTRY_INVALID_INDEX != listIndex)
      {
        if (ZGP_NWK_ADDRESS_GROUP_INIT == destAddressing.aliasSrcAddr)
          destAddressing.aliasSrcAddr = derivedAlias;
        if (!allocateAndSendNotification(tunnelingEntry, &destAddressing))
        {
          zgpFreeMemReqBuffer();
          return false;
        }

        listIndex++;
      }
    }while (ZGP_ENTRY_INVALID_INDEX != listIndex);
  }
  if (!inUnicastMode && proxyTableEntry->options.derivedGroupGps)
  {
    destAddressing.addrMode = APS_GROUP_ADDRESS;
    destAddressing.radius = 0;
    if (proxyTableEntry->tableGenericInfo.groupCastRadius)
      destAddressing.radius = proxyTableEntry->tableGenericInfo.groupCastRadius;

      destAddressing.aliasSeqNumber = getAliasSeqNo(tunnelingEntry->macSeqNo, DERIVED_GROUPCAST);
      destAddressing.addr.groupAddress = derivedAlias;
      destAddressing.aliasSrcAddr = derivedAlias;
      if (proxyTableEntry->options.assignedAlias)
        destAddressing.aliasSrcAddr = proxyTableEntry->tableGenericInfo.gpdAssignedAlias;

      if (!allocateAndSendNotification(tunnelingEntry, &destAddressing))
      {
        zgpFreeMemReqBuffer();
        return false;
      }
  }
  zgpFreeMemReqBuffer();
  return true;
}

/**************************************************************************//**
  \brief get next available short address/group address in the list

  \param[in] index - current index
             entry - pointer to proxy entry
             addr - pointer to address to be assigned

  \return next available index
******************************************************************************/
static uint8_t getNextAvailableShortAddressOrGroupAddressFromList(uint8_t index, ZGP_ProxyTableEntry_t *entry, ZGP_Addressing_t *addressing)
{
  uint8_t listSize = ZGP_SINK_GROUP_LIST_SIZE;

  if (APS_SHORT_ADDRESS == addressing->addrMode)
    listSize = ZGP_LIGHT_WEIGHT_SINK_ADDRESS_LIST_SIZE;

  while(index < listSize)
  {
    if (APS_SHORT_ADDRESS == addressing->addrMode)
    {
      if (entry->zgpProxyLightWeightSinkAddrlist[index].sinkNwkAddr != ZGP_NWK_ADDRESS_GROUP_INIT)
      {
        addressing->addr.shortAddress = entry->zgpProxyLightWeightSinkAddrlist[index].sinkNwkAddr;
        return index;
      }
    }
    else
    {
      if (entry->tableGenericInfo.zgpSinkGrouplist[index].sinkGroup != ZGP_NWK_ADDRESS_GROUP_INIT)
      {
        addressing->addr.groupAddress = entry->tableGenericInfo.zgpSinkGrouplist[index].sinkGroup;
        addressing->aliasSrcAddr = entry->tableGenericInfo.zgpSinkGrouplist[index].alias;
        return index;
      }
    }
    index++;
  }

  return ZGP_ENTRY_INVALID_INDEX;
}

/**************************************************************************//**
  \brief sending notification in commissioning/operational mode

  \param[in] isCommNotification - true for commissioning mode
                                  false for operational mode
             additionalInfo - CommissioningMode info for commissioning mode
                              entry info for operational mode
             dstubInd - dstub indication data

  \return zcl status
******************************************************************************/
ZCL_Status_t zgpClientSendNotification(bool isCommNotification, void *additionalInfo, \
                                             ZGP_LowDataInd_t *dstubInd)
{
  uint8_t payLoadIndex = 0;
  gpClientNotificationOptions_t notificationOptions;
  zgpClientTunnelingEntry_t *tunnelingEntry;
  bool isMicReq = false;

  memset((void *)&notificationOptions, 0x00, sizeof(notificationOptions));

  if (isCommNotification && dstubInd->gpdfSecurityLevel && ((ZGP_DSTUB_AUTH_FAILURE == dstubInd->status) || \
                                          (ZGP_DSTUB_UNPROCESSED == dstubInd->status)))
  {
      notificationOptions.gpCommNotificationOptions.securityProcFailed = true;
      isMicReq = true;
  }
    
  if (NULL == (tunnelingEntry = allocAndInitTunnelingEntries(isMicReq, dstubInd)))
    return ZCL_FAILURE_STATUS;

  tunnelingEntry->commNotification = isCommNotification;

  if (isCommNotification)
  {
    zgpProxyBasicModeInfo_t *proxyModeInfo = (zgpProxyBasicModeInfo_t *)additionalInfo;
    notificationOptions.gpCommNotificationOptions.appId = dstubInd->applicationId;
    notificationOptions.gpCommNotificationOptions.bidirectionalCapability = 0;
    notificationOptions.gpCommNotificationOptions.rxAfterTx = dstubInd->rxAfterTx;
    notificationOptions.gpCommNotificationOptions.proxyInfoPresent = 1;
    notificationOptions.gpCommNotificationOptions.securityKeyType = dstubInd->gpdfKeyType;
    notificationOptions.gpCommNotificationOptions.securityLevel = dstubInd->gpdfSecurityLevel;
    notificationOptions.gpCommNotificationOptions.reserved = 0;

    if (proxyModeInfo->inUnicastMode)
    {
      tunnelingEntry->groupModeTime = INVALID_TUNNELING_TIME;
    }
    else
    {
      tunnelingEntry->lightweightUnicastModeTime = INVALID_TUNNELING_TIME;
    }
  }
  else
  {
    ZGP_ProxyTableEntry_t *entry = (ZGP_ProxyTableEntry_t *)additionalInfo;
    notificationOptions.gpNotificationOptions.appId = dstubInd->applicationId;
    notificationOptions.gpNotificationOptions.bidirectionalCapability = 0;
    notificationOptions.gpNotificationOptions.gpTxQueueFull = 0x01;
    notificationOptions.gpNotificationOptions.rxAfterTx = dstubInd->rxAfterTx;
    notificationOptions.gpNotificationOptions.proxyInfoPresent = 1;
    notificationOptions.gpNotificationOptions.securityKeyType = dstubInd->gpdfKeyType;
    notificationOptions.gpNotificationOptions.securityLevel = dstubInd->gpdfSecurityLevel;
    // By default enabling all comm. mode later will be overwritten
    notificationOptions.gpNotificationOptions.alsoUnicast = 1;
    tunnelingEntry->entryInfo.filterField.appId = dstubInd->applicationId;
    tunnelingEntry->entryInfo.filterField.commMode = ALL_COMMUNICATION_MODE;
    tunnelingEntry->entryInfo.filterField.entryType = ZGP_PROXY_ENTRY;
    tunnelingEntry->entryInfo.filterField.nonEmptyIndexForRead= ZGP_ENTRY_INVALID_INDEX;
    memcpy(&tunnelingEntry->entryInfo.gpdId, &entry->tableGenericInfo.gpdId, sizeof(tunnelingEntry->entryInfo.gpdId));
    tunnelingEntry->entryInfo.endPoint = entry->tableGenericInfo.endPoint;

    if (!entry->options.lightWeightUnicastGps)
    {
      notificationOptions.gpNotificationOptions.alsoUnicast = 0;
      tunnelingEntry->lightweightUnicastModeTime = INVALID_TUNNELING_TIME;
    }

    notificationOptions.gpNotificationOptions.alsoCommissionedGroup = entry->options.commissionedGroupGps;
    notificationOptions.gpNotificationOptions.alsoDerivedGroup = entry->options.derivedGroupGps;
    if (!notificationOptions.gpNotificationOptions.alsoCommissionedGroup && !notificationOptions.gpNotificationOptions.alsoDerivedGroup)
    {
      // both the group comm. mode is not set so making group tunneling time invalid
      tunnelingEntry->groupModeTime = INVALID_TUNNELING_TIME;
    }
  }
  memcpy((void *)&tunnelingEntry->notificationBuffer.buffer[payLoadIndex], (void *)&notificationOptions , sizeof(notificationOptions));
  payLoadIndex += sizeof(notificationOptions);

  payLoadIndex += gpBuildPayloadAndAddressingHeader(dstubInd, &tunnelingEntry->notificationBuffer.buffer[payLoadIndex], \
                                                    isMicReq);

  tunnelingEntry->cmdPayloadLength = payLoadIndex;
  addToTunnelingTimer(tunnelingEntry);

  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
  \brief Framing payload and addressing header

  \param[in] dstubInd - dstub data indication
             gpNotificationPayload - payload
             includeMic - status of mic
  \return zcl status
******************************************************************************/
static uint8_t gpBuildPayloadAndAddressingHeader(ZGP_LowDataInd_t *dstubInd, uint8_t *gpNotificationPayload, \
                                                 bool includeMic)
{
  uint8_t payLoadIndex = 0;
  uint32_t securityFrameCounter;

  if (ZGP_SRC_APPID == dstubInd->applicationId)
  {
    uint32_t srcId = dstubInd->srcId;

    memcpy((void *)&gpNotificationPayload[payLoadIndex], (void *)&srcId , sizeof(dstubInd->srcId));
    payLoadIndex += sizeof(dstubInd->srcId);
  }
  else
  {
    uint64_t extAddr = dstubInd->srcAddress.ext;

    memcpy((void *)&gpNotificationPayload[payLoadIndex], (void *)&extAddr , sizeof(dstubInd->srcAddress.ext));
    payLoadIndex += sizeof(dstubInd->srcAddress.ext);
    gpNotificationPayload[payLoadIndex++] = dstubInd->endPoint;
  }

  securityFrameCounter = dstubInd->gpdSecurityFrameCounter;
  memcpy((void *)&gpNotificationPayload[payLoadIndex], (void *)&securityFrameCounter, sizeof(dstubInd->gpdSecurityFrameCounter));
  payLoadIndex += sizeof(dstubInd->gpdSecurityFrameCounter);

  gpNotificationPayload[payLoadIndex++] = dstubInd->gpdCommandId;
  gpNotificationPayload[payLoadIndex++] = dstubInd->gpdAsduLength;
  memcpy((void *)&gpNotificationPayload[payLoadIndex], dstubInd->gpdAsdu , dstubInd->gpdAsduLength);
  payLoadIndex += dstubInd->gpdAsduLength;

  // As per the spec., all proxy should be having proxy info present in the notification
  // Refer the spec. A.3.3.4.3 & A.3.3.4.1
  {
    ShortAddr_t nwkAddr = NWK_GetShortAddr();
    zgpGppGpdLink_t gpdLink;

    memcpy((void *)&gpNotificationPayload[payLoadIndex], (void *)&nwkAddr, sizeof(nwkAddr));
    payLoadIndex += sizeof(nwkAddr);

    if (dstubInd->rssi < RSSI_LOW_THRESHOLD_VALUE)
      dstubInd->rssi = RSSI_LOW_THRESHOLD_VALUE;
    else if (dstubInd->rssi > RSSI_HIGH_THRESHOLD_VALUE)
      dstubInd->rssi = RSSI_HIGH_THRESHOLD_VALUE;

    gpdLink.rssi = (dstubInd->rssi +  RSSI_OFFSET_VALUE) / 2;
    gpdLink.linkQuality = (dstubInd->linkQuality / LQI_STEP_SIZE);
    memcpy(&gpNotificationPayload[payLoadIndex], (void *)&gpdLink, sizeof(zgpGppGpdLink_t));
    payLoadIndex++;
  }

  if (includeMic)
  {
    memcpy(&gpNotificationPayload[payLoadIndex], &dstubInd->mic, sizeof(dstubInd->mic));
    payLoadIndex += sizeof(dstubInd->mic);
  }

  return payLoadIndex;
}

/**************************************************************************//**
  \brief Allocate and initialize tunnelingEntry

  \param[in  isSecProcFailedSet - status of sec. processing failed bit
             dstubInd - dstub data indication

  \return pointer to payloadllocated entry's address
******************************************************************************/
static zgpClientTunnelingEntry_t *allocAndInitTunnelingEntries(bool isSecProcFailedSet, \
                                                               ZGP_LowDataInd_t *dstubInd)
{
  uint8_t notificationOverhead = 11; // 2 - options, 4 - security frame counter, 1 - commandId, 1 - cmdLength 3 - proxy info
  zgpClientTunnelingEntry_t * tunnelingEntry;

  if (ZGP_SRC_APPID == dstubInd->applicationId)
  {
    notificationOverhead += ZGP_SRC_ID_LENGTH; // src ID
  }
  else
  {
    notificationOverhead += 9; // IEEE addr and end point
  }

  if (isSecProcFailedSet)
  {
    notificationOverhead += ZGP_SEC_FOURBYTE_MIC; // security frame counter
  }

  tunnelingEntry = getFreeTunnelingEntry(notificationOverhead + dstubInd->gpdAsduLength);

  if (tunnelingEntry)
  {
    tunnelingEntry->macSeqNo = dstubInd->seqNumber;
    tunnelingEntry->lightweightUnicastModeTime = DMIN_U;
    tunnelingEntry->groupModeTime = DMIN_U;

    // initializing unicast and groupcast tunneling time by default
    // Later this can be overridden by the respective notification handler function
    if (dstubInd->rxAfterTx)
    {
      tunnelingEntry->groupModeTime += zgpClientGetTimeDelayFromLqi(dstubInd->linkQuality);
      tunnelingEntry->lightweightUnicastModeTime = DMIN_B;
    }
 
    if (tunnelingEntry->lightweightUnicastModeTime % HAL_APPTIMERINTERVAL)
      tunnelingEntry->lightweightUnicastModeTime = tunnelingEntry->lightweightUnicastModeTime + (HAL_APPTIMERINTERVAL - tunnelingEntry->lightweightUnicastModeTime % HAL_APPTIMERINTERVAL);

    if (tunnelingEntry->groupModeTime % HAL_APPTIMERINTERVAL)
      tunnelingEntry->groupModeTime = tunnelingEntry->groupModeTime + (HAL_APPTIMERINTERVAL - tunnelingEntry->groupModeTime % HAL_APPTIMERINTERVAL);
  }
  return tunnelingEntry;
}

/**************************************************************************//**
  \brief  get alias sequence no

  \param[in] macSeqNo - rxd mac seq.no
  \param[in] commMode - communication mode

  \return alias sequence no
 ******************************************************************************/
static uint8_t getAliasSeqNo(uint8_t macSeqNo, zgpCommunicationMode_t commMode)
{
  if (DERIVED_GROUPCAST == commMode)
    return macSeqNo;
  else if(PRECOMMISSIONED_GROUPCAST == commMode)
    return ((macSeqNo - 9) % 256);
  else
    return ((macSeqNo - 14) % 256); // For broadcast GP notification
}
/**************************************************************************//**
  \brief Build the proxyTableEntry in OTA format

  \param[in] proxyTableOTA - payload to be populated
             proxyTableEntry - proxy table entry

  \return payload length
******************************************************************************/
uint16_t zgppBuildOTAProxyTableEntry( uint8_t *proxyTableOTA, ZGP_ProxyTableEntry_t *proxyTableEntry)
{
  // for OTA format of proxy table entry refer A.3.4.2.2.1 Over the air transmission of Proxy Table 
  uint16_t otaEntryIndex = 0;
  zgpProxyTableEntryOptions_t options;
  //copy options field
  options = proxyTableEntry->options;
  memcpy(&proxyTableOTA[otaEntryIndex], &options, sizeof(zgpProxyTableEntryOptions_t));
  otaEntryIndex += sizeof(zgpProxyTableEntryOptions_t);

  // copy GPD address
  otaEntryIndex += zgpAddGpdSourceAddrToOtaPayload((ZGP_ApplicationId_t)options.appId, &proxyTableEntry->tableGenericInfo,\
                         &proxyTableOTA[otaEntryIndex]);

  // Assigned Alias
  if(options.assignedAlias)
  {
    uint16_t assignedAlias;
    assignedAlias = proxyTableEntry->tableGenericInfo.gpdAssignedAlias;
    memcpy(&proxyTableOTA[otaEntryIndex], &assignedAlias, sizeof(assignedAlias));
    otaEntryIndex += sizeof(assignedAlias);
  }

  otaEntryIndex += zgpGenericAddSecurityFields(&proxyTableOTA[otaEntryIndex], &proxyTableEntry->tableGenericInfo, \
                                             options.securityUse, options.macSeqNumCapability);

  // Lightweight sink address list
  if(options.lightWeightUnicastGps)
  {
    uint8_t *noOfNonEmptyLWSinkAddr;
    noOfNonEmptyLWSinkAddr = &proxyTableOTA[otaEntryIndex];
    *noOfNonEmptyLWSinkAddr = 0;
    otaEntryIndex += sizeof(uint8_t);

    for(uint8_t i = 0; i < ZGP_LIGHT_WEIGHT_SINK_ADDRESS_LIST_SIZE; i++)
    {
      if(proxyTableEntry->zgpProxyLightWeightSinkAddrlist[i].sinkNwkAddr != ZGP_NWK_ADDRESS_GROUP_INIT)
      {
        ExtAddr_t sinkIeeeAddr;
        uint16_t sinkNwkAddr;
        sinkIeeeAddr = proxyTableEntry->zgpProxyLightWeightSinkAddrlist[i].sinkIeeeAddr;
        memcpy(&proxyTableOTA[otaEntryIndex], &sinkIeeeAddr, sizeof(sinkIeeeAddr));
        otaEntryIndex += sizeof(sinkIeeeAddr);

        sinkNwkAddr = proxyTableEntry->zgpProxyLightWeightSinkAddrlist[i].sinkNwkAddr;
        memcpy(&proxyTableOTA[otaEntryIndex], &sinkNwkAddr, sizeof(sinkNwkAddr));
        otaEntryIndex += sizeof(sinkNwkAddr);
        (*noOfNonEmptyLWSinkAddr)++;
      }
    }
  }

  //  sink group address list
  if(options.commissionedGroupGps)
  {
    otaEntryIndex += zgpAddPreCommissionedGroupList(&proxyTableEntry->tableGenericInfo, &proxyTableOTA[otaEntryIndex]);
  }
#if ZGP_PROXY_ADVANCED == 1
  //Search counter
  if(!options.entryActive || !options.entryValid)
  {
    proxyTableOTA[otaEntryIndex++] = proxyTableEntry->searchCounter;
  }
#endif
  // Adding groupcast radius
  proxyTableOTA[otaEntryIndex] = proxyTableEntry->tableGenericInfo.groupCastRadius;
  otaEntryIndex++;

  return otaEntryIndex;
}

/**************************************************************************//**
\brief Get the tunneling entry
 \param[in] bufferLength - Length of the buffer
  \return NULL
******************************************************************************/
static zgpClientTunnelingEntry_t * getFreeTunnelingEntry(uint8_t bufferLength)
{
  // This needs to be updated when we have multiple entries
  for (uint8_t i = 0; i < TOTAL_NO_OF_TUNNELING_ENTRIES; i++)
  {
    if (LENGTH_OF_FREE_ENTRY == proxyTunnelingEntries[i].cmdPayloadLength)
    {
      if (proxyTunnelingEntries[i].notificationBuffer.bufferLength >= bufferLength)
        return &proxyTunnelingEntries[i];
    }
  }
  return NULL;
}

/**************************************************************************//**
  \brief Stop the on-going tunneling if proxy entry is removed in-between
  \param[in] appId -application ID
             gpdId -group ID
             endPoint -endpoint
  \return NONE
******************************************************************************/
void zgpClientStopTunneling(ZGP_ApplicationId_t appId, ZGP_GpdId_t *gpdId, uint8_t endPoint)
{
  for (uint8_t i = 0; i < TOTAL_NO_OF_TUNNELING_ENTRIES; i++)
  {
    if (LENGTH_OF_FREE_ENTRY != proxyTunnelingEntries[i].cmdPayloadLength)
    {
      if (gpdId)
      {
        if ( (appId == proxyTunnelingEntries[i].entryInfo.filterField.appId) && \
             (((ZGP_SRC_APPID == appId) && (gpdId->gpdSrcId == proxyTunnelingEntries[i].entryInfo.gpdId.gpdSrcId)) || \
             ((ZGP_IEEE_ADDR_APPID == appId) && (gpdId->gpdIeeeAddr == proxyTunnelingEntries[i].entryInfo.gpdId.gpdIeeeAddr) && \
             (endPoint == proxyTunnelingEntries[i].entryInfo.endPoint) )))
        {
          proxyTunnelingEntries[i].cmdPayloadLength = LENGTH_OF_FREE_ENTRY;
        }
      }
      else
        proxyTunnelingEntries[i].cmdPayloadLength = LENGTH_OF_FREE_ENTRY;
    }
  }

  if (!gpdId)
    stopGppTunnelingTimer();
}
/**************************************************************************//**
\brief To get delay for the link quality

\param[in] linkQuality - link quality
  \return timedelay
******************************************************************************/
uint8_t zgpClientGetTimeDelayFromLqi(uint8_t linkQuality)
{
  if (linkQuality > LINK_QUALITY_MAX_VAL)
    linkQuality = LINK_QUALITY_MAX_VAL;
  return (LINK_QUALITY_MAX_VAL - linkQuality) *TUNNELING_DELAY_STEP_SIZE;
}
/**************************************************************************//**
\brief Attribute Event indication handler(to indicate when attr values have
        read or written)

\param[in] addressing - pointer to addressing information;
\param[in] attributeId - attribute id
\param[in] event - Event triggering this callback
******************************************************************************/
static void zgpProxyAttrEventInd(ZCL_Addressing_t *addressing, ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event)
{
  if(event == ZCL_WRITE_ATTRIBUTE_EVENT)
  {
    if(attributeId == ZCL_GP_CLUSTER_CLIENT_GP_SHARED_SECURITY_KEY_TYPE_ATTRIBUTE_ID)
    {
      PDS_Store(ZGP_PROXY_SHARED_KEY_TYPE_MEM_ID);
    }
    else if(attributeId == ZCL_GP_CLUSTER_CLIENT_GP_SHARED_SECURITY_KEY_ATTRIBUTE_ID)
    {
      PDS_Store(ZGP_PROXY_SHARED_KEY_MEM_ID);
    }
    else if(attributeId == ZCL_GP_CLUSTER_CLIENT_GP_LINK_KEY_ATTRIBUTE_ID)
    {
      PDS_Store(ZGP_PROXY_LINK_KEY_MEM_ID);
    }
  }
}

#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
/**************************************************************************//**
\brief To populate one report descriptor payload

\param[in] commReqAppInfo - info of the Current session entry which completed successful commissioning
\param[in] tempPayload - currReportPayload
******************************************************************************/
static uint8_t zgpBuildReportDescPayload(ZGP_GpdAppInfo_t *commReqAppInfo, uint8_t *tempPayload)
{
  uint8_t noOfDPdescriptor = commReqAppInfo->reportDescriptor->noOfDatapointDescriptor;
  uint8_t pairingConfigIndex = 0;
  uint8_t remLengthOfOctetsIndex = 0;
  uint8_t remLengthOfOctets = 0;
  uint8_t timeOutOptionsPresentAndSize = 0;
  memcpy(&tempPayload[pairingConfigIndex++], (void*)&commReqAppInfo->reportDescriptor->reportId, sizeof(uint8_t));
  memcpy(&tempPayload[pairingConfigIndex++], (void*)&commReqAppInfo->reportDescriptor->reportOptions, sizeof(uint8_t));
         
  if (IS_ZGP_REPORT_DESCRIPTOR_TIMEOUT_PRESENT(commReqAppInfo->reportDescriptor->reportOptions))
  {
    if(MAX_SINGLE_REPORT_DESC_LEN < (pairingConfigIndex + sizeof(commReqAppInfo->reportDescriptor->timeoutPeriod)))
       return INVALID_LENGTH;
    
     uint16_t timeoutPeriod = commReqAppInfo->reportDescriptor->timeoutPeriod;
     memcpy(&tempPayload[pairingConfigIndex], (void*)&timeoutPeriod, sizeof(uint16_t));
     pairingConfigIndex += sizeof(uint16_t);
     timeOutOptionsPresentAndSize += sizeof(uint16_t);
  } 
  remLengthOfOctetsIndex = pairingConfigIndex;
  if(MAX_SINGLE_REPORT_DESC_LEN < (pairingConfigIndex + sizeof(remLengthOfOctetsIndex)))
    return INVALID_LENGTH;

  memcpy(&tempPayload[pairingConfigIndex++], (void*)&remLengthOfOctets, sizeof(uint8_t));
  
  for(uint8_t i = 0; i < noOfDPdescriptor; i++)
  {
    uint8_t attrRecordCount = ZGP_DATAPOINT_DESC_NO_OF_ATTRIBUTE_RECORDS(commReqAppInfo->reportDescriptor->dataPointDescriptor->dataPointOptions) + 1;
    uint8_t dataPointOptions = commReqAppInfo->reportDescriptor->dataPointDescriptor->dataPointOptions;
    uint16_t clusterID = commReqAppInfo->reportDescriptor->dataPointDescriptor->clusterId;
    if(MAX_SINGLE_REPORT_DESC_LEN < (pairingConfigIndex + sizeof(clusterID)+ sizeof(dataPointOptions)))
      return INVALID_LENGTH;
    memcpy(&tempPayload[pairingConfigIndex++], (void*)&dataPointOptions, sizeof(uint8_t));
    memcpy(&tempPayload[pairingConfigIndex], (void*)&clusterID, sizeof(uint16_t));
    pairingConfigIndex += sizeof(uint16_t);
    if (IS_ZGP_DATAPOINT_DESC_MANUFAC_ID_PRESENT(dataPointOptions))
    {
      if(MAX_SINGLE_REPORT_DESC_LEN < (pairingConfigIndex + sizeof(uint16_t)))
        return INVALID_LENGTH;
      uint16_t manfacId = commReqAppInfo->reportDescriptor->dataPointDescriptor->manufacturerId;
      memcpy(&tempPayload[pairingConfigIndex], (void*)&manfacId, sizeof(uint16_t));
      pairingConfigIndex += sizeof(uint16_t);
    }
    for(uint8_t k = 0; k < attrRecordCount; k++)
    {
      if(MAX_SINGLE_REPORT_DESC_LEN < (pairingConfigIndex + sizeof(ZGP_AttributeRecord_t)))
        return INVALID_LENGTH;
      uint16_t attrId = commReqAppInfo->reportDescriptor->dataPointDescriptor->attrRecord[k].attrId;
      uint8_t attrDataType = commReqAppInfo->reportDescriptor->dataPointDescriptor->attrRecord[k].attrDataType;
      uint8_t attrOptions = commReqAppInfo->reportDescriptor->dataPointDescriptor->attrRecord[k].attrOptions;
      uint8_t attrOffsetWithinReport = commReqAppInfo->reportDescriptor->dataPointDescriptor->attrRecord[k].attrOffsetWithinReport;
      memcpy(&tempPayload[pairingConfigIndex], &attrId, sizeof(uint16_t));
      pairingConfigIndex += sizeof(uint16_t);
      memcpy(&tempPayload[pairingConfigIndex++], (void*)&attrDataType, sizeof(uint8_t));
      memcpy(&tempPayload[pairingConfigIndex++], (void*)&attrOptions, sizeof(uint8_t));
      if (IS_ZGP_ATTR_REPORTED(commReqAppInfo->reportDescriptor->dataPointDescriptor->attrRecord[k].attrOptions))
      {
        memcpy(&tempPayload[pairingConfigIndex++], (void*)&attrOffsetWithinReport, sizeof(uint8_t));
      }
      if (IS_ZGP_ATTR_VALUE_PRESENT(commReqAppInfo->reportDescriptor->dataPointDescriptor->attrRecord[k].attrOptions))
      {
        uint8_t attrValue[ZGP_ATTRIBUTE_MAX_SIZE] = {0};
        for(uint8_t arrIndex = 0; arrIndex < ZGP_ATTRIBUTE_MAX_SIZE; arrIndex++)
            attrValue[arrIndex] = commReqAppInfo->reportDescriptor->dataPointDescriptor->attrRecord[k].attrValue[arrIndex];
        uint8_t attrLength = ZCL_GetAttributeLength(commReqAppInfo->reportDescriptor->dataPointDescriptor->attrRecord[k].attrDataType, \
                                    (void*)&attrValue);
         if (attrLength > ZGP_ATTRIBUTE_MAX_SIZE)
           attrLength = ZGP_ATTRIBUTE_MAX_SIZE;
         memcpy(&tempPayload[pairingConfigIndex], (void*)&attrValue, attrLength);
         pairingConfigIndex += attrLength;
      }
    } 
      if(commReqAppInfo->reportDescriptor->dataPointDescriptor->nextDataPointDescriptor != NULL)
        commReqAppInfo->reportDescriptor->dataPointDescriptor = commReqAppInfo->reportDescriptor->dataPointDescriptor->nextDataPointDescriptor;
      else
        break;
   }
     remLengthOfOctets = pairingConfigIndex - sizeof(uint8_t) - sizeof(uint8_t) - sizeof(uint8_t) - timeOutOptionsPresentAndSize;/*(totalreportlength - reportID -report options - remLengthOfOctets - timeOutPeriod*/
     memcpy(&tempPayload[remLengthOfOctetsIndex], (void*)&remLengthOfOctets, sizeof(uint8_t));
     return pairingConfigIndex;
}

/**************************************************************************//**
\brief To populate Application descriptor payload

\param[in] commReqAppInfo - info of the Current session entry which completed successful commissioning
\param[in] gpPairingTempPayload - Application desc payload to be sent out in GPPairingConfig
\param[in] payloadLength - PairingConfig info payload length
\param[in] maxPairingConfigPayloadSize - available zcl payload length to send pairing config

Note: The below API is designed to accomodate a single report of max paylad size supported by ZCL
If a single report exceeds a ZCL payload size the report will be dropped
******************************************************************************/
static ZGP_InfraDeviceStatus_t zgpBuildApplDescInPairingConfigPayload(ZGP_GpdAppInfo_t *commReqAppInfo, uint8_t *gpPairingTempPayload, uint8_t *payloadLength, uint8_t maxPairingConfigPayloadSize)
{
  uint8_t appDescPayloadIndex = 0;
  uint8_t tempPayload[MAX_SINGLE_REPORT_DESC_LEN] = {0};
  uint8_t noOfReportsInCurrAppDescr = 0;
  uint8_t noOfRepIndex = 0;
  uint8_t appInfoValue = 0x00;//As per test spec 4.3.4.3 expected outcome appInfo present and it should be 0x00
  memcpy(&gpPairingTempPayload[appDescPayloadIndex], (void*)&appInfoValue, sizeof(commReqAppInfo->appInfoOptions));
  appDescPayloadIndex += sizeof(commReqAppInfo->appInfoOptions);
  memcpy(&gpPairingTempPayload[appDescPayloadIndex++], (void*)&commReqAppInfo->totalNoofReports, sizeof(uint8_t));
  noOfRepIndex = appDescPayloadIndex;
  memcpy(&gpPairingTempPayload[appDescPayloadIndex++], &noOfReportsInCurrAppDescr, sizeof(uint8_t));
  for(uint8_t i = 0; i < gpPairingConfigPendingInfo.pairingConfigAppInfo.noOfReportsPendingInGpPairingConfig; i++)
  {
    uint8_t currReportEntryLength = 0; 
    currReportEntryLength = zgpBuildReportDescPayload(commReqAppInfo, tempPayload);
    if(currReportEntryLength < (maxPairingConfigPayloadSize - appDescPayloadIndex))
    {
      noOfReportsInCurrAppDescr++;
      memcpy(&gpPairingTempPayload[noOfRepIndex], &noOfReportsInCurrAppDescr, sizeof(uint8_t));
      memcpy(&gpPairingTempPayload[appDescPayloadIndex], &tempPayload[0], currReportEntryLength);
      appDescPayloadIndex += currReportEntryLength;
      if(commReqAppInfo->reportDescriptor->nextReportDescriptor != NULL)
        commReqAppInfo->reportDescriptor = commReqAppInfo->reportDescriptor->nextReportDescriptor; 
      else if(commReqAppInfo->reportDescriptor->nextReportDescriptor == NULL)
        break;
    }
    else if(INVALID_LENGTH == currReportEntryLength)
       return ZGP_INSUFFICIENT_SPACE_STATUS;        //return insufficient status
    else // if(currReportEntryLength > (maxPairingConfigPayloadSize - appDescPayloadIndex))
    {
      memcpy((void*)&gpPairingConfigPendingInfo.pairingConfigAppInfo.reportDescriptor, (void*)&commReqAppInfo->reportDescriptor, sizeof(ZGP_ReportDescriptor_t));
      gpPairingConfigPendingInfo.maxPayloadSize = maxPairingConfigPayloadSize;
      gpPairingConfigPendingInfo.payloadLength = 0;
      break;
    }
  }
  gpPairingConfigPendingInfo.pairingConfigAppInfo.noOfReportsPendingInGpPairingConfig = (gpPairingConfigPendingInfo.pairingConfigAppInfo.noOfReportsPendingInGpPairingConfig) - noOfReportsInCurrAppDescr;
  *payloadLength = appDescPayloadIndex;
  return ZGP_SUCCESS;
}
#endif
/**************************************************************************//**
  \brief Build and send ZGP pairing config command
  \param[in] addrMode - address mode
  \param[in] dstAddr - destination address
  \param[in] pairingConfigCmdInfo - pairing config inputs
  \param[in] commReqAppInfo - Application information
  \param[in] endpointInfo - paired endpoints information

  \return zcl status
******************************************************************************/
ZGP_InfraDeviceStatus_t ZGPH_SendGpPairingConfigCmd (APS_AddrMode_t addrMode, APS_Address_t *dstAddr, ZGP_PairingConfigCmdInfo_t pairingConfigCmdInfo,
                                                       ZGP_GpdAppInfo_t *commReqAppInfo, ZGP_EndpointInfo_t endPointInfo)
{
  uint8_t gpPairingPayload[ZCL_MAX_TX_ZSDU_SIZE] = {0};
  ZgpGpPairingConfigOptions_t pairingConfigCmdOptions;
  ZgpGpPairingConfigActions_t pairingConfigCmdActions;
  uint16_t pairingConfigIndex = 0;
  ZGP_SinkTableEntry_t *sinkTableEntry = (ZGP_SinkTableEntry_t *)zgpGetMemReqBuffer();
  ZGP_TableOperationField_t  tableOperationField = {.entryType = ZGP_SINK_ENTRY, .commMode = pairingConfigCmdInfo.commMode, .appId = pairingConfigCmdInfo.appId, \
  .nonEmptyIndexForRead = ZGP_ENTRY_INVALID_INDEX};

  uint8_t retVal = ZGP_SUCCESS;
  ZCL_Addressing_t dstZclAddr;

  uint8_t zclMaxApsAsduSize = 0;
  zclMaxApsAsduSize = APS_MAX_NON_SECURITY_ASDU_SIZE;
#ifdef _NWK_CONCENTRATOR_
  bool nwkConcentrator = false;
  CS_ReadParameter(CS_NWK_CONCENTRATOR_CONFIG_ID, &nwkConcentrator);
  if (!nwkConcentrator)
  zclMaxApsAsduSize += NWK_MAX_SOURCE_ROUTE_SUBFRAME_LENGTH;
#endif
  // initializing the local structures to 0
  memset(&pairingConfigCmdOptions, 0 , sizeof(ZgpGpPairingConfigOptions_t));
  memset(&pairingConfigCmdActions, 0 , sizeof(ZgpGpPairingConfigActions_t));
  memset(sinkTableEntry, 0, sizeof(ZGP_SinkTableEntry_t));
  memset(&dstZclAddr, 0x00, sizeof(dstZclAddr));

  memcpy(&dstZclAddr.addr, dstAddr, sizeof(APS_Address_t));
  dstZclAddr.addrMode = addrMode;
  dstZclAddr.aliasSrcAddr = NWK_NO_SHORT_ADDR;

  if(pairingConfigCmdInfo.action.action != REMOVE_GPD)
  {

    if(ENTRY_NOT_AVAILABLE == ZGPL_ReadTableEntryFromNvm((void *)sinkTableEntry, tableOperationField, \
                                                     &pairingConfigCmdInfo.gpdId, pairingConfigCmdInfo.gpdEndPoint))
    {
      zgpFreeMemReqBuffer();
      return ZGP_ENTRY_NOT_AVAILABLE; 
    }
  }

  //Actions
  pairingConfigCmdActions.sendGpPairing = pairingConfigCmdInfo.action.sendGpPairing;
  pairingConfigCmdActions.action = pairingConfigCmdInfo.action.action;
  
  memcpy(&gpPairingPayload[pairingConfigIndex], &pairingConfigCmdActions, sizeof(pairingConfigCmdActions));
  pairingConfigIndex += sizeof(pairingConfigCmdActions);
  
  //Options
  pairingConfigCmdOptions.appId = pairingConfigCmdInfo.appId;
  pairingConfigCmdOptions.communicationMode = pairingConfigCmdInfo.commMode;
  pairingConfigCmdOptions.rxOnCapability = sinkTableEntry->options.rxOnCapability;
  pairingConfigCmdOptions.fixedLocation = sinkTableEntry->options.fixedLocation;
  if(pairingConfigCmdActions.action == EXTEND_SINKTABLE_ENTRY || pairingConfigCmdActions.action == REPLACE_SINKTABLE_ENTRY)
  {
    pairingConfigCmdOptions.seqNumCapabilities = sinkTableEntry->options.seqNumCapabilities;
    pairingConfigCmdOptions.assignedAlias = sinkTableEntry->options.assignedAlias;
    pairingConfigCmdOptions.securityUse = sinkTableEntry->options.securityUse;
  }
  else
  {
    pairingConfigCmdOptions.seqNumCapabilities = false;
    pairingConfigCmdOptions.assignedAlias = false;
    pairingConfigCmdOptions.securityUse = false;
  }
  if(NULL != commReqAppInfo)
    pairingConfigCmdOptions.applicationInfoPresent = (commReqAppInfo->appInfoOptions.manufacturerIdPresent || commReqAppInfo->appInfoOptions.modelIdPresent
                                                      || commReqAppInfo->appInfoOptions.gpdCommandPresent || commReqAppInfo->appInfoOptions.clusterListPresent 
                                                      ||commReqAppInfo->appInfoOptions.switchInformationPresent || commReqAppInfo->appInfoOptions.appDescriptionCommandFollows);
  else
    pairingConfigCmdOptions.applicationInfoPresent  = false;

  pairingConfigCmdOptions.reserved = 0;

  memcpy(&gpPairingPayload[pairingConfigIndex], &pairingConfigCmdOptions, sizeof(pairingConfigCmdOptions));
  pairingConfigIndex += sizeof(pairingConfigCmdOptions);

  // Address info
  if(ZGP_SRC_APPID == pairingConfigCmdInfo.appId)
  {
    uint32_t gpdSrcId;
    gpdSrcId = pairingConfigCmdInfo.gpdId.gpdSrcId;
        //if Src ID = 0x00000000 don't send pairingConfigCmd
    if((!ZGPL_IsValidSrcId(gpdSrcId, ZGP_FRAME_DATA, true))
#if MICROCHIP_APPLICATION_SUPPORT != 1
     || (gpdSrcId == ZGP_ALL_SRC_ID)
#endif
      )
    {
      return ZGP_INVALID_ACTION;
    }
    memcpy(&gpPairingPayload[pairingConfigIndex], &gpdSrcId, sizeof(uint32_t));
    pairingConfigIndex += sizeof(uint32_t);
  }
  else if(ZGP_IEEE_ADDR_APPID == pairingConfigCmdInfo.appId)
  {
    ExtAddr_t gpdIeeeAddr;
    gpdIeeeAddr = pairingConfigCmdInfo.gpdId.gpdIeeeAddr;
    // if  IeeeAddr = 0x0000000000000000 don't send pairingConfigCmd
    if((gpdIeeeAddr == 0x0000000000000000) 
#if MICROCHIP_APPLICATION_SUPPORT != 1
        || (gpdIeeeAddr == ZGP_ALL_IEEE_ADDR)
#endif
      )
    {
      return ZGP_INVALID_ACTION;
    }
    memcpy(&gpPairingPayload[pairingConfigIndex], &gpdIeeeAddr, sizeof(ExtAddr_t));
    pairingConfigIndex += sizeof(pairingConfigCmdInfo.gpdId.gpdIeeeAddr);
    // endpoint 
    gpPairingPayload[pairingConfigIndex++] = pairingConfigCmdInfo.gpdEndPoint;
  }

   if(REMOVE_GPD != pairingConfigCmdInfo.action.action)
  {
    gpPairingPayload[pairingConfigIndex++] = sinkTableEntry->deviceId;
  }
  else
  {
    gpPairingPayload[pairingConfigIndex++] = pairingConfigCmdInfo.deviceId;
  }

  if(REMOVE_GPD != pairingConfigCmdInfo.action.action && APPLICATION_DESCRIPTION != pairingConfigCmdInfo.action.action)
  {
    // grouplist
    if(PRECOMMISSIONED_GROUPCAST == pairingConfigCmdInfo.commMode)
    {
      zgpTableGenericInfo_t genericInfo;
      uint8_t groupListLength;
      memcpy(genericInfo.zgpSinkGrouplist, sinkTableEntry->tableGenericInfo.zgpSinkGrouplist, sizeof(ZGP_SinkGroup_t) * ZGP_SINK_GROUP_LIST_SIZE);
      groupListLength = zgpAddPreCommissionedGroupList(&genericInfo, gpPairingPayload + pairingConfigIndex);
      pairingConfigIndex += groupListLength;
    }
    if(pairingConfigCmdActions.action == EXTEND_SINKTABLE_ENTRY || pairingConfigCmdActions.action == REPLACE_SINKTABLE_ENTRY)
    {
      //Assigned alias
      if(sinkTableEntry->options.assignedAlias)
      {
        uint16_t gpdAssignedAlias;
        gpdAssignedAlias = sinkTableEntry->tableGenericInfo.gpdAssignedAlias;
        memcpy(&gpPairingPayload[pairingConfigIndex], &gpdAssignedAlias, sizeof(gpdAssignedAlias));
        pairingConfigIndex += sizeof(gpdAssignedAlias);
      }
    }
  }
  else if (PRECOMMISSIONED_GROUPCAST == pairingConfigCmdInfo.commMode)
  {
    // Init. the group count to zero for REMOVE_GPD action
    gpPairingPayload[pairingConfigIndex++] = 0x00;
  }

  // groupCast radius
  gpPairingPayload[pairingConfigIndex++] = sinkTableEntry->tableGenericInfo.groupCastRadius;

  if(pairingConfigCmdActions.action == EXTEND_SINKTABLE_ENTRY || pairingConfigCmdActions.action == REPLACE_SINKTABLE_ENTRY)
  {
    // security related
    if(sinkTableEntry->options.securityUse)
    {
      memcpy(&gpPairingPayload[pairingConfigIndex], &sinkTableEntry->tableGenericInfo.securityOptions, sizeof(sinkTableEntry->tableGenericInfo.securityOptions));
      pairingConfigIndex += sizeof(sinkTableEntry->tableGenericInfo.securityOptions);
    }
    if(sinkTableEntry->options.securityUse || 
        (pairingConfigCmdOptions.seqNumCapabilities || !sinkTableEntry->options.securityUse))
    {
      uint32_t gpdSecurityFrameCounter;
      gpdSecurityFrameCounter = sinkTableEntry->tableGenericInfo.gpdSecurityFrameCounter;
      memcpy(&gpPairingPayload[pairingConfigIndex], &gpdSecurityFrameCounter, sizeof(uint32_t));
      pairingConfigIndex += sizeof(uint32_t);
    }
    if(sinkTableEntry->options.securityUse)
    {
      memcpy(&gpPairingPayload[pairingConfigIndex], &sinkTableEntry->tableGenericInfo.securityKey, SECURITY_KEY_SIZE);
      pairingConfigIndex += SECURITY_KEY_SIZE;
    }
  }
  
  if ((PRECOMMISSIONED_GROUPCAST == pairingConfigCmdInfo.commMode) && ( 0xFE != endPointInfo.noOfPairedEndPoints ))
  {
    zgpFreeMemReqBuffer();
    return ZGP_INVALID_ACTION;
  }

  gpPairingPayload[pairingConfigIndex] = endPointInfo.noOfPairedEndPoints;
  pairingConfigIndex += 1;

  if( endPointInfo.noOfPairedEndPoints !=0x00 && endPointInfo.noOfPairedEndPoints < 0xFD)
  {
    //check whether the endpoint list can be copied to gpPairingpayload
    if((zclMaxApsAsduSize - ZCL_FRAME_STANDARD_HEADER_SIZE) < (pairingConfigIndex + endPointInfo.noOfPairedEndPoints))
    {
      zgpFreeMemReqBuffer();
      return ZGP_INSUFFICIENT_SPACE_STATUS; 
    }
    memcpy(&gpPairingPayload[pairingConfigIndex],endPointInfo.pairedEndpoints, endPointInfo.noOfPairedEndPoints);
    pairingConfigIndex += endPointInfo.noOfPairedEndPoints;
  }

  if(pairingConfigCmdActions.action == EXTEND_SINKTABLE_ENTRY || pairingConfigCmdActions.action == REPLACE_SINKTABLE_ENTRY)
  {
   // app info
   if(pairingConfigCmdOptions.applicationInfoPresent)
   {
     if((zclMaxApsAsduSize - ZCL_FRAME_STANDARD_HEADER_SIZE) < (pairingConfigIndex + sizeof(commReqAppInfo->appInfoOptions)))
     {
       zgpFreeMemReqBuffer();
       return ZGP_INSUFFICIENT_SPACE_STATUS; 
     }
     memcpy(&gpPairingPayload[pairingConfigIndex], &commReqAppInfo->appInfoOptions, sizeof(commReqAppInfo->appInfoOptions));
     pairingConfigIndex += sizeof(commReqAppInfo->appInfoOptions);

     // manufacturer Id
     if(commReqAppInfo->appInfoOptions.manufacturerIdPresent)
     {
       uint16_t manufacturerId;
       manufacturerId = commReqAppInfo->manfacId;
       if((zclMaxApsAsduSize - ZCL_FRAME_STANDARD_HEADER_SIZE) < (pairingConfigIndex + sizeof(commReqAppInfo->manfacId)))
       {
         zgpFreeMemReqBuffer();
         return ZGP_INSUFFICIENT_SPACE_STATUS; 
       }
       memcpy(&gpPairingPayload[pairingConfigIndex], &manufacturerId, sizeof(commReqAppInfo->manfacId));
       pairingConfigIndex += sizeof(commReqAppInfo->manfacId);
     }
     if(commReqAppInfo->appInfoOptions.modelIdPresent)
     {
       uint16_t modelId;
       modelId = commReqAppInfo->modelId;
       if((zclMaxApsAsduSize - ZCL_FRAME_STANDARD_HEADER_SIZE) < (pairingConfigIndex + sizeof(commReqAppInfo->modelId)))
       {
         zgpFreeMemReqBuffer();
         return  ZGP_INSUFFICIENT_SPACE_STATUS; 
       }
       memcpy(&gpPairingPayload[pairingConfigIndex], &modelId, sizeof(commReqAppInfo->modelId));
       pairingConfigIndex += sizeof(commReqAppInfo->modelId);
     }
     if(commReqAppInfo->appInfoOptions.gpdCommandPresent)
     {
       if((zclMaxApsAsduSize - ZCL_FRAME_STANDARD_HEADER_SIZE) < (pairingConfigIndex + sizeof(commReqAppInfo->noOfGpdCmds) + commReqAppInfo->noOfGpdCmds))
       {
         zgpFreeMemReqBuffer();
         return ZGP_INSUFFICIENT_SPACE_STATUS; 
       }       
       gpPairingPayload[pairingConfigIndex++] = commReqAppInfo->noOfGpdCmds;
       if(commReqAppInfo->noOfGpdCmds)
       {
         memcpy(&gpPairingPayload[pairingConfigIndex], commReqAppInfo->gpdCommandList, commReqAppInfo->noOfGpdCmds);
         pairingConfigIndex += commReqAppInfo->noOfGpdCmds;
       }
     }
     
     if(commReqAppInfo->appInfoOptions.clusterListPresent)
     {
       if((zclMaxApsAsduSize - ZCL_FRAME_STANDARD_HEADER_SIZE) < (pairingConfigIndex + sizeof(commReqAppInfo->noOfClusters) + commReqAppInfo->noOfClusters.numOfServerCluster*2
                                                      + commReqAppInfo->noOfClusters.numOfClientCluster*2))
       {
         zgpFreeMemReqBuffer();
         return ZGP_INSUFFICIENT_SPACE_STATUS; 
       }
       memcpy(&gpPairingPayload[pairingConfigIndex++], &commReqAppInfo->noOfClusters, sizeof(commReqAppInfo->noOfClusters));
       if(commReqAppInfo->noOfClusters.numOfServerCluster)
       {
         memcpy(&gpPairingPayload[pairingConfigIndex], commReqAppInfo->clusterListServer, commReqAppInfo->noOfClusters.numOfServerCluster*2);
         pairingConfigIndex += commReqAppInfo->noOfClusters.numOfServerCluster * 2;
       }
       if(commReqAppInfo->noOfClusters.numOfClientCluster)
       {
         memcpy(&gpPairingPayload[pairingConfigIndex], commReqAppInfo->clusterListClient, commReqAppInfo->noOfClusters.numOfClientCluster*2);
         pairingConfigIndex += commReqAppInfo->noOfClusters.numOfClientCluster * 2;
       }
     }
#ifdef ZGP_ENABLE_GENERIC_8_CONTACT_SWITCH_SUPPORT
     if(commReqAppInfo->appInfoOptions.switchInformationPresent)
     {
      if((zclMaxApsAsduSize - ZCL_FRAME_STANDARD_HEADER_SIZE) < (pairingConfigIndex + sizeof(commReqAppInfo->switchInfo.switchInfoLength) + commReqAppInfo->switchInfo.switchInfoLength))
       {
         zgpFreeMemReqBuffer();
         return ZGP_INSUFFICIENT_SPACE_STATUS; 
       }
      //Negative Scenario handling(Do not trigger transmission if it has invalid value)
      //TBD to include this handling
       if(commReqAppInfo->switchInfo.switchInfoLength != ZGP_GENERIC_SWITCH_MIN_LENGTH) //value 0x02 defined as per spec A.4.2.1.1.10 
        return ZGP_INVALID_ACTION;
     // As per spec A.4.2.1.1.10 
       if(commReqAppInfo->switchInfo.currContactStatus == INVALID_CONTACT_VALUE || commReqAppInfo->switchInfo.genericSwitchConfig.noOfContacts == INVALID_CONTACT_VALUE)
       return ZGP_INVALID_ACTION;
       
       memcpy(&gpPairingPayload[pairingConfigIndex++], &commReqAppInfo->switchInfo.switchInfoLength, sizeof(commReqAppInfo->switchInfo.switchInfoLength));
       if(commReqAppInfo->switchInfo.switchInfoLength)
       {
         memcpy(&gpPairingPayload[pairingConfigIndex++], (void*)&commReqAppInfo->switchInfo.genericSwitchConfig, sizeof(uint8_t));
         memcpy(&gpPairingPayload[pairingConfigIndex++], (void*)&commReqAppInfo->switchInfo.currContactStatus, sizeof(uint8_t));
       }
     }
#endif
   }
  }
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
  if(pairingConfigCmdActions.action == APPLICATION_DESCRIPTION)
  {
    uint8_t retValAppDesc = ZGP_SUCCESS;
    uint8_t maxPairingConfigPayloadSize = zclMaxApsAsduSize - ZCL_FRAME_STANDARD_HEADER_SIZE - pairingConfigIndex;
    uint8_t *gpPairingTempPayload = &gpPairingPayload[pairingConfigIndex];
    uint8_t payloadLength = 0;
    retValAppDesc = zgpBuildApplDescInPairingConfigPayload(commReqAppInfo, gpPairingTempPayload, &payloadLength, maxPairingConfigPayloadSize);
    pairingConfigIndex += payloadLength;
    if(retValAppDesc == ZGP_INSUFFICIENT_SPACE_STATUS)
    {
      zgpFreeMemReqBuffer();
      return ZGP_INSUFFICIENT_SPACE_STATUS;
    }
    if(gpPairingConfigPendingInfo.pairingConfigAppInfo.noOfReportsPendingInGpPairingConfig != 0)
   {
     memcpy((void*)&gpPairingConfigPendingInfo.pairingConfigCmdInfo, (void*)&pairingConfigCmdInfo, sizeof(ZGP_PairingConfigCmdInfo_t));
     memcpy((void*)&gpPairingConfigPendingInfo.endPointInfo, (void*)&endPointInfo, sizeof(ZGP_EndpointInfo_t));
     memcpy((void*)&gpPairingConfigPendingInfo.pairingConfigAppInfo.appInfoOptions, (void*)&commReqAppInfo->appInfoOptions, sizeof(zgpGpdAppInfoOptions_t));
   }
  }
#endif
  retVal = ZGPH_SendCmdInRawMode(&dstZclAddr, ZCL_FRAME_CONTROL_DIRECTION_CLIENT_TO_SERVER, ZCL_GP_CLUSTER_SERVER_GP_PAIRING_CONFIGURATION_COMMAND_ID, pairingConfigIndex, gpPairingPayload);

  zgpFreeMemReqBuffer();
  if(retVal == ZCL_INSUFFICIENT_SPACE_STATUS)
    return ZGP_INSUFFICIENT_SPACE_STATUS;

  return (ZGP_InfraDeviceStatus_t)ZGP_SUCCESS_STATUS;
}
#endif // APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
#endif // _GREENPOWER_SUPPORT_

// eof zgpClusterClient.c
