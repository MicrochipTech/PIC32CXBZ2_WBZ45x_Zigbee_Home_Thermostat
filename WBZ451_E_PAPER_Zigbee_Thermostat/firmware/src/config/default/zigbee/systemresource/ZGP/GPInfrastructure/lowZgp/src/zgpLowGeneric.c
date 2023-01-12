/*******************************************************************************
  Zigbee green power Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpLowGeneric.c

  Summary:
    This file contains the zgp Low generic feature implementation.

  Description:
    This file contains the zgp Low generic feature implementation.
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

#if (APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC)
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpDstub.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpLowGeneric.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLow.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowGpdf.h>
#include <zdo/include/zdo.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpLowMem.h>
#include <security/serviceprovider/include/sspHash.h>
/******************************************************************************
                    Defines section
******************************************************************************/
#define GROUP_KEY_HMAC_TEXT_SIZE      3U
#define ZGP_HMAC_TEXT_SIZE_MAX        8U
#define ZGP_HMAC_TEXTBUFFER_SIZE_MAX  SECURITY_KEY_SIZE + ZGP_HMAC_TEXT_SIZE_MAX + SECURITY_KEY_SIZE
#define ZGP_HMAC_TEXT_START_POS       16U


/******************************************************************************
                    Prototypes section
******************************************************************************/
static void zgpGenericUpdateDuplicateTable(void);
static bool isSecurityMatching(zgpTableGenericInfo_t *tableGenericInfo, ZGP_LowDataInd_t *dstubDataInd, \
                               zgpDstubSecResponse_t *secResp);
static bool secRequestHandling(ZGP_LowDataInd_t *dstubDataInd, zgpDstubSecResponse_t *secResp, \
                                      void *tableEntry,ZGP_EntryType_t tableType);
static zgpDuplicateCheckStatus_t zgpGenericAddToDuplicateTable(zgpDuplicateEntryParameters_t *duplicateEntryParameters);
#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC
static void zgpHmacKeyDerivationConf(SSP_KeyedHashMacConf_t *conf);
#endif
static bool zgpVerifyAddressConflictInTables(ZDO_DeviceAnnceReq_t *devAnnounce, ZGP_EntryType_t tableType);
static void processDeviceAnnounce(SYS_EventId_t id, SYS_EventData_t data);
static void zgpZdpResponse(ZDO_ZdpResp_t *resp);

/******************************************************************************
                   Static variables section
******************************************************************************/
static zgpDuplicateTable_t zgpDuplicateTable;
static ZGP_Mode_t sinkMode;
static ZGP_Mode_t proxyMode;

#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC
uint8_t hmacText[ZGP_HMAC_TEXTBUFFER_SIZE_MAX] = {0}; // For hash function
static uint8_t gpdKeyToBeDrived[ZGP_SECURITY_KEY_LENGTH];
#endif
static SYS_EventReceiver_t deviceAnnceReceiver = {.func = processDeviceAnnounce};

static union 
{
    ZGP_ProxyTableEntry_t proxyEntry;
    ZGP_SinkTableEntry_t sinkEntry;
} tableEntryBuffer;

zgpZdpReq_t zgpZdpReq = 
{
  .busy = false,
};
/******************************************************************************
                    Implementations section
******************************************************************************/

/**************************************************************************//**
\brief Initialize zgp generic feature.
******************************************************************************/
void ZGPL_Init(void)
{
  zgpDstubInit();
  sinkMode = OPERATIONAL_MODE;
  proxyMode = OPERATIONAL_MODE;

  // Initialize the duplicate table
  for (uint8_t index = 0; index < ZGP_DUPLICATE_TABLE_SIZE; index++)
    zgpDuplicateTable.zgpDuplicateEntry[index].isActive = false;

  SYS_SubscribeToEvent(BC_ZDP_DEVICE_ANNOUNCE_RECEIVED, &deviceAnnceReceiver);
}

/**************************************************************************//**
\brief To set the Device in operational/Commissioning Mode

\param[in] - isProxy - true for proxy, false for sink
          - mode  - commissioning/operational  

\return  None
******************************************************************************/
void ZGPL_SetDeviceMode(bool isProxy, ZGP_Mode_t mode)
{
  if (isProxy)
    proxyMode = mode;
  else
    sinkMode = mode;
}

/**************************************************************************//**
\brief To get the Device Mode (operational/Commissioning Mode)

\param[in] - isProxy - true for proxy, false for sink

\return CurrentMode of Device
******************************************************************************/
ZGP_Mode_t ZGPL_GetDeviceMode(bool isProxy)
{
  if (isProxy)
    return proxyMode;
  else
    return sinkMode;
}

/**************************************************************************//**
\brief process the duplicate packet check request

\param[in] duplicateEntryParameters - duplicate entry parameters

\return   Entry Status on Duplicate Check
******************************************************************************/
static zgpDuplicateCheckStatus_t zgpGenericAddToDuplicateTable(zgpDuplicateEntryParameters_t *duplicateEntryParameters)
{
  uint8_t index;
  uint8_t availableEntryIndex = 0xff;

  zgpGenericUpdateDuplicateTable();
  for (index = 0; index < ZGP_DUPLICATE_TABLE_SIZE; index++)
  {
    if (zgpDuplicateTable.zgpDuplicateEntry[index].isActive)
    {
      if (duplicateEntryParameters->applicationId == zgpDuplicateTable.zgpDuplicateEntry[index].zgpDuplicateEntryParameters.applicationId)
      {
        if ( ((ZGP_SRC_APPID == duplicateEntryParameters->applicationId) && (duplicateEntryParameters->gpdId.gpdSrcId == zgpDuplicateTable.zgpDuplicateEntry[index].zgpDuplicateEntryParameters.gpdId.gpdSrcId)) || \
             ((ZGP_IEEE_ADDR_APPID == duplicateEntryParameters->applicationId) && (duplicateEntryParameters->gpdId.gpdIeeeAddr == zgpDuplicateTable.zgpDuplicateEntry[index].zgpDuplicateEntryParameters.gpdId.gpdIeeeAddr) && 
               (duplicateEntryParameters->endpoint == zgpDuplicateTable.zgpDuplicateEntry[index].zgpDuplicateEntryParameters.endpoint)) )
        {
          if ((duplicateEntryParameters->gpdfSecurityLevel == zgpDuplicateTable.zgpDuplicateEntry[index].zgpDuplicateEntryParameters.gpdfSecurityLevel) && \
              (duplicateEntryParameters->seqNoSecurityFrameCounter == zgpDuplicateTable.zgpDuplicateEntry[index].zgpDuplicateEntryParameters.seqNoSecurityFrameCounter))
            break;
        }
        else
          continue;
        
      }
    }
    else
      availableEntryIndex = index;
  }

  if (index == ZGP_DUPLICATE_TABLE_SIZE)
  {
    // entry is not found so add it to the table
    if (availableEntryIndex == 0xff)
      return NO_ENTRY_AVAILABLE;
    else
    {
      zgpDuplicateTable.zgpDuplicateEntry[availableEntryIndex].isActive = true;
      zgpDuplicateTable.zgpDuplicateEntry[availableEntryIndex].entryTtl = ZGP_DUPLICATE_TIMEOUT;
      memcpy(&zgpDuplicateTable.zgpDuplicateEntry[availableEntryIndex].zgpDuplicateEntryParameters,duplicateEntryParameters,sizeof(zgpDuplicateEntryParameters_t));
      return ENTRY_ADDED;
    }
  }
  else
    return DUPLICATE_ENTRY;
}

/**************************************************************************//**
\brief Update duplicate table

\param[in] None

\return   None
******************************************************************************/
static void zgpGenericUpdateDuplicateTable(void)
{
  BcTime_t currentTime = HAL_GetSystemTime();
  BcTime_t timeDiff =  currentTime - zgpDuplicateTable.tableLastTimeStamp;

  for (uint8_t index = 0; index < ZGP_DUPLICATE_TABLE_SIZE; index++)
  {
    if (timeDiff > ZGP_DUPLICATE_TIMEOUT)
    {
      zgpDuplicateTable.zgpDuplicateEntry[index].isActive = false;
    }
    else if (zgpDuplicateTable .zgpDuplicateEntry[index].isActive)
    {
      zgpDuplicateTable.zgpDuplicateEntry[index].entryTtl -= timeDiff;
      if (zgpDuplicateTable.zgpDuplicateEntry[index].entryTtl <= 0)
        zgpDuplicateTable.zgpDuplicateEntry[index].isActive = false;
    }
  }
  zgpDuplicateTable.tableLastTimeStamp = currentTime;
}



/**************************************************************************//**
\brief Processing SEC request from dstub

\param[in] tableGenericInfo - table generic info. of the entry
\param[in] dstubDataInd - dStub data indication
\param[in] secResp - security response

\return   true - if security parameters are Matching
          false - otherwise
******************************************************************************/
static bool isSecurityMatching(zgpTableGenericInfo_t *tableGenericInfo, ZGP_LowDataInd_t *dstubDataInd, \
                               zgpDstubSecResponse_t *secResp)
{
  if (tableGenericInfo->securityOptions.securityLevel == dstubDataInd->gpdfSecurityLevel)
  {
    if (dstubDataInd->gpdfSecurityLevel == ZGP_SECURITY_LEVEL_0)
    {
      return true;
    }
    else if (dstubDataInd->gpdSecurityFrameCounter <= tableGenericInfo->gpdSecurityFrameCounter)
    {
      secResp->status = ZGP_DSTUB_DROP_FRAME;
      return false;
    }
    secResp->gpdfKeyType = (ZGP_SecKeyType_t)tableGenericInfo->securityOptions.securityKeyType;

    if (dstubDataInd->service.sharedOrIndividualkeyType)
    {
      switch (tableGenericInfo->securityOptions.securityKeyType)
      {
        case ZGP_KEY_TYPE_DERIVED_INDIVIDUAL_ZGPD_KEY:
        case ZGP_KEY_TYPE_OOB_ZGPD_KEY:
        break;
        default:
         secResp->status = ZGP_DSTUB_DROP_FRAME;
         return false;
      } 
    }
    else
    {
      switch (tableGenericInfo->securityOptions.securityKeyType)
      {
        case ZGP_KEY_TYPE_NWK_KEY:
        case ZGP_KEY_TYPE_NWKKEY_DERIVED_GROUP_KEY:
        case ZGP_KEY_TYPE_ZGPD_GROUP_KEY:
        break;
        default:
         secResp->status = ZGP_DSTUB_DROP_FRAME;
         return false;
      } 
    }
    return true;
  }
  return false;
}

/**************************************************************************//**
\brief To check whether the key is valid or not

\return   true - valid
          false - invalid
******************************************************************************/
bool ZGPL_IskeyValid(uint8_t *key)
{
  for (uint8_t i = 0; i < ZGP_SECURITY_KEY_LENGTH; i++)
  {
    if (key[i] != 0x00)
      return true;
  }
  return false;
}
/**************************************************************************//**
\brief Processinf SEC request from dstub

\param[in] dstubDataInd - dstub data indication
\param[in] secResp - security response

\return   None
******************************************************************************/
void zgpGenericSecRequest(ZGP_LowDataInd_t *dstubDataInd, zgpDstubSecResponse_t *secResp)
{
  // Buffer allocation for proxy/sink table
  void *tableEntry = &tableEntryBuffer;

  secResp->status = ZGP_DSTUB_MATCH;
  secResp->gpdfKeyType = ZGP_KEY_TYPE_NO_KEY;

#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC
  if (secRequestHandling(dstubDataInd, secResp, tableEntry, ZGP_SINK_ENTRY))
#endif
  {
    secResp->status = ZGP_DSTUB_MATCH;
    secRequestHandling(dstubDataInd, secResp, tableEntry, ZGP_PROXY_ENTRY);
  }
}
/**************************************************************************//**
\brief sec. Request processing

\param[in] dstubDataInd - dstub data indication
\param[in] secResp - security response

\return   true - the caller can proceed with proxy sec. processing
          false - sink sec. processing successful so no need to process for proxy
******************************************************************************/
static bool secRequestHandling(ZGP_LowDataInd_t *dstubDataInd, zgpDstubSecResponse_t *secResp, \
                                      void *tableEntry, ZGP_EntryType_t tableType)
{
  ZGP_ReadOperationStatus_t entryStatus;
  ZGP_GpdId_t gpdId;
  zgpTableGenericInfo_t *tableGenericInfo = &((ZGP_ProxyTableEntry_t *)tableEntry)->tableGenericInfo;
  bool inCommissioningMode = (COMMISSIONING_MODE == ZGPL_GetDeviceMode(true));

#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC
  if (ZGP_SINK_ENTRY == tableType)
  {
    tableGenericInfo = &((ZGP_SinkTableEntry_t *)tableEntry)->tableGenericInfo;
    inCommissioningMode = (COMMISSIONING_MODE == ZGPL_GetDeviceMode(false));
  }
#endif

  gpdId.gpdSrcId = dstubDataInd->srcId;
  if (ZGP_IEEE_ADDR_APPID == dstubDataInd->applicationId)
    gpdId.gpdIeeeAddr = dstubDataInd->srcAddress.ext;

  {
    ZGP_TableOperationField_t  tableOperationField = {.entryType = tableType, .commMode = ALL_COMMUNICATION_MODE, .appId = dstubDataInd->applicationId, \
    .nonEmptyIndexForRead = ZGP_ENTRY_INVALID_INDEX};

    entryStatus = ZGPL_ReadTableEntryFromNvm((void *)tableEntry, tableOperationField, &gpdId, dstubDataInd->endPoint);
  }

  if (inCommissioningMode)
  {
    if (ZGP_PROXY_ENTRY == tableType)
    {
      secResp->status = ZGP_DSTUB_PASS_UNPROCESSED;
    }

    if (!dstubDataInd->gpdfSecurityLevel)
    {
      secResp->gpdSecurityFrameCounter = dstubDataInd->gpdSecurityFrameCounter;
      return false;
    }
    else
    {
#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC      
      if (ZGP_SINK_ENTRY == tableType)
      {
        if (ENTRY_NOT_AVAILABLE == entryStatus)
        {
          secResp->status = ZGP_DSTUB_PASS_UNPROCESSED;
          return false;
        }
      }
      else
#endif
      {
        // For proxy
        // As per spec. A.3.4.3 security handling for security failed GPDFs
        secResp->gpdfKeyType = ZGP_KEY_TYPE_NO_KEY;
        if (ZGP_SECURITY_KEY_INDIVIDUAL == dstubDataInd->service.sharedOrIndividualkeyType)
          secResp->gpdfKeyType = ZGP_KEY_TYPE_OOB_ZGPD_KEY;
      }
    }
  }

  else if (ACTIVE_ENTRY_AVAILABLE != entryStatus)
  {
    return true;
  }

  if (ENTRY_NOT_AVAILABLE != entryStatus)
  {
    if (isSecurityMatching(tableGenericInfo, dstubDataInd, secResp))
    {
      if (tableGenericInfo->securityOptions.securityLevel)
      {
        if(ZGPL_IskeyValid(&tableGenericInfo->securityKey[0]))
        {
          memcpy(&secResp->gpdKey, &tableGenericInfo->securityKey[0] , sizeof(secResp->gpdKey));
          secResp->status = ZGP_DSTUB_MATCH;
        }
        else
        {
          secResp->status = ZGP_DSTUB_DROP_FRAME;
          return false;
        }
      }

      secResp->gpdSecurityFrameCounter = dstubDataInd->gpdSecurityFrameCounter;
      if (ZGP_IEEE_ADDR_APPID == dstubDataInd->applicationId)
      {
        if ((dstubDataInd->endPoint != tableGenericInfo->endPoint) && \
            (APP_INDEPENDENT_END_POINT != tableGenericInfo->endPoint) && \
            (ALL_END_POINT != tableGenericInfo->endPoint) && \
            (APP_INDEPENDENT_END_POINT != dstubDataInd->endPoint) && \
            (ALL_END_POINT != dstubDataInd->endPoint))
          secResp->status = ZGP_DSTUB_TX_THEN_DROP;
      }
    }
    else if (ZGP_DSTUB_PASS_UNPROCESSED != secResp->status)
    {
      // for proxy in commissioning mode, don't drop the frame
      // so we are checking ZGP_DSTUB_PASS_UNPROCESSED
      secResp->status = ZGP_DSTUB_DROP_FRAME;
    }
  }

  return false;
}

/**************************************************************************//**
\brief check for duplicate packet for the data indication

\param[in] dstubDataInd - dstub data indication

\return   true - non-duplicate packet
          false otherwise
******************************************************************************/
bool ZGPL_CheckForDuplicate(ZGP_LowDataInd_t *dStubDataInd)
{
  zgpDuplicateEntryParameters_t duplicateParam;

  duplicateParam.applicationId = dStubDataInd->applicationId;
  duplicateParam.gpdfSecurityLevel = dStubDataInd->gpdfSecurityLevel;
  duplicateParam.seqNoSecurityFrameCounter = dStubDataInd->gpdSecurityFrameCounter;

  if (ZGP_SRC_APPID == duplicateParam.applicationId)
  {
    duplicateParam.gpdId.gpdSrcId = dStubDataInd->srcId;
  }
  else
  {
    duplicateParam.endpoint = dStubDataInd->endPoint;
    duplicateParam.gpdId.gpdIeeeAddr = dStubDataInd->srcAddress.ext;
  }

  if (ENTRY_ADDED == zgpGenericAddToDuplicateTable(&duplicateParam))
    return true;
  else
    return false;
}
/**************************************************************************//**
\brief To get alias addr for the given srcId/Ieee addr

\param[in] gpdId - gpdId of the Entry

return - aliasSrcAddr
******************************************************************************/
uint16_t ZGPL_GetAliasSourceAddr(ZGP_GpdId_t *gpdId)
{
  uint16_t derivedAlias;
  uint8_t *gpdIdIeeeAddr = (uint8_t *)gpdId;
  
  derivedAlias = gpdIdIeeeAddr[0] | (gpdIdIeeeAddr[1] << 8);

  if ((derivedAlias == 0x0000) || (derivedAlias > 0xfff7))
  {
    uint16_t temp;
    uint16_t xorDerivedAlias;

    // As per spec. A.3.6.3.3.1
    temp = gpdIdIeeeAddr[2] | (gpdIdIeeeAddr[3] << 8);
    xorDerivedAlias =  derivedAlias ^ temp;

    if ((xorDerivedAlias == 0x0000) || (xorDerivedAlias > 0xfff7))
    {
      if (derivedAlias == 0x0000)
        return 0x0007;
      else
        return (derivedAlias - 0x0008);
    }
    else
      return xorDerivedAlias;
  }
  else
    return derivedAlias;
}
/**************************************************************************//**
  \brief Verify for address conflicts for addresses present in sink/proxy tables

  \param[in] devAnnounce
  \param[in] tableType - sink or proxy table

  \return true - Address conflict detected
          false - No address conflict
******************************************************************************/
static bool zgpVerifyAddressConflictInTables(ZDO_DeviceAnnceReq_t *devAnnounce, ZGP_EntryType_t tableType)
{
  ZGP_ProxyTableEntry_t *entry = (ZGP_ProxyTableEntry_t *)&tableEntryBuffer;
  uint16_t alias;
  ZGP_GpdId_t gpIdInfo;
  ZGP_TableOperationField_t  tableOperationField = {.entryType = (uint8_t)tableType, .commMode = ALL_COMMUNICATION_MODE, \
  .nonEmptyIndexForRead = 0};
  zgpTableGenericInfo_t *tableGenericInfo = (zgpTableGenericInfo_t *)&entry->tableGenericInfo;
  ZGP_ApplicationId_t appId = (ZGP_ApplicationId_t)entry->options.appId;
  bool isDerivedGroupCast = false;
  // Reading all the entries and checking with received device_annce address
  while(ENTRY_NOT_AVAILABLE != ZGPL_ReadTableEntryFromNvm((void *)entry, tableOperationField, &gpIdInfo, ALL_END_POINT))
  {
    isDerivedGroupCast = entry->options.derivedGroupGps;
#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC
    if (ZGP_SINK_ENTRY == tableType)
    {
      tableGenericInfo = (zgpTableGenericInfo_t *)&((ZGP_SinkTableEntry_t *)entry)->tableGenericInfo;
      appId = (ZGP_ApplicationId_t)((ZGP_SinkTableEntry_t *)entry)->options.appId;
      isDerivedGroupCast = (DERIVED_GROUPCAST == ((ZGP_SinkTableEntry_t *)entry)->options.communicationMode);
    }
#endif
    if (ZGP_PROXY_ENTRY == tableType)
    {
      for(uint8_t listIndex = 0; listIndex < ZGP_LIGHT_WEIGHT_SINK_ADDRESS_LIST_SIZE; listIndex++)
      {
        ExtAddr_t ieeeAddress;
        ieeeAddress = entry->zgpProxyLightWeightSinkAddrlist[listIndex].sinkIeeeAddr;
        if(entry->zgpProxyLightWeightSinkAddrlist[listIndex].sinkNwkAddr != ZGP_NWK_ADDRESS_GROUP_INIT
              && ieeeAddress == devAnnounce->ieeeAddrLocal)
        {
          entry->zgpProxyLightWeightSinkAddrlist[listIndex].sinkNwkAddr = devAnnounce->nwkAddrLocal;
          return true;
        }
      }
    }

    for(uint8_t listIndex = 0; listIndex < ZGP_SINK_GROUP_LIST_SIZE; listIndex++)
    {
      alias = tableGenericInfo->zgpSinkGrouplist[listIndex].alias;
      if((alias == devAnnounce->nwkAddrLocal) && (alias != ZGP_NWK_ADDRESS_GROUP_INIT) && (ZGP_DEVICE_ANNCE_EXT_ADDR != devAnnounce->ieeeAddrLocal))
      {
        ZGPL_SendDeviceAnnounceCmd(alias, ZGP_DEVICE_ANNCE_EXT_ADDR);
        return true;
      }
    }

    if (ZGP_SRC_APPID == appId)
      gpIdInfo.gpdSrcId = tableGenericInfo->gpdId.gpdSrcId;
    else
      gpIdInfo.gpdIeeeAddr = tableGenericInfo->gpdId.gpdIeeeAddr;

    alias = ZGP_NWK_ADDRESS_GROUP_INIT;
    if (tableGenericInfo->gpdAssignedAlias != ZGP_NWK_ADDRESS_GROUP_INIT)
      alias = tableGenericInfo->gpdAssignedAlias;
    else  if(isDerivedGroupCast)
      alias = ZGPL_GetAliasSourceAddr(&gpIdInfo);

    if ((ZGP_NWK_ADDRESS_GROUP_INIT != alias) && (devAnnounce->nwkAddrLocal == alias) && (ZGP_DEVICE_ANNCE_EXT_ADDR != devAnnounce->ieeeAddrLocal))
    {
      ZGPL_SendDeviceAnnounceCmd(alias, ZGP_DEVICE_ANNCE_EXT_ADDR);
      return true;
    }

    tableOperationField.nonEmptyIndexForRead++;
  }
  return false;
}

/**************************************************************************//**
  \brief Handling of device announce received event.

  \param eventId - event identifier.
  \param data - event data.

  \return - None
******************************************************************************/
static void processDeviceAnnounce(SYS_EventId_t id, SYS_EventData_t data)
{
  ZDO_DeviceAnnceReq_t *devAnnounce;

  devAnnounce = &((ZDO_ZdpReqFrame_t *)data)->deviceAnnce;

  if (devAnnounce->nwkAddrLocal == NWK_GetShortAddr())
  {
    // This is local device announce. Incase this is sent because of GPD alias conflict
    // Remove the GPD entry from addressMap table
    ExtAddr_t extAddr = ZGP_DEFAULT_IEEE_ADDR;
    NWK_RemoveFromAddressMap(&extAddr);
  }

#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC
  if (!zgpVerifyAddressConflictInTables(devAnnounce, ZGP_SINK_ENTRY))
#endif
  {
    zgpVerifyAddressConflictInTables(devAnnounce, ZGP_PROXY_ENTRY);
  }

}

/**************************************************************************//**
  \brief Sending device annce

  \param[in] nwkAddr - nwk addr to be placed in the device annce
  \param[in] extAddr- ext addr to be placed in the device annce

  \return None.
 ******************************************************************************/
void ZGPL_SendDeviceAnnounceCmd(uint16_t nwkAddr, uint64_t extAddr)
{
  ZDO_DeviceAnnceReq_t *deviceAnnceReq;

  if (zgpZdpReq.busy)
    return;

  deviceAnnceReq = &zgpZdpReq.zdpReq.req.reqPayload.deviceAnnce;

  zgpZdpReq.busy = true;
  zgpZdpReq.zdpReq.ZDO_ZdpResp = zgpZdpResponse;
  zgpZdpReq.zdpReq.reqCluster = DEVICE_ANNCE_CLID;
  zgpZdpReq.zdpReq.dstAddrMode = APS_SHORT_ADDRESS;
  zgpZdpReq.zdpReq.dstAddress.shortAddress = BROADCAST_ADDR_RX_ON_WHEN_IDLE;

  deviceAnnceReq->nwkAddrLocal = nwkAddr;
  deviceAnnceReq->ieeeAddrLocal = extAddr;
  deviceAnnceReq->macCapability.allocateAddress = false;
  deviceAnnceReq->macCapability.alternatePANCoordinator = false;
  deviceAnnceReq->macCapability.deviceType = 0;
  deviceAnnceReq->macCapability.powerSource = false;
  deviceAnnceReq->macCapability.rxOnWhenIdle = false;
  deviceAnnceReq->macCapability.securityCapability = false;

  ZDO_ZdpReq(&zgpZdpReq.zdpReq);
}

/**************************************************************************//**
\brief device announce command response callback

\param[in] resp - response
******************************************************************************/
static void zgpZdpResponse(ZDO_ZdpResp_t *resp)
{
  zgpZdpReq.busy = false;
  (void)resp;
}

/**************************************************************************//**
\brief checking src id validity
  
\param[in]- srcId - srcId to be validated
            frameType - dataFrame / Maintanence Frame
            isPairingReq - SrcId received via pairingConfig/Commissioning

return - true - if srcId is valid
         false - otherwise
******************************************************************************/
bool ZGPL_IsValidSrcId(uint32_t srcId, ZGP_FrameType_t frameType, bool isPairingReq)
{
  if (((srcId >= ZGP_FIRST_RESERVED_SRC_ID) && (srcId <= ZGP_LAST_RESERVED_SRC_ID)) || \
       (!isPairingReq && (ZGP_ALL_SRC_ID == srcId)))
    return false;

  if ((ZGP_FRAME_DATA == frameType) && (ZGP_INVALID_SRC_ID == srcId))
    return false;

  return true;
}

#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC
/**************************************************************************//**
  \brief Sending simple descritor request

  \param[in] shortAddr - addr of the node
  \param[in] ep - endpoint of the node

  \return None.
 ******************************************************************************/
void ZGPL_SendSimpleDescReq(ShortAddr_t addr,uint8_t ep)
{
  ZDO_ZdpReq_t *zdpReq;

  if (zgpZdpReq.busy)
    return;

  zgpZdpReq.busy = true;
  zdpReq = &zgpZdpReq.zdpReq;
  ZDO_SimpleDescReq_t *simpleDescReq = &zdpReq->req.reqPayload.simpleDescReq;

  zdpReq->ZDO_ZdpResp              = zgpZdpResponse;
  zdpReq->reqCluster               = SIMPLE_DESCRIPTOR_CLID;
  zdpReq->dstAddrMode              = APS_SHORT_ADDRESS; 
  zdpReq->dstAddress.shortAddress  = addr;
  simpleDescReq->nwkAddrOfInterest = addr;
  simpleDescReq->endpoint          = ep;
  ZDO_ZdpReq(zdpReq);
}
/**************************************************************************//**
\brief To derive the GPD security key from network/group key. This is mainly used by high sink.
  
\param[in] sinkTableEntry - sink entry
             derivedKey - key to be derived
             keyedHashMacCb - callback function

\return None
******************************************************************************/
void ZGPL_KeyDerivationHmac(ZGP_SinkTableEntry_t* sinkTableEntry, uint8_t *derivedKey, void (*keyedHashMacCb)(void))
{
  SSP_KeyedHashMacReq_t* zgpHmacReq = zgpGetMemKeyedHmacReq();
  if(ZGP_KEY_TYPE_NWKKEY_DERIVED_GROUP_KEY == sinkTableEntry->tableGenericInfo.securityOptions.securityKeyType)
  {
    hmacText[ZGP_HMAC_TEXT_START_POS]     = 'Z';
    hmacText[ZGP_HMAC_TEXT_START_POS + 1] = 'G';
    hmacText[ZGP_HMAC_TEXT_START_POS + 2] = 'P';
    zgpHmacReq->textSize = GROUP_KEY_HMAC_TEXT_SIZE;
  }
  else if(ZGP_KEY_TYPE_DERIVED_INDIVIDUAL_ZGPD_KEY == sinkTableEntry->tableGenericInfo.securityOptions.securityKeyType)
  {
    if(ZGP_SRC_APPID == sinkTableEntry->options.appId)
    {
      uint32_t gpdSrcId;
      gpdSrcId = sinkTableEntry->tableGenericInfo.gpdId.gpdSrcId;
      memcpy(&hmacText[ZGP_HMAC_TEXT_START_POS], &gpdSrcId, sizeof(uint32_t));
      zgpHmacReq->textSize = sizeof(uint32_t);
    }
    else if(ZGP_IEEE_ADDR_APPID == sinkTableEntry->options.appId)
    {
      uint64_t gpdIeeeAddr;
      gpdIeeeAddr = sinkTableEntry->tableGenericInfo.gpdId.gpdIeeeAddr;
      memcpy(&hmacText[ZGP_HMAC_TEXT_START_POS], &gpdIeeeAddr, sizeof(uint64_t));
      zgpHmacReq->textSize = sizeof(uint64_t);
    }
  }
  zgpHmacReq->text = &hmacText[ZGP_HMAC_TEXT_START_POS];
  memcpy(gpdKeyToBeDrived, sinkTableEntry->tableGenericInfo.securityKey, sizeof(gpdKeyToBeDrived));
  zgpHmacReq->key = gpdKeyToBeDrived;
  zgpHmacReq->hash_i = derivedKey;
  zgpHmacReq->SSP_KeyedHashMacConf = zgpHmacKeyDerivationConf;
  zgpGetMem()->zgpKeyedHashMacCb = keyedHashMacCb;
  SSP_KeyedHashMacReq(zgpHmacReq);
}
/**************************************************************************//**
\brief confirmation for key derivation

\param[in] conf - confirmation parameters

return None
******************************************************************************/
static void zgpHmacKeyDerivationConf(SSP_KeyedHashMacConf_t *conf)
{
  zgpGetMem()->zgpKeyedHashMacCb();
  // Need to handle the confirmation status
  (void)conf;
}
#endif // APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC
#endif //APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
#endif // _GREENPOWER_SUPPORT_

// eof zgpLowGeneric.c
