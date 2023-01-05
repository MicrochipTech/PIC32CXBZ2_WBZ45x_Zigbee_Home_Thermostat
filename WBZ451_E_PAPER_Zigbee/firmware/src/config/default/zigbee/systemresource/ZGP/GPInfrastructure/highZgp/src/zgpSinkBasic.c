/*******************************************************************************
  Zigbee green power sink basic Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpSinkBasic.c

  Summary:
    This file contains the zgp proxy basic implementation.

  Description:
    This file contains the zgp proxy basic implementation.
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
#include <zdo/include/zdo.h>
#include <zcl/include/zcl.h>
#include <zcl/include/zclInt.h>
#include <security/serviceprovider/include/sspCommon.h>
#include <zcl/include/zclParser.h>
#include <zcl/include/zclAttributes.h>
#include <zcl/include/zclGreenPowerCluster.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterGeneric.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterServer.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpClusterZclInterface.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLow.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighGeneric.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighMem.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpSinkTable.h>
#include <zgp/include/zgpDbg.h>
#include <configserver/include/configserver.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpSinkBasic.h>
#include <nwk/include/nwk.h>
#include <systemenvironment/include/sysEvents.h>
#include <systemenvironment/include/sysAssert.h>
#include <pds/include/wlPdsMemIds.h>
#include <zllplatform/ZLL/N_DeviceInfo/include/N_DeviceInfo_Bindings.h>
#include <zllplatform/ZLL/N_DeviceInfo/include/N_DeviceInfo.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpCluster.h>
#if MICROCHIP_APPLICATION_SUPPORT == 1
#include <bdb/include/bdb.h>
#endif
/******************************************************************************
                    Defines section
******************************************************************************/
// As per the current spec. this feature not supported so having value zero
#define ZGP_PROXY_COMM_MODE_CHANNEL 0x00
#define GROUP_KEY_HMAC_TEXT_SIZE      3U
#define ZGP_HMAC_TEXT_SIZE_MAX        8U
#define ZGP_HMAC_TEXTBUFFER_SIZE_MAX  (SECURITY_KEY_SIZE + ZGP_HMAC_TEXT_SIZE_MAX + SECURITY_KEY_SIZE + SECURITY_KEY_SIZE) // Need to analyze about additional 16 bytes
#define ZGP_HMAC_TEXT_START_POS       16U
#define ZGP_REPORT_PARSING_SUCCESS    0x00
#define ZGP_SRC_ID_LEN                0x04
#define ZGP_IEEE_ADDR_LEN             0x08
/******************************************************************************
                    External section
******************************************************************************/
extern zgpCommReqPairingConfigSessionEntry_t commSessionEntries[];
extern zgpSinkRxdGpdfIndBuffer_t zgpRxdGpdfIndBuffer;

/******************************************************************************
                    Types section
******************************************************************************/
typedef enum PACK
{
  SEC_IDLE = 0x00,
  SEC_KEY_ENCRYPTION,
  SEC_KEY_DECRYPTION,
  SEC_GPDF_DECRYPTION,
  SEC_KEY_DERIVATION
} secProcContextState_t;

typedef union PACK
{
  SSP_ZgpDecryptFrameReq_t zgpSinkDecryptReq;
  SSP_ZgpEncryptFrameReq_t zgpSinkEncryptReq;
  SSP_KeyedHashMacReq_t zgpSinkKeyedHashReq;
} secReq_t;

typedef struct PACK
{
  ZGP_ApplicationId_t appId;
  ZGP_GpdId_t gpdId;
  uint8_t endPoint;
  ZGP_FrameType_t frameType;
  uint8_t cmdId;
  uint8_t cmdPayloadLength;
  uint8_t *cmdPayload;
  uint8_t tempMasterTxChannel;
  uint16_t tempMasterAddr;
  bool sendInDirectMode;
} gpdfRsp_t;

typedef struct PACK
{
  secReq_t secReq;

  union
  {
    // key encryption/decryption data
    struct
    {
      uint8_t sessionIndex;
      uint8_t securedKeyPayload[ZGP_COMM_KEY_HEADER_LEN + ZGP_SECURITY_KEY_LENGTH + ZGP_SEC_FOURBYTE_MIC*2];
      uint8_t key[ZGP_SECURITY_KEY_LENGTH];
    } keySecProcessing;
    // GPDF decryption data
    struct
    {
      uint8_t dataBuffer[ZGP_MAX_GSDU_SIZE];
      uint8_t asduLength;
      uint8_t key[ZGP_SECURITY_KEY_LENGTH];
      ZGP_LowDataInd_t sinkGpdfDataInd;
      uint16_t tempMasterAddr;
      bool sendInDirectMode;
    } gpdfSecProcessing;
    struct
    {
      uint8_t sessionIndex;
      uint8_t hmacText[ZGP_HMAC_TEXTBUFFER_SIZE_MAX]; // For hash function
      uint8_t gpdDerivedKey[ZGP_SECURITY_KEY_LENGTH];
    } keyDerivation;
  };
} secProcData_t;

typedef struct PACK
{
  secProcContextState_t secContextState;
  secProcData_t secProcData;
} gpdfSecProcContextData_t;

/******************************************************************************
                    Prototypes section
******************************************************************************/
static bool filterRxGpdf(ZGP_LowDataInd_t *ind);
static bool decryptRxGpdf(gpdfSecProcContextData_t *secContextData, ZGP_LowDataInd_t *ind);
static void decryptRxGpdfConfirm(SSP_ZgpDecryptFrameConf_t *conf);
static sinkCommProcState_t commReqDecryptGpdKey(uint8_t sessionIndex);
static void commReqDecryptGpdKeyconfirm(SSP_ZgpDecryptFrameConf_t *conf);
static bool rxGpdfParsing(ZGP_LowDataInd_t *ind, bool responseToGpd, uint16_t proxyAddr);
static void encryptGpdKeyconfirm(SSP_ZgpEncryptFrameConf_t *conf);
static void sendGpdfInDirectMode(gpdfRsp_t *gpdfRsp);
static void sendGpdfResponse(gpdfRsp_t *gpdfRsp);
static bool createSinkTableEntry(ZGP_SinkTableEntry_t *sinkEntry,ZGP_LowDataInd_t *ind);
static bool updateEntryInfo(ZGP_SinkTableEntry_t *sinkEntry,ZGP_ApplicationId_t appId, ZGP_GpdId_t *gpdId,\
                            uint8_t endPoint);
static bool sinkBasicFuncMatching(uint8_t *deviceId, ZGP_GpdAppInfo_t *appInfo);
static void sinkBasicCommWindowTimerFired(void);
static void sinkBasicCheckCommMode(void);
static void sinkBasicExitCommMode(void);
static bool isEndPointRegistered(uint8_t endPointId);
static void triggerSuccessCommissioningEvent(uint8_t sessionEntryIndex);
static void triggerGpdfCommandRxdEvent(ZGP_GpdId_t *gpdId, ZGP_LowDataInd_t *ind);
static sinkCommProcState_t zgpGenericKeyDerivationHmac(uint8_t entryIndex);
static void zgpHmacKeyDerivationConf(SSP_KeyedHashMacConf_t *conf);
static void processRxdGpdf(ZGP_LowDataInd_t *ind, bool responseToGpd, uint16_t proxyAddr);
static void sendGpdfResponse(gpdfRsp_t *gpdfRsp);
static sinkCommProcState_t checkForCommissioningCompletion(uint8_t sessionEntryIndex);
static sinkCommProcState_t commissioningCompletionhandling(uint8_t sessionEntryIndex);
static void flushoutTxQueueForGpd(uint8_t sessionEntryIndex);
static bool checkForGpdKeyDerivation(zgpCommReq_t *commReq, ZGP_SecKeyType_t *derivedKeyType, uint8_t sessionIndex);
static sinkCommProcState_t frameAndSendCommissioningReply(uint8_t sessionEntryIndex);
static sinkCommProcState_t encryptTxGpdKey(uint8_t sessionEntryIndex);
static gpdfSecProcContextData_t * getSecProcContextDataBuffer(secProcContextState_t secState);
static void fillSecProcContextData(gpdfSecProcContextData_t *secProcContextData, \
           ZGP_LowDataInd_t *gpdfDataInd, bool responseToGpd, uint16_t gppShortAddr);
static void initCommSessionEntry(uint8_t sessionIndex, ZGP_LowDataInd_t *ind, ZGP_GpdId_t *gpdId, bool responseToGpd);
static void processCommissioningReqCmd(ZGP_LowDataInd_t *ind, bool responseToGpd, ZGP_GpdId_t *gpdId, uint16_t proxyAddr);
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
static void processAppDescriptionCmd(ZGP_LowDataInd_t *ind, bool responseToGpd, ZGP_GpdId_t *gpdId, uint16_t proxyAddr);
#endif
static void createSinkEntry(ZGP_SinkTableEntry_t *sinkEntry, uint8_t sessionEntryIndex);
static void commissioningHandler(uint8_t sessionEntryIndex);
static bool checkForPrecommissinedCommModeInfo(ZGP_LowDataInd_t *ind, ZGP_GpdId_t *gpdId);
static sinkCommProcState_t handleCommReply(uint8_t sessionEntryIndex);
static bool isCompatibleWithExistingCommReq(zgpCommReq_t *prevReq, zgpCommReq_t *rxdCommReq, ZGP_GpdAppInfo_t *prevAppinfo, \
                                             ZGP_GpdAppInfo_t *rxdAppInfo);

#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
/*New API*/
static void startPairingConfigTimer(uint16_t interval);
static void stopPairingConfigTimer(void);
static void pairingConfigTimeoutCallback(void);
static HAL_AppTimer_t pairingConfigTimer;
#endif
extern ZGP_GpPairingConfigPendingInfo_t gpPairingConfigPendingInfo;

/******************************************************************************
                    local variables
******************************************************************************/
static zgpSinkBasicModeInfo_t sinkBasicModeInfo;
static HAL_AppTimer_t commWindowTimer =
{
  .mode     = TIMER_ONE_SHOT_MODE,
  .callback = sinkBasicCommWindowTimerFired
};

static gpdfSecProcContextData_t gpdfSecProcContextData;
/******************************************************************************
                    Global variables
******************************************************************************/
ZGP_SinkGroup_t zgpSinkGroupEntry;

/******************************************************************************
                    Implementations section
******************************************************************************/
/**************************************************************************//**
  \brief Initialize zgp sink.

  \param[in] None

  \return None.
******************************************************************************/
void ZGPH_SinkBasicInit(void)
{
  zgpClusterServerInit();

  ZGPL_SetDeviceMode(false, OPERATIONAL_MODE);
  zgpSinkGroupEntry.sinkGroup = ZGP_NWK_ADDRESS_GROUP_INIT;
  zgpSinkGroupEntry.alias = ZGP_NWK_ADDRESS_GROUP_INIT;
  if (PDS_IsAbleToRestore(ZGP_SINK_GROUP_ENTRY_MEM_ID))
    PDS_Restore(ZGP_SINK_GROUP_ENTRY_MEM_ID);

}

/**************************************************************************//**
  \brief Putting local sink in commissioning mode

  \param[in] sinkCommModeOptions - commissioning mode options of sink
             endPoint - Endpoint to be put in commissioning mode
             unicastComm - status of unicast commissioining

  \return status.
******************************************************************************/
ZGP_InfraDeviceStatus_t ZGPH_PutLocalSinkInCommissioningMode(ZGP_SinkCommissioningModeOptions_t *sinkCommModeOptions, \
                                         uint8_t endPoint, bool unicastComm)
{
  ZGP_GpsSecurityLevelAttribute_t *secLevel = (ZGP_GpsSecurityLevelAttribute_t *)&zgpClusterServerAttributes.gpsSecurityLevel.value;

  // As per the current spec.(r16), commissioning is not allowed when involveTc is set
#if MICROCHIP_APPLICATION_SUPPORT == 1
  if (!BDB_GetBdbNodeIsOnANetwork() || secLevel->involveTc)
#else
  if (N_DeviceInfo_IsFactoryNew()|| secLevel->involveTc)
#endif
    return ZGP_NOT_IN_EXPECTED_STATE;

  // check the endpoint is registered
  if ((ALL_END_POINT != endPoint) && !isEndPointRegistered(endPoint))
  {
    // the given endPoint is not registered
    return ZGP_ENDPOINT_NOT_REGISTERED;
  }

  if (sinkCommModeOptions->action)
  {
    if (COMMISSIONING_MODE == ZGPL_GetDeviceMode(false))
      return ZGP_ALREADY_IN_COMMISSIONING_MODE;
    ZGPL_SetDeviceMode(false, COMMISSIONING_MODE);

    commWindowTimer.interval = zgpClusterServerAttributes.gpsCommissioningWindow.value * 1000;

    HAL_StopAppTimer(&commWindowTimer);
    HAL_StartAppTimer(&commWindowTimer);

    sinkBasicModeInfo.commissioningGpdEp = endPoint;
  }
  else
  {
    HAL_StopAppTimer(&commWindowTimer);
    sinkBasicExitCommMode();
  }

  if (sinkCommModeOptions->involveProxies)
  {
    zgpGpProxyCommModeOptions_t proxyOptions;
    
    memset(&proxyOptions, 0, sizeof(zgpGpProxyCommModeOptions_t));
    proxyOptions.action = sinkCommModeOptions->action;
    if(zgpClusterServerAttributes.gpsCommissioningExitMode.value & GPS_EXIT_ON_COMM_WINDOW_EXPIRATION)
      proxyOptions.commWindowPresent = true;
    if(zgpClusterServerAttributes.gpsCommissioningExitMode.value & GPS_EXIT_ON_FIRST_PAIRING_SUCCESS)
      proxyOptions.exitMode =  GPP_EXIT_ON_FIRST_PAIRING_SUCCESS;
    if(zgpClusterServerAttributes.gpsCommissioningExitMode.value & GPS_EXIT_ON_PROXY_COMM_MODE_EXIT)
      proxyOptions.exitMode |=  GPP_EXIT_ON_EXIT_PROXY_COMM_MODE;
    proxyOptions.channelPresent = false; // As per the current spec., this is set to 0 always
    proxyOptions.unicastCommunication = unicastComm;

    ZGPH_SendProxyCommissioningModeCommand(proxyOptions, zgpClusterServerAttributes.gpsCommissioningWindow.value, \
                                 ZGP_PROXY_COMM_MODE_CHANNEL);
  }

  return ZGP_SUCCESS;
}

/**************************************************************************//**
  \brief exits commissioning mode on comm window timeout

  \param[in] None

  \return None.
******************************************************************************/
static void sinkBasicCommWindowTimerFired(void)
{
  sinkBasicExitCommMode();
}

/**************************************************************************//**
  \brief get security processing context data buffer

  \param[in] secState - context from which this is called

  \return sec. context buffer pointer
******************************************************************************/
static gpdfSecProcContextData_t * getSecProcContextDataBuffer(secProcContextState_t contextState)
{
  if (SEC_IDLE ==(uint8_t) gpdfSecProcContextData.secContextState)
  {
    memset(&gpdfSecProcContextData, 0x00, sizeof(gpdfSecProcContextData));
    gpdfSecProcContextData.secContextState = contextState;
    return &gpdfSecProcContextData;
  }
  else
    return NULL;
}

/**************************************************************************//**
  \brief Free security processing context data buffer

  \param[in] gpdfSecProcContextData - sec. context buffer pointer
             contextState - context state of the buffer

  \return sec. context buffer pointer
******************************************************************************/
static void freeSecContextData(gpdfSecProcContextData_t *gpdfSecProcContextData, secProcContextState_t contextState)
{
  if (contextState == gpdfSecProcContextData->secContextState)
  {
    gpdfSecProcContextData->secContextState = SEC_IDLE;
  }
  else
  {
    // this is not expected.. need to raise the assert
  }
}

/**************************************************************************//**
  \brief Fill security processing context data buffer

  \param[in] secProcContextData - sec. context buffer pointer
             gpdfDataInd - gpdf indication
             responseToGpd - response to gpd required
             contextState - context state of the buffer
             gppShortAddr - temp master short addr

  \return None
******************************************************************************/
static void fillSecProcContextData(gpdfSecProcContextData_t *secProcContextData, \
           ZGP_LowDataInd_t *gpdfDataInd, bool responseToGpd, uint16_t gppShortAddr)
{
  secProcContextData->secProcData.gpdfSecProcessing.tempMasterAddr = gppShortAddr;
  secProcContextData->secProcData.gpdfSecProcessing.sendInDirectMode = responseToGpd;
  memcpy(&secProcContextData->secProcData.gpdfSecProcessing.sinkGpdfDataInd, gpdfDataInd, sizeof(secProcContextData->secProcData.gpdfSecProcessing.sinkGpdfDataInd));
  secProcContextData->secProcData.gpdfSecProcessing.sinkGpdfDataInd.gpdAsdu = &secProcContextData->secProcData.gpdfSecProcessing.dataBuffer[0];
  memcpy(secProcContextData->secProcData.gpdfSecProcessing.sinkGpdfDataInd.gpdAsdu, gpdfDataInd->gpdAsdu, gpdfDataInd->gpdAsduLength);
  secProcContextData->secProcData.gpdfSecProcessing.asduLength = gpdfDataInd->gpdAsduLength;
}

/**************************************************************************//**
  \brief Handling GPDF receiving from cluster client.

  \param[in] gpdfDataInd - gpdf indication
             gppShortAddr - proxy address

  \return None.
******************************************************************************/
void zgpSinkGpdfHandling(ZGP_LowDataInd_t *gpdfDataInd, bool responseToGpd, uint16_t gppShortAddr)
{
  // decryption of secured GPDF
  if (ZGP_DSTUB_UNPROCESSED == (uint8_t)gpdfDataInd->status)
  {
    gpdfSecProcContextData_t *secProcContextData = getSecProcContextDataBuffer(SEC_GPDF_DECRYPTION);
    if (secProcContextData)
    {
      fillSecProcContextData(secProcContextData, gpdfDataInd, responseToGpd, gppShortAddr);
      if (!decryptRxGpdf(secProcContextData, gpdfDataInd))
      {
        freeSecContextData(secProcContextData, SEC_GPDF_DECRYPTION);
      }
    }
  }
  else
    processRxdGpdf(gpdfDataInd, responseToGpd, gppShortAddr);
}

/**************************************************************************//**
  \brief Indication callback for rxd GPDF

  \param[in] ind - pointer to dstub data indication

  \return None.
******************************************************************************/
void zgpSinkDstubDataInd(const ZGP_LowDataInd_t *const ind)
{
  zgpSinkGpdfHandling((ZGP_LowDataInd_t *)ind, true, NWK_GetShortAddr());
}

/**************************************************************************//**
  \brief GPDF processing

  \param[in] ind - pointer to dstub data indication
             responseToGpd - respond to GPD(proximity mode)
             proxyAddr - proxy network addr(multi-hop commissioning)

  \return None.
******************************************************************************/
static void processRxdGpdf(ZGP_LowDataInd_t *ind, bool responseToGpd, uint16_t proxyAddr)
{
  if (!filterRxGpdf(ind))
  {
    return;
  }

  if (!rxGpdfParsing(ind, responseToGpd, proxyAddr))
      return;
}

/**************************************************************************//**
  \brief processing channel request command

  \param[in] ind - pointer to dstub data indication
             responseToGpd - response to gpd
             proxyAddr - temp master addr

  \return None.
******************************************************************************/
static void processChannelReqCmd(ZGP_LowDataInd_t *ind, bool responseToGpd, uint16_t proxyAddr)
{
  gpdfRsp_t sinkGpdfRsp;
  uint8_t channelConfigPayload[sizeof(zgpChannelConfig_t)];

  memset(&sinkGpdfRsp, 0x00, sizeof(sinkGpdfRsp));
  sinkGpdfRsp.cmdPayload = &channelConfigPayload[0];
  if ((!ind->rxAfterTx) && (!responseToGpd))
  {
    return;
  }

  memset(&channelConfigPayload[0], 0x00, sizeof(channelConfigPayload));
  // Sending the channel config
  zgpChannelConfig_t *channelConfig = (zgpChannelConfig_t *)sinkGpdfRsp.cmdPayload;
  zgpChannelReq_t *channelReq = (zgpChannelReq_t *)ind->gpdAsdu;

  ind->gpdCommandId = ZGP_CHANNEL_CONFIG_CMD_ID;
  channelConfig->basic = true;
  channelConfig->operationalChannel = NWK_GetLogicalChannel() - ZGP_CHANNEL_OFFSET;

  if ((channelReq->rxChannelInNextAttempt != channelConfig->operationalChannel) && responseToGpd)
  {
    // Trigger channel change request
    zgpGenericChannelChangeHandling(channelReq->rxChannelInNextAttempt + ZGP_CHANNEL_OFFSET);
  }
  sinkGpdfRsp.tempMasterTxChannel = channelReq->rxChannelInNextAttempt;
  sinkGpdfRsp.cmdPayloadLength = sizeof(zgpChannelConfig_t);
  sinkGpdfRsp.cmdId = ZGP_CHANNEL_CONFIG_CMD_ID;
  sinkGpdfRsp.appId = ind->applicationId;
  sinkGpdfRsp.gpdId.gpdSrcId = ind->srcId;
  sinkGpdfRsp.tempMasterAddr = proxyAddr;
  sinkGpdfRsp.sendInDirectMode = responseToGpd;

  sendGpdfResponse(&sinkGpdfRsp);

}

/**************************************************************************//**
  \brief processing commissioning success command

  \param[in] ind - pointer to dstub data indication
             gpdId - gpd id

  \return None.
******************************************************************************/
static void processCommissioningSuccessReqCmd(ZGP_LowDataInd_t *ind, ZGP_GpdId_t *gpdId)
{
  zgpSessionIndex_t sessionIndex = {.entryIndex = ZGP_INVALID_SESSION_ENTRY_INDEX, .freeIndex = ZGP_INVALID_SESSION_ENTRY_INDEX};

  // fecth the sink entry from session entry
  zgpServerGetCommReqPairingConfigSessionTableIndex(ind->applicationId, gpdId, ind->endPoint, &sessionIndex);

  if ((ZGP_INVALID_SESSION_ENTRY_INDEX == sessionIndex.entryIndex) || \
      (COMM_REQ_ENTRY != sessionIndex.entryStatus) || \
      (COMM_WAIT_FOR_SUCCCESS_IND != commSessionEntries[sessionIndex.entryIndex].commState))
    return;

  if ((commSessionEntries[sessionIndex.entryIndex].commReq.extOptions.securityLevelCapabilities == (uint8_t)ind->gpdfSecurityLevel) && \
      ((!(uint8_t)ind->gpdfSecurityLevel) ||
       ((uint8_t)ind->gpdfSecurityLevel && (ind->gpdSecurityFrameCounter > commSessionEntries[sessionIndex.entryIndex].commReq.gpdOutgoingCounter))))
  {
    commSessionEntries[sessionIndex.entryIndex].commReq.gpdOutgoingCounter = ind->gpdSecurityFrameCounter;

    commSessionEntries[sessionIndex.entryIndex].commState = COMM_SUCCESSFUL_SESSION;
    flushoutTxQueueForGpd(sessionIndex.entryIndex);
    commissioningHandler(sessionIndex.entryIndex);
  }
  else
  {
    // Incorrect success frame received
    // can be notified to the application
    flushoutTxQueueForGpd(sessionIndex.entryIndex);
  }

}

/**************************************************************************//**
  \brief flushing out txQueue for the gpd of the given entry session indes

  \param[in] sessionEntryIndex - session entry index

  \return None.
******************************************************************************/
static void flushoutTxQueueForGpd(uint8_t sessionEntryIndex)
{
  // Release the pending Tx entries for this GPD
  ZGP_GpdfDataReq_t *dStubDataReq = (ZGP_GpdfDataReq_t *)zgpGetMemReqBuffer();
  dStubDataReq->action = false;
  dStubDataReq->applicationId = commSessionEntries[sessionEntryIndex].appId;

  // Since action is false, dStubDataReq->gpAsdu(payload) won't be used
  // so not initialized
  if(ZGP_SRC_APPID == dStubDataReq->applicationId)
  {
    dStubDataReq->srcId = commSessionEntries[sessionEntryIndex].gpdId.gpdSrcId;
  }
  else if(ZGP_IEEE_ADDR_APPID == dStubDataReq->applicationId)
  {
    dStubDataReq->gpdIeeeAddress = commSessionEntries[sessionEntryIndex].gpdId.gpdIeeeAddr;
    dStubDataReq->endpoint = commSessionEntries[sessionEntryIndex].endPoint;
  }

  ZGPL_GpdfDataRequest(dStubDataReq);
  zgpFreeMemReqBuffer();
}

/**************************************************************************//**
  \brief process gpd command

  \param[in] ind - pointer to dstub data indication
             gpdId - gpd id

  \return None.
******************************************************************************/
static void processGpdCommand(ZGP_LowDataInd_t *ind, ZGP_GpdId_t *gpdId)
{
  ZGP_SinkTableEntry_t *sinkEntry = (ZGP_SinkTableEntry_t *)zgpGetMemReqBuffer();
  uint32_t frameCounter;
  ZGP_ReadOperationStatus_t entryStatus;

  ZGP_TableOperationField_t  tableOperationField = {.entryType = ZGP_SINK_ENTRY, .commMode = ALL_COMMUNICATION_MODE, .appId = ind->applicationId, \
                                                  .nonEmptyIndexForRead = ZGP_ENTRY_INVALID_INDEX};

  entryStatus = ZGPL_ReadTableEntryFromNvm((void *)sinkEntry, tableOperationField, gpdId, ind->endPoint);

  if ((ACTIVE_ENTRY_AVAILABLE != entryStatus) || (sinkEntry->tableGenericInfo.securityOptions.securityLevel != (uint8_t)ind->gpdfSecurityLevel) || \
      (sinkEntry->tableGenericInfo.securityOptions.securityLevel && ((sinkEntry->tableGenericInfo.securityOptions.securityKeyType != (uint8_t)ind->gpdfKeyType) || \
      (ind->gpdSecurityFrameCounter <= sinkEntry->tableGenericInfo.gpdSecurityFrameCounter))))
  {
    bool errorCondition = true;
    if(entryStatus == ENTRY_NOT_AVAILABLE)
    {
      if ((COMMISSIONING_MODE == ZGPL_GetDeviceMode(false)) && ind->autoCommissioning)
      {
        if (createSinkTableEntry(sinkEntry,ind))
          errorCondition = false;
      }
    }
    if(errorCondition)
    {
      zgpFreeMemReqBuffer();
      return;
    }
  }
#ifdef ZGP_ENABLE_GENERIC_8_CONTACT_SWITCH_SUPPORT
  else if((entryStatus == ACTIVE_ENTRY_AVAILABLE) && ((COMMISSIONING_MODE == ZGPL_GetDeviceMode(false)) && ind->autoCommissioning))
  {
    if(sinkEntry->deviceId == (uint8_t)ZGP_GENERIC_8_CONTACT_SWITCH)
    {
      ZGP_GpdAppInfo_t appInfo;
      appInfo.noOfGpdCmds = 1;
      appInfo.gpdCommandList[0] = ind->gpdCommandId;
      appInfo.switchInfo.currContactStatus = *(ind->gpdAsdu);
      zgpTransRemoveAndAddEventHandler(COMM_PROC_TYPE, sinkEntry, &appInfo, 1, &sinkBasicModeInfo.commissioningGpdEp);
     }
   }
#endif
  frameCounter = ind->gpdSecurityFrameCounter;
  ZGPL_FrameCounterReadorUpdateOnNvm(&frameCounter, tableOperationField, gpdId, ind->endPoint, \
                                    true);

  zgpFreeMemReqBuffer();
  triggerGpdfCommandRxdEvent(gpdId, ind);
}

/**************************************************************************//**
  \brief process decommissioning  command

  \param[in] ind - pointer to dstub data indication
             gpdId - gpd id

  \return None.
******************************************************************************/
static void processDecommissioningCmd(ZGP_LowDataInd_t *ind, ZGP_GpdId_t *gpdId)
{
  ZGP_SinkTableEntry_t *sinkEntry = (ZGP_SinkTableEntry_t *)zgpGetMemReqBuffer();
  ZGP_TableOperationField_t  tableOperationField = {.entryType = ZGP_SINK_ENTRY, .commMode = ALL_COMMUNICATION_MODE, .appId = ind->applicationId, \
                                                   .nonEmptyIndexForRead = ZGP_ENTRY_INVALID_INDEX};

  if (ENTRY_NOT_AVAILABLE != ZGPL_ReadTableEntryFromNvm((void *)sinkEntry, tableOperationField, gpdId, ind->endPoint))
  {
    if ((sinkEntry->tableGenericInfo.securityOptions.securityLevel == ind->gpdfSecurityLevel) && \
       ((!ind->gpdfSecurityLevel) ||
       (ind->gpdfSecurityLevel && (ind->gpdSecurityFrameCounter > sinkEntry->tableGenericInfo.gpdSecurityFrameCounter))))
    {
      ZGPL_DeleteTableEntryFromNvm(tableOperationField, gpdId, ind->endPoint);
      ZGPH_SendGpPairing(sinkEntry,REMOVE_GPD, 0xFF);
      zgpTransRemoveAndAddEventHandler(NONE, sinkEntry, NULL, 0x00, NULL);
      //to send GpPairingConfig only for PreCommissioned Groupcast
      if (PRECOMMISSIONED_GROUPCAST == sinkEntry->options.communicationMode)
      {
        ZGP_PairingConfigCmdInfo_t pairingConfigCmdInfo;
        ZGP_EndpointInfo_t endPointInfo;
        ZGP_GpdAppInfo_t *commReqAppInfo = NULL;
        APS_Address_t dstAddr; 
        dstAddr.shortAddress = BROADCAST_ADDR_RX_ON_WHEN_IDLE;
        uint32_t srcId = sinkEntry->tableGenericInfo.gpdId.gpdSrcId;
        uint64_t IeeeAddr = sinkEntry->tableGenericInfo.gpdId.gpdIeeeAddr;

        memset(&pairingConfigCmdInfo, 0 , sizeof(ZGP_PairingConfigCmdInfo_t));
        memset(&endPointInfo, 0, sizeof(ZGP_EndpointInfo_t));

        pairingConfigCmdInfo.appId = (ZGP_ApplicationId_t)sinkEntry->options.appId;
        if(pairingConfigCmdInfo.appId == ZGP_SRC_APPID)
          memcpy(&pairingConfigCmdInfo.gpdId, &srcId, ZGP_SRC_ID_LEN);
        else if(pairingConfigCmdInfo.appId  == ZGP_IEEE_ADDR_APPID)
          memcpy(&pairingConfigCmdInfo.gpdId, &IeeeAddr, ZGP_IEEE_ADDR_LEN);
        pairingConfigCmdInfo.gpdEndPoint = sinkEntry->tableGenericInfo.endPoint;
        pairingConfigCmdInfo.action.action = REMOVE_GPD;
        pairingConfigCmdInfo.action.sendGpPairing = false;
        pairingConfigCmdInfo.deviceId = sinkEntry->deviceId;
        pairingConfigCmdInfo.commMode = PRECOMMISSIONED_GROUPCAST;

        endPointInfo.noOfPairedEndPoints = 0xFE;
        zgpFreeMemReqBuffer();
        ZGPH_SendGpPairingConfigCmd(APS_SHORT_ADDRESS, &dstAddr, pairingConfigCmdInfo, commReqAppInfo, endPointInfo);
      }
    }

  }
  zgpFreeMemReqBuffer();
}

/**************************************************************************//**
  \brief Check the compatibility of the rxd comm. req with the existing one in
         the session entry

  \param[in] prevReq - prev comm. req
             rxdCommReq - rxd comm. req
             prevAppinfo - prev app info
             rxdAppInfo - rxd app. info

  \return true if compatible
          false otherwise
******************************************************************************/
static bool isCompatibleWithExistingCommReq(zgpCommReq_t *prevReq, zgpCommReq_t *rxdCommReq, ZGP_GpdAppInfo_t *prevAppinfo, \
                                             ZGP_GpdAppInfo_t *rxdAppInfo)
{
  if (!memcmp(&prevReq->options, &rxdCommReq->options, sizeof(zgpCommCmdReqOptions_t)) && \
      !memcmp(&prevAppinfo->appInfoOptions, &rxdAppInfo->appInfoOptions, sizeof(zgpGpdAppInfoOptions_t)))
  {
    return true;
  }
  return false;
}

/**************************************************************************//**
  \brief parse the comm. req command fields

  \param[in] ind - gpdf indication
             sessionEntryIndex - session entry index

  \return true if successful parsing
          false otherwise
******************************************************************************/
static bool parseCommissioningReqFields(ZGP_LowDataInd_t *ind, uint8_t sessionEntryIndex)
{
  uint8_t commReqIndex = 0;
  zgpCommReq_t *commReq = zgpGetMemReqBuffer();
  ZGP_GpdAppInfo_t appInfo;

  memset(&appInfo, 0x00, sizeof(ZGP_GpdAppInfo_t));
  memset(commReq, 0x00, sizeof(zgpCommReq_t));
  commReq->gpdDeviceId = ind->gpdAsdu[commReqIndex++]; // Skipping the device id
  memcpy(&commReq->options, &ind->gpdAsdu[commReqIndex], sizeof(commReq->options));
  commReqIndex++;

  if (commReq->options.extOptionsField)
  {
    ZGP_GpsSecurityLevelAttribute_t *gpsSecurity = (ZGP_GpsSecurityLevelAttribute_t *) &zgpClusterServerAttributes.gpsSecurityLevel.value;

    memcpy(&commReq->extOptions, &ind->gpdAsdu[commReqIndex], sizeof(commReq->extOptions));
    commReqIndex++;

  //Since in case of subsequent commissioning any keyType can be used by GPD we are checking only for ZGP_KEY_TYPE_NO_KEY
    if ((ind->gpdAsduLength < commReqIndex) || \
        (commReq->extOptions.securityLevelCapabilities < gpsSecurity->minGpdSecurityLevel) ||
        (ZGP_SECURITY_LEVEL_1 == commReq->extOptions.securityLevelCapabilities) ||
        ((!commReq->options.gpSecurityKeyRequest)&& commReq->extOptions.securityLevelCapabilities && \
          ((commReq->extOptions.keyType == ZGP_KEY_TYPE_NO_KEY))) || \
          (FULL_UNICAST == zgpClusterServerAttributes.gpsCommunicationMode.value))
    {
      zgpFreeMemReqBuffer();
      return false;
    }

    if (commReq->extOptions.gpdKeyPresent)
    {
      if (ind->gpdAsduLength < (commReqIndex + ZGP_SECURITY_KEY_LENGTH))
      {
        zgpFreeMemReqBuffer();
        return false;
      }

      memcpy(&commReq->gpdKey[0], &ind->gpdAsdu[commReqIndex], sizeof(commReq->gpdKey));
      commReqIndex += sizeof(commReq->gpdKey);

      if (commReq->extOptions.gpdKeyEncryption)
      {
        if (ind->gpdAsduLength < (commReqIndex + sizeof(commReq->gpdKeyMic)))
        {
          zgpFreeMemReqBuffer();
          return false;
        }

        memcpy(&commReq->gpdKeyMic, &ind->gpdAsdu[commReqIndex], sizeof(commReq->gpdKeyMic));
        commReqIndex += sizeof(commReq->gpdKeyMic); // MIC field
      }
      else
      {
        if (gpsSecurity->protectionWithGpLinkKey)
        {
          zgpFreeMemReqBuffer();
          return false;
        }
      }

    }
    if (commReq->extOptions.gpdOutgoingCounterPresent)
    {
      if (ind->gpdAsduLength < (commReqIndex + ZGP_SEC_FRAME_CTR_LENGTH))
      {
        zgpFreeMemReqBuffer();
        return false;
      }

      memcpy(&commReq->gpdOutgoingCounter, &ind->gpdAsdu[commReqIndex], sizeof(commReq->gpdOutgoingCounter));
      commReqIndex += sizeof(commReq->gpdOutgoingCounter);
    }
    else if(commReq->options.macSeqNoCapability)
    {
      commReq->gpdOutgoingCounter = (uint32_t)ind->seqNumber;
    }
  }
  else
  {
    if (commReq->options.gpSecurityKeyRequest)
    {
      // should be no security mode
      // If sink supports no security mode, then we can send comm. reply otherwise
      // we can drop the frame
      //options.securityLevel = gpsSecurity->minGpdSecurityLevel; // basic security level
    }
  }

  if (ind->gpdAsduLength < commReqIndex)
  {
    zgpFreeMemReqBuffer();
    return false;
  }

  if (commReq->options.applicationInfoPresent)
  {
    // parse the app. info
    if (!zgpParsingAppInfoFromPayload(ind->gpdAsdu, &commReqIndex, &appInfo, ind->gpdAsduLength))
    {
      zgpFreeMemReqBuffer();
      return false;
    }
  }

  if (commSessionEntries[sessionEntryIndex].validActionRxd)
  {
    // we have already rxd commissioning request. Nee
    // Need to check whether the rxd one is matching the existing one
    if (!isCompatibleWithExistingCommReq(&commSessionEntries[sessionEntryIndex].commReq, commReq,\
      &commSessionEntries[sessionEntryIndex].gpdAppInfo, &appInfo))
    {
      zgpFreeMemReqBuffer();
      return false;
    }
  }
  memcpy(&commSessionEntries[sessionEntryIndex].commReq , commReq, sizeof(zgpCommReq_t));

  {
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
    // We make sure application descritor information is not overwritten
    // while updating other application information
    uint8_t totalNoofReports = commSessionEntries[sessionEntryIndex].gpdAppInfo.totalNoofReports;
    ZGP_ReportDescriptor_t *reportDescriptor = commSessionEntries[sessionEntryIndex].gpdAppInfo.reportDescriptor; // array of data point descriptors
#endif
    memcpy(&commSessionEntries[sessionEntryIndex].gpdAppInfo, &appInfo, sizeof(ZGP_GpdAppInfo_t));
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
    commSessionEntries[sessionEntryIndex].gpdAppInfo.totalNoofReports = totalNoofReports;
    commSessionEntries[sessionEntryIndex].gpdAppInfo.reportDescriptor = reportDescriptor;
#endif
  }
  zgpFreeMemReqBuffer();
  return true;
}

/**************************************************************************//**
  \brief init. comm. session entry

  \param[in] sessionEntryIndex - session entry index
             ind - gpdf indication
             gpdId - gpd id
             responseToGpd - response to gpd

  \return None
******************************************************************************/
static void initCommSessionEntry(uint8_t sessionIndex, ZGP_LowDataInd_t *ind, ZGP_GpdId_t *gpdId, bool responseToGpd)
{
  commSessionEntries[sessionIndex].entryStatus = COMM_REQ_ENTRY;
  commSessionEntries[sessionIndex].appId = ind->applicationId;

  commSessionEntries[sessionIndex].endPoint = ind->endPoint;
  memcpy(&commSessionEntries[sessionIndex].gpdId, gpdId, sizeof(commSessionEntries[sessionIndex].gpdId));
  commSessionEntries[sessionIndex].validActionRxd = false;
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
  // will be initialized during processing
  commSessionEntries[sessionIndex].gpdAppInfo.totalNoofReports = 0x00;
  commSessionEntries[sessionIndex].gpdAppInfo.reportDescriptor = NULL;
  memset(&commSessionEntries[sessionIndex].rxdReportDescriptorMask[0], 0x00, sizeof(commSessionEntries[sessionIndex].rxdReportDescriptorMask));
#endif
  commSessionEntries[sessionIndex].isBidirectionalCommissioning = false;
  commSessionEntries[sessionIndex].sendInDirectMode = responseToGpd;
  memset(&commSessionEntries[sessionIndex].commReq, 0x00, sizeof(commSessionEntries[sessionIndex].commReq));
  memset(&commSessionEntries[sessionIndex].gpdAppInfo, 0x00, sizeof(commSessionEntries[sessionIndex].gpdAppInfo));
  commSessionEntries[sessionIndex].commState = COMM_IDLE;
}

/**************************************************************************//**
  \brief handle comm. reply

  \param[in] sessionEntryIndex - session entry index

  \return new sink comm state
******************************************************************************/
static sinkCommProcState_t handleCommReply(uint8_t sessionEntryIndex)
{
  sinkCommProcState_t commState = COMM_FRAME_AND_SEND_COMM_REPLY;
  zgpCommReq_t *commReq = &commSessionEntries[sessionEntryIndex].commReq;
  commSessionEntries[sessionEntryIndex].isKeyUpdated = true;

  if (!commSessionEntries[sessionEntryIndex].isBidirectionalCommissioning)
  {
    // Refer A.4.2.1.1.2 - GP Security Key request & panID request sub-fields shall be ignored
    //if rxAftertx is not set
    if (commReq->options.extOptionsField && commReq->extOptions.securityLevelCapabilities &&!ZGPL_IskeyValid(commReq->gpdKey))
      commState = COMM_COMPLETE;
    else
      commState = COMM_SUCCESSFUL_SESSION;

  }
  else
  {
    if (commReq->options.gpSecurityKeyRequest)
    {
        
      if((commReq->extOptions.keyType ==  zgpClusterServerAttributes.gpSharedSecurityKeyType.value) && 
        (memcmp(commReq->gpdKey, &zgpClusterServerAttributes.gpSharedSecuritykey.value[0], sizeof(ZGP_SECURITY_KEY_LENGTH)) == 0))
      {
        commSessionEntries[sessionEntryIndex].isKeyUpdated = false;
        memcpy(commSessionEntries[sessionEntryIndex].secKey ,commSessionEntries[sessionEntryIndex].commReq.gpdKey, \
                                       sizeof(commSessionEntries[sessionEntryIndex].secKey));
       return commState;
      }   
      ZGP_SecKeyType_t secType;

      commState = COMM_TX_GPD_KEY_ENCRYPTION;
      if (!checkForGpdKeyDerivation(commReq, &secType, sessionEntryIndex))
        commState = COMM_COMPLETE;
      else
        commReq->extOptions.keyType = secType;

      if ((ZGP_KEY_TYPE_NWKKEY_DERIVED_GROUP_KEY == commReq->extOptions.keyType) ||
           (ZGP_KEY_TYPE_DERIVED_INDIVIDUAL_ZGPD_KEY == commReq->extOptions.keyType))
        commState = COMM_GPD_KEY_DERIVATION;
    }
  }
  memcpy(commSessionEntries[sessionEntryIndex].secKey ,commSessionEntries[sessionEntryIndex].commReq.gpdKey, \
                                       sizeof(commSessionEntries[sessionEntryIndex].secKey));
  return commState;
}

/**************************************************************************//**
  \brief main commissionig handler

  \param[in] sessionEntryIndex - session entry index

  \return None
******************************************************************************/
static void commissioningHandler(uint8_t sessionEntryIndex)
{
  sinkCommProcState_t *commState = &commSessionEntries[sessionEntryIndex].commState;

  if (COMM_RXD_GPD_KEY_DECRYPTION == *commState)
  {
    *commState = commReqDecryptGpdKey(sessionEntryIndex);
  }
  if (COMM_WAITING_FOR_COMPLETE_COM_REQ_INFO == *commState)
  {
    *commState = checkForCommissioningCompletion(sessionEntryIndex);
  }
  if (COMM_PROCESS_COMPLETE_COMM_INFO == *commState)
  {
    *commState = handleCommReply(sessionEntryIndex);
  }
  if (COMM_GPD_KEY_DERIVATION == *commState)
  {
    // nothing to add waiting for key derivation to complete
    *commState = zgpGenericKeyDerivationHmac(sessionEntryIndex);
  }
  if (COMM_TX_GPD_KEY_ENCRYPTION == *commState)
  {
    // nothing to add waiting for tx key encryption
    *commState = encryptTxGpdKey(sessionEntryIndex);
  }
  if (COMM_FRAME_AND_SEND_COMM_REPLY == *commState)
  {
    *commState = frameAndSendCommissioningReply(sessionEntryIndex);
  }
  if (COMM_WAIT_FOR_SUCCCESS_IND == *commState)
  {
    // nothing to do.. waiting for GPD success cmd
  }
  if (COMM_SUCCESSFUL_SESSION == *commState)
  {
    *commState = commissioningCompletionhandling(sessionEntryIndex);
  }
  if (COMM_COMPLETE == *commState)
  {
    zgpServerFreeSessionEntry(sessionEntryIndex);
  }
  if(COMM_WAIT_FOR_PAIRING_CONFIG_TX == *commState)
  {
    //do nothing wait for timer expiry
  }
}

/**************************************************************************//**
  \brief checking for required info for pre-commissioned comm. mode during commissioning

  \param[in] ind - gpd indication
             gpdId - gpd id

  \return true if the required info is available
          false otherwise
******************************************************************************/
static bool checkForPrecommissinedCommModeInfo(ZGP_LowDataInd_t *ind, ZGP_GpdId_t *gpdId)
{
  ZGP_SinkTableEntry_t *sinkEntry = (ZGP_SinkTableEntry_t *)zgpGetMemReqBuffer();
  ZGP_TableOperationField_t  tableOperationField = {.entryType = ZGP_SINK_ENTRY, .commMode = ALL_COMMUNICATION_MODE, .appId = ind->applicationId, \
      .nonEmptyIndexForRead = ZGP_ENTRY_INVALID_INDEX};
  ZGP_ReadOperationStatus_t entryStatus;

  entryStatus = ZGPL_ReadTableEntryFromNvm((void *)sinkEntry, tableOperationField, gpdId, ind->endPoint);
  bool validEntryAvailable = false;

  if (ENTRY_NOT_AVAILABLE != entryStatus)
  {
    if (!ZGPL_SinkEntryIsPairingInfoEmpty(sinkEntry))
    {
      // atleast one group entry available
      validEntryAvailable = true;
    }
  }
  if (ZGP_NWK_ADDRESS_GROUP_INIT != zgpSinkGroupEntry.sinkGroup)
    validEntryAvailable = true;

  zgpFreeMemReqBuffer();
  // Existing entry doesn't have atleast one group entry and local sink group entry is also not valid
  return validEntryAvailable;
}

/**************************************************************************//**
  \brief process commissioning request command

  \param[in] ind - gpd indication
             responseToGpd - response to Gpd
             gpdId - gpd id
             proxyAddr - tempMaster addr

  \return None
******************************************************************************/
static void processCommissioningReqCmd(ZGP_LowDataInd_t *ind, bool responseToGpd, ZGP_GpdId_t *gpdId, uint16_t proxyAddr)
{
  zgpSessionIndex_t sessionIndex = {.entryIndex = ZGP_INVALID_SESSION_ENTRY_INDEX, .freeIndex = ZGP_INVALID_SESSION_ENTRY_INDEX};

  if ((ind->gpdAsduLength < ZGP_FRAME_COMM_REQ_BASIC_FRAME_LENGTH) || \
      ((ZGP_SRC_APPID == ind->applicationId)&& (ZGP_INVALID_SRC_ID == ind->srcId)) || \
      ((ZGP_IEEE_ADDR_APPID == ind->applicationId)&& (!ind->srcAddress.ext)))
  {
    return;
  }

  if (PRECOMMISSIONED_GROUPCAST == zgpClusterServerAttributes.gpsCommunicationMode.value)
  {
    if (!checkForPrecommissinedCommModeInfo(ind, gpdId))
    {
      return;
    }
  }

  zgpServerGetCommReqPairingConfigSessionTableIndex(ind->applicationId, gpdId, ind->endPoint, &sessionIndex);

  if (ZGP_INVALID_SESSION_ENTRY_INDEX == sessionIndex.entryIndex)
  {
    // entry is not available
    if (ZGP_INVALID_SESSION_ENTRY_INDEX == sessionIndex.freeIndex)
    {
      return;
    }
  }
  else if (COMM_REQ_ENTRY == sessionIndex.entryStatus)
  {
    if (commSessionEntries[sessionIndex.entryIndex].validActionRxd)
    {
      // Accpet only if the previously rxd comm. req is processed
      if ((COMM_WAITING_FOR_COMPLETE_COM_REQ_INFO != commSessionEntries[sessionIndex.entryIndex].commState) && \
          (COMM_WAIT_FOR_SUCCCESS_IND != commSessionEntries[sessionIndex.entryIndex].commState))
        return;
    }
  }
  else
  {
    // Entry is available but it is of PAIRING_CONFIG type
    return;
  }


  if (ZGP_INVALID_SESSION_ENTRY_INDEX == sessionIndex.entryIndex)
  {
    initCommSessionEntry(sessionIndex.freeIndex, ind, gpdId, responseToGpd);
    sessionIndex.entryIndex = sessionIndex.freeIndex;
  }

  // allocate entry from commissioning session
  if (!parseCommissioningReqFields(ind, sessionIndex.entryIndex))
  {
    return;
  }
 
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
  // total no. of reports in the received application description should be matching
  // the previosuly received(if received) application description's value.Otherwise drop the frame.
  if (commSessionEntries[sessionIndex.entryIndex].gpdAppInfo.totalNoofReports && \
      !commSessionEntries[sessionIndex.entryIndex].gpdAppInfo.appInfoOptions.appDescriptionCommandFollows)
  {
    zgpServerFreeSessionEntry(sessionIndex.entryIndex);
    return;
  }
#else
  if (commSessionEntries[sessionIndex.entryIndex].gpdAppInfo.appInfoOptions.appDescriptionCommandFollows)
  {
    zgpServerFreeSessionEntry(sessionIndex.entryIndex);
    return;
  }
#endif

  commSessionEntries[sessionIndex.entryIndex].tempMasterAddr = proxyAddr;

  if (!commSessionEntries[sessionIndex.entryIndex].gpdAppInfo.appInfoOptions.appDescriptionCommandFollows)
  {
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
    memset(&commSessionEntries[sessionIndex.entryIndex].rxdReportDescriptorMask[0], 0xFF, sizeof(commSessionEntries[sessionIndex.freeIndex].rxdReportDescriptorMask));
#endif
    commSessionEntries[sessionIndex.entryIndex].isBidirectionalCommissioning = ind->rxAfterTx;
  }

  if (commSessionEntries[sessionIndex.entryIndex].commReq.extOptions.gpdKeyPresent && \
      commSessionEntries[sessionIndex.entryIndex].commReq.extOptions.gpdKeyEncryption)
  {
    commSessionEntries[sessionIndex.entryIndex].commState = COMM_RXD_GPD_KEY_DECRYPTION;
  }
  else
  {
    commSessionEntries[sessionIndex.entryIndex].commState = COMM_WAITING_FOR_COMPLETE_COM_REQ_INFO;
    commSessionEntries[sessionIndex.entryIndex].validActionRxd = true;
  }

  commissioningHandler(sessionIndex.entryIndex);
}

/**************************************************************************//**
  \brief check for commissioning completion

  \param[in] sessionEntryIndex - session entry index

  \return next state after processing
******************************************************************************/
static sinkCommProcState_t checkForCommissioningCompletion(uint8_t sessionEntryIndex)
{
  bool packetsRxd = true;

#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
  packetsRxd = zgpCommSessionAllPacketsRceived(sessionEntryIndex);
#endif
  if (packetsRxd)
  {
    // check for application match
    ZGP_TransTableIndicationInfo_t transTableInfo;
    zgpCommReq_t *commReq = &commSessionEntries[sessionEntryIndex].commReq;
    uint8_t derivedDeviceId = commReq->gpdDeviceId;

    memset(&transTableInfo, 0x00, sizeof(transTableInfo));

    transTableInfo.transTableIndType = APP_FUNCTIONALITY_CHECK;

    transTableInfo.indicationData.appFuncCheckInfo.deviceId = &derivedDeviceId;
    transTableInfo.indicationData.appFuncCheckInfo.appInfo = &commSessionEntries[sessionEntryIndex].gpdAppInfo;
    transTableInfo.indicationData.appFuncCheckInfo.isMatching = true;
    transTableInfo.indicationData.appFuncCheckInfo.noOfEndPoints = 1;
    transTableInfo.indicationData.appFuncCheckInfo.endPointList = &sinkBasicModeInfo.commissioningGpdEp;
    SYS_PostEvent(BC_EVENT_ZGPH_TRANS_TABLE_INDICATION, (SYS_EventData_t)&transTableInfo);
    if (!transTableInfo.indicationData.appFuncCheckInfo.isMatching)
    {
      return COMM_COMPLETE;
    }

#if MICROCHIP_APPLICATION_SUPPORT == 1
    // if device id is updated for non-supported device id rxd & based on received cmd
    // then cmd already added so make noOfGpdCmds zero
    if ((ZGP_UNSPECIFIED_DEVICE_ID != derivedDeviceId) && (derivedDeviceId != commReq->gpdDeviceId))
    {
      commSessionEntries[sessionEntryIndex].gpdAppInfo.noOfGpdCmds = 0;
    }
    commReq->gpdDeviceId = derivedDeviceId;
#endif
    // process for commissioning reply
    return COMM_PROCESS_COMPLETE_COMM_INFO;
  }
  return COMM_WAITING_FOR_COMPLETE_COM_REQ_INFO;
}

#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
/**************************************************************************//**
  \brief Start/restart the pairingConfig timer

  \param[in] interval - timer interval
******************************************************************************/
static void startPairingConfigTimer(uint16_t interval)
{
  pairingConfigTimer.interval = interval;
  pairingConfigTimer.mode = TIMER_ONE_SHOT_MODE;
  pairingConfigTimer.callback = pairingConfigTimeoutCallback;
  HAL_StartAppTimer(&pairingConfigTimer);
}

/**************************************************************************//**
\brief Stop the pairingConfig timer
******************************************************************************/
static void stopPairingConfigTimer(void)
{
  pairingConfigTimer.interval = 0;
  HAL_StopAppTimer(&pairingConfigTimer);
}
/**************************************************************************//**
\brief pairingConfig timer callback
******************************************************************************/
static void pairingConfigTimeoutCallback(void)
{
    stopPairingConfigTimer();
    uint8_t pairingConfigTxStatus = ZGP_SUCCESS;
    gpPairingConfigPendingInfo.dstAddr.shortAddress = BROADCAST_ADDR_RX_ON_WHEN_IDLE;
    pairingConfigTxStatus = ZGPH_SendGpPairingConfigCmd(APS_SHORT_ADDRESS, &gpPairingConfigPendingInfo.dstAddr, gpPairingConfigPendingInfo.pairingConfigCmdInfo, &gpPairingConfigPendingInfo.pairingConfigAppInfo
                                                     ,gpPairingConfigPendingInfo.endPointInfo);
    if(ZGP_SUCCESS == pairingConfigTxStatus)
    {
      if(gpPairingConfigPendingInfo.pairingConfigAppInfo.noOfReportsPendingInGpPairingConfig == 0)
      {
        commSessionEntries[gpPairingConfigPendingInfo.commSessionEntryIndex].commState = COMM_COMPLETE;
        commissioningHandler(gpPairingConfigPendingInfo.commSessionEntryIndex);
      }
      else
        startPairingConfigTimer(1000);//start the timer again
    }
}
#endif
/**************************************************************************//**
  \brief handler after commissioning completion

  \param[in] sessionEntryIndex - session entry index

  \return next state after processing
******************************************************************************/
static sinkCommProcState_t commissioningCompletionhandling(uint8_t sessionEntryIndex)
{
  ZGP_SinkTableEntry_t *sinkEntry = zgpGetMemReqBuffer();
  zgpCommReq_t *commReq = &commSessionEntries[sessionEntryIndex].commReq;

  ZGP_ReadOperationStatus_t entryStatus;
 
  ZGP_TableOperationField_t  tableOperationField = {.entryType = ZGP_SINK_ENTRY, .commMode = ALL_COMMUNICATION_MODE, .appId = commSessionEntries[sessionEntryIndex].appId, \
                                                  .nonEmptyIndexForRead = ZGP_ENTRY_INVALID_INDEX};

  entryStatus = ZGPL_ReadTableEntryFromNvm((void *)sinkEntry, tableOperationField, &commSessionEntries[sessionEntryIndex].gpdId, commSessionEntries[sessionEntryIndex].endPoint);

  createSinkEntry(sinkEntry, sessionEntryIndex);

  if (!ZGPL_AddOrUpdateTableEntryOnNvm((void *)sinkEntry, UPDATE_ENTRY, ZGP_SINK_ENTRY))
  {
    zgpFreeMemReqBuffer();
    return COMM_COMPLETE;
  }
  commReq->appInfo = &commSessionEntries[sessionEntryIndex].gpdAppInfo;
  // This is for uni-directional commissioning so trigger success commissioning event from here
  triggerSuccessCommissioningEvent(sessionEntryIndex);
  if((ENTRY_NOT_AVAILABLE == entryStatus) || (commReq->gpdDeviceId != (uint8_t)ZGP_GENERIC_8_CONTACT_SWITCH))
  {
   zgpServerSendGpPairingForEntry(sinkEntry, EXTEND_SINKTABLE_ENTRY);
   zgpServerAddGroup(sinkEntry , true);
  }
  
  //to send GpPairingConfig only for PreCommissioned Groupcast
  if (PRECOMMISSIONED_GROUPCAST == sinkEntry->options.communicationMode)
  {
    ZGP_PairingConfigCmdInfo_t pairingConfigCmdInfo;
    ZGP_EndpointInfo_t endPointInfo;
    ZGP_GpdAppInfo_t *commReqAppInfo = commReq->appInfo;
    APS_Address_t dstAddr; 
    dstAddr.shortAddress = BROADCAST_ADDR_RX_ON_WHEN_IDLE;
    uint32_t srcId = sinkEntry->tableGenericInfo.gpdId.gpdSrcId;
    uint64_t IeeeAddr = sinkEntry->tableGenericInfo.gpdId.gpdIeeeAddr;

    memset(&pairingConfigCmdInfo, 0x00 , sizeof(ZGP_PairingConfigCmdInfo_t));
    memset(&gpPairingConfigPendingInfo, 0x00, sizeof(ZGP_GpPairingConfigPendingInfo_t));
    memset(&endPointInfo, 0x00, sizeof(ZGP_EndpointInfo_t));

    pairingConfigCmdInfo.appId = (ZGP_ApplicationId_t)sinkEntry->options.appId;
    if(pairingConfigCmdInfo.appId == ZGP_SRC_APPID)
      memcpy(&pairingConfigCmdInfo.gpdId, &srcId, ZGP_SRC_ID_LEN);
    else if(pairingConfigCmdInfo.appId  == ZGP_IEEE_ADDR_APPID)
      memcpy(&pairingConfigCmdInfo.gpdId, &IeeeAddr, ZGP_IEEE_ADDR_LEN);
    pairingConfigCmdInfo.gpdEndPoint = sinkEntry->tableGenericInfo.endPoint;
    pairingConfigCmdInfo.action.action = EXTEND_SINKTABLE_ENTRY;
    pairingConfigCmdInfo.action.sendGpPairing = false;
    pairingConfigCmdInfo.commMode = PRECOMMISSIONED_GROUPCAST;

    endPointInfo.noOfPairedEndPoints = PRECOMMISSIONED_PAIRED_ENDPOINT;
    gpPairingConfigPendingInfo.deviceId = sinkEntry->deviceId;
    zgpTransRemoveAndAddEventHandler(COMM_PROC_TYPE, sinkEntry, commReq->appInfo, 1, &sinkBasicModeInfo.commissioningGpdEp);
    zgpFreeMemReqBuffer();
    ZGPH_SendGpPairingConfigCmd(APS_SHORT_ADDRESS, &dstAddr, pairingConfigCmdInfo, commReqAppInfo, endPointInfo);
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT   
if(commSessionEntries[sessionEntryIndex].gpdAppInfo.appInfoOptions.appDescriptionCommandFollows)
{
   uint8_t status = ZGP_SUCCESS;
   pairingConfigCmdInfo.action.action = APPLICATION_DESCRIPTION;
   gpPairingConfigPendingInfo.pairingConfigAppInfo.totalNoofReports = commReqAppInfo->totalNoofReports;
   gpPairingConfigPendingInfo.pairingConfigAppInfo.noOfReportsPendingInGpPairingConfig = commReqAppInfo->totalNoofReports;
   status = ZGPH_SendGpPairingConfigCmd(APS_SHORT_ADDRESS, &dstAddr, pairingConfigCmdInfo, commReqAppInfo, endPointInfo);
   
   if(status == ZGP_SUCCESS)
   {
     if(gpPairingConfigPendingInfo.pairingConfigAppInfo.noOfReportsPendingInGpPairingConfig != 0)
     {
       gpPairingConfigPendingInfo.commSessionEntryIndex = sessionEntryIndex;
       startPairingConfigTimer(ZGP_APPLICATION_DESCR_PAIRING_CONFIG_TIMEOUT);
       return COMM_WAIT_FOR_PAIRING_CONFIG_TX;
     }
   }
}
#endif
 }
  
if(PRECOMMISSIONED_GROUPCAST != sinkEntry->options.communicationMode)
{
  zgpFreeMemReqBuffer();
  zgpTransRemoveAndAddEventHandler(COMM_PROC_TYPE, sinkEntry, commReq->appInfo, 1, &sinkBasicModeInfo.commissioningGpdEp);
}
  // Pairing is done so disabling commissioning mode
  sinkBasicCheckCommMode();
  return COMM_COMPLETE;
}

/**************************************************************************//**
  \brief create the entry after successful commissioning

  \param[in] sink entry - entry buffer
             sessionEntryIndex - session entry index

  \return none
******************************************************************************/
static void createSinkEntry(ZGP_SinkTableEntry_t *sinkEntry, uint8_t sessionEntryIndex)
{
  zgpCommReq_t *commReq = &commSessionEntries[sessionEntryIndex].commReq;

  ZGPL_ResetTableEntry((void *)sinkEntry, ZGP_SINK_ENTRY);
  sinkEntry->options.appId = commSessionEntries[sessionEntryIndex].appId;
  sinkEntry->options.communicationMode = zgpClusterServerAttributes.gpsCommunicationMode.value;

  memcpy(&sinkEntry->tableGenericInfo.gpdId, &commSessionEntries[sessionEntryIndex].gpdId, sizeof(ZGP_GpdId_t));
  if (PRECOMMISSIONED_GROUPCAST == sinkEntry->options.communicationMode)
  {
    ZGPL_AddSinkGroupEntry(sinkEntry, &zgpSinkGroupEntry);
  }
  sinkEntry->options.assignedAlias = false;


  sinkEntry->tableGenericInfo.endPoint = commSessionEntries[sessionEntryIndex].endPoint;
  sinkEntry->tableGenericInfo.gpdAssignedAlias = 0x0;
  sinkEntry->options.seqNumCapabilities = commReq->options.macSeqNoCapability;
  sinkEntry->options.fixedLocation = commReq->options.fixedLocation;
  sinkEntry->options.rxOnCapability = commReq->options.rxOnCapability;
  sinkEntry->deviceId = commReq->gpdDeviceId;
  sinkEntry->tableGenericInfo.gpdSecurityFrameCounter = commReq->gpdOutgoingCounter;//sinkBasicGpdfProcessing.sinkGpdfRsp.gpdOutgoingFrameCounter;
  sinkEntry->tableGenericInfo.groupCastRadius = 0; // Need to analyze
  sinkEntry->tableGenericInfo.securityOptions.securityLevel = commReq->extOptions.securityLevelCapabilities;
  sinkEntry->options.securityUse = false;
  if (commReq->extOptions.securityLevelCapabilities)
    sinkEntry->options.securityUse = true;
  sinkEntry->tableGenericInfo.securityOptions.securityKeyType = commReq->extOptions.keyType;
  memcpy(&sinkEntry->tableGenericInfo.securityKey[0], &commSessionEntries[sessionEntryIndex].secKey[0], sizeof(sinkEntry->tableGenericInfo.securityKey));
}

/**************************************************************************//**
  \brief derive the key of key is requested in commissioning req.

  \param[in] commReq - comm. req
             derivedKeyType - derived key type
             sessionIndex - entry index

  \return true if valid key type
          false otherwise
******************************************************************************/
static bool checkForGpdKeyDerivation(zgpCommReq_t *commReq, ZGP_SecKeyType_t *derivedKeyType, uint8_t sessionIndex)
{
  *derivedKeyType = zgpServerGetSharedSecurityKeyType();

  switch (*derivedKeyType)
  {
    case ZGP_KEY_TYPE_NWK_KEY:
      memcpy(commReq->gpdKey, NWK_GetActiveKey(), ZGP_SECURITY_KEY_LENGTH);
      return true;
    case ZGP_KEY_TYPE_ZGPD_GROUP_KEY:
      memcpy(commReq->gpdKey, zgpServerGetSharedSecurityKey(), ZGP_SECURITY_KEY_LENGTH);
      return true;
    case ZGP_KEY_TYPE_NWKKEY_DERIVED_GROUP_KEY:
      memcpy(commReq->gpdKey, NWK_GetActiveKey(), sizeof(commReq->gpdKey));
      return true;
    case ZGP_KEY_TYPE_DERIVED_INDIVIDUAL_ZGPD_KEY:
      memcpy(commReq->gpdKey, zgpServerGetSharedSecurityKey(), sizeof(commReq->gpdKey));
      return true;
    case ZGP_KEY_TYPE_OOB_ZGPD_KEY:
    case ZGP_KEY_TYPE_NO_KEY:
    case ZGP_KEY_TYPE_RESERVED1:
    case ZGP_KEY_TYPE_RESERVED2:
      if (ZGP_KEY_TYPE_OOB_ZGPD_KEY != commReq->extOptions.keyType)
        return false;
      *derivedKeyType = ZGP_KEY_TYPE_OOB_ZGPD_KEY;
      // Sending the key as part comm. reply is not required if key type is OOB even it is requested
      // making keyRequest flag false
      //commReq->options.gpSecurityKeyRequest = false;
      return true;
    default:
       return false;
  }
}

/**************************************************************************//**
  \brief frame and send the commissioning reply

  \param[in] sessionIndex - entry index

  \return next state after processing
******************************************************************************/
static sinkCommProcState_t frameAndSendCommissioningReply(uint8_t sessionEntryIndex)
{
  // frame commissioning reply fields and call sendGpResponse
  uint8_t commReplyIndex = 0;
  uint8_t options = 0x00;
  gpdfRsp_t gpdfResponse;
  uint8_t gpdfRspPayload[sizeof(zgpCommReply_t)];
  zgpCommReq_t *commReq = &commSessionEntries[sessionEntryIndex].commReq;
  bool secKeyPresent = commReq->options.gpSecurityKeyRequest;
  
  ZGP_SecKeyType_t gpsKeyType = zgpServerGetSharedSecurityKeyType();

  gpdfResponse.cmdPayload = &gpdfRspPayload[0];
  gpdfResponse.cmdId = ZGP_COMMISSIONING_REPLY_CMD_ID;

  if (ZGP_KEY_TYPE_OOB_ZGPD_KEY == commReq->extOptions.keyType || !commSessionEntries[sessionEntryIndex].isKeyUpdated)
    secKeyPresent = false;
  
  options = commReq->options.panidRequest | (secKeyPresent << 1) | \
            (secKeyPresent << 2) | \
            (commReq->extOptions.securityLevelCapabilities << 3) | \
            (commReq->extOptions.keyType << 5);
   
  gpdfResponse.cmdPayload[commReplyIndex] = options;
  commReplyIndex++;

  if (commReq->options.panidRequest)
  {
    uint16_t panId = NWK_GetPanId();

    memcpy(&gpdfResponse.cmdPayload[commReplyIndex], &panId, sizeof(PanId_t));
    commReplyIndex += sizeof(PanId_t);
  }

  if (secKeyPresent)
  {
    uint32_t securityFrameCounter = commReq->gpdOutgoingCounter + 1;

    memcpy(&gpdfResponse.cmdPayload[commReplyIndex], &commReq->gpdKey[0], sizeof(commReq->gpdKey));
    commReplyIndex += sizeof(commReq->gpdKey);

    memcpy(&gpdfResponse.cmdPayload[commReplyIndex], &commReq->gpdKeyMic, sizeof(commReq->gpdKeyMic));
    commReplyIndex += sizeof(commReq->gpdKeyMic);

    memcpy(&gpdfResponse.cmdPayload[commReplyIndex], &securityFrameCounter, sizeof(commReq->gpdOutgoingCounter));
    commReplyIndex += sizeof(commReq->gpdOutgoingCounter);
  }


  gpdfResponse.appId = commSessionEntries[sessionEntryIndex].appId;
  memcpy(&gpdfResponse.gpdId, &commSessionEntries[sessionEntryIndex].gpdId, sizeof(gpdfResponse.gpdId));
  gpdfResponse.endPoint = commSessionEntries[sessionEntryIndex].endPoint;
  gpdfResponse.frameType = ZGP_FRAME_DATA;
  gpdfResponse.cmdPayloadLength = commReplyIndex;
  gpdfResponse.tempMasterTxChannel = NWK_GetLogicalChannel() - ZGP_CHANNEL_OFFSET;
  gpdfResponse.tempMasterAddr = commSessionEntries[sessionEntryIndex].tempMasterAddr;
  gpdfResponse.sendInDirectMode = commSessionEntries[sessionEntryIndex].sendInDirectMode;

  sendGpdfResponse(&gpdfResponse);
  return COMM_WAIT_FOR_SUCCCESS_IND;
}
/**************************************************************************//**
  \brief initial filtering of GPDF based on sink mode

  \param[in] ind - dstub data indication

  \return true - accept the frame
          false - drop the frame
******************************************************************************/
static bool filterRxGpdf(ZGP_LowDataInd_t *ind)
{
  if(!zgpGenericNotInTransmitChannnel())
    return false;

  if (ind->gpdfSecurityLevel && (ind->status == ZGP_DSTUB_AUTH_FAILURE))
    return false;

  if (COMMISSIONING_MODE == ZGPL_GetDeviceMode(false))
  {
    if (ZGP_COMMISSIONING_REPLY_CMD_ID <= ind->gpdCommandId)
      return false;
    else if (ZGP_COMMISSIONING_CMD_ID > ind->gpdCommandId)
    {
      // for data GPDF
      if (ind->autoCommissioning && ind->rxAfterTx)
        return false;
    }
    else if (ZGP_CHANNEL_REQUEST_CMD_ID != ind->gpdCommandId)
    {
       // for commissioning frames
      if (ind->autoCommissioning)
         return false;
    }
  }
  else
  {
    if ((ZGP_COMMISSIONING_CMD_ID <= ind->gpdCommandId) && \
         (ZGP_DECOMMISSIONING_CMD_ID != ind->gpdCommandId))
      return false;
  }
  return true;
}
/**************************************************************************//**
  \brief Decrypting gpd key in comm. request

  \param[in] sessionIndex - entry index

  \return next state after processing
******************************************************************************/
static sinkCommProcState_t commReqDecryptGpdKey(uint8_t sessionIndex)
{
  uint8_t key[ZGP_SECURITY_KEY_LENGTH] = CS_ZGP_SECURITY_LINK_KEY;
  uint8_t index = 0;
  zgpCommReq_t *commReq = &commSessionEntries[sessionIndex].commReq;
  gpdfSecProcContextData_t *secProcContextData;

  // Need to allocate sec. proc. entry
  secProcContextData = getSecProcContextDataBuffer(SEC_KEY_DECRYPTION);
  if (!secProcContextData)
  {
    return COMM_COMPLETE;
  }
  // need to decrypt the key
  secProcContextData->secProcData.secReq.zgpSinkDecryptReq.pdu = secProcContextData->secProcData.keySecProcessing.securedKeyPayload;
  secProcContextData->secProcData.secReq.zgpSinkDecryptReq.key = secProcContextData->secProcData.keySecProcessing.key;
  memcpy((void *)secProcContextData->secProcData.secReq.zgpSinkDecryptReq.key, key, ZGP_SECURITY_KEY_LENGTH);

  secProcContextData->secProcData.keySecProcessing.sessionIndex =  sessionIndex;
  secProcContextData->secProcData.secReq.zgpSinkDecryptReq.dir = ZGP_TX_BY_ZGPD;
  secProcContextData->secProcData.secReq.zgpSinkDecryptReq.appId = commSessionEntries[sessionIndex].appId;
  secProcContextData->secProcData.secReq.zgpSinkDecryptReq.securityLevel = ZGP_SECURITY_LEVEL_3;

  if (ZGP_SRC_APPID == commSessionEntries[sessionIndex].appId)
  {
    secProcContextData->secProcData.secReq.zgpSinkDecryptReq.srcID = commSessionEntries[sessionIndex].gpdId.gpdSrcId;
    memcpy(&secProcContextData->secProcData.secReq.zgpSinkDecryptReq.pdu[index], (void *)&secProcContextData->secProcData.secReq.zgpSinkDecryptReq.srcID, ZGP_COMM_KEY_HEADER_LEN);
    secProcContextData->secProcData.secReq.zgpSinkDecryptReq.securityFrameCounter = commSessionEntries[sessionIndex].gpdId.gpdSrcId;
  }
  else
  {
    secProcContextData->secProcData.secReq.zgpSinkDecryptReq.extAddr = commSessionEntries[sessionIndex].gpdId.gpdIeeeAddr;
    memcpy(&secProcContextData->secProcData.secReq.zgpSinkDecryptReq.pdu[index], (void *)&secProcContextData->secProcData.secReq.zgpSinkDecryptReq.extAddr, ZGP_COMM_KEY_HEADER_LEN);
    secProcContextData->secProcData.secReq.zgpSinkDecryptReq.securityFrameCounter = (uint32_t)secProcContextData->secProcData.secReq.zgpSinkDecryptReq.extAddr;
  }

  index += ZGP_COMM_KEY_HEADER_LEN;
  secProcContextData->secProcData.secReq.zgpSinkDecryptReq.headerLength = ZGP_COMM_KEY_HEADER_LEN;
  secProcContextData->secProcData.secReq.zgpSinkDecryptReq.payloadLength = ZGP_SEC_FOURBYTE_MIC + \
                                                                            ZGP_SECURITY_KEY_LENGTH;
  memcpy(&secProcContextData->secProcData.secReq.zgpSinkDecryptReq.pdu[index], commReq->gpdKey, \
                                               ZGP_SECURITY_KEY_LENGTH);

  index += ZGP_SECURITY_KEY_LENGTH;
  memcpy(&secProcContextData->secProcData.secReq.zgpSinkDecryptReq.pdu[index], (void *)&commReq->gpdKeyMic, \
                                               ZGP_SEC_FOURBYTE_MIC);

  secProcContextData->secProcData.secReq.zgpSinkDecryptReq.SSP_ZgpDecryptFrameConf = commReqDecryptGpdKeyconfirm;
  SSP_ZgpDecryptFrameReq(&secProcContextData->secProcData.secReq.zgpSinkDecryptReq);

  return COMM_RXD_GPD_KEY_DECRYPTION;
}

#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
/**************************************************************************//**
  \brief process app. description command

  \param[in] ind - dstub data indication
             responseToGpd - response to gpd
             gpdId - gpd id
             proxyAddr = tempMaster addr

  \return None
******************************************************************************/
static void processAppDescriptionCmd(ZGP_LowDataInd_t *ind, bool responseToGpd, ZGP_GpdId_t *gpdId, uint16_t proxyAddr)
{
  zgpSessionIndex_t sessionIndex = {.entryIndex = ZGP_INVALID_SESSION_ENTRY_INDEX, .freeIndex = ZGP_INVALID_SESSION_ENTRY_INDEX};
  zgpRxdReportInfo_t rxdReportInfo;

  memset(&rxdReportInfo, 0x00, sizeof(rxdReportInfo));
  if (((ZGP_SRC_APPID == ind->applicationId)&& (ZGP_INVALID_SRC_ID == ind->srcId)) || \
      ((ZGP_IEEE_ADDR_APPID == ind->applicationId)&& (!ind->srcAddress.ext)))
  {
    return;
  }

  memset(&rxdReportInfo, 0x00, sizeof(rxdReportInfo));
  zgpServerGetCommReqPairingConfigSessionTableIndex(ind->applicationId, gpdId, ind->endPoint, &sessionIndex);

  if (ZGP_INVALID_SESSION_ENTRY_INDEX == sessionIndex.entryIndex)
  {
    // entry is not available
    if (ZGP_INVALID_SESSION_ENTRY_INDEX == sessionIndex.freeIndex)
    {
      return;
    }
  }

  if (ZGP_REPORT_PARSING_SUCCESS != (uint8_t)zgpParseReportDescriptorFields(&rxdReportInfo, ind->gpdAsdu, 0x00,\
                                             ind->gpdAsduLength, sessionIndex.entryIndex))
  {
    return;
  }

  if (ZGP_INVALID_SESSION_ENTRY_INDEX == sessionIndex.entryIndex)
  {
    initCommSessionEntry(sessionIndex.freeIndex, ind, gpdId, responseToGpd);
    sessionIndex.entryIndex = sessionIndex.freeIndex;
  }
  else if (COMM_REQ_ENTRY != sessionIndex.entryStatus)
  {
    return;
  }

  // Check whether this is the last application description by looking at the report index
  if (commSessionEntries[sessionIndex.entryIndex].gpdAppInfo.totalNoofReports && \
      (commSessionEntries[sessionIndex.entryIndex].gpdAppInfo.totalNoofReports != rxdReportInfo.totalNoOfReports))
  {
    zgpServerFreeReportDataDescBufer(rxdReportInfo.rxdReportDescriptors);
    return;
  }

  if ((1 << ((rxdReportInfo.totalNoOfReports - 1) % 8)) & rxdReportInfo.rxdReportDescriptorMask[(rxdReportInfo.totalNoOfReports-1) / 8])
  {
    // This is the last app. description command
    // so check rxAfterTx for unidirectional/bidirectional commissioning
    commSessionEntries[sessionIndex.entryIndex].isBidirectionalCommissioning = ind->rxAfterTx;
  }

  zgpServerProcessAppDescCommand(&rxdReportInfo, sessionIndex.entryIndex);

  if (COMM_RXD_GPD_KEY_DECRYPTION != commSessionEntries[sessionIndex.entryIndex].commState)
  {
    // If key decryption is in progress, then wait for decryption to complete
    // On completion of decryption, state changed to COMM_WAITING_FOR_COMPLETE_COM_REQ_INFO by default
    // If key decryption is not in progress, change the state here
    commSessionEntries[sessionIndex.entryIndex].commState = COMM_WAITING_FOR_COMPLETE_COM_REQ_INFO;
    commSessionEntries[sessionIndex.entryIndex].tempMasterAddr = proxyAddr;

    commissioningHandler(sessionIndex.entryIndex);
  }
}
#endif

/**************************************************************************//**
  \brief Parsing the received GPDF

  \param[in] ind - pointer to dstub data indication
             responseToGpd - response to gpd
             proxyAddr - temp addr

  \return true - continue with gpdf processing
          false - not proceeding with gpdf processing
******************************************************************************/
static bool rxGpdfParsing(ZGP_LowDataInd_t *ind, bool responseToGpd, uint16_t proxyAddr)
{
  ZGP_GpdId_t gpdId;

  gpdId.gpdIeeeAddr = ind->srcAddress.ext;
  if (ZGP_SRC_APPID == ind->applicationId)
    gpdId.gpdSrcId = ind->srcId;

  switch (ind->gpdCommandId)
  {
    case ZGP_COMMISSIONING_CMD_ID:
    {
      processCommissioningReqCmd(ind, responseToGpd, &gpdId, proxyAddr);
    }
    break;
    case ZGP_DECOMMISSIONING_CMD_ID:
    {
      processDecommissioningCmd(ind, &gpdId);
    }
    break;
    case ZGP_COMMISSIONING_SUCCESS_CMD_ID:
    {
      processCommissioningSuccessReqCmd(ind, &gpdId);
    }
    break;
    case ZGP_CHANNEL_REQUEST_CMD_ID:
    {
      processChannelReqCmd(ind, responseToGpd, proxyAddr);
    }
    break;
    case ZGP_APPLICATION_DESCRIPTION_CMD_ID:
    {
#ifdef ZGP_ENABLE_MULTI_SENSOR_SUPPORT
      processAppDescriptionCmd(ind, responseToGpd, &gpdId, proxyAddr);
#endif
    }
    break;
    case ZGP_COMMISSIONING_REPLY_CMD_ID:
    case ZGP_CHANNEL_CONFIG_CMD_ID:
    break;
    default: //ZGP_ON_CMD_ID & ZGP_OFF_CMD_ID GPDF cluster command Ids
    {
      processGpdCommand(ind, &gpdId);
    }
    break;
  }
  return true;
}

/**************************************************************************//**
  \brief Sending the response in direct and tunneling mode

  \param[in] gpdfRsp - response information

  \return None
******************************************************************************/
static void sendGpdfResponse(gpdfRsp_t *gpdfRsp)
{
  ZCL_Addressing_t dstAddr;

  memset(&dstAddr, 0, sizeof(ZCL_Addressing_t));

  // Handling
  if (gpdfRsp->sendInDirectMode)
  {
    // Queue the response by calling dStubDataRequest
    sendGpdfInDirectMode(gpdfRsp);
  }

  {
    // Buffer allocation for gpResponse i.e. uint8_t gpResponse[sizeof(ZCL_GpResp_t)];
    uint8_t *gpResponse = (uint8_t *)zgpGetMemReqBuffer();
    uint8_t *zclPayload = NULL;

    uint8_t payloadLength = 0;

    {
      zgpGpRespOptions_t options;

      options.appId  = gpdfRsp->appId;
      options.txOnEndpointMatch = 1;
      options.reserved = 0;

      zclPayload = &gpResponse[0];
      memcpy(&gpResponse[payloadLength++], (void *)&options, sizeof(options));
      memcpy(&gpResponse[payloadLength], (void *)&gpdfRsp->tempMasterAddr, sizeof(gpdfRsp->tempMasterAddr));
      payloadLength += sizeof(gpdfRsp->tempMasterAddr);
      gpResponse[payloadLength++] = gpdfRsp->tempMasterTxChannel;
      if (ZGP_SRC_APPID == gpdfRsp->appId)
      {
        uint32_t srcId = gpdfRsp->gpdId.gpdSrcId;

        memcpy(&gpResponse[payloadLength], (void *)&srcId, sizeof(gpdfRsp->gpdId.gpdSrcId));
        payloadLength += sizeof(gpdfRsp->gpdId.gpdSrcId);
      }
      else
      {
        ExtAddr_t extAddr = gpdfRsp->gpdId.gpdIeeeAddr;

        memcpy(&gpResponse[payloadLength], (void *)&extAddr, sizeof(gpdfRsp->gpdId.gpdIeeeAddr));
        payloadLength += sizeof(gpdfRsp->gpdId.gpdIeeeAddr);
        gpResponse[payloadLength++] = gpdfRsp->endPoint;
      }
      gpResponse[payloadLength++] = gpdfRsp->cmdId;
      gpResponse[payloadLength++] = gpdfRsp->cmdPayloadLength;
      if (gpdfRsp->cmdPayloadLength)
      {
        memcpy(&gpResponse[payloadLength], gpdfRsp->cmdPayload, gpdfRsp->cmdPayloadLength);
        payloadLength += gpdfRsp->cmdPayloadLength;
      }

      dstAddr.addr.shortAddress = BROADCAST_ADDR_RX_ON_WHEN_IDLE;
      dstAddr.addrMode = APS_SHORT_ADDRESS;
      dstAddr.aliasSrcAddr = NWK_NO_SHORT_ADDR;
      // valid payload is available
      ZGPH_SendCmdInRawMode(&dstAddr, ZCL_CLUSTER_SIDE_CLIENT, ZCL_GP_CLUSTER_CLIENT_GP_RESPONSE_COMMAND_ID, payloadLength, zclPayload);
    }
    zgpFreeMemReqBuffer();
  }
}
/**************************************************************************//**
  \brief Decrypting the received GPDF

  \param[in] secContextData - sec. context data buffer
             dstubInd - gpdf indication

  \return true - continue with gpdf processing
          false - not proceeding with gpdf processing
******************************************************************************/
static bool decryptRxGpdf(gpdfSecProcContextData_t *secContextData, ZGP_LowDataInd_t *dstubInd)
{
  ZGP_SinkTableEntry_t *sinkTableEntry = (ZGP_SinkTableEntry_t *)zgpGetMemReqBuffer();
  ZGP_GpdId_t gpdId;
  ZGP_ReadOperationStatus_t entryStatus;
  ZGP_TableOperationField_t  tableOperationField = {.entryType = ZGP_SINK_ENTRY, .commMode = ALL_COMMUNICATION_MODE, .appId = dstubInd->applicationId, \
  .nonEmptyIndexForRead = ZGP_ENTRY_INVALID_INDEX};
  uint8_t securityKey[ZGP_SECURITY_KEY_LENGTH];
  ZGP_SecLevel_t secLevel;
  ZGP_SecKeyType_t secKeyType;

  gpdId.gpdIeeeAddr = dstubInd->srcAddress.ext;

  if (ZGP_SRC_APPID == dstubInd->applicationId)
    gpdId.gpdSrcId = dstubInd->srcId;

  entryStatus = ZGPL_ReadTableEntryFromNvm((void *)sinkTableEntry, tableOperationField, &gpdId, dstubInd->endPoint);

  if (ENTRY_NOT_AVAILABLE == entryStatus)
  {
    zgpSessionIndex_t sessionIndex = {.entryIndex = ZGP_INVALID_SESSION_ENTRY_INDEX, .freeIndex = ZGP_INVALID_SESSION_ENTRY_INDEX};

    // Check in the commissioning session - sink entry for incomplete pairing
    zgpServerGetCommReqPairingConfigSessionTableIndex(dstubInd->applicationId, &gpdId, dstubInd->endPoint, &sessionIndex);
    if ((ZGP_INVALID_SESSION_ENTRY_INDEX == sessionIndex.entryIndex) || \
         (COMM_REQ_ENTRY != sessionIndex.entryStatus))
    {
      zgpFreeMemReqBuffer();
      return false;
    }
    memcpy(securityKey, commSessionEntries[sessionIndex.entryIndex].secKey, sizeof(securityKey));
    secLevel = (ZGP_SecLevel_t)commSessionEntries[sessionIndex.entryIndex].commReq.extOptions.securityLevelCapabilities;
    secKeyType = (ZGP_SecKeyType_t)commSessionEntries[sessionIndex.entryIndex].commReq.extOptions.keyType;
  }
  else
  {
    memcpy(securityKey, sinkTableEntry->tableGenericInfo.securityKey, sizeof(securityKey));
    secLevel = (ZGP_SecLevel_t)sinkTableEntry->tableGenericInfo.securityOptions.securityLevel;
    secKeyType = (ZGP_SecKeyType_t)sinkTableEntry->tableGenericInfo.securityOptions.securityKeyType;
  }
  zgpFreeMemReqBuffer();

  {
    uint8_t gpdfHeaderLength = ZGP_MAX_HEADER_LENGTH; // NWK FC(1) + ext FC(1) + srcID(4) + securityFrameCunter
    ZGP_NwkFrameControl_t nwkFrameControl;
    ZGP_ExtNwkFrameControl_t nwkExtFrameControl;
    uint8_t index = 0;

    // If sink table entry is not available, then device is not commissioned and
    // sending secured packet so we can drop the frame -- TBD else part
    if ((uint8_t)dstubInd->gpdfSecurityLevel != secLevel)
    {
      return false;
    }

    nwkFrameControl.frametype = dstubInd->frameType;
    nwkFrameControl.isExtNwkFcf = 1;
    nwkFrameControl.zigbeeProtVersion = GREENPOWER_PROTOCOL_VERSION;
    nwkFrameControl.autoCommissioning = dstubInd->autoCommissioning;

    nwkExtFrameControl.appId = dstubInd->applicationId;
    nwkExtFrameControl.direction = ZGP_TX_BY_ZGPD;
    nwkExtFrameControl.rxAfterTx = dstubInd->rxAfterTx;
    nwkExtFrameControl.secKey = (ZGP_SecKey_t)0;
    if (secKeyType >= ZGP_KEY_TYPE_OOB_ZGPD_KEY)
      nwkExtFrameControl.secKey = (ZGP_SecKey_t)1;
    nwkExtFrameControl.secLevel = (ZGP_SecLevel_t)(dstubInd->gpdfSecurityLevel);

    secContextData->secProcData.secReq.zgpSinkDecryptReq.pdu = &secContextData->secProcData.gpdfSecProcessing.dataBuffer[0];
    secContextData->secProcData.secReq.zgpSinkDecryptReq.key = &secContextData->secProcData.gpdfSecProcessing.key[0];

    memcpy(&secContextData->secProcData.secReq.zgpSinkDecryptReq.pdu[index++], (void *)&nwkFrameControl, sizeof(nwkFrameControl));
    memcpy(&secContextData->secProcData.secReq.zgpSinkDecryptReq.pdu[index++], (void *)&nwkExtFrameControl, sizeof(nwkExtFrameControl));

    memcpy((void *)secContextData->secProcData.secReq.zgpSinkDecryptReq.key, securityKey, ZGP_SECURITY_KEY_LENGTH);
    secContextData->secProcData.secReq.zgpSinkDecryptReq.securityFrameCounter = dstubInd->gpdSecurityFrameCounter;
    secContextData->secProcData.secReq.zgpSinkDecryptReq.securityLevel = (uint8_t)(dstubInd->gpdfSecurityLevel);
    secContextData->secProcData.secReq.zgpSinkDecryptReq.appId = dstubInd->applicationId;
    secContextData->secProcData.secReq.zgpSinkDecryptReq.dir = ZGP_TX_BY_ZGPD;
    secContextData->secProcData.secReq.zgpSinkDecryptReq.extAddr = 0x00;
    if (ZGP_IEEE_ADDR_APPID == dstubInd->applicationId)
    {
      secContextData->secProcData.secReq.zgpSinkDecryptReq.extAddr = dstubInd->srcAddress.ext;
      gpdfHeaderLength = ZGP_MAX_HEADER_LENGTH_FOR_IEEE_APPID;
      secContextData->secProcData.secReq.zgpSinkDecryptReq.pdu[index++] = dstubInd->endPoint;
    }
    else
   {
      memcpy(&secContextData->secProcData.secReq.zgpSinkDecryptReq.pdu[index], (void *)&dstubInd->srcId, sizeof(dstubInd->srcId));
      index += sizeof(dstubInd->srcId);
      secContextData->secProcData.secReq.zgpSinkDecryptReq.srcID = dstubInd->srcId;
    }

    memcpy(&secContextData->secProcData.secReq.zgpSinkDecryptReq.pdu[index], (void *)&dstubInd->gpdSecurityFrameCounter, sizeof(dstubInd->gpdSecurityFrameCounter));
    index += sizeof(dstubInd->gpdSecurityFrameCounter);

    secContextData->secProcData.secReq.zgpSinkDecryptReq.macHeader[0] = 0x00;
    secContextData->secProcData.secReq.zgpSinkDecryptReq.macHeader[1] = 0x00;
    secContextData->secProcData.secReq.zgpSinkDecryptReq.macHeader[2] = 0x00;
    secContextData->secProcData.secReq.zgpSinkDecryptReq.macHeader[3] = 0x00;
    secContextData->secProcData.secReq.zgpSinkDecryptReq.headerLength = gpdfHeaderLength;

    secContextData->secProcData.secReq.zgpSinkDecryptReq.pdu[index++] = dstubInd->gpdCommandId;
    memcpy(&secContextData->secProcData.secReq.zgpSinkDecryptReq.pdu[index], dstubInd->gpdAsdu, dstubInd->gpdAsduLength);
    index += dstubInd->gpdAsduLength;

    memcpy(&secContextData->secProcData.secReq.zgpSinkDecryptReq.pdu[index], (void *)&dstubInd->mic, sizeof(dstubInd->mic));
    index += sizeof(dstubInd->mic);

    secContextData->secProcData.secReq.zgpSinkDecryptReq.payloadLength = index - gpdfHeaderLength;
    secContextData->secProcData.secReq.zgpSinkDecryptReq.SSP_ZgpDecryptFrameConf = decryptRxGpdfConfirm;
    SSP_ZgpDecryptFrameReq(&secContextData->secProcData.secReq.zgpSinkDecryptReq);
  }

  return true;
}

/**************************************************************************//**
  \brief Trigger GPDF command received event

  \param[in] gpdId - gpd id info
             ind - dstub data indication

  \return None
******************************************************************************/
static void triggerGpdfCommandRxdEvent(ZGP_GpdId_t *gpdId, ZGP_LowDataInd_t *ind)
{
  ZGP_IndicationInfo_t *indicationInfo = (ZGP_IndicationInfo_t *)zgpGetMemReqBuffer();

  memset(indicationInfo, 0x00, sizeof(ZGP_IndicationInfo_t));
  indicationInfo->indicationType = GPD_COMMAND_RECEIVED;
  memcpy((void *)&indicationInfo->indicationData.gpdCommand.gpdId, (void *)gpdId, \
          sizeof(indicationInfo->indicationData.gpdCommand.gpdId));
  indicationInfo->indicationData.gpdCommand.appId = ind->applicationId;
  indicationInfo->indicationData.gpdCommand.endPoint = ind->endPoint;
  indicationInfo->indicationData.gpdCommand.cmdId = ind->gpdCommandId;
  indicationInfo->indicationData.gpdCommand.cmdPayloadLength = ind->gpdAsduLength;
  indicationInfo->indicationData.gpdCommand.cmdPayload = ind->gpdAsdu;

  SYS_PostEvent(BC_EVENT_ZGPH_INDICATION, (SYS_EventData_t)indicationInfo);
  zgpFreeMemReqBuffer();
}

/**************************************************************************//**
  \brief Decryption confirmation for incoming gpdf

  \param[in] conf - confirmation callback of decryption

  \return None
******************************************************************************/
static void decryptRxGpdfConfirm(SSP_ZgpDecryptFrameConf_t *conf)
{
  gpdfSecProcContextData_t *secContextData = GET_PARENT_BY_FIELD(gpdfSecProcContextData_t, secProcData.secReq.zgpSinkDecryptReq.confirm, conf);
  bool responseToGpd = secContextData->secProcData.gpdfSecProcessing.sendInDirectMode;
  uint16_t proxyAddr = secContextData->secProcData.gpdfSecProcessing.tempMasterAddr;

  if(conf->status == SSP_SUCCESS_STATUS)
  {
    uint8_t headerLength = ZGP_MAX_HEADER_LENGTH;

    if (ZGP_IEEE_ADDR_APPID == secContextData->secProcData.gpdfSecProcessing.sinkGpdfDataInd.applicationId)
      headerLength = ZGP_MAX_HEADER_LENGTH_FOR_IEEE_APPID;

    secContextData->secProcData.gpdfSecProcessing.sinkGpdfDataInd.gpdCommandId = secContextData->secProcData.secReq.zgpSinkDecryptReq.pdu[headerLength];
    zgpRxdGpdfIndBuffer.dstubDataInd.gpdAsdu = &zgpRxdGpdfIndBuffer.gpdPayload[0];
    memcpy(&zgpRxdGpdfIndBuffer.dstubDataInd, &secContextData->secProcData.gpdfSecProcessing.sinkGpdfDataInd, sizeof(zgpRxdGpdfIndBuffer.dstubDataInd));
    if (secContextData->secProcData.gpdfSecProcessing.sinkGpdfDataInd.gpdAsduLength)
    {
      memcpy(&zgpRxdGpdfIndBuffer.dstubDataInd.gpdAsdu[0], &secContextData->secProcData.secReq.zgpSinkDecryptReq.pdu[headerLength + 1], \
                     secContextData->secProcData.gpdfSecProcessing.sinkGpdfDataInd.gpdAsduLength);
    }
    processRxdGpdf(&zgpRxdGpdfIndBuffer.dstubDataInd, responseToGpd, proxyAddr);
  }
  freeSecContextData(secContextData, SEC_GPDF_DECRYPTION);
}

/**************************************************************************//**
  \brief Update EntryInfo

  \param[in] sinkEntry - EntryInfo to be updated
             appId - app id
             gpdId - gpd id
             endPoint - end point

  \return true for valid information
          false otherwise
******************************************************************************/
static bool updateEntryInfo(ZGP_SinkTableEntry_t *sinkEntry, ZGP_ApplicationId_t appId, ZGP_GpdId_t *gpdId,\
                     uint8_t endPoint)
{
  ZGPL_ResetTableEntry((void *)sinkEntry, ZGP_SINK_ENTRY);
  sinkEntry->options.appId = appId;
  sinkEntry->options.communicationMode = zgpClusterServerAttributes.gpsCommunicationMode.value;

  if (PRECOMMISSIONED_GROUPCAST == sinkEntry->options.communicationMode)
  {
    if (ZGP_NWK_ADDRESS_GROUP_INIT != zgpSinkGroupEntry.sinkGroup)
    {
       sinkEntry->tableGenericInfo.zgpSinkGrouplist[0].sinkGroup = zgpSinkGroupEntry.sinkGroup;
       sinkEntry->tableGenericInfo.zgpSinkGrouplist[0].alias = zgpSinkGroupEntry.alias;
       if (ZGP_NWK_ADDRESS_GROUP_INIT == zgpSinkGroupEntry.alias)
         sinkEntry->tableGenericInfo.zgpSinkGrouplist[0].alias = ZGPL_GetAliasSourceAddr(gpdId);
    }
    else
    {
      return false;
    }
  }
  sinkEntry->options.assignedAlias = false;

  memcpy(&sinkEntry->tableGenericInfo.gpdId, gpdId, sizeof(ZGP_GpdId_t));
  sinkEntry->tableGenericInfo.endPoint = endPoint;

  sinkEntry->tableGenericInfo.gpdAssignedAlias = 0x0;
  sinkEntry->tableGenericInfo.groupCastRadius = 0x00;

  return true;
}

/**************************************************************************//**
  \brief Functionality Matching of the device

  \param[in]deviceId - deviceId of the Entry

  \return true - if functionality matching is successful
          false - functionality matching failed
******************************************************************************/
static bool sinkBasicFuncMatching(uint8_t *deviceId, ZGP_GpdAppInfo_t *appInfo)
{

  ZGP_TransTableIndicationInfo_t transTableInfo;

  memset(&transTableInfo, 0x00, sizeof(transTableInfo));

  transTableInfo.transTableIndType = APP_FUNCTIONALITY_CHECK;

  transTableInfo.indicationData.appFuncCheckInfo.deviceId = deviceId;
  transTableInfo.indicationData.appFuncCheckInfo.appInfo = appInfo;
  transTableInfo.indicationData.appFuncCheckInfo.isMatching = true;
  SYS_PostEvent(BC_EVENT_ZGPH_TRANS_TABLE_INDICATION, (SYS_EventData_t)&transTableInfo);
  if (!transTableInfo.indicationData.appFuncCheckInfo.isMatching)
    return false;

#if MICROCHIP_APPLICATION_SUPPORT == 1
  // if device id is updated for non-supported device id rxd & based on received cmd
  // then cmd already added so make noOfGpdCmds zero
  if (ZGP_UNSPECIFIED_DEVICE_ID != *deviceId)
  {
    appInfo->noOfGpdCmds = 0;
  }
#endif
  return true;
}

/**************************************************************************//**
  \brief Forming gpdf response for AutoCommissioning bit Set in Data GPDF

  \param[in]sinkEntry - sinkEntry to be created, ind - dstub data indication

  \return true - continue with gpdf processing
          false - not proceeding with gpdf processing
******************************************************************************/
static bool createSinkTableEntry(ZGP_SinkTableEntry_t *sinkEntry,ZGP_LowDataInd_t *ind)
{
  ZGP_GpdId_t gpdId;

  gpdId.gpdIeeeAddr = ind->srcAddress.ext;
  if (ZGP_SRC_APPID == ind->applicationId)
  {
    gpdId.gpdSrcId = ind->srcId;
  }

  if (updateEntryInfo(sinkEntry,ind->applicationId, &gpdId, ind->endPoint))
  {
    ZGP_GpdAppInfo_t appInfo;
    memset(&appInfo, 0x00, sizeof(ZGP_GpdAppInfo_t));
    appInfo.noOfGpdCmds = 1;
    appInfo.gpdCommandList[0] = ind->gpdCommandId;
    uint8_t *derivedDeviceId = &sinkEntry->deviceId;
    sinkBasicFuncMatching(derivedDeviceId, &appInfo);
#ifdef ZGP_ENABLE_GENERIC_8_CONTACT_SWITCH_SUPPORT    
    if(*derivedDeviceId == (uint8_t)ZGP_GENERIC_8_CONTACT_SWITCH)
      appInfo.switchInfo.currContactStatus = *(ind->gpdAsdu);
#endif
    if (ZGPL_AddOrUpdateTableEntryOnNvm((void *)sinkEntry, UPDATE_ENTRY, ZGP_SINK_ENTRY))
    {
      sinkEntry->options.communicationMode = zgpClusterServerAttributes.gpsCommunicationMode.value;
      zgpServerAddGroup(sinkEntry , true);
      zgpServerSendGpPairingForEntry(sinkEntry, EXTEND_SINKTABLE_ENTRY);
      zgpTransRemoveAndAddEventHandler(COMM_PROC_TYPE, sinkEntry, &appInfo, 1, &sinkBasicModeInfo.commissioningGpdEp);
      return true;
    }
  }
  return false;
}
/**************************************************************************//**
  \brief triggering event for success commissioning

  \param[in]  sessionEntryIndex - entry index

  \return None
******************************************************************************/
static void triggerSuccessCommissioningEvent(uint8_t sessionEntryIndex)
{
  ZGP_IndicationInfo_t indicationInfo;

  memset(&indicationInfo, 0x00, sizeof(indicationInfo));
  indicationInfo.indicationType = SUCCESSFUL_COMMISSIONING;
  memcpy((void *)&indicationInfo.indicationData.commssionedDeviceInfo.gpdId, (void *)&commSessionEntries[sessionEntryIndex].gpdId, \
          sizeof(indicationInfo.indicationData.commssionedDeviceInfo.gpdId));
  indicationInfo.indicationData.commssionedDeviceInfo.appId = commSessionEntries[sessionEntryIndex].appId;
  indicationInfo.indicationData.commssionedDeviceInfo.endPoint = commSessionEntries[sessionEntryIndex].endPoint;
  indicationInfo.indicationData.commssionedDeviceInfo.deviceId = commSessionEntries[sessionEntryIndex].commReq.gpdDeviceId;
  indicationInfo.indicationData.commssionedDeviceInfo.commReqInfoPresent = true;

  memcpy(&indicationInfo.indicationData.commssionedDeviceInfo.commReq, &commSessionEntries[sessionEntryIndex].commReq, sizeof(zgpCommReq_t));

  SYS_PostEvent(BC_EVENT_ZGPH_INDICATION, (SYS_EventData_t)&indicationInfo);
}
/**************************************************************************//**
  \brief Encrypting gpd key

  \param[in]  sessionEntryIndex - entry index

  \return next state after processing
******************************************************************************/
static sinkCommProcState_t encryptTxGpdKey(uint8_t sessionEntryIndex)
{
  uint8_t key[ZGP_SECURITY_KEY_LENGTH] = CS_ZGP_SECURITY_LINK_KEY;
  uint8_t index = 0;
  uint32_t mic = 0;
  SSP_ZgpEncryptFrameReq_t *zgpSinkEncryptReq;

  gpdfSecProcContextData_t *secContextData = getSecProcContextDataBuffer(SEC_KEY_ENCRYPTION);

  if (NULL == secContextData)
    return COMM_COMPLETE;

  zgpSinkEncryptReq = &secContextData->secProcData.secReq.zgpSinkEncryptReq;
  secContextData->secProcData.keySecProcessing.sessionIndex = sessionEntryIndex;

  // need to encrypt the key
  zgpSinkEncryptReq->pdu = secContextData->secProcData.keySecProcessing.securedKeyPayload;
  zgpSinkEncryptReq->key = secContextData->secProcData.keySecProcessing.key;
  memcpy((void *)zgpSinkEncryptReq->key, key, ZGP_SECURITY_KEY_LENGTH);

  zgpSinkEncryptReq->dir = ZGP_TX_TO_ZGPD;
  zgpSinkEncryptReq->appId = commSessionEntries[sessionEntryIndex].appId;
  zgpSinkEncryptReq->securityLevel = ZGP_SECURITY_LEVEL_3;
  zgpSinkEncryptReq->securityFrameCounter = commSessionEntries[sessionEntryIndex].commReq.gpdOutgoingCounter + 1;

  if (ZGP_SRC_APPID == commSessionEntries[sessionEntryIndex].appId)
  {
    zgpSinkEncryptReq->srcID = commSessionEntries[sessionEntryIndex].gpdId.gpdSrcId;
    memcpy(&zgpSinkEncryptReq->pdu[index], (void *)&zgpSinkEncryptReq->srcID, ZGP_COMM_KEY_HEADER_LEN);
  }
  else
  {
    zgpSinkEncryptReq->extAddr = commSessionEntries[sessionEntryIndex].gpdId.gpdIeeeAddr;
    memcpy(&zgpSinkEncryptReq->pdu[index], (void *)&zgpSinkEncryptReq->extAddr, ZGP_COMM_KEY_HEADER_LEN);
  }

  index += ZGP_COMM_KEY_HEADER_LEN;
  zgpSinkEncryptReq->headerLength = ZGP_COMM_KEY_HEADER_LEN;
  zgpSinkEncryptReq->payloadLength = ZGP_SECURITY_KEY_LENGTH;
  memcpy(&zgpSinkEncryptReq->pdu[index], commSessionEntries[sessionEntryIndex].commReq.gpdKey, ZGP_SECURITY_KEY_LENGTH);

  index += ZGP_SECURITY_KEY_LENGTH;

  memcpy(&zgpSinkEncryptReq->pdu[index], &mic, sizeof(mic));
  index += sizeof(mic);

  memcpy(&zgpSinkEncryptReq->pdu[index], &zgpSinkEncryptReq->securityFrameCounter, \
                                           sizeof(zgpSinkEncryptReq->securityFrameCounter));

  zgpSinkEncryptReq->SSP_ZgpEncryptFrameConf = encryptGpdKeyconfirm;
  SSP_ZgpEncryptFrameReq(zgpSinkEncryptReq);

  return COMM_TX_GPD_KEY_ENCRYPTION;
}

/**************************************************************************//**
  \brief Decryption confirmation for gpd key

  \param[in] conf - confirmation data

  \return None
******************************************************************************/
static void commReqDecryptGpdKeyconfirm(SSP_ZgpDecryptFrameConf_t *conf)
{
  gpdfSecProcContextData_t *secContextData = GET_PARENT_BY_FIELD(gpdfSecProcContextData_t, secProcData.secReq.zgpSinkDecryptReq.confirm, conf);
  uint8_t entryIndex = secContextData->secProcData.keySecProcessing.sessionIndex;

  if(SSP_SUCCESS_STATUS != conf->status)
  {
    // Dropping the session since key decryption in req is failing
    commSessionEntries[entryIndex].commState = COMM_COMPLETE;
  }
  else
  {
    uint8_t index = secContextData->secProcData.keySecProcessing.sessionIndex;
    zgpCommReq_t *commReq = &commSessionEntries[index].commReq;

    memcpy(&commReq->gpdKey[0], &secContextData->secProcData.keySecProcessing.securedKeyPayload[ZGP_COMM_KEY_HEADER_LEN], \
                                                                      sizeof(commReq->gpdKey));

    commSessionEntries[entryIndex].validActionRxd = true;
    commSessionEntries[entryIndex].commState = COMM_WAITING_FOR_COMPLETE_COM_REQ_INFO;
  }
  freeSecContextData(secContextData, SEC_KEY_DECRYPTION);
  commissioningHandler(entryIndex);
}

/**************************************************************************//**
  \brief Encryption confirmation for gpd key

  \param[in] conf - confirmation data

  \return None
******************************************************************************/
static void encryptGpdKeyconfirm(SSP_ZgpEncryptFrameConf_t *conf)
{
  gpdfSecProcContextData_t *secContextData = GET_PARENT_BY_FIELD(gpdfSecProcContextData_t, secProcData.secReq.zgpSinkEncryptReq.confirm, conf);
  uint8_t index = secContextData->secProcData.keySecProcessing.sessionIndex;

  if(SSP_SUCCESS_STATUS != conf->status)
  {
    commSessionEntries[index].commState = COMM_COMPLETE;
  }
  else
  {
    uint8_t encryptionHeaderLength = ZGP_COMM_KEY_HEADER_LEN;

    memcpy(&commSessionEntries[index].commReq.gpdKey[0], &secContextData->secProcData.secReq.zgpSinkEncryptReq.pdu[encryptionHeaderLength] ,\
             sizeof(commSessionEntries[index].commReq.gpdKey));
    encryptionHeaderLength += ZGP_SECURITY_KEY_LENGTH;

    memcpy(&commSessionEntries[index].commReq.gpdKeyMic, &secContextData->secProcData.secReq.zgpSinkEncryptReq.pdu[encryptionHeaderLength] ,\
             sizeof(commSessionEntries[index].commReq.gpdKeyMic));
    encryptionHeaderLength += sizeof(uint32_t);

    // which is populated in frameAndSendCommissioningReply function
    commSessionEntries[index].commState = COMM_FRAME_AND_SEND_COMM_REPLY;

  }
  freeSecContextData(secContextData, SEC_KEY_ENCRYPTION);
  commissioningHandler(index);
}

/**************************************************************************//**
  \brief To send the GPDF in direct mode to GPD

  \param[in] ind - dstub data indication

  \return None
******************************************************************************/
static void sendGpdfInDirectMode(gpdfRsp_t *gpdfRsp)
{
  ZGP_GpdfDataReq_t *dStubDataReq = (ZGP_GpdfDataReq_t *)zgpGetMemReqBuffer();

  dStubDataReq->applicationId = gpdfRsp->appId;
  dStubDataReq->txOptions.gpdfFrameType = gpdfRsp->frameType;
  if(ZGP_SRC_APPID == gpdfRsp->appId)
  {
    dStubDataReq->srcId = gpdfRsp->gpdId.gpdSrcId;
  }
  else
  {
    dStubDataReq->gpdIeeeAddress = gpdfRsp->gpdId.gpdIeeeAddr;
    dStubDataReq->endpoint = gpdfRsp->endPoint;
  }

  dStubDataReq->action = true;
  dStubDataReq->txOptions.txOnMatchingEndpoint = 1; // for App id 2
  dStubDataReq->txOptions.requireMacAck = false;
  dStubDataReq->txOptions.performCsma = false;
  dStubDataReq->frameDir = ZGP_TX_TO_ZGPD;
  dStubDataReq->gpdCommandId = (ZGP_CommandId_t)gpdfRsp->cmdId;
  dStubDataReq->gpdAsdu = gpdfRsp->cmdPayload;
  dStubDataReq->gpdAsduLength = gpdfRsp->cmdPayloadLength;

  dStubDataReq->gpTxQueueEntryLifeTime = DEF_QUEUE_ENTRY_LIFETIME;

  ZGPL_GpdfDataRequest(dStubDataReq);
  zgpFreeMemReqBuffer();
}

/**************************************************************************//**
  \brief Check sink basic exit commissioning mode

  \param[in] None

  \return None
******************************************************************************/
static void sinkBasicCheckCommMode(void)
{
  uint8_t exitMode = zgpClusterServerAttributes.gpsCommissioningExitMode.value;

  if (exitMode & GPS_EXIT_ON_FIRST_PAIRING_SUCCESS)
  {
    HAL_StopAppTimer(&commWindowTimer);
    sinkBasicExitCommMode();
  }
}

/**************************************************************************//**
  \brief Sink basic exit commissioning mode handling

  \param[in] None

  \return None
******************************************************************************/
static void sinkBasicExitCommMode(void)
{
  zgpSessionIndex_t sessionIndex = {.entryIndex = ZGP_INVALID_SESSION_ENTRY_INDEX, .freeIndex = ZGP_INVALID_SESSION_ENTRY_INDEX};

  ZGPL_SetDeviceMode(false, OPERATIONAL_MODE);
  // Freeing the buffer
  // Need to flush out all the pairing session entries of COMM_REQ type
  while(zgpServerGetCommReqEntry(&sessionIndex))
  {
    flushoutTxQueueForGpd(sessionIndex.entryIndex);
    zgpServerFreeSessionEntry(sessionIndex.entryIndex);
  }
}

/**************************************************************************//**
  \brief To check whether endpoint is registered

  \param[in] endPointId - application end point

  \return true - ep registered
          false - not registered
******************************************************************************/
static bool isEndPointRegistered(uint8_t endPointId)
{
  APS_RegisterEndpointReq_t *epReq = NULL;

  //To get eachEndpoint and verify if the cluster is available in the particular endpoint(scan through all the endpoints)
  epReq = APS_NextEndpoint(epReq);
  while(NULL != epReq)
  {
    if (epReq->simpleDescriptor->endpoint == endPointId)
      return true;
    epReq = APS_NextEndpoint(epReq);
  }
  return false;
}

/**************************************************************************//**
  \brief Add/Remove sink table entry

  \param[in] sinkEntry - entry fields(if proper security key(non-zero) is not provided,
                         this will be derived based on gpsSharedKeyType
  \param[in] action - EXTEND_SINKTABLE_ENTRY/REMOVE_GPD supported

  \return status.
 ******************************************************************************/
ZGP_InfraDeviceStatus_t ZGP_UpdateLocalSinkEntry(ZGP_SinkTableEntry_t *sinkEntry, ZGP_SinkTableActions_t action)
{
  zgpSinkTableStatus_t entryStatus;

  if (EXTEND_SINKTABLE_ENTRY == action)
  {
    if (!ZGPL_AddOrUpdateTableEntryOnNvm((void *)sinkEntry, UPDATE_ENTRY, ZGP_SINK_ENTRY))
      return ZGP_NO_FREE_ENTRY;
  }
  else if (REMOVE_GPD == action)
  {
    entryStatus = zgpSinkRemoveGpdEntry(sinkEntry, true);
    if (ZGP_SINK_TABLE_ENTRY_NOT_AVAILABLE == entryStatus)
      return ZGP_ENTRY_NOT_AVAILABLE;
  }
  else
    return ZGP_INVALID_ACTION;

  return ZGP_SUCCESS;
}

/**************************************************************************//**
\brief Setting the zgp sink group id

\param[in] sinkGroupId - sink group Id to be set

\return Status.
 ******************************************************************************/
ZGP_InfraDeviceStatus_t ZGP_SetSinkGroupEntry(GroupAddr_t sinkGroupId)
{
  zgpSinkGroupEntry.sinkGroup = sinkGroupId;
  PDS_Store(ZGP_SINK_GROUP_ENTRY_MEM_ID);
  return ZGP_SUCCESS;
}
/**************************************************************************//**
\brief Setting the assigned alias to be used for the next GPD device which gets commissioned

\param[in] gpdAssignedAlias - The commissioned 16-bit ID to be used as alias for this GPD

\return Status.
******************************************************************************/
ZGP_InfraDeviceStatus_t ZGP_SetGPDAssignedAlias(ShortAddr_t gpdAssignedAlias)
{
  zgpSinkGroupEntry.alias = gpdAssignedAlias;
  PDS_Store(ZGP_SINK_GROUP_ENTRY_MEM_ID);
  return ZGP_SUCCESS;
}

/**************************************************************************//**
  \brief derivation of the key

  \param[in] entryIndex - entry index

  \return next state after processing.
******************************************************************************/
static sinkCommProcState_t zgpGenericKeyDerivationHmac(uint8_t entryIndex)
{
  gpdfSecProcContextData_t *secContextData = getSecProcContextDataBuffer(SEC_KEY_DERIVATION);
  zgpCommReq_t *commReq = &commSessionEntries[entryIndex].commReq;
  ZGP_SecKeyType_t secType = (ZGP_SecKeyType_t)commReq->extOptions.keyType;

  if (NULL == secContextData)
    return COMM_COMPLETE;

  SSP_KeyedHashMacReq_t* zgpHmacReq = &secContextData->secProcData.secReq.zgpSinkKeyedHashReq;
  if(ZGP_KEY_TYPE_NWKKEY_DERIVED_GROUP_KEY == secType)
  {
    secContextData->secProcData.keyDerivation.hmacText[ZGP_HMAC_TEXT_START_POS]     = 'Z';
    secContextData->secProcData.keyDerivation.hmacText[ZGP_HMAC_TEXT_START_POS + 1] = 'G';
    secContextData->secProcData.keyDerivation.hmacText[ZGP_HMAC_TEXT_START_POS + 2] = 'P';
    zgpHmacReq->textSize = GROUP_KEY_HMAC_TEXT_SIZE;
  }
  else if(ZGP_KEY_TYPE_DERIVED_INDIVIDUAL_ZGPD_KEY == secType)
  {
    if(ZGP_SRC_APPID == commSessionEntries[entryIndex].appId)
    {
      uint32_t gpdSrcId;
      gpdSrcId = commSessionEntries[entryIndex].gpdId.gpdSrcId;
      memcpy(&secContextData->secProcData.keyDerivation.hmacText[ZGP_HMAC_TEXT_START_POS], &gpdSrcId, sizeof(uint32_t));
      zgpHmacReq->textSize = sizeof(uint32_t);
    }
    else if(ZGP_IEEE_ADDR_APPID == commSessionEntries[entryIndex].appId)
    {
      uint64_t gpdIeeeAddr;
      gpdIeeeAddr = commSessionEntries[entryIndex].gpdId.gpdIeeeAddr;
      memcpy(&secContextData->secProcData.keyDerivation.hmacText[ZGP_HMAC_TEXT_START_POS], &gpdIeeeAddr, sizeof(uint64_t));
      zgpHmacReq->textSize = sizeof(uint64_t);
    }
  }
  secContextData->secProcData.keyDerivation.sessionIndex = entryIndex;
  zgpHmacReq->text = &secContextData->secProcData.keyDerivation.hmacText[ZGP_HMAC_TEXT_START_POS];
  zgpHmacReq->key = commReq->gpdKey;
  zgpHmacReq->hash_i = &secContextData->secProcData.keyDerivation.gpdDerivedKey[0];
  zgpHmacReq->SSP_KeyedHashMacConf = zgpHmacKeyDerivationConf;
  SSP_KeyedHashMacReq(zgpHmacReq);

  return COMM_GPD_KEY_DERIVATION;
}
/**************************************************************************//**
  \brief key derivation confirmation for gpd key

  \param[in] conf - confirmation data

  \return None
******************************************************************************/
static void zgpHmacKeyDerivationConf(SSP_KeyedHashMacConf_t *conf)
{
  gpdfSecProcContextData_t *secContextData = GET_PARENT_BY_FIELD(gpdfSecProcContextData_t, secProcData.secReq.zgpSinkKeyedHashReq.confirm, conf);
  uint8_t index = secContextData->secProcData.keyDerivation.sessionIndex;

  if(SSP_SUCCESS_STATUS != conf->status)
  {
    commSessionEntries[index].commState = COMM_COMPLETE;
  }
  else
  {
    memcpy(&commSessionEntries[index].commReq.gpdKey[0], &secContextData->secProcData.keyDerivation.gpdDerivedKey[0] ,\
             sizeof(commSessionEntries[index].commReq.gpdKey));
    memcpy(commSessionEntries[index].secKey ,commSessionEntries[index].commReq.gpdKey, \
                                       sizeof(commSessionEntries[index].secKey));
    commSessionEntries[index].commState = COMM_TX_GPD_KEY_ENCRYPTION;
  }
  freeSecContextData(secContextData, SEC_KEY_DERIVATION);
  commissioningHandler(index);
}

/**************************************************************************//**
\brief Setting the zgp sink group id

\param[in] sinkGroupId - sink group Id to be set

\return Status.
 ******************************************************************************/
ZGP_InfraDeviceStatus_t ZGPH_SetSinkGroupEntry(GroupAddr_t sinkGroupId)
{
  zgpSinkGroupEntry.sinkGroup = sinkGroupId;
  PDS_Store(ZGP_SINK_GROUP_ENTRY_MEM_ID);
  return ZGP_SUCCESS;
}

/**************************************************************************//**
\brief Setting the assigned alias to be used for the next GPD device which gets commissioned

\param[in] gpdAssignedAlias - The commissioned 16-bit ID to be used as alias for this GPD

\return Status.
******************************************************************************/
ZGP_InfraDeviceStatus_t ZGPH_SetGPDAssignedAlias(ShortAddr_t gpdAssignedAlias)
{
  zgpSinkGroupEntry.alias = gpdAssignedAlias;
  PDS_Store(ZGP_SINK_GROUP_ENTRY_MEM_ID);
  return ZGP_SUCCESS;
}

/**************************************************************************//**
  \brief Add/Remove sink table entry

  \param[in] sinkEntry - entry fields(if proper security key(non-zero) is not provided,
                         this will be derived based on gpsSharedKeyType
  \param[in] action - EXTEND_SINKTABLE_ENTRY/REMOVE_GPD supported

  \return status.
 ******************************************************************************/
ZGP_InfraDeviceStatus_t ZGPH_UpdateLocalSinkEntry(ZGP_SinkTableEntry_t *sinkEntry, ZGP_SinkTableActions_t action)
{
  zgpSinkTableStatus_t entryStatus;

  if(((ZGP_SRC_APPID == sinkEntry->options.appId) && ((!ZGPL_IsValidSrcId(sinkEntry->tableGenericInfo.gpdId.gpdSrcId, ZGP_FRAME_DATA, true))
#if MICROCHIP_APPLICATION_SUPPORT != 1
     || (sinkEntry->tableGenericInfo.gpdId.gpdSrcId == ZGP_ALL_SRC_ID)
#endif
      ))
      || ((ZGP_IEEE_ADDR_APPID == sinkEntry->options.appId) && ((!sinkEntry->tableGenericInfo.gpdId.gpdIeeeAddr)
#if MICROCHIP_APPLICATION_SUPPORT != 1
     || (sinkEntry->tableGenericInfo.gpdId.gpdIeeeAddr == ZGP_ALL_IEEE_ADDR)
#endif
      )))
      return ZGP_INVALID_ACTION;

  if (EXTEND_SINKTABLE_ENTRY == action)
  {
    if (!ZGPL_AddOrUpdateTableEntryOnNvm((void *)sinkEntry, UPDATE_ENTRY, ZGP_SINK_ENTRY))
      return ZGP_NO_FREE_ENTRY;
  }
  else if (REMOVE_GPD == action)
  {
    entryStatus = zgpSinkRemoveGpdEntry(sinkEntry, true);
    if (ZGP_SINK_TABLE_ENTRY_NOT_AVAILABLE == entryStatus)
      return ZGP_ENTRY_NOT_AVAILABLE;
  }
  else
    return ZGP_INVALID_ACTION;

  return ZGP_SUCCESS;
}
#endif // #if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
#endif // _GREENPOWER_SUPPORT_

// eof zgpSinkBasic.c
