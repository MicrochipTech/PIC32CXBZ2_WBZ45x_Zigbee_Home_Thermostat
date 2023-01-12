/*******************************************************************************
  Zigbee green power higher generic Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpHighGeneric.c

  Summary:
    This file contains the zgp High generic feature implementation.

  Description:
    This file contains the zgp High generic feature implementation.
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
#include <zgp/GPInfrastructure/highZgp/include/private/zgpProxyTable.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpProxyBasic.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterClient.h>
#include <zdo/include/zdo.h>
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
#include <zgp/GPInfrastructure/highZgp/include/private/zgpSinkBasic.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpSinkTable.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterServer.h>
#endif
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighMem.h>
#include <security/serviceprovider/include/sspHash.h>
#if MICROCHIP_APPLICATION_SUPPORT == 1
#include <bdb/include/bdb.h>
#else
#include <zllplatform/zllplatform/ZLL/N_DeviceInfo/include/N_DeviceInfo_Bindings.h>
#include <zllplatform/ZLL/N_DeviceInfo/include/N_DeviceInfo.h>
#endif

/******************************************************************************
                    Defines section
******************************************************************************/
#define CHANNEL_CHANGE_TIMEOUT        5000UL

/******************************************************************************
                    Types section
******************************************************************************/


/******************************************************************************
                    Prototypes section
******************************************************************************/

static void channelChangeTimerFired(void);
static void dStubChannelConfigTxInd(void);
static void zgpSetChannel(uint8_t channel);
static void zgpSetChannelConf(MAC_SetConf_t *conf);
static void dstubDataIndObserver(SYS_EventId_t id, SYS_EventData_t data);
static void dstubDataConfObserver(SYS_EventId_t id, SYS_EventData_t data);

/******************************************************************************
                   Static variables section
******************************************************************************/
static HAL_AppTimer_t channelChangeTimer =
{
  .mode     = TIMER_ONE_SHOT_MODE,
  .interval = CHANNEL_CHANGE_TIMEOUT,
  .callback = channelChangeTimerFired
};

static zgp_MacReq_t zgpMacReq;

static SYS_EventReceiver_t dstubDataIndReceiver = { .func = dstubDataIndObserver};
static SYS_EventReceiver_t dstubDataConfReceiver = { .func = dstubDataConfObserver};

/******************************************************************************
                    Implementations section
******************************************************************************/

/**************************************************************************//**
\brief Initialize High zgp generic feature.
******************************************************************************/
void zgpHighGenericInit(void)
{
  SYS_SubscribeToEvent(BC_EVENT_ZGPL_GPDF_INDICATION, &dstubDataIndReceiver);
  zgpMacReq.inTransmitChannel = false;
}

/**************************************************************************//**
\brief Handling channel change request from proxy/sink basic

\param[in] tempChannel - temporary channel
\return true\false
******************************************************************************/
bool zgpGenericChannelChangeHandling(uint8_t tempChannel)
{
  // This is applicable only for channel config
  // Channel should switch back after 5 sec.
  if (!zgpMacReq.inTransmitChannel)
  {
    zgpSetChannel(tempChannel);
    HAL_StopAppTimer(&channelChangeTimer);
    HAL_StartAppTimer(&channelChangeTimer);
    SYS_SubscribeToEvent(BC_EVENT_ZGPL_GPDF_TX_CONFIRM, &dstubDataConfReceiver);
    return true;
  }
  return false;
}

/**************************************************************************//**
  \brief Sets the current channel.

  \param[in] channel - the channel to switch to.
******************************************************************************/
static void zgpSetChannel(uint8_t channel)
{
  zgpMacReq.channelSet.attrId.phyPibId = PHY_PIB_CURRENT_CHANNEL_ID;
  zgpMacReq.channelSet.attrValue.phyPibAttr.channel = channel;
  zgpMacReq.channelSet.MAC_SetConf = zgpSetChannelConf;
  MAC_SetReq(&zgpMacReq.channelSet);
}

/**************************************************************************//**
  \brief Confirmation on MAC channel setting.

  \param[in] conf - MAC-Set.Confirm parameters.
******************************************************************************/
static void zgpSetChannelConf(MAC_SetConf_t *conf)
{
  uint16_t proxyOperatingChannel = NWK_GetLogicalChannel();
  MAC_SetReq_t *setReq = GET_PARENT_BY_FIELD(MAC_SetReq_t,
	  confirm, conf);

  if (setReq->attrValue.phyPibAttr.channel == proxyOperatingChannel)
    zgpMacReq.inTransmitChannel = false;
  else
    zgpMacReq.inTransmitChannel = true;
}

/**************************************************************************//**
\brief Transmit channel check - To Check proxy/Sink is in transmit channel
******************************************************************************/
bool zgpGenericNotInTransmitChannnel(void)
{
  // Drop the all the frames except channel request in transmit channel
  // In case of channel req., only maintenance frame with autocommissioing false is allowed
  // As per spec A.3.9.1 step-9
  if (zgpMacReq.inTransmitChannel)
     return false;

  return true;
}

/**************************************************************************//**
\brief callback from dstub after sending channel config
******************************************************************************/
static void dStubChannelConfigTxInd(void)
{
  uint16_t proxyOperatingChannel = NWK_GetLogicalChannel();

  HAL_StopAppTimer(&channelChangeTimer);
  if (zgpMacReq.inTransmitChannel)
    zgpSetChannel(proxyOperatingChannel);
  SYS_UnsubscribeFromEvent(BC_EVENT_ZGPL_GPDF_TX_CONFIRM, &dstubDataConfReceiver);
}

/**************************************************************************//**
\brief callback from channel change timer(5 sec.)
******************************************************************************/
static void channelChangeTimerFired(void)
{
  uint16_t proxyOperatingChannel = NWK_GetLogicalChannel();
  ZGP_GpdfDataReq_t *dStubDataReq = (ZGP_GpdfDataReq_t *)zgpGetMemReqBuffer();

  zgpSetChannel(proxyOperatingChannel);
  // Need to remove the entry from gpTxQueue on timer expiry as per spec A.3.9.1 step-9
  dStubDataReq->applicationId = ZGP_SRC_APPID;
  dStubDataReq->srcId = 0x0000000;
  dStubDataReq->action = false;
  //dStubDataReq->zgpDstubDataConf = NULL;
  // Since action is false, dStubDataReq->gpAsdu(payload) won't be used
  // so not initializ
  ZGPL_GpdfDataRequest(dStubDataReq);
  zgpFreeMemReqBuffer();
}

/**************************************************************************//**
\brief Event Indication callback for dStubDataTX.

\param[in] id - Event ID
           data - Event Data

\return   None
******************************************************************************/
static void dstubDataConfObserver(SYS_EventId_t id, SYS_EventData_t data)
{
  ZGP_GpdfTxConfirm_t *txConfirm = (ZGP_GpdfTxConfirm_t *)data;

  if (ZGP_CHANNEL_CONFIG_CMD_ID == txConfirm->cmdId)
  {
    dStubChannelConfigTxInd();
  }
}

/**************************************************************************//**
\brief Event Indication callback for dStubDataRX.

\param[in] id - Event ID
           data - Event Data

\return   None
******************************************************************************/
static void dstubDataIndObserver(SYS_EventId_t id, SYS_EventData_t data)
{
#if MICROCHIP_APPLICATION_SUPPORT == 1
  if(!BDB_GetBdbNodeIsOnANetwork())
#else 
  if (N_DeviceInfo_IsFactoryNew())
#endif
    return;
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
  zgpSinkDstubDataInd((ZGP_LowDataInd_t *)data);
#endif  
  zgpProxyDstubDataInd((ZGP_LowDataInd_t*)data);
  (void)id;
}

#endif // APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
#endif // _GREENPOWER_SUPPORT_

// eof zgpHighGeneric.c
