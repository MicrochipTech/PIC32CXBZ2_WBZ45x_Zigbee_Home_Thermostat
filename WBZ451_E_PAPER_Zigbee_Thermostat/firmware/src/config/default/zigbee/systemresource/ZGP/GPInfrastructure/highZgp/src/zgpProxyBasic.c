/*******************************************************************************
  Zigbee green power proxy basic Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpProxyBasic.c

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

#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC

/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcl.h>
#include <zcl/include/zclGreenPowerCluster.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterClient.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterGeneric.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpInfraDevice.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpCluster.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighGeneric.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpProxyTable.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighMem.h>
#include <zgp/include/zgpDbg.h>
#include <configserver/include/configserver.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpProxyBasic.h>
#include <nwk/include/nwk.h>
#include <systemenvironment/include/sysEvents.h>
#include <systemenvironment/include/sysAssert.h>
#include <pds/include/wlPdsMemIds.h>

/******************************************************************************
                    Defines section
******************************************************************************/
#define DEF_SINK_ADDRESS              0xFFFF

/******************************************************************************
                    Types section
******************************************************************************/


/******************************************************************************
                    Prototypes section
******************************************************************************/
static void zgppProcessProxyCommMode(void *dataFrame);
static ZCL_Status_t zgppProcessPairingCmd(void *dataFrame);
static void zgppProcessProxyGpRespCmd(void* dataFrame);
static void proxyBasicCommWindowTimerFired(void);
static void checkForEntryAndForwardInOperationalMode(zgpProxyDataFrameType_t dataFrameType, void *dataFrame);
static void checkAndForwardInCommissioningMode(zgpProxyDataFrameType_t frameType, ZGP_LowDataInd_t *dstubInd);
static bool dstubHandlingForRxdGpResponse(void* dataFrame, bool *firstToForward);
static void proxyTableHandlingForRxdGpResponse(void* dataFrame, bool firstToForward);

/******************************************************************************
                    Global variables
******************************************************************************/
static zgpProxyBasicModeInfo_t zgpProxyBasicModeInfo;

static HAL_AppTimer_t commWindowTimer =
{
  .mode     = TIMER_ONE_SHOT_MODE,
  .interval = COMMISSIONING_WINDOW_DEFAULT_VALUE_IN_MSEC,
  .callback = proxyBasicCommWindowTimerFired
};

/******************************************************************************
                    Implementations section
******************************************************************************/

/**************************************************************************//**
  \brief Initialize zgp proxy.

  \param[in] None

  \return None.
******************************************************************************/
void ZGPH_ProxyBasicInit(void)
{
  ZGPL_Init();
  ZGPL_NvmTableInit();
  zgpHighGenericInit();
  zgpClusterGenericInit();
  zgpClusterClientInit();

  ZGPL_SetDeviceMode(true, OPERATIONAL_MODE);
  zgpProxyBasicModeInfo.commModeType = NO_COMM_MODE;
  zgpProxyBasicModeInfo.sinkAddr = DEF_SINK_ADDRESS;
}

/**************************************************************************//**
\brief Handling dstub data indication
******************************************************************************/
void zgpProxyDstubDataInd(const ZGP_LowDataInd_t *const ind)
{
  zgpProxyDataFrameType_t frameType;
  bool validFrame = true;

  if ((uint8_t)(ind->gpdfSecurityLevel))
  {
    if ((uint8_t)(ind->status) != ZGP_DSTUB_SECURITY_SUCCESS)
    {
      frameType = GPDF_UNPROCESSED_DATA_FRAME;
      zgpProxyDataHandling(frameType, (void *)ind);
      validFrame = false;
    }
  }

  if (validFrame)
  {
    if (ind->gpdCommandId < ZGP_COMMISSIONING_REPLY_CMD_ID)
    {
      if (ind->gpdCommandId == ZGP_CHANNEL_REQUEST_CMD_ID || ind->gpdCommandId == ZGP_COMMISSIONING_CMD_ID ||\
          ind->gpdCommandId == ZGP_COMMISSIONING_SUCCESS_CMD_ID || ind->gpdCommandId == ZGP_DECOMMISSIONING_CMD_ID || ind->gpdCommandId == ZGP_APPLICATION_DESCRIPTION_CMD_ID ||\
            (ind->gpdCommandId >= ZGP_MANUFAC_SPECIFIC_CMD0_FRAME && ind->gpdCommandId <= ZGP_MANUFAC_SPECIFIC_CMDF_FRAME)||\
            (ind->gpdCommandId >= ZGP_RESERVED_CMD1 && ind->gpdCommandId <= ZGP_RESERVED_CMDB))
        frameType = (zgpProxyDataFrameType_t)ind->gpdCommandId;
      else
        frameType = GPDF_DATA_FRAME;

      if (GPDF_DATA_FRAME == frameType)
      {
        if (ind->autoCommissioning && ind->rxAfterTx)
          return;
      }
      else if (ZGP_CHANNEL_REQUEST_CMD_ID != ind->gpdCommandId)
      {
        if (ind->autoCommissioning)
          return;
      }
      zgpProxyDataHandling(frameType, (void *)ind);
    }
  }
}

/**************************************************************************//**
\brief Handling incoming frame.

\param[in] dataFrameType -type of data frame
           dataFrame -dataFrame info
******************************************************************************/
ZCL_Status_t zgpProxyDataHandling(zgpProxyDataFrameType_t dataFrameType, void *dataFrame)
 {
  ZCL_Status_t status = ZCL_SUCCESS_STATUS;

  if (OPERATIONAL_MODE == ZGPL_GetDeviceMode(true))
  {
    switch(dataFrameType)
    {
      case GPDF_DATA_FRAME:
      case GPDF_COMMISSIONING_FRAME:
      case GPDF_APPLICATION_DESCR_FRAME:
      case GPDF_DECOMMISSIONING_FRAME:
        // check for proxy table entry
        // then forward to sink
        checkForEntryAndForwardInOperationalMode(dataFrameType, dataFrame);
      break;
      case GPDF_CHANNEL_REQUEST_FRAME:
        // as per spec A.3.5.2.3 gp_data indication handling for AUTH_FAILURE
      case GPDF_UNPROCESSED_DATA_FRAME:
      break;
      case GPDF_COMMISSIONING_SUCCESS_FRAME:
      // Drop the frame as per spec. A.3.9.1 step 17
      break;
      case ZDO_DEVICE_ANNOUNCE_FRAME:
      {
        // Check in the proxy table entry for matching sink address then update it
      }
      break;
      case GP_CLUSTER_GP_PAIRING_CMD:
      {
        status = zgppProcessPairingCmd(dataFrame);
      }
      break;
      case GP_CLUSTER_GP_PROXY_COMMISSIONING_CMD:
      {
        zgppProcessProxyCommMode(dataFrame);
      }
      break;
      case GP_CLUSTER_GP_RESPONSE_CMD:
      {
       // check for cluster command - GP commissioning mode/GP pairing/GP response
        zgppProcessProxyGpRespCmd(dataFrame);
      }
      break;
      default:
        SYS_E_ASSERT_ERROR(false, ZGP_PROXYBASIC_DATAFRAMETYPE0);
      break;
    }
  }
  else if (COMMISSIONING_MODE == ZGPL_GetDeviceMode(true))
  {
    switch(dataFrameType)
    {
      case GPDF_DECOMMISSIONING_FRAME:
      case GPDF_COMMISSIONING_SUCCESS_FRAME:
      case GPDF_CHANNEL_REQUEST_FRAME:
      case GPDF_COMMISSIONING_FRAME:
      case GPDF_APPLICATION_DESCR_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMD0_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMD1_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMD2_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMD3_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMD4_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMD5_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMD6_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMD7_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMD8_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMD9_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMDA_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMDB_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMDC_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMDD_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMDE_FRAME:
      case GPDF_MANUFAC_SPECIFIC_CMDF_FRAME:
      case GPDF_RESERVED_CMD1:
      case GPDF_RESERVED_CMD2:
      case GPDF_RESERVED_CMD3:
      case GPDF_RESERVED_CMD4:
      case GPDF_RESERVED_CMD5:
      case GPDF_RESERVED_CMD6:
      case GPDF_RESERVED_CMD7:
      case GPDF_RESERVED_CMD8:
      case GPDF_RESERVED_CMD9:
      case GPDF_RESERVED_CMDA:
      case GPDF_RESERVED_CMDB:
      case GPDF_UNPROCESSED_DATA_FRAME:
      {
        ZGP_LowDataInd_t *dstubInd = (ZGP_LowDataInd_t *)dataFrame;
        checkAndForwardInCommissioningMode(dataFrameType, dstubInd);
      }
      break;
      case GPDF_DATA_FRAME:
      {
        ZGP_LowDataInd_t *dstubInd = (ZGP_LowDataInd_t *)dataFrame;
        if (dstubInd->autoCommissioning)
        {
          if (!dstubInd->rxAfterTx) // As per spec. A.3.9.1 step 12
            checkAndForwardInCommissioningMode(dataFrameType, dstubInd);
        }
      }
      break;
      case ZDO_DEVICE_ANNOUNCE_FRAME:
      break;
      case GP_CLUSTER_GP_PAIRING_CMD:
      {
        status = zgppProcessPairingCmd(dataFrame);
      }
      break;
      case GP_CLUSTER_GP_PROXY_COMMISSIONING_CMD:
      {
        zgppProcessProxyCommMode(dataFrame);
      }
      break;
      case GP_CLUSTER_GP_RESPONSE_CMD:
      {
        zgppProcessProxyGpRespCmd(dataFrame);
      }
      break;
      default:
        SYS_E_ASSERT_ERROR(false, ZGP_PROXYBASIC_DATAFRAMETYPE0);
      break;
    }
  }
  return status;
}
/**************************************************************************//**
\brief Process the received proxy commissioning mode command
param[in] dataFrame - data frame info
******************************************************************************/
static void zgppProcessProxyCommMode(void *dataFrame)
{
  ZGP_GpProxyCommMode_t *gpProxyCommMode;
  gpProxyCommMode = (ZGP_GpProxyCommMode_t *)dataFrame;

  if(DEF_SINK_ADDRESS != zgpProxyBasicModeInfo.sinkAddr && gpProxyCommMode->srcSinkNwkAddress != zgpProxyBasicModeInfo.sinkAddr)
  {
    // drop the frame since this sink is not same as the sink which put the proxy in comm mode
    return;
  }

  // Enter Commissioning mode
  if(true == gpProxyCommMode->options.action)
  {
    ZGPL_SetDeviceMode(true, COMMISSIONING_MODE);
    zgpProxyBasicModeInfo.inUnicastMode = gpProxyCommMode->options.unicastCommunication;

    zgpProxyBasicModeInfo.sinkAddr = gpProxyCommMode->srcSinkNwkAddress;

    // process exit mode field
    // Either On GP Proxy Commissioning Mode (exit) or On first Pairing success
    // SHALL be set to 0b1 at the same time. Refer: A.3.3.2.4
    zgpProxyBasicModeInfo.commModeType = COMM_MODE_WAIT_FOR_TIMEOUT;
    commWindowTimer.interval = COMMISSIONING_WINDOW_DEFAULT_VALUE_IN_MSEC;

    if(gpProxyCommMode->options.exitMode & GPP_EXIT_ON_EXIT_PROXY_COMM_MODE)
    {
      zgpProxyBasicModeInfo.commModeType |= COMM_MODE_WAIT_FOR_EXIT;
    }
    if(gpProxyCommMode->options.exitMode & GPP_EXIT_ON_FIRST_PAIRING_SUCCESS)
    {
      zgpProxyBasicModeInfo.commModeType |= COMM_MODE_WAIT_FOR_PAIRING_SUCCESS;
    }
    if(gpProxyCommMode->options.commWindowPresent)
    {
      commWindowTimer.interval = gpProxyCommMode->commWindow*1000;
    }
    HAL_StopAppTimer(&commWindowTimer);
    HAL_StartAppTimer(&commWindowTimer);
  }
  else // Exit Commissioning mode
  {
    if (zgpProxyBasicModeInfo.commModeType & COMM_MODE_WAIT_FOR_EXIT)
    {
      HAL_StopAppTimer(&commWindowTimer);
      ZGPL_SetDeviceMode(true, OPERATIONAL_MODE);
      zgpProxyBasicModeInfo.commModeType = NO_COMM_MODE;
      zgpProxyBasicModeInfo.sinkAddr = DEF_SINK_ADDRESS;
      zgpClientStopTunneling((ZGP_ApplicationId_t)0x00/*dummy*/, NULL, 0x00/*dummy*/);
      ZGPL_FlushTxQueue();
    }
  }
}
/**************************************************************************//**
\brief exits commissioning mode on comm window timeout
******************************************************************************/
static void proxyBasicCommWindowTimerFired(void)
{
  ZGPL_SetDeviceMode(true, OPERATIONAL_MODE);
  zgpProxyBasicModeInfo.commModeType = NO_COMM_MODE;
  zgpProxyBasicModeInfo.sinkAddr = DEF_SINK_ADDRESS;
  zgpClientStopTunneling((ZGP_ApplicationId_t)0x00/*dummy*/, NULL, 0x00/*dummy*/);
  ZGPL_FlushTxQueue();
}
/**************************************************************************//**
\brief exits commissioning mode on comm window timeout
\param [in] dataFrame -dataFrame info
******************************************************************************/
static ZCL_Status_t zgppProcessPairingCmd(void* dataFrame)
{
  ZGP_GpPairing_t *gpPairingCmd;
  ZCL_Status_t status = ZCL_SUCCESS_STATUS;
  
  gpPairingCmd = (ZGP_GpPairing_t *)dataFrame;
  // add the pairing for this GPD based on AddSink
  if(true == gpPairingCmd->options.addSink && false == gpPairingCmd->options.removeGpd)
  {
    // Add this entry to the proxyTable
    if (zgpProxyTableCreateOrUpdateEntry(gpPairingCmd, true) == ZGP_PROXY_TABLE_FULL)
    {
      status = ZCL_INSUFFICIENT_SPACE_STATUS;
    }
  }
  else if(gpPairingCmd->options.addSink == false)
  {
    if (gpPairingCmd->options.removeGpd == true)
    {
      ZGP_TableOperationField_t filterField = {.appId = gpPairingCmd->options.appId, .entryType = ZGP_PROXY_ENTRY, .commMode = ALL_COMMUNICATION_MODE};

      ZGPL_DeleteTableEntryFromNvm(filterField, &gpPairingCmd->gpdId, gpPairingCmd->endpoint);
    }
    else
    {
      if (zgpProxyTableCreateOrUpdateEntry(gpPairingCmd, false) == ZGP_PROXY_TABLE_ENTRY_NOT_AVAILABLE)
      {
        status = ZCL_NOT_FOUND_STATUS;
      }
    }
  }
  // Exit commissioning mode on receiving any valid GP pairing cmd
  // As per spec. A.3.5.2.1
  if (zgpProxyBasicModeInfo.commModeType & COMM_MODE_WAIT_FOR_PAIRING_SUCCESS)
  {
    HAL_StopAppTimer(&commWindowTimer);
    ZGPL_SetDeviceMode(true, OPERATIONAL_MODE);
    zgpProxyBasicModeInfo.commModeType = NO_COMM_MODE;
    zgpProxyBasicModeInfo.sinkAddr = DEF_SINK_ADDRESS;
    zgpClientStopTunneling((ZGP_ApplicationId_t)0x00/*dummy*/, NULL, 0x00/*dummy*/);
    ZGPL_FlushTxQueue();
  }
  return status;
}
/**************************************************************************//**
\brief dstub data req handling on receiving gp response
\param [in] dataFrame -dataFrame info
******************************************************************************/
static bool dstubHandlingForRxdGpResponse(void* dataFrame, bool *firstToForward)
{
  ZGP_GpResp_t *gpResponse = (ZGP_GpResp_t *)dataFrame;
  ZGP_GpdfDataReq_t *dStubDataReq = (ZGP_GpdfDataReq_t *)zgpGetMemReqBuffer();
  ShortAddr_t proxyNwkAddr = 0;
  uint16_t proxyOperatingChannel = NWK_GetLogicalChannel();

  *firstToForward = false;
  uint8_t dstubDataReqPayload[MAX_PAYLOAD_BY_GPD];

  memset(&dstubDataReqPayload[0], 0x00, sizeof(dstubDataReqPayload));
  // Init. dstub data req payload buffer
  dStubDataReq->gpdAsdu = (uint8_t *)&dstubDataReqPayload[0];
  dStubDataReq->applicationId = (ZGP_ApplicationId_t)gpResponse->options.appId;
  dStubDataReq->txOptions.gpdfFrameType = ZGP_FRAME_DATA;
  if(ZGP_SRC_APPID == dStubDataReq->applicationId)
  {
    dStubDataReq->srcId = gpResponse->gpdId.gpdSrcId;
    if (!dStubDataReq->srcId)
    {
      if ((OPERATIONAL_MODE == ZGPL_GetDeviceMode(true)))
      {
        zgpFreeMemReqBuffer();
        return false;
      }
      dStubDataReq->txOptions.gpdfFrameType = ZGP_FRAME_MAINTENANCE;
    }
  }
  else if(ZGP_IEEE_ADDR_APPID == dStubDataReq->applicationId)
  {
    dStubDataReq->gpdIeeeAddress = gpResponse->gpdId.gpdIeeeAddr;
    dStubDataReq->endpoint = gpResponse->endpoint;
  }

  // check tempMaster short address is the proxy nwk address
  proxyNwkAddr = NWK_GetShortAddr();
  if(proxyNwkAddr == gpResponse->tempMasterShortAddr /*&& false == gpResponse->nonUnicast*/)
  {
    dStubDataReq->action = true;
    dStubDataReq->txOptions.txOnMatchingEndpoint = gpResponse->options.txOnEndpointMatch; // for App id 2
    dStubDataReq->txOptions.requireMacAck = false;
    dStubDataReq->txOptions.performCsma = false;
    dStubDataReq->frameDir = ZGP_TX_TO_ZGPD;
    dStubDataReq->gpdCommandId = (ZGP_CommandId_t)gpResponse->gpdCmdId;
    memcpy(dStubDataReq->gpdAsdu, gpResponse->gpdCmdPayload, gpResponse->gpdCmdPayloadLength);
    dStubDataReq->gpdAsduLength = gpResponse->gpdCmdPayloadLength;
    dStubDataReq->gpTxQueueEntryLifeTime = DEF_QUEUE_ENTRY_LIFETIME;
    gpResponse->tempMasterTxChannel =  (gpResponse->tempMasterTxChannel & 0x0f) + ZGP_CHANNEL_OFFSET;
    if ((proxyOperatingChannel != gpResponse->tempMasterTxChannel) && \
        (dStubDataReq->gpdCommandId == ZGP_CHANNEL_CONFIG_CMD_ID))
    {
      if (!zgpGenericChannelChangeHandling(gpResponse->tempMasterTxChannel))
      {
        zgpFreeMemReqBuffer();
        return false;
      }
    }
    *firstToForward = true;

  }
  else /*if(proxyNwkAddr != gpResponse->tempMasterShortAddr)*/
  {
    dStubDataReq->action = false;
    dStubDataReq->txOptions.txOnMatchingEndpoint = gpResponse->options.txOnEndpointMatch; // for App id 2
  }

  ZGPL_GpdfDataRequest(dStubDataReq);
  zgpFreeMemReqBuffer();

  return true;
}

/**************************************************************************//**
\brief process proxy table on receiving gp response command
\param [in] dataFrame -dataFrame info
******************************************************************************/
static void proxyTableHandlingForRxdGpResponse(void* dataFrame, bool firstToForward)
{
  ZGP_GpResp_t *gpResponse = (ZGP_GpResp_t *)dataFrame;
  ZGP_ProxyTableEntry_t *proxyTableEntry = (ZGP_ProxyTableEntry_t *)zgpGetMemReqBuffer();
  ZGP_TableOperationField_t tableOperationField = {.entryType = ZGP_PROXY_ENTRY, .commMode = ALL_COMMUNICATION_MODE, .appId = gpResponse->options.appId,\
                                                    .nonEmptyIndexForRead = ZGP_ENTRY_INVALID_INDEX};
  ZGP_ReadOperationStatus_t entryStatus;

  entryStatus = ZGPL_ReadTableEntryFromNvm((void *)proxyTableEntry, tableOperationField, &gpResponse->gpdId, gpResponse->endpoint);
  // Since proxyTableEntry->options.firstToFoward is of 1 bit size, typecasting to bool while comparing.
  if ((ENTRY_NOT_AVAILABLE != entryStatus) && (firstToForward != (bool)proxyTableEntry->options.firstToFoward))
  {
    proxyTableEntry->options.firstToFoward = firstToForward;
    // Need to update the entry
    ZGPL_AddOrUpdateTableEntryOnNvm((void *)proxyTableEntry, UPDATE_ENTRY, ZGP_PROXY_ENTRY);
  }
  zgpFreeMemReqBuffer();
}

/**************************************************************************//**
\brief process gp response command
\param [in] dataFrame -dataFrame info
******************************************************************************/
static void zgppProcessProxyGpRespCmd(void* dataFrame)
{
  bool firstToFwd = false;

  if (dstubHandlingForRxdGpResponse(dataFrame, &firstToFwd))
    proxyTableHandlingForRxdGpResponse(dataFrame, firstToFwd);
}

/**************************************************************************//**
\brief check for proxy table matchign rxd data and fwd to sink in opera. mode

\param[in] dataFrameType - type of data frame
           dataFrame - data frame info
******************************************************************************/
static void checkForEntryAndForwardInOperationalMode(zgpProxyDataFrameType_t dataFrameType, void *dataFrame)
{
  ZGP_LowDataInd_t *dstubInd = (ZGP_LowDataInd_t *)dataFrame;
  ZGP_ProxyTableEntry_t *proxyTableEntry;
  ZGP_GpdId_t gpdId;
  ZGP_ReadOperationStatus_t entryStatus;
  ZGP_TableOperationField_t tableOperationField = {.entryType = ZGP_PROXY_ENTRY, .commMode = ALL_COMMUNICATION_MODE, .appId = dstubInd->applicationId, \
  .nonEmptyIndexForRead = ZGP_ENTRY_INVALID_INDEX};

  gpdId.gpdSrcId = dstubInd->srcId;
  if (ZGP_IEEE_ADDR_APPID == dstubInd->applicationId)
    gpdId.gpdIeeeAddr = dstubInd->srcAddress.ext;

  if(!zgpGenericNotInTransmitChannnel())
    return;

  proxyTableEntry = (ZGP_ProxyTableEntry_t *)zgpGetMemReqBuffer();
  entryStatus = ZGPL_ReadTableEntryFromNvm((void *)proxyTableEntry, tableOperationField, &gpdId, dstubInd->endPoint);

  if (ACTIVE_ENTRY_AVAILABLE == entryStatus)
  {
    if (((uint8_t)dstubInd->status == ZGP_DSTUB_SECURITY_SUCCESS) || \
        ((uint8_t)dstubInd->status == ZGP_DSTUB_NO_SECURITY))
    {
      uint32_t frameCounter = dstubInd->gpdSecurityFrameCounter;
      proxyTableEntry->tableGenericInfo.gpdSecurityFrameCounter = dstubInd->gpdSecurityFrameCounter;
      // Update the frameCounter
      ZGPL_FrameCounterReadorUpdateOnNvm(&frameCounter, tableOperationField, &gpdId, dstubInd->endPoint, \
                                        true);
    }

    if (!proxyTableEntry->options.inRange)
    {
      //update the entry
      proxyTableEntry->options.inRange = 1;
      ZGPL_AddOrUpdateTableEntryOnNvm((void *)proxyTableEntry, UPDATE_ENTRY, ZGP_PROXY_ENTRY);
    }

    if (dataFrameType != GPDF_DATA_FRAME)
      dstubInd->rxAfterTx = false;

    zgpClientSendNotification(false, (void *)proxyTableEntry, \
                              dstubInd);
  }
  zgpFreeMemReqBuffer();
}
/**************************************************************************//**
\brief check for proxy table matchign rxd data and fwd to sink in comm. mode

\[param[in]frameType -frame type
           dstubInd -dstub indication
******************************************************************************/
static void checkAndForwardInCommissioningMode(zgpProxyDataFrameType_t frameType, ZGP_LowDataInd_t *dstubInd)
{
  if(!zgpGenericNotInTransmitChannnel())
    return;

  if ((GPDF_CHANNEL_REQUEST_FRAME == frameType) || \
      ((GPDF_COMMISSIONING_FRAME == frameType) && (dstubInd->rxAfterTx)))
  {
    dstubInd->rxAfterTx = true;
  }
  else if ((GPDF_COMMISSIONING_SUCCESS_FRAME == frameType) && (!dstubInd->rxAfterTx))
  {
    ZGP_ProxyTableEntry_t *proxyTableEntry;
    ZGP_GpdId_t gpdId;
    ZGP_ReadOperationStatus_t entryStatus;
    ZGP_TableOperationField_t tableOperationField = {.entryType = ZGP_PROXY_ENTRY, .commMode = ALL_COMMUNICATION_MODE, .appId = dstubInd->applicationId, \
    .nonEmptyIndexForRead = ZGP_ENTRY_INVALID_INDEX};

    gpdId.gpdSrcId = dstubInd->srcId;
    if (ZGP_IEEE_ADDR_APPID == dstubInd->applicationId)
      gpdId.gpdIeeeAddr = dstubInd->srcAddress.ext;
    proxyTableEntry = (ZGP_ProxyTableEntry_t *)zgpGetMemReqBuffer();
    entryStatus = ZGPL_ReadTableEntryFromNvm((void *)proxyTableEntry, tableOperationField, &gpdId, dstubInd->endPoint);

    if (ACTIVE_ENTRY_AVAILABLE == entryStatus)
    {
      if ((uint8_t)dstubInd->status == ZGP_DSTUB_SECURITY_SUCCESS)
      {
        uint32_t frameCounter = dstubInd->gpdSecurityFrameCounter;
        proxyTableEntry->tableGenericInfo.gpdSecurityFrameCounter = dstubInd->gpdSecurityFrameCounter;
        // Update the frameCounter
        ZGPL_FrameCounterReadorUpdateOnNvm(&frameCounter, tableOperationField, &gpdId, dstubInd->endPoint, \
                                        true);
      }
    }
  }
   zgpClientSendNotification(true, (void *)&zgpProxyBasicModeInfo, \
                                         dstubInd);
   zgpFreeMemReqBuffer();
}

/**************************************************************************//**
\brief To get sink network address in unicast commissioning mode
******************************************************************************/
uint16_t zgpProxyBasicGetSinkNwkAddr(void)
{
  return zgpProxyBasicModeInfo.sinkAddr;
}
#endif // #if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
#endif // _GREENPOWER_SUPPORT_

// eof zgpProxyBasic.c
