/*******************************************************************************
  Zigbee green power Dedicated stub Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpDstub.c

  Summary:
    This file contains the dedicated GP stub implementation.

  Description:
    This file contains the dedicated GP stub implementation.
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

#if (APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC)
/******************************************************************************
                   Includes section
******************************************************************************/
#include <zgp/include/zgpCommon.h>
#include <mac_phy/mac_env/include/macenvPib.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpDstub.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpCstub.h>
#include <zgp/include/zgpDbg.h>
#include <security/serviceprovider/include/sspSfp.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpPacket.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpTaskManager.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLow.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpLowGeneric.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowGpdf.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowNvmTable.h>
#if MICROCHIP_APPLICATION_SUPPORT == 1
#include <zllplatform/ZLL/N_DeviceInfo/include/N_DeviceInfo_Bindings.h>
#include <zllplatform/ZLL/N_DeviceInfo/include/N_DeviceInfo.h>
#endif
#include <systemenvironment/include/sysAssert.h>

/******************************************************************************
                            Definitions section.
******************************************************************************/
#define ZGP_DSTUB_TX_OFFSET   20
#define ZGP_GPDF_RETREIS  2
// If the expected app timer offset is close to 10ms(>= 7ms), then
// we can rely on appTimer interval so that we can meet gpTxOffset of 23-21
// ohterwise we can procced with the current interval with the task checking for
// offset time
#define ZGP_APP_TIME_THRESHOLD_FOR_TASK 6u
#define ZGP_DSTUB_APPROX_ENCRYPTION_TIME_IN_MSEC 4

/******************************************************************************
                   Static functions prototype section
******************************************************************************/
static void cstubDataInd(MAC_DataInd_t *indParams);
static void zgpTxQueueProcessing(ZGP_GpdfDataReq_t *dstubDataReq);
static void zgpFillCstubReq(zgpPacket_t *outPkt, ZGP_GpdfDataReq_t *dstubReq);
static void zgpDstubTxTimerHandler(void);
static zgpPacket_t* zgpGetMatchingTxQueueEntry(ZGP_ApplicationId_t appId, \
                          ZGP_SourceId_t srcId, ExtAddr_t ieeeAddr, uint8_t endPoint);
static bool zgpSendPacket(zgpPacket_t *outPkt);
static void zgpMacDataReqConfirm(MAC_DataConf_t *conf);
static void zgpSspEncryptconfirm(SSP_ZgpEncryptFrameConf_t *conf);
static void zgpDstubEncryptOutPacket(zgpPacket_t *outPacket);
static void zgpSspDecryptconfirm(SSP_ZgpDecryptFrameConf_t *conf);
static void zgpDstubDecryptInPacket(zgpPacket_t *inPkt);
static bool zgpProcessRxdFrame(zgpPacket_t *pkt);
static bool processMaintenanceFrame(zgpPacket_t *packet);
static bool securityProcessingOfRxdGpdf(zgpPacket_t *inPkt);
static void startGpTxOffsetTimer(uint8_t processingTime);
static bool parseDataIndFields(zgpPacket_t *inPkt, MAC_DataInd_t *indParams);
static void processIncomingGpdf(zgpPacket_t *packet);
static bool processDataFrame(zgpPacket_t *inPkt);
static void processPendingRxGpdfFrames(void);
static void getMatchingTxQueueEntryAndSend(zgpPacket_t *inputPkt);
static bool operatingOnNwkChannel(void);

/******************************************************************************
                   Global variables section
******************************************************************************/

/******************************************************************************
                           Types section
******************************************************************************/

/******************************************************************************
                   Static variables section
******************************************************************************/
static DECLARE_QUEUE(zgpDstubTxQueue);
static DECLARE_QUEUE(zgpDstubRxQueue);

static HAL_AppTimer_t zgpDstubTxTimer =
{
  .mode     = TIMER_ONE_SHOT_MODE,
  .callback = zgpDstubTxTimerHandler
};
static zgpPacket_t* outPktInProgress;
static uint8_t txTimerOffsetInms = 0;
static uint8_t noOfGpdfTxRetries = 0;
static BcTime_t taskPostSystemTime;
static bool enableDirectMode = true;

/******************************************************************************
                   Implementation section
******************************************************************************/
/**************************************************************************//**
  \brief Init. dGP stub.

  \param none.
  \return none.
******************************************************************************/
void zgpDstubInit(void)
{
  zgpResetPacketManager();
  // Registering callback for cgpDataInd to cStub
  zgpCstubGpDataIndRegisterCallback(cstubDataInd);
  zgpCstubInit();
}

/**************************************************************************//**
  \brief cStub data indication to dstub

  \param[in] indParams - CGP-DATA indication primitive's parameters, see
      ZGP spec.
  \return None.
 ******************************************************************************/
static void cstubDataInd(MAC_DataInd_t *indParams)
{
  zgpPacket_t *inPkt;

  if (!enableDirectMode)
    return;

//  if (N_DeviceInfo_IsFactoryNew() || (indParams->msduLength < ZGP_BASIC_FRAME_LENGTH))
  if (indParams->msduLength < ZGP_BASIC_FRAME_LENGTH)
  {
    return;
  }
  // Allocation of the received frame
  inPkt = zgpAllocPacket(ZGP_DSTUB_INPUT_DATA_PACKET);
  if (inPkt)
  {
    if (parseDataIndFields(inPkt, indParams))
    {
      putQueueElem(&zgpDstubRxQueue, inPkt);
      processIncomingGpdf(inPkt);
    }
    else
    {
      inPkt->state = ZGP_PACKET_IDLE;
      zgpFreeInOutPacket((void *)&inPkt->pkt.in);
    }
    
  }
}

/**************************************************************************//**
  \brief Security processing of the rxd data frame

  \param[in] secReqProcessing - pointer to secRequestProcessing

  \return true - Proceed with processing data gdpf
          false - Don't proceed with prrocessing data gpdf
 ******************************************************************************/
static bool securityProcessingOfRxdGpdf(zgpPacket_t *inPkt)
{
  zgpDstubSecResponse_t secResponse;
  ZGP_LowDataInd_t *dstubDataInd = (ZGP_LowDataInd_t *)&inPkt->pkt.in.zgpDataInd;

  zgpGenericSecRequest(dstubDataInd, &secResponse);
  dstubDataInd->gpdfKeyType = secResponse.gpdfKeyType;
  inPkt->pkt.in.secRspCode = secResponse.status;
  switch (secResponse.status)
  {
    case ZGP_DSTUB_MATCH:
    case ZGP_DSTUB_TX_THEN_DROP:
      if (!dstubDataInd->gpdfSecurityLevel)
      {
        dstubDataInd->status = ZGP_DSTUB_NO_SECURITY;
      }
      else
      {
        // By default we set to ZGP_DSTUB_SECURITY_SUCCESS
        // Will be updated later after decryption
        dstubDataInd->status = ZGP_DSTUB_SECURITY_SUCCESS;
        memcpy((void *)&inPkt->pkt.in.secKey[0], (void *)&secResponse.gpdKey[0], sizeof(ZGP_Security_Key_t));
      }
    break;
    case ZGP_DSTUB_DROP_FRAME:
      // dropped
      return false;
    break;
    case ZGP_DSTUB_PASS_UNPROCESSED:
      // Forward the packet without security processing
      dstubDataInd->status = ZGP_DSTUB_UNPROCESSED;
    break;
  }
  return true;
}

/**************************************************************************//**
  \brief Process the rxd maintenance frame

  \param[in] dstubDataInd - dstub data indication
             forwardToUpperLayer -  flag indicating forwarding rxd GPDF to upper layer

  \return true - Proceed with processing maintenance frame
          false - Don't proceed with prrocessing maintenance frame
 ******************************************************************************/
static bool processMaintenanceFrame(zgpPacket_t *inPkt)
{
  ZGP_LowDataInd_t *dstubDataInd = &inPkt->pkt.in.zgpDataInd;

  if (inPkt->pkt.in.service.rxdFrameLength < ZGP_MAINTENACE_FRAME_CMD_LENGTH )
    return false;

  dstubDataInd->gpdCommandId = (uint8_t)*inPkt->pkt.in.service.rxdFramePtr++;
  inPkt->pkt.in.service.rxdFrameLength--;

  if ((dstubDataInd->gpdCommandId != ZGP_CHANNEL_REQUEST_CMD_ID) || \
      ((OPERATIONAL_MODE == ZGPL_GetDeviceMode(true))
#if (APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC)
       && (OPERATIONAL_MODE == ZGPL_GetDeviceMode(false))
#endif
                                   ))
    return false;

  dstubDataInd->applicationId = ZGP_SRC_APPID;
  dstubDataInd->srcId = ZGP_MAINTENANCE_FRAME_SRC_ID;
  dstubDataInd->gpdSecurityFrameCounter = dstubDataInd->seqNumber;
  dstubDataInd->gpdfSecurityLevel = ZGP_SECURITY_LEVEL_0;
  dstubDataInd->gpdfKeyType = ZGP_KEY_TYPE_NO_KEY;
  dstubDataInd->status = ZGP_DSTUB_NO_SECURITY;
  inPkt->pkt.in.zgpDataInd.gpdAsduLength = inPkt->pkt.in.service.rxdFrameLength;

  if (inPkt->pkt.in.service.rxdFrameLength)
    memcpy(inPkt->pkt.in.zgpDataInd.gpdAsdu, inPkt->pkt.in.service.rxdFramePtr, inPkt->pkt.in.service.rxdFrameLength);

  if (!dstubDataInd->autoCommissioning)
  {
    getMatchingTxQueueEntryAndSend(inPkt);
  }
  else
    inPkt->state = ZGP_PACKET_INPKT_INDICATION_TO_HIGHER_LAYER;

  if (operatingOnNwkChannel())
    return true;
  else
    return false;
}

/**************************************************************************//**
  \brief timer handler for GPDF tx offset and tx duration

  \return None.
 ******************************************************************************/
static void zgpDstubTxTimerHandler(void)
{
  noOfGpdfTxRetries = 0;

  if (!txTimerOffsetInms)
  {
    MAC_DataReq(&outPktInProgress->pkt.out.macDataReq);
  }
  else
  {
    taskPostSystemTime = HAL_GetSystemTime();
    // post the task
    zgpPostTask(ZGP_DSTUB_TX_GPDF);
  }

}

/**************************************************************************//**
  \brief GP-Data-handler for indication

  \param[in] None
  \return None.
 ******************************************************************************/
void zgpDstubRxIndicationHandler(void)
{
  zgpPacket_t *inPkt = (zgpPacket_t *)getQueueElem(&zgpDstubRxQueue);

  while (inPkt)
  {
    if (ZGP_PACKET_INPKT_INDICATION_TO_HIGHER_LAYER == inPkt->state)
    {
      SYS_PostEvent(BC_EVENT_ZGPL_GPDF_INDICATION, (SYS_EventData_t)&inPkt->pkt.in.zgpDataInd);
      inPkt->state = ZGP_PACKET_IDLE;
      processIncomingGpdf(inPkt);
      // Need to send one indication at a time for processing
      if (getQueueElem(&zgpDstubRxQueue))
      {
        zgpPostTask(ZGP_DSTUB_DATA_INDICATION);
        return;
      }
    }
    inPkt = (zgpPacket_t *)getNextQueueElem(inPkt);
  }
}

/**************************************************************************//**
  \brief task handler for tx GPDF

  \return None.
 ******************************************************************************/
void zgpDstubTxTaskHandler(void)
{
  uint8_t elapsedTime = HAL_GetElapsedAppTimerTimeSinceLastIsr()/1000;
  BcTime_t currentSysTime = HAL_GetSystemTime();

  if ((currentSysTime > taskPostSystemTime) || (txTimerOffsetInms <= elapsedTime))
  {
    MAC_DataReq(&outPktInProgress->pkt.out.macDataReq);
  }
  else
    zgpPostTask(ZGP_DSTUB_TX_GPDF);
}

/**************************************************************************//**
  \brief Process the pending rx GPDFs after current Tx complete

  \param[in] conf - data confirmation structure
  \return None.
 ******************************************************************************/
static void processPendingRxGpdfFrames(void)
{
  zgpPacket_t *inPkt = (zgpPacket_t *)getQueueElem(&zgpDstubRxQueue);

  while (inPkt)
  {
    // Need to check any security processing in place
    // if so should not forward
    if ((ZGP_PAKCET_INPKT_INDICATION_ON_HOLD == inPkt->state) || \
        (ZGP_PACKET_INPKT_RESPONSE_SENDING_IN_PROCESS == inPkt->state))
    {
      inPkt->state = ZGP_PACKET_INPKT_INDICATION_TO_HIGHER_LAYER;
      processIncomingGpdf(inPkt);
    }
    inPkt = (zgpPacket_t *)getNextQueueElem(inPkt);
  } 
}

/**************************************************************************//**
  \brief confirmation of MAC data req

  \param[in] conf - data confirmation structure
  \return None.
 ******************************************************************************/
static void zgpMacDataReqConfirm(MAC_DataConf_t *conf)
{
  zgpOutputPacket_t *outputPkt = GET_PARENT_BY_FIELD(zgpOutputPacket_t,
	  macDataReq.confirm, conf);
  zgpPacket_t *pkt = GET_PARENT_BY_FIELD(zgpPacket_t,
	  pkt.out, outputPkt);

  noOfGpdfTxRetries++;
  if (noOfGpdfTxRetries >= ZGP_GPDF_RETREIS)
  {
    ZGP_GpdfTxConfirm_t confirmStatus;
    // need to raise the callback for the packet sent out
    // so that proxy will switch back to operational channel
    confirmStatus.cmdId = outputPkt->zgpDstubDataReq.gpdCommandId;
    confirmStatus.status = MAC_SUCCESS_STATUS;
    SYS_PostEvent(BC_EVENT_ZGPL_GPDF_TX_CONFIRM, (SYS_EventData_t)&confirmStatus);

    // Deallocating the output packet after sent out
    deleteQueueElem(&zgpDstubTxQueue, pkt);
    pkt->state = ZGP_PACKET_IDLE;
    zgpFreeInOutPacket(outputPkt);
    outPktInProgress = NULL;
    // Need to check input packet and forward the frame to upper layer
    processPendingRxGpdfFrames();
  }
  else
  {
    // Resend the GPDF
    MAC_DataReq(&outPktInProgress->pkt.out.macDataReq);
  }
}
/**************************************************************************//**
  \brief To raise the request for decryption of incoming Dstub data packet

  \param[in] resp - pointer to ZGP DSTUB-DATA.response parameters structure.
  \return None.
 ******************************************************************************/
static void zgpDstubDecryptInPacket(zgpPacket_t *inPkt)
{
  ZGP_LowDataInd_t *dstubDataInd = &inPkt->pkt.in.zgpDataInd;

  {
    // extract the mic
    memcpy((void *)&dstubDataInd->mic, \
    (inPkt->pkt.in.service.rxdFramePtr + inPkt->pkt.in.service.rxdFrameLength - sizeof(dstubDataInd->mic)), \
    sizeof(dstubDataInd->mic));

    inPkt->state = ZGP_PACKET_INPKT_SECURITY_IN_PROCESS;
    inPkt->pkt.in.zgpDecryptReq.pdu = inPkt->macPayload;
    inPkt->pkt.in.zgpDecryptReq.securityFrameCounter = dstubDataInd->gpdSecurityFrameCounter;//dstubSecRsp->gpdSecurityFrameCounter;
    inPkt->pkt.in.zgpDecryptReq.securityLevel = dstubDataInd->gpdfSecurityLevel;//dstubSecRsp->gpdfSecurityLevel;
    inPkt->pkt.in.zgpDecryptReq.appId = dstubDataInd->applicationId;
    inPkt->pkt.in.zgpDecryptReq.extAddr = 0x00;
    if (ZGP_IEEE_ADDR_APPID == inPkt->pkt.in.zgpDecryptReq.appId)
      inPkt->pkt.in.zgpDecryptReq.extAddr = dstubDataInd->srcAddress.ext;
    else if(ZGP_SRC_APPID == inPkt->pkt.in.zgpDecryptReq.appId)
      inPkt->pkt.in.zgpDecryptReq.srcID = dstubDataInd->srcId;

    inPkt->pkt.in.zgpDecryptReq.macHeader[0] = 0x00;
    inPkt->pkt.in.zgpDecryptReq.macHeader[1] = 0x00;
    inPkt->pkt.in.zgpDecryptReq.macHeader[2] = 0x00;
    inPkt->pkt.in.zgpDecryptReq.macHeader[3] = 0x00;
    // For secured packets, dstubDataInd->gpdAsduLength has the mac payload length
    // mac payload length - remaining length gives the header length
    inPkt->pkt.in.zgpDecryptReq.headerLength = dstubDataInd->gpdAsduLength - inPkt->pkt.in.service.rxdFrameLength;

    // Complete payload needs to be copied - header + payload(command Id + command payload) + MIC
    memcpy(inPkt->pkt.in.zgpDecryptReq.pdu, /*payLoad*/ inPkt->pkt.in.service.rxdFramePtr - inPkt->pkt.in.zgpDecryptReq.headerLength, dstubDataInd->gpdAsduLength/*payLoadLength + inPkt->pkt.in.zgpDecryptReq.headerLength + sizeof(dstubDataInd->mic)*/);

    // This is payload length i.e. command id + command payload not including MIC
    inPkt->pkt.in.zgpDecryptReq.payloadLength = inPkt->pkt.in.service.rxdFrameLength;
    inPkt->pkt.in.zgpDecryptReq.dir = ZGP_TX_BY_ZGPD;
    inPkt->pkt.in.zgpDecryptReq.SSP_ZgpDecryptFrameConf = zgpSspDecryptconfirm;
    SSP_ZgpDecryptFrameReq(&inPkt->pkt.in.zgpDecryptReq);
    // Updating the proper gsdu length which is used as macpayloadLength previously
    dstubDataInd->gpdAsduLength = inPkt->pkt.in.service.rxdFrameLength - sizeof(dstubDataInd->mic) - ZGP_CMD_ID_LENGTH/*1 byte*/;
  }
}

/**************************************************************************//**
  \brief To send out GPDF

  \param[in] outPkt - pointer to output packet.
  \return true - packet is scheduled to be sent
          false - already sending in progress
 ******************************************************************************/
static bool zgpSendPacket(zgpPacket_t *outPkt)
{
  // For proxy basic, we are maintaining multiple instance of gpTxOffset
  // When we implement proxy advanced, we may need to support multiple instances of
  // gpTxOffset to service multiple GPDs in parallel
  if (!outPktInProgress)
  {
    outPktInProgress = outPkt;

    if (ZGP_SECURITY_LEVEL_0 != outPkt->pkt.out.zgpEncryptReq.securityLevel)
      // trigger security processing(encrypting the frame to be sent)
      zgpDstubEncryptOutPacket(outPkt);
    else
    {
      // no security processing so processing time 0
      startGpTxOffsetTimer(0);
    }
    return true;
  }
  else
    return false;
}

/**************************************************************************//**
  \brief To start timer for gpTxOffset

  \param[in] processingTime - processing timer already consumed.
  \return None.
 ******************************************************************************/
static void startGpTxOffsetTimer(uint8_t processingTime)
{
  uint8_t elapsedTime;
  uint8_t timerIntervalWithOffset;
  uint8_t txTimerInterval = ZGP_DSTUB_TX_OFFSET - processingTime;

  txTimerOffsetInms = 0;
  elapsedTime = HAL_GetElapsedAppTimerTimeSinceLastIsr()/1000;
  timerIntervalWithOffset = txTimerInterval + elapsedTime;
  zgpDstubTxTimer.interval = (timerIntervalWithOffset / HAL_APPTIMERINTERVAL) * HAL_APPTIMERINTERVAL;

  if ((timerIntervalWithOffset % HAL_APPTIMERINTERVAL) <= ZGP_APP_TIME_THRESHOLD_FOR_TASK)
    txTimerOffsetInms = timerIntervalWithOffset % HAL_APPTIMERINTERVAL;
  else
    zgpDstubTxTimer.interval += HAL_APPTIMERINTERVAL;

  HAL_StartAppTimer(&zgpDstubTxTimer);
}

/**************************************************************************//**
  \brief To raise the request for encryption of outgoing Dstub data packet

  \param[in] resp - pointer to ZGP DSTUB-DATA.response parameters structure.
  \return None.
 ******************************************************************************/
static void zgpDstubEncryptOutPacket(zgpPacket_t *outPacket)
{
  outPacket->pkt.out.zgpEncryptReq.pdu = outPacket->pkt.out.macDataReq.msdu;
  outPacket->pkt.out.zgpEncryptReq.appId = outPacket->pkt.out.zgpDstubDataReq.applicationId;
  outPacket->pkt.out.zgpEncryptReq.srcID = outPacket->pkt.out.zgpDstubDataReq.srcId;
  outPacket->pkt.out.zgpEncryptReq.extAddr = outPacket->pkt.out.zgpDstubDataReq.gpdIeeeAddress;
  outPacket->pkt.out.zgpEncryptReq.dir = ZGP_TX_TO_ZGPD;

  outPacket->pkt.out.zgpEncryptReq.macHeader[0] = 0x00;
  outPacket->pkt.out.zgpEncryptReq.macHeader[1] = 0x00;
  outPacket->pkt.out.zgpEncryptReq.macHeader[2] = 0x00;
  outPacket->pkt.out.zgpEncryptReq.macHeader[3] = 0x00;
  outPacket->pkt.out.zgpEncryptReq.payloadLength = outPacket->pkt.out.zgpDstubDataReq.gpdAsduLength + 1/*command id*/;
  outPacket->pkt.out.zgpEncryptReq.headerLength = outPacket->pkt.out.macDataReq.msduLength - outPacket->pkt.out.zgpDstubDataReq.gpdAsduLength - 1/*command id */;
  outPacket->pkt.out.macDataReq.msduLength += sizeof(uint32_t); // TO include MIC
  outPacket->pkt.out.zgpEncryptReq.SSP_ZgpEncryptFrameConf = zgpSspEncryptconfirm;
  SSP_ZgpEncryptFrameReq(&outPacket->pkt.out.zgpEncryptReq);
}

/******************************************************************************
  \brief  callback from SSP component on completion of Encryption
  \param  confirm - confirm parameters of operation.
  \return none.
******************************************************************************/
static void zgpSspEncryptconfirm(SSP_ZgpEncryptFrameConf_t *conf)
{
  zgpOutputPacket_t *outputPkt = GET_PARENT_BY_FIELD(zgpOutputPacket_t,
	  zgpEncryptReq.confirm, conf);
  zgpPacket_t *pkt = GET_PARENT_BY_FIELD(zgpPacket_t,
	  pkt.out, outputPkt);

  if(conf->status == SSP_SUCCESS_STATUS)
  {
    // Approx. security processing time ~ 4ms
    startGpTxOffsetTimer(ZGP_DSTUB_APPROX_ENCRYPTION_TIME_IN_MSEC);
  }
  else
  {
    // Need to remove the entry from queue and free the buffer
    deleteQueueElem(&zgpDstubTxQueue, pkt);
    pkt->state = ZGP_PACKET_IDLE;
    zgpFreeInOutPacket(outputPkt);
    outPktInProgress = NULL;
    processPendingRxGpdfFrames();
  }
}

/**************************************************************************//**
  \brief process the incoming frame

  \param[in] packet - incoming packet

  \return None.
 ******************************************************************************/
static void processIncomingGpdf(zgpPacket_t *packet)
{
  if (ZGP_PACKET_INPKT_MAINTENANCE_FRAME_PROCESSING == packet->state)
  {
    if (!processMaintenanceFrame(packet))
    {
      packet->state = ZGP_PACKET_IDLE;
    }
  }
  if (ZGP_PACKET_INPKT_DATA_FRAME_PROCESSING == packet->state)
  {
    if (!processDataFrame(packet))
    {
      packet->state = ZGP_PACKET_IDLE;
    }
    
  }
  if (ZGP_PACKET_INPKT_TO_PROCESS_FOR_RESPONSE == packet->state)
  {
    // need to check the tx queue and forward to higher layer
    if (!zgpProcessRxdFrame(packet))
    {
      packet->state = ZGP_PACKET_IDLE;
    }
  }
  if (ZGP_PACKET_INPKT_INDICATION_TO_HIGHER_LAYER == packet->state)
  {
    zgpPostTask(ZGP_DSTUB_DATA_INDICATION);
  }
  if(ZGP_PACKET_IDLE == packet->state)
  {
    deleteQueueElem(&zgpDstubRxQueue, packet);
    zgpFreeInOutPacket((void *)&packet->pkt.in);
  }

}

/**************************************************************************//**
  \brief process the incoming data frame

  \param[in] inPkt - incoming packet

  \return true - valid data frame
          false - invalid frame
 ******************************************************************************/
static bool processDataFrame(zgpPacket_t *inPkt)
{
  ZGP_LowDataInd_t *dstubDataInd = (ZGP_LowDataInd_t *)&inPkt->pkt.in.zgpDataInd;

  if ((inPkt->pkt.in.service.rxdFrameLength < ZGP_CMD_ID_LENGTH) || \
       (ZGP_SECURITY_LEVEL_1 == dstubDataInd->gpdfSecurityLevel) || \
       (dstubDataInd->autoCommissioning && dstubDataInd->rxAfterTx))
    return false;

  // Duplicate filtering needs framecounter so extracting framecounter before
  // duplicate filter processing
  if (dstubDataInd->gpdfSecurityLevel)
  {
    // Need to check for command id + minimum sec. header
    if (inPkt->pkt.in.service.rxdFrameLength < (ZGP_FRAME_MIN_SECURITY_HEADER + ZGP_CMD_ID_LENGTH))
      return false;
    memcpy((void *)&dstubDataInd->gpdSecurityFrameCounter, inPkt->pkt.in.service.rxdFramePtr, sizeof(dstubDataInd->gpdSecurityFrameCounter));
    inPkt->pkt.in.service.rxdFramePtr += sizeof(dstubDataInd->gpdSecurityFrameCounter);
    inPkt->pkt.in.service.rxdFrameLength -= sizeof(dstubDataInd->gpdSecurityFrameCounter);
  }
  else
  {
    dstubDataInd->gpdSecurityFrameCounter = dstubDataInd->seqNumber;
  }

  if (!ZGPL_CheckForDuplicate(dstubDataInd))
    return false;

  // packet is not duplicated so proceed with security processing
  if (!securityProcessingOfRxdGpdf(inPkt))
    return false;

  if ((ZGP_DSTUB_NO_SECURITY == dstubDataInd->status) || \
       (ZGP_DSTUB_UNPROCESSED == dstubDataInd->status))
  {
    // extract the command id
    dstubDataInd->gpdCommandId = (uint8_t)(*inPkt->pkt.in.service.rxdFramePtr);
    inPkt->pkt.in.service.rxdFramePtr++;
    inPkt->pkt.in.service.rxdFrameLength--;

    if (dstubDataInd->gpdfSecurityLevel)
    {
      // This is for UNPROCESSED secured packet
      memcpy((void *)&dstubDataInd->mic, \
       (inPkt->pkt.in.service.rxdFramePtr + inPkt->pkt.in.service.rxdFrameLength - sizeof(dstubDataInd->mic)), \
        sizeof(dstubDataInd->mic));
      inPkt->pkt.in.service.rxdFrameLength -= sizeof(dstubDataInd->mic);
    }
    // extract command payload
    if (inPkt->pkt.in.service.rxdFrameLength)
    {
      memcpy(dstubDataInd->gpdAsdu, inPkt->pkt.in.service.rxdFramePtr, inPkt->pkt.in.service.rxdFrameLength);
    }
    dstubDataInd->gpdAsduLength = inPkt->pkt.in.service.rxdFrameLength;

    if (ZGP_DSTUB_UNPROCESSED == dstubDataInd->status)
    {
      // By default we need to forward all the UNPROCESSED packets to higher layer
      inPkt->state = ZGP_PACKET_INPKT_INDICATION_TO_HIGHER_LAYER;
      if (!dstubDataInd->gpdfSecurityLevel)
      {
        if (ZGP_COMMISSIONING_CMD_ID == dstubDataInd->gpdCommandId || ZGP_APPLICATION_DESCRIPTION_CMD_ID == dstubDataInd->gpdCommandId || (dstubDataInd->gpdCommandId >= ZGP_MANUFAC_SPECIFIC_CMD0_FRAME && dstubDataInd->gpdCommandId <= ZGP_MANUFAC_SPECIFIC_CMDF_FRAME) ||\
            (dstubDataInd->gpdCommandId >= ZGP_RESERVED_CMD1&& dstubDataInd->gpdCommandId <= ZGP_RESERVED_CMDB))
        {
          if (dstubDataInd->autoCommissioning)
            return false;

          if (dstubDataInd->rxAfterTx)
          {
            // Since rxAfterTx is set, we need to check the gpTxQueue
            inPkt->state = ZGP_PACKET_INPKT_TO_PROCESS_FOR_RESPONSE;
          }
        }
        else if(ZGP_COMMISSIONING_CMD_ID > dstubDataInd->gpdCommandId)
        {
          if (dstubDataInd->autoCommissioning)
          {
            if (dstubDataInd->rxAfterTx)
              return false;
          }
          else
            return false;
        }
      }
      else
      {
        // As per A.3.9.2.1, for proxy, if protected gpdf with auto-commissioning is set 
        // and no proxy table entry in commissioning mode, then it should be dropped
        if (dstubDataInd->autoCommissioning)
          return false;
      }
    }
    else
    {
      inPkt->state = ZGP_PACKET_INPKT_TO_PROCESS_FOR_RESPONSE;
    }
  }
  else
  {
    // This is secured packet so need to decrypt the packet
    zgpDstubDecryptInPacket(inPkt);
  }

  return true;
}

/**************************************************************************//**
  \brief This is to know whether the device operates in Operational Channel/Transmit channel

  \param[in] - none

  \return true - if device is on operational channel
          false - otherwise
 ******************************************************************************/
static bool operatingOnNwkChannel(void)
{
  if (MAC_GetChannel() == NWK_GetLogicalChannel())
    return true;
  return false;
}
/**************************************************************************//**
  \brief This is to parse the incoming packet fields

  \param[in] inPkt - incoming packet
             indParams - MAC-DATA indication primitive's parameters 
  \return true - for valid frame
          false - otherwise
 ******************************************************************************/
static bool parseDataIndFields(zgpPacket_t *inPkt, MAC_DataInd_t *indParams)
{
  ZGP_LowDataInd_t *dstubDataInd = (ZGP_LowDataInd_t *)&inPkt->pkt.in.zgpDataInd;
  ZGP_NwkFrameControl_t *dstubNwkFrameControl;
  ZGP_ExtNwkFrameControl_t *dstubExtNwkFrameControl;
  bool parseSrcId = false;

  dstubDataInd->applicationId = ZGP_SRC_APPID; // Default application id as per spec. A.1.4.1.3
  dstubDataInd->gpdfSecurityLevel = ZGP_SECURITY_LEVEL_0; // Default application id as per spec. A.1.4.1.3
  dstubDataInd->rxAfterTx = 0;
  dstubDataInd->linkQuality = indParams->linkQuality;
  dstubDataInd->rssi = indParams->rssi;
  dstubDataInd->seqNumber = indParams->dsn;
  dstubDataInd->srcAddrMode = indParams->srcAddrMode;
  dstubDataInd->srcPanId = indParams->srcPANId;
  dstubDataInd->srcAddress = indParams->srcAddr;
  dstubDataInd->endPoint = 0;
  dstubDataInd->gpdAsdu = &inPkt->macPayload[0];
  // Initially this is having msduLength later will be updated based on options/security
  dstubDataInd->gpdAsduLength = indParams->msduLength;
  inPkt->pkt.in.service.rxdFramePtr = (uint8_t *)indParams->msdu;
  inPkt->pkt.in.service.rxdFrameLength = indParams->msduLength;
  inPkt->pkt.in.zgpDecryptReq.key = &inPkt->pkt.in.secKey[0];

  dstubNwkFrameControl = (ZGP_NwkFrameControl_t *)inPkt->pkt.in.service.rxdFramePtr++;
  inPkt->pkt.in.service.rxdFrameLength--;
  dstubDataInd->autoCommissioning = dstubNwkFrameControl->autoCommissioning;
  dstubDataInd->frameType = dstubNwkFrameControl->frametype;
  dstubDataInd->service.sharedOrIndividualkeyType = ZGP_SECURITY_KEY_NONE;

  if (ZGP_FRAME_MAINTENANCE == dstubNwkFrameControl->frametype)
  {
    inPkt->state = ZGP_PACKET_INPKT_MAINTENANCE_FRAME_PROCESSING;
  }
  else if ((ZGP_FRAME_DATA == dstubNwkFrameControl->frametype) && \
            operatingOnNwkChannel())
  {
    inPkt->state = ZGP_PACKET_INPKT_DATA_FRAME_PROCESSING;
  }
  else
    return false;

  if (dstubNwkFrameControl->isExtNwkFcf)
  {
    if (inPkt->pkt.in.service.rxdFrameLength < ZGP_EXT_NWK_FCF_LENGTH)
      return false;
    dstubExtNwkFrameControl = (ZGP_ExtNwkFrameControl_t *)inPkt->pkt.in.service.rxdFramePtr++;
    inPkt->pkt.in.service.rxdFrameLength--;
    dstubDataInd->applicationId = (ZGP_ApplicationId_t)dstubExtNwkFrameControl->appId;
    dstubDataInd->gpdfSecurityLevel = (ZGP_SecLevel_t)dstubExtNwkFrameControl->secLevel;
    dstubDataInd->service.sharedOrIndividualkeyType = dstubExtNwkFrameControl->secKey;
    dstubDataInd->rxAfterTx = dstubExtNwkFrameControl->rxAfterTx;

    if (ZGP_SRC_APPID == dstubDataInd->applicationId)
    {
      parseSrcId = true;

    }
    else if (ZGP_IEEE_ADDR_APPID == dstubDataInd->applicationId)
    {
       if (inPkt->pkt.in.service.rxdFrameLength < ZGP_ENDPOINT_LENGTH)
        return false;

      // Drop the frame if the source address mode is not extended or
      // source extended address is zero
      if ((MAC_EXT_ADDR_MODE != indParams->srcAddrMode) ||
           (!dstubDataInd->srcAddress.ext))
      {
        return false;
      }
      dstubDataInd->endPoint = *inPkt->pkt.in.service.rxdFramePtr++;
      inPkt->pkt.in.service.rxdFrameLength--;
      dstubDataInd->srcAddress.ext = indParams->srcAddr.ext;
      dstubDataInd->srcId = 0x00;
    }
  }
  else if (ZGP_FRAME_DATA == dstubNwkFrameControl->frametype)
  {
    // In case of data frame without extended frame control, we assume SRC_ID
    // so src id is expected
    parseSrcId = true;

  }

  if (parseSrcId)
  {
    if (inPkt->pkt.in.service.rxdFrameLength < ZGP_SRC_ID_LENGTH)
     return false;
    memcpy((void *)&dstubDataInd->srcId, inPkt->pkt.in.service.rxdFramePtr, sizeof(dstubDataInd->srcId));
    inPkt->pkt.in.service.rxdFramePtr += sizeof(dstubDataInd->srcId);
    inPkt->pkt.in.service.rxdFrameLength -= sizeof(dstubDataInd->srcId);
    dstubDataInd->srcAddress.ext = 0;
    if (!ZGPL_IsValidSrcId(dstubDataInd->srcId, dstubNwkFrameControl->frametype, false))
    {
      return false;
    }
  }

  return true;
}
/******************************************************************************
  \brief  To get the matching entry in gpTxQueue
  \param  appId - appId of the entry
          srcId - srcId of the Entry
          ieeeAddr - Ext Addr of Entry
          endpoint - endPoint of Entry

  \return pointer to packet.
******************************************************************************/
static zgpPacket_t* zgpGetMatchingTxQueueEntry(ZGP_ApplicationId_t appId, ZGP_SourceId_t srcId, \
                                                    ExtAddr_t ieeeAddr, uint8_t endPoint)
{
  bool matchingEntryFound = false;
  zgpPacket_t *elementPtr = (zgpPacket_t *)getQueueElem(&zgpDstubTxQueue);

  while (elementPtr)
  {
    if (elementPtr->pkt.out.zgpDstubDataReq.applicationId == appId)
    {
      if ((ZGP_SRC_APPID == appId) && \
           (elementPtr->pkt.out.zgpDstubDataReq.srcId == srcId))
      {
        // check the action flag. If flag is false, then remove the entry
        // If true, then overwrite the existing entry
        matchingEntryFound = true;
        break;
      }
      else if ((ZGP_IEEE_ADDR_APPID == appId) && \
                (elementPtr->pkt.out.zgpDstubDataReq.gpdIeeeAddress == ieeeAddr))
      {
        if ((APP_INDEPENDENT_END_POINT == elementPtr->pkt.out.zgpDstubDataReq.endpoint) || \
             (ALL_END_POINT == elementPtr->pkt.out.zgpDstubDataReq.endpoint) || \
               (endPoint == elementPtr->pkt.out.zgpDstubDataReq.endpoint))
        {
          matchingEntryFound = true;
          break;
        }
      }
    }
    elementPtr = (zgpPacket_t *)getNextQueueElem(elementPtr);
  }

  if (matchingEntryFound)
    return elementPtr;
  else
    return NULL;
}

/******************************************************************************
  \brief  Get the matching entry in gpTxQueue and send the output packet
  \param  pkt - pointet to the packet
  \return none.
******************************************************************************/
static void getMatchingTxQueueEntryAndSend(zgpPacket_t *pkt)
{
  zgpPacket_t* zgpPacketToBeSent = NULL;
  zgpInputPacket_t *inputPkt = &pkt->pkt.in;

  zgpPacketToBeSent = zgpGetMatchingTxQueueEntry(inputPkt->zgpDataInd.applicationId, \
                                                 inputPkt->zgpDataInd.srcId, inputPkt->zgpDataInd.srcAddress.ext, \
                                                 inputPkt->zgpDataInd.endPoint);

  if (zgpPacketToBeSent)
  {
    if (zgpPacketToBeSent->state == ZGP_PACKET_OUTPKT_IN_TXQUEUE)
    {
      uint8_t securityFrameCounterOffset = 0;
      ZGP_NwkFrameControl_t *nwkFrameControlField = (ZGP_NwkFrameControl_t *) &zgpPacketToBeSent->pkt.out.macDataReq.msdu[0];


      if (ZGP_SECURITY_LEVEL_0 != inputPkt->zgpDataInd.gpdfSecurityLevel)
      {
        zgpPacketToBeSent->pkt.out.zgpEncryptReq.securityLevel = inputPkt->zgpDataInd.gpdfSecurityLevel;// ->zgpDecryptReq.securityLevel;
        zgpPacketToBeSent->pkt.out.zgpEncryptReq.securityFrameCounter = inputPkt->zgpDecryptReq.securityFrameCounter;
        memcpy((void *)&zgpPacketToBeSent->pkt.out.zgpEncryptReq.key[0], (void *)&inputPkt->zgpDecryptReq.key[0] , sizeof(ZGP_Security_Key_t));
      }
      zgpPacketToBeSent->pkt.out.macDataReq.performCsma = false;


      securityFrameCounterOffset += ZGP_NWK_FCF_LENGTH;

      if (nwkFrameControlField->isExtNwkFcf)
      {
        ZGP_ExtNwkFrameControl_t *ExtNwkFrameControlField = (ZGP_ExtNwkFrameControl_t *) &zgpPacketToBeSent->pkt.out.macDataReq.msdu[1];

        securityFrameCounterOffset += ZGP_EXT_NWK_FCF_LENGTH;

        if (ZGP_SRC_APPID == inputPkt->zgpDataInd.applicationId)
          securityFrameCounterOffset += ZGP_SRC_ID_LENGTH;
        else
          securityFrameCounterOffset += ZGP_ENDPOINT_LENGTH;

        ExtNwkFrameControlField->secKey = (ZGP_SecKey_t)0;
        if (inputPkt->zgpDataInd.gpdfKeyType >= ZGP_KEY_TYPE_OOB_ZGPD_KEY)
          ExtNwkFrameControlField->secKey = (ZGP_SecKey_t)1;
        ExtNwkFrameControlField->secLevel = inputPkt->zgpDataInd.gpdfSecurityLevel;
      }
      if ((ZGP_SECURITY_LEVEL_2 == zgpPacketToBeSent->pkt.out.zgpEncryptReq.securityLevel) || \
          (ZGP_SECURITY_LEVEL_3 == zgpPacketToBeSent->pkt.out.zgpEncryptReq.securityLevel))
      {
        memcpy((void *)&zgpPacketToBeSent->pkt.out.macDataReq.msdu[securityFrameCounterOffset], &zgpPacketToBeSent->pkt.out.zgpEncryptReq.securityFrameCounter, \
                           sizeof(uint32_t));
      }
      else
      {
        // Need to advance the payload
        uint8_t payloadOffset = zgpPacketToBeSent->pkt.out.macDataReq.msduLength - securityFrameCounterOffset - sizeof(uint32_t);
        for (uint8_t index = 0; index < payloadOffset; index++)
          zgpPacketToBeSent->pkt.out.macDataReq.msdu[securityFrameCounterOffset + index] = zgpPacketToBeSent->pkt.out.macDataReq.msdu[securityFrameCounterOffset + sizeof(uint32_t) + index];
        zgpPacketToBeSent->pkt.out.macDataReq.msduLength -= sizeof(uint32_t);
      }

      if (zgpSendPacket(zgpPacketToBeSent))
      {
        zgpPacketToBeSent->state = ZGP_PACKET_OUTPKT_TX_IN_PROCESS;
        pkt->state = ZGP_PACKET_INPKT_RESPONSE_SENDING_IN_PROCESS;
      }
      else
      {
        // Instead of forwarding imm., we are putting on hold
        // will be forwarded after the in-progress packet is forwarded
        // to make sure the higher layer receives the packet in the same sequence
        pkt->state = ZGP_PAKCET_INPKT_INDICATION_ON_HOLD;
      }
    }
    else
    {
      // ALready the packet is being sent.so we can hold this packet
      pkt->state = ZGP_PAKCET_INPKT_INDICATION_ON_HOLD;
    }
  }
  else
  {
    pkt->state = ZGP_PACKET_INPKT_INDICATION_TO_HIGHER_LAYER;
  }
}
/**************************************************************************//**
  \brief process the received GPDF i.e. checking for matching entry in txQueue

  \param[in] pkt - pointer to rxd GPDF packet

  \return true - valid frame
          false otherwise
 ******************************************************************************/
static bool zgpProcessRxdFrame(zgpPacket_t *pkt)
{
  zgpInputPacket_t *inputPkt = &pkt->pkt.in;

  if (inputPkt->zgpDataInd.gpdCommandId < ZGP_COMMISSIONING_REPLY_CMD_ID)
  {
    if (inputPkt->zgpDataInd.rxAfterTx)
    {
      getMatchingTxQueueEntryAndSend(pkt);
    }
    else
    {
      // Forwarding the packet which has matching entry as per spec. A.3.8.1 Figure-87
      pkt->state = ZGP_PACKET_INPKT_INDICATION_TO_HIGHER_LAYER;
    }
  }
  return true;
}
/******************************************************************************
  \brief  callback from SSP component on completion of Decryption
  \param  confirm - confirm parameters of operation.
  \return none.
******************************************************************************/
static void zgpSspDecryptconfirm(SSP_ZgpDecryptFrameConf_t *conf)
{
  zgpInputPacket_t *inputPkt = GET_PARENT_BY_FIELD(zgpInputPacket_t,
	  zgpDecryptReq.confirm, conf);
  zgpPacket_t *pkt = GET_PARENT_BY_FIELD(zgpPacket_t,
	  pkt.in, inputPkt);

  SYS_E_ASSERT_FATAL( (ZGP_PACKET_INPKT_SECURITY_IN_PROCESS == pkt->state), ZGP_DSTUB_SECURED_FRAME_INVALID_STATE);
  inputPkt->zgpDataInd.gpdSecurityFrameCounter = conf->securityFrameCounter;
  inputPkt->zgpDataInd.gpdAsdu = pkt->macPayload + inputPkt->zgpDecryptReq.headerLength + 1;
  inputPkt->zgpDataInd.gpdCommandId = pkt->macPayload[inputPkt->zgpDecryptReq.headerLength];
  if(conf->status == SSP_SUCCESS_STATUS)
  {
    // change the state to process the received frame
    pkt->state =  ZGP_PACKET_INPKT_TO_PROCESS_FOR_RESPONSE;
    //zgpProcessRxdFrame(pkt);
    inputPkt->zgpDataInd.status = ZGP_DSTUB_SECURITY_SUCCESS;
    
  }
  else
  {
    // security failed so forward to higher layer
    pkt->state =  ZGP_PACKET_INPKT_INDICATION_TO_HIGHER_LAYER;
    inputPkt->zgpDataInd.status = ZGP_DSTUB_AUTH_FAILURE;
  }
  processIncomingGpdf(pkt);
}

/**************************************************************************//**
  \brief GP-Data-Request

  Raising from higher layer to dStub

  \param[in] dstubDataReq - request parameters
  \return None.
 ******************************************************************************/
void ZGPL_GpdfDataRequest(ZGP_GpdfDataReq_t *zgpGpdfDataReq)
{
  bool addEntry = false;
  zgpPacket_t *elementPtr = (zgpPacket_t *)getQueueElem(&zgpDstubTxQueue);

  if (!elementPtr)
  {
    if (!zgpGpdfDataReq->action)
    {
      // Action is to remove but no entry is available
      // so return with ZGP_ENTRY_REMOVED
      zgpGpdfDataReq->confirm.status = ZGP_ENTRY_REMOVED;
      addEntry = false;
    }
    else
    {
      // Action is to add the entry & no entry in the queue
      addEntry = true;
    }
  }
  else
  {
    zgpTxQueueProcessing(zgpGpdfDataReq);
    if (ZGP_ENTRY_ADDED == zgpGpdfDataReq->confirm.status)
      addEntry = true;
  }

  if (addEntry && (zgpGpdfDataReq->action))
  {
    zgpPacket_t *outPkt;

    zgpGpdfDataReq->confirm.status = ZGP_ENTRY_ADDED;
    outPkt = zgpAllocPacket(ZGP_DSTUB_OUTPUT_DATA_PACKET);
    if (outPkt)
    {
      zgpFillCstubReq(outPkt, zgpGpdfDataReq);
      putQueueElem(&zgpDstubTxQueue, outPkt);
    }
    else
    {
      zgpGpdfDataReq->confirm.status = ZGP_TX_QUEUE_FULL;
    }
  }

}

/**************************************************************************//**
  \brief Flush out gp Tx queue

  \param[in] None
  \return None.
 ******************************************************************************/
void ZGPL_FlushTxQueue(void)
{
  zgpPacket_t *elementPtr;

  while (NULL != (elementPtr = (zgpPacket_t *) getQueueElem(&zgpDstubTxQueue)))
  {
    if (ZGP_PACKET_OUTPKT_IN_TXQUEUE == elementPtr->state)
    {
      //Delete from queue
      deleteQueueElem(&zgpDstubTxQueue, elementPtr);
      elementPtr->state = ZGP_PACKET_IDLE;
      //Free the memory
      zgpFreeInOutPacket(&elementPtr->pkt.out);
    }
  }
}
/**************************************************************************//**
  \brief GP-Tx-Queue Handler

  \param[in] dstubDataReq - Ptr to dstubDataReq

  \return None.
 ******************************************************************************/
static void zgpTxQueueProcessing(ZGP_GpdfDataReq_t *dstubDataReq)
{
  bool matchingEntryFound;
  zgpPacket_t *elementPtr = (zgpPacket_t *)getQueueElem(&zgpDstubTxQueue);

  while (elementPtr)
  {
    matchingEntryFound = false;
    if (elementPtr->pkt.out.zgpDstubDataReq.applicationId == dstubDataReq->applicationId)
    {
      if ((ZGP_SRC_APPID == dstubDataReq->applicationId) && \
           (elementPtr->pkt.out.zgpDstubDataReq.srcId == dstubDataReq->srcId))
      {
        // check the action flag. If flag is false, then remove the entry
        // If true, then overwrite the existing entry
        matchingEntryFound = true;
      }
      else if ((ZGP_IEEE_ADDR_APPID == dstubDataReq->applicationId) && \
                (elementPtr->pkt.out.zgpDstubDataReq.gpdIeeeAddress == dstubDataReq->gpdIeeeAddress))
      {
        if (dstubDataReq->txOptions.txOnMatchingEndpoint)
        {
          if (dstubDataReq->endpoint == elementPtr->pkt.out.zgpDstubDataReq.endpoint)
          {
            matchingEntryFound = true;
          }
        }
        else
        {
          matchingEntryFound = true;
          if (dstubDataReq->action)
          {
            if (elementPtr->pkt.out.zgpDstubDataReq.txOptions.txOnMatchingEndpoint && \
                (ZGP_PACKET_OUTPKT_IN_TXQUEUE == elementPtr->state))
            {
              // remove the entry
              zgpPacket_t *elementPtrToBeRemoved;
              elementPtrToBeRemoved = elementPtr;
              elementPtr = (zgpPacket_t *)getNextQueueElem(elementPtr);
              deleteQueueElem(&zgpDstubTxQueue, elementPtrToBeRemoved);
              elementPtrToBeRemoved->state = ZGP_PACKET_IDLE;
              zgpFreeInOutPacket(&elementPtrToBeRemoved->pkt.out);
              dstubDataReq->confirm.status = ZGP_ENTRY_REMOVED;
              continue;
            }
          }
        }
      }
    }
    if (matchingEntryFound)
    {
      // If the current element is being sent, then skip any operation on this element
      if (ZGP_PACKET_OUTPKT_IN_TXQUEUE == elementPtr->state)
      {
        if (!dstubDataReq->action)
        {
          deleteQueueElem(&zgpDstubTxQueue, elementPtr);
          // need to deallocate the memory
          elementPtr->state = ZGP_PACKET_IDLE;
          zgpFreeInOutPacket(&elementPtr->pkt.out);
          dstubDataReq->confirm.status = ZGP_ENTRY_REMOVED;
        }
        else
        {
          zgpFillCstubReq(elementPtr, dstubDataReq);
          dstubDataReq->confirm.status = ZGP_ENTRY_REPLACED;
        }
      }
      else
        dstubDataReq->confirm.status = ZGP_GPDF_SENDING_FINALIZED;
      return;
    }
    elementPtr = (zgpPacket_t *)getNextQueueElem(elementPtr);
  }

  if (dstubDataReq->action)
    dstubDataReq->confirm.status = ZGP_ENTRY_ADDED;
  else
    dstubDataReq->confirm.status = ZGP_ENTRY_REMOVED;

}

/**************************************************************************//**
  \brief This is to fill the CstubDataReq and send output Packet

  \param[in] outPkt - Buffer for the output packet
             dstubReq - data to be sent out

  \return None.
 ******************************************************************************/
static void zgpFillCstubReq(zgpPacket_t *outPkt, ZGP_GpdfDataReq_t *dstubReq)
{
  uint8_t index = 0;

  outPkt->state = ZGP_PACKET_OUTPKT_IN_TXQUEUE;
  memcpy((void *)&outPkt->pkt.out.zgpDstubDataReq, dstubReq, sizeof(ZGP_GpdfDataReq_t));
  outPkt->pkt.out.zgpDstubDataReq.gpdAsduLength = dstubReq->gpdAsduLength;

  outPkt->pkt.out.zgpEncryptReq.key = &outPkt->pkt.out.secKey[0];

  if (ZGP_IEEE_ADDR_APPID == dstubReq->applicationId)
  {
    outPkt->pkt.out.macDataReq.dstAddrMode = MAC_EXT_ADDR_MODE;
    memcpy(&outPkt->pkt.out.macDataReq.dstAddr.ext, &dstubReq->gpdIeeeAddress, sizeof(outPkt->pkt.out.macDataReq.dstAddr.ext));
  }
  else
  {
    outPkt->pkt.out.macDataReq.dstAddrMode = MAC_SHORT_ADDR_MODE;
    outPkt->pkt.out.macDataReq.dstAddr.sh = 0xffff;
  }
  outPkt->pkt.out.macDataReq.dstPanId = 0xffff;
  outPkt->pkt.out.macDataReq.msdu = outPkt->macPayload + MAC_MAX_MPDU_UNSECURED_OVERHEAD_NO_SRC_ADDR_IEEE_DST_ADDR_MODE;
  outPkt->pkt.out.macDataReq.msduHandle = dstubReq->gpepHandle; 
  outPkt->pkt.out.macDataReq.srcAddrMode = MAC_NO_ADDR_MODE;
  outPkt->pkt.out.macDataReq.performCsma = dstubReq->txOptions.performCsma;
  outPkt->pkt.out.macDataReq.txOptions = MAC_NO_TXOPTION;
  if (dstubReq->txOptions.requireMacAck)
    outPkt->pkt.out.macDataReq.txOptions = MAC_ACK_TXOPTION;
  outPkt->pkt.out.macDataReq.MAC_DataConf = zgpMacDataReqConfirm;
  {
    ZGP_NwkFrameControl_t *nwkFrameControl = (ZGP_NwkFrameControl_t *)&outPkt->pkt.out.macDataReq.msdu[index++];

    nwkFrameControl->frametype = ZGP_FRAME_DATA;
    nwkFrameControl->isExtNwkFcf = 1;
    if (((ZGP_SRC_APPID == dstubReq->applicationId) && (0 == dstubReq->srcId)) || \
         (ZGP_FRAME_MAINTENANCE == outPkt->pkt.out.zgpDstubDataReq.txOptions.gpdfFrameType))
    {
      nwkFrameControl->frametype = ZGP_FRAME_MAINTENANCE;
      nwkFrameControl->isExtNwkFcf = 0;
    }
    nwkFrameControl->zigbeeProtVersion = GREENPOWER_PROTOCOL_VERSION;
    nwkFrameControl->autoCommissioning = 0;
    if (nwkFrameControl->isExtNwkFcf)
    {
      ZGP_ExtNwkFrameControl_t *extNwkFrameControl = (ZGP_ExtNwkFrameControl_t *)&outPkt->pkt.out.macDataReq.msdu[index++];

      extNwkFrameControl->appId = dstubReq->applicationId;
      extNwkFrameControl->direction = ZGP_TX_TO_ZGPD;
      extNwkFrameControl->secKey = ZGP_SECURITY_KEY_NONE;
      extNwkFrameControl->secLevel = ZGP_KEY_TYPE_NO_KEY;
      extNwkFrameControl->rxAfterTx = false;

      if (ZGP_SRC_APPID == dstubReq->applicationId)
      {
        memcpy(&outPkt->pkt.out.macDataReq.msdu[index], &dstubReq->srcId, sizeof(dstubReq->srcId));
        index += sizeof(dstubReq->srcId);
      }
      else
      {
        memcpy(&outPkt->pkt.out.macDataReq.msdu[index], &dstubReq->endpoint, sizeof(dstubReq->endpoint));
        index += sizeof(dstubReq->endpoint);
      }
    }
  }

  // assuming security counter is present
  index += sizeof(uint32_t);

  memcpy(&outPkt->pkt.out.macDataReq.msdu[index++], &dstubReq->gpdCommandId, sizeof(ZGP_CommandId_t));
  memcpy(&outPkt->pkt.out.macDataReq.msdu[index], dstubReq->gpdAsdu, dstubReq->gpdAsduLength);
  index += dstubReq->gpdAsduLength;
  outPkt->pkt.out.macDataReq.msduLength = index;
}
/**************************************************************************//**
  \brief Enable/Disable direct mode of GPDF

  \param[in] directModeEnable - enable/disable flag

  \return None.
 ******************************************************************************/
void ZGPL_EnableDisableDirectMode(bool enabled)
{
  enableDirectMode = enabled;
}
#endif // #if (APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC)
//eof zgpDstub.c
