/*******************************************************************************
  Zigbee green power cluster generic Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpClusterGeneric.c

  Summary:
    This file contains the green power cluster generic implementation.

  Description:
    This file contains the green power cluster generic implementation.
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
#include <zgp/GPInfrastructure/highZgp/include/zgpCluster.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighGeneric.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighMem.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterGeneric.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterServer.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterClient.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpClusterZclInterface.h>
#include <zcl/include/zclAttributes.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpProxyTable.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpSinkTable.h>
#include <zgp/include/zgpDbg.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighMem.h>
#include <systemenvironment/include/sysAssert.h>

/******************************************************************************
                    Define section
******************************************************************************/
#define SECURITY_LEVEL_MASK   0x3
#define ZGP_INVALID_ENTRY_LENGTH 0xFFFF

/******************************************************************************
                    Prototypes section
******************************************************************************/
static void readAttributeResp(ZCL_Notify_t *ntfy);
static void writeAttributeResp(ZCL_Notify_t *ntfy);
static void tableRespNotify(ZCL_Notify_t *ntfy);
static void gpReadAttributeIndEventListener(SYS_EventId_t eventId, SYS_EventData_t data);
static uint16_t zgpGetTableAttribute(uint8_t *attrValuePtr, uint16_t maxLength, bool isProxyTable);

/******************************************************************************
                    Internal variables
******************************************************************************/
static SYS_EventReceiver_t gpReadAttributeIndEvent = { .func = gpReadAttributeIndEventListener};

/******************************************************************************
                    Implementations section
******************************************************************************/
/**************************************************************************//**
\brief Initialize Green power cluster generic interface.
******************************************************************************/
void zgpClusterGenericInit(void)
{
  SYS_SubscribeToEvent(BC_ZCL_EVENT_ACTION_REQUEST, &gpReadAttributeIndEvent);

}

/**************************************************************************//**
  \brief  ZCL action request event handler, 
          handles the ZCL_ACTION_READ_ATTR_REQUEST for attribute specific validation

  \param[in] eventId - system event
  \param[in] data - this field must contain pointer to the BcZCLActionReq_t structure,

  \return eventId.
 ******************************************************************************/
static void gpReadAttributeIndEventListener(SYS_EventId_t eventId, SYS_EventData_t data)
{
  BcZCLActionReq_t *const actionReq = (BcZCLActionReq_t*)data;
  bool proxyTableReq = true;

  if (ZCL_ACTION_READ_ATTR_REQUEST == actionReq->action)
  {
    ZCLActionReadAttrReq_t *zclReadAttrReq;
    zclReadAttrReq = (ZCLActionReadAttrReq_t*)actionReq->context;
    uint16_t attrLength;

    if ((GREEN_POWER_ENDPOINT == zclReadAttrReq->endpointId) && (GREEN_POWER_CLUSTER_ID == zclReadAttrReq->clusterId))
    {
      if (((ZCL_FRAME_CONTROL_DIRECTION_SERVER_TO_CLIENT == zclReadAttrReq->clusterSide) && \
          (ZCL_GP_CLUSTER_CLIENT_PROXY_TABLE_ATTRIBUTE_ID == zclReadAttrReq->attrId))
#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC 
           || ((ZCL_FRAME_CONTROL_DIRECTION_CLIENT_TO_SERVER == zclReadAttrReq->clusterSide) && \
               (ZCL_GP_CLUSTER_SERVER_SINK_TABLE_ATTRIBUTE_ID == zclReadAttrReq->attrId))
#endif             
          )
      {
#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC         
        if ((ZCL_FRAME_CONTROL_DIRECTION_CLIENT_TO_SERVER == zclReadAttrReq->clusterSide) && \
             (ZCL_GP_CLUSTER_SERVER_SINK_TABLE_ATTRIBUTE_ID == zclReadAttrReq->attrId))
        {
          proxyTableReq = false;
        }
#endif
        attrLength = zgpGetTableAttribute(&zclReadAttrReq->attrValue[2], zclReadAttrReq->attrLength, proxyTableReq);
        if (ZGP_INVALID_ATTR_LENGTH != attrLength)
        {
          zclReadAttrReq->attrValue[0] = attrLength & 0xff;
          zclReadAttrReq->attrValue[1] = (attrLength >> 8)& 0xff;
          zclReadAttrReq->lengthField = sizeof(uint16_t);
        }
        zclReadAttrReq->attrLength = attrLength;
        actionReq->denied = 0;
      }
    }
  }
#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC 
  else if (ZCL_ACTION_WRITE_ATTR_REQUEST == actionReq->action)
  {
    ZCLActionWriteAttrReq_t *const zclWriteAttrReq = (ZCLActionWriteAttrReq_t*)actionReq->context;

    if ((GREEN_POWER_CLUSTER_ID == zclWriteAttrReq->clusterId) && \
        (ZCL_FRAME_CONTROL_DIRECTION_CLIENT_TO_SERVER == zclWriteAttrReq->clusterSide))
    {
      uint8_t attrValue = *((uint8_t*)(zclWriteAttrReq->attrValue));
      actionReq->denied = 0;

      if (ZCL_GP_CLUSTER_SERVER_GPS_COMMUNICATION_MODE_ATTRIBUTE_ID == zclWriteAttrReq->attrId)
      {
        if (FULL_UNICAST == attrValue)
        {
          actionReq->denied = 1; 
          zclWriteAttrReq->status = ZCL_INVALID_FIELD_STATUS;
        }
          
      }
      else if (ZCL_GP_CLUSTER_SERVER_GPS_SECURITY_LEVEL_ATTRIBUTE_ID == zclWriteAttrReq->attrId)
      {
        if (ZGP_SECURITY_LEVEL_1 == (attrValue & SECURITY_LEVEL_MASK))
        {
          actionReq->denied = 1;
          zclWriteAttrReq->status = ZCL_INVALID_FIELD_STATUS;
        }
      }
    }
  }
#endif
  (void)eventId;
}

/**************************************************************************//**
  \brief Sending read attribute command

  \param[in] addr - dst addr
  \param[in] dir - server to client / client to server
  \param[in] attrId - attribute id

  \return zcl status
******************************************************************************/
ZCL_Status_t ZGPH_SendReadAttribute(uint16_t addr, uint8_t dir, uint16_t attrId)
{
  ZCL_Request_t *req;
  ZCL_NextElement_t element;
  ZCL_ReadAttributeReq_t readAttrReqElement;

  if (!(req = ZCL_CommandManagerAllocCommand()))
    return ZCL_INSUFFICIENT_SPACE_STATUS;

  /* Fill the attribute identifier to read from */
  readAttrReqElement.id = attrId;

  /* Fill the details of ZCL command payload */
  element.payloadLength = 0;
  element.payload = req->requestPayload;
  element.content = &readAttrReqElement;
  element.id = ZCL_READ_ATTRIBUTES_COMMAND_ID;  
  ZCL_PutNextElement(&element);

  req->dstAddressing.addrMode = APS_SHORT_ADDRESS;
  req->dstAddressing.addr.shortAddress = addr;
  req->ZCL_Notify = readAttributeResp;

  zgpClusterGenericSendZgpCommand(req, dir|ATTRIBUTE_CMD_MASK, ZCL_READ_ATTRIBUTES_COMMAND_ID, element.payloadLength); 

  return ZCL_SUCCESS_STATUS;
}

/**************************************************************************//**
\brief ZCL command response
  \param[in] ntfy - notification
 \return - NONE
******************************************************************************/
static void readAttributeResp(ZCL_Notify_t *ntfy)
{
  cmdprintf(" <-Green Power read attribute response: %x \r\n", ntfy->status);
}

/**************************************************************************//**
  \brief Send ZGP cluster command in raw mode

  \param[in] dstAddr - dst addr
  \param[in] dir - server to client / client to server
  \param[in] cmdId - cluster cmd id
  \param[in] payloadLength - length of payload to be sent
  \param[in] payload - payload to be sent

  \return zcl status
******************************************************************************/
ZCL_Status_t ZGPH_SendCmdInRawMode(ZCL_Addressing_t *dstAddr, bool dir, uint8_t cmdId, uint8_t payLoadLength, uint8_t  *payLoad)
{
  ZCL_Request_t *req;
  uint8_t *gpPairingPayload;
  uint8_t dir_attr_cmd_mask = (uint8_t)dir;

  if (!(req = ZCL_CommandManagerAllocCommand()))
    return ZCL_INSUFFICIENT_SPACE_STATUS;

  if(cmdId == ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
  {
    dir_attr_cmd_mask |= ATTRIBUTE_CMD_MASK;
    req->ZCL_Notify = writeAttributeResp;
  }
  
  memcpy(&req->dstAddressing, dstAddr, sizeof(ZCL_Addressing_t));

  gpPairingPayload = (uint8_t *)req->requestPayload;
  memcpy((void *)gpPairingPayload, payLoad, payLoadLength);

  zgpClusterGenericSendZgpCommand(req, (uint8_t)dir_attr_cmd_mask, cmdId, payLoadLength);

  return ZCL_SUCCESS_STATUS;
  
}

/**************************************************************************//**
  \brief send zgp cluster command

  \param[in] req - pointer to ZCL request
             options - direction/attribute cmd information
             cmdId - command id of the request
             payLoadLength - length of the command payload length

  \return none
******************************************************************************/
void zgpClusterGenericSendZgpCommand(ZCL_Request_t *req, uint8_t options, uint8_t cmdId, \
                                      uint8_t payLoadLength)
{
  req->dstAddressing.endpointId = GREEN_POWER_ENDPOINT;
  req->dstAddressing.manufacturerSpecCode = false;
  req->dstAddressing.profileId = GREEN_POWER_PROFILE_ID;
  req->dstAddressing.clusterId = GREEN_POWER_CLUSTER_ID;
  req->dstAddressing.clusterSide = (options & DIR_MASK)? 1 : 0;
  req->endpointId = GREEN_POWER_ENDPOINT;
  req->requestLength = payLoadLength;
  req->id = cmdId;
  req->defaultResponse = ZCL_FRAME_CONTROL_DISABLE_DEFAULT_RESPONSE;
  if(!(options & ZGP_RESPONSE_MASK))
    req->dstAddressing.sequenceNumber = ZGP_GETNEXTSEQNUMBER();

#ifndef DISABLE_ZCL_ATTRIBUTE_REQ_SUPPORT
  if (options & ATTRIBUTE_CMD_MASK)
    ZCL_CommandManagerSendAttribute(req);
  else
#endif
    ZCL_CommandManagerSendCommand(req);
}

#if MICROCHIP_APPLICATION_SUPPORT != 1
/**************************************************************************//**
  \brief Send ZGP cluster command in raw mode
 
  \param[in] addr - dst addr
  \param[in] dir - server to client / client to server
  \param[in] cmdId - cluster cmd id
  \param[in] payloadLength - length of payload to be sent
  \param[in] payload - payload to be sent

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
                                           Notify pNotificationCallback)
{
  ZCL_Request_t *req;
  uint8_t *cmdPayload;

  if (!(req = ZCL_CommandManagerAllocCommand()))
    return ZCL_INSUFFICIENT_SPACE_STATUS;

  memcpy(&req->dstAddressing, dstAddr, sizeof(ZCL_Addressing_t));

  cmdPayload = (uint8_t *)req->requestPayload;
  memcpy((void *)cmdPayload, pZclPayload, zclPayloadLength);
  req->ZCL_Notify =  pNotificationCallback;

  req->dstAddressing.endpointId = GREEN_POWER_ENDPOINT;
  req->dstAddressing.manufacturerSpecCode = false;
  req->dstAddressing.profileId = GREEN_POWER_PROFILE_ID;
  req->dstAddressing.clusterId = clusterId;
  req->dstAddressing.clusterSide = direction;
  req->dstAddressing.sequenceNumber = sequenceNumber;
  req->endpointId = GREEN_POWER_ENDPOINT;
  req->requestLength = zclPayloadLength;
  req->id = commandId;
  req->defaultResponse = disableDefaultResponse;

  if (frameType == ZCL_STANDARD_REQ_TYPE)
    ZCL_CommandManagerSendAttribute(req);
  else
    ZCL_CommandManagerSendCommand(req);
  return ZCL_SUCCESS_STATUS;
}
#endif
/**************************************************************************//**
  \brief confirmation callback for proxy/sink table response sent

  \param[in] ntfy - callback info

  \return None
******************************************************************************/
static void tableRespNotify(ZCL_Notify_t *ntfy)
{
  (void)ntfy; 
}

/**************************************************************************//**
  \brief Parsing the received proxy/sink Table request

  \param[in] payloadlength & payload - payload info
             gpTableReq - request structure to be populated

  \return commandLength - INVALID_CMD_LENGTH - invalid command frame
                          otherwise valid commandLength
******************************************************************************/
uint8_t gpParsingTableReq(uint8_t payloadLength, uint8_t *payload, \
                                          ZGP_TableRequest_t *gpTableReq)
{
  uint8_t payloadIndex = 0;

  memcpy(&gpTableReq->options, &payload[payloadIndex], sizeof(gpTableReq->options));
  payloadIndex += sizeof(gpTableReq->options);

  if(GPD_ID_REF == gpTableReq->options.requestType)
  {
    uint8_t gpdIdLength = ZGPH_ParseGpdIdFromPayload((ZGP_ApplicationId_t)gpTableReq->options.appId, &gpTableReq->gpdId, \
                                                                &gpTableReq->ep, &payload[payloadIndex]);
    if (INVALID_CMD_LENGTH == gpdIdLength)
      return gpdIdLength;
    payloadIndex += gpdIdLength;
  }
  else if(INDEX_REF == gpTableReq->options.requestType)
  {
    memcpy(&gpTableReq->index, &payload[payloadIndex], sizeof(gpTableReq->index));
    payloadIndex += sizeof(gpTableReq->index);
  }
  else
    return INVALID_CMD_LENGTH;

  if (payloadIndex > payloadLength)
    return INVALID_CMD_LENGTH;
  else
    return payloadIndex;
}

/**************************************************************************//**
  \brief Send GP proxy/sink table response

  \param[in] isProxyReq-status of proxy request
             tableReq-table request info
             address- addressing info

  \return status - status of the response.
******************************************************************************/
ZCL_Status_t zgpSendTableResponse(bool isProxyReq, ZGP_TableRequest_t *tableReq, ZCL_Addressing_t *address)
{
  ZCL_Request_t *req;
  ZGP_TableResp_t *tableResp;
  uint8_t *entryPayload;
  ZGP_ProxyTableEntry_t *tableEntry;
  uint8_t nonEmptyEntriesCount;
  uint8_t otaEntriesTotalLength = 0;
  uint8_t rspOverhead = 0;
  uint8_t tableSize = ZGP_PROXY_TABLE_SIZE;
  uint8_t OTAEntrySize = sizeof(ZGP_ProxyTableEntry_t) + 1;
  ZGP_EntryType_t entryType = ZGP_PROXY_ENTRY;
  uint8_t zclMaxApsAsduSize = 0;
  zclMaxApsAsduSize = APS_MAX_NON_SECURITY_ASDU_SIZE;
#ifdef _NWK_CONCENTRATOR_
  bool nwkConcentrator = false;
  CS_ReadParameter(CS_NWK_CONCENTRATOR_CONFIG_ID, &nwkConcentrator);
  if (!nwkConcentrator)
  zclMaxApsAsduSize += NWK_MAX_SOURCE_ROUTE_SUBFRAME_LENGTH;
#endif
  if (!(req = ZCL_CommandManagerAllocCommand()))
    return ZCL_INSUFFICIENT_SPACE_STATUS;

  tableResp = (ZGP_TableResp_t *)req->requestPayload;
  entryPayload = &tableResp->tableEntries[0];
  tableEntry = (ZGP_ProxyTableEntry_t *)zgpGetMemReqBuffer();

  nonEmptyEntriesCount = ZGPL_TotalNonEmptyEntries(isProxyReq);
  tableResp->noOfNonEmptyTableEntries = nonEmptyEntriesCount;
  rspOverhead = sizeof(ZCL_Status_t) + sizeof(tableResp->noOfNonEmptyTableEntries)
                + sizeof(tableResp->startIndex) + sizeof(tableResp->entriesCount);

#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC      
  if (!isProxyReq)
  {
    tableSize = ZGP_SINK_TABLE_SIZE;
    OTAEntrySize = sizeof(ZGP_SinkTableEntry_t) + 1;
    entryType = ZGP_SINK_ENTRY;
  }
#endif 

  if(nonEmptyEntriesCount == 0)
  {
    //As per 1.1 spec the status should be not found
    tableResp->status = ZCL_NOT_FOUND_STATUS;
    tableResp->entriesCount = 0;
    if(GPD_ID_REF == tableReq->options.requestType)
    {
      tableResp->startIndex = ZGP_ENTRY_INVALID_INDEX;
    }
    else if(INDEX_REF == tableReq->options.requestType)
    {
      tableResp->startIndex = tableReq->index;
    }
  }
  else
  {
    if(GPD_ID_REF == tableReq->options.requestType)
    {
      tableResp->startIndex = ZGP_ENTRY_INVALID_INDEX;
      ZGP_ReadOperationStatus_t entryStatus;
      ZGP_TableOperationField_t  tableOperationField = {.entryType = entryType, .commMode = ALL_COMMUNICATION_MODE, .appId = tableReq->options.appId, \
      .nonEmptyIndexForRead = ZGP_ENTRY_INVALID_INDEX};

      entryStatus = ZGPL_ReadTableEntryFromNvm((void *)tableEntry, tableOperationField, &tableReq->gpdId, tableReq->ep);

      if(ACTIVE_ENTRY_AVAILABLE == entryStatus)
      {
        uint16_t entryLength = ZGP_INVALID_ENTRY_LENGTH;
        tableResp->entriesCount = 1;
        if (isProxyReq)
          entryLength = zgppBuildOTAProxyTableEntry(entryPayload, (ZGP_ProxyTableEntry_t *)tableEntry);
#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC        
        else
          entryLength = zgpsBuildOTASinkTableEntry(entryPayload, (ZGP_SinkTableEntry_t *)tableEntry);
#endif        
        if ((entryLength > OTAEntrySize) || (ZGP_INVALID_ENTRY_LENGTH == entryLength))
        {
          // this is not expected.. So raise assert
          SYS_E_ASSERT_ERROR(false, ZGP_TABLE_ENTRY_INVALID_LENGTH);
          tableResp->status = ZCL_FAILURE_STATUS;
        }
        else
        {
          otaEntriesTotalLength += entryLength;
          tableResp->status = ZCL_SUCCESS_STATUS;
        }
      }
      else
      {
        tableResp->status = ZCL_NOT_FOUND_STATUS;
        tableResp->entriesCount = 0;
      }
    }
    else if(INDEX_REF == tableReq->options.requestType)
    {
      tableResp->startIndex = tableReq->index;
      tableResp->entriesCount = 0x00;
      if(ZGPL_TotalNonEmptyEntries(isProxyReq) < (tableReq->index + 1))
      {
        tableResp->status = ZCL_NOT_FOUND_STATUS;
      }
      else
      {
        uint8_t maxRspPayload = zclMaxApsAsduSize - ZCL_FRAME_STANDARD_HEADER_SIZE - rspOverhead;
        ZGP_ReadOperationStatus_t entryStatus;
        ZGP_TableOperationField_t  tableOperationField = {.entryType = entryType, .commMode = ALL_COMMUNICATION_MODE, .nonEmptyIndexForRead = ZGP_ENTRY_INVALID_INDEX};

        tableResp->status = ZCL_SUCCESS_STATUS;
        // fill all the not empty entries starting from enumerated index 
        // index 0 indicates, first non empty entry in proxy table
        for(uint8_t i = tableReq->index; i < tableSize; i++)
        {
          tableOperationField.nonEmptyIndexForRead = i;
          entryStatus = ZGPL_ReadTableEntryFromNvm((void *)tableEntry, tableOperationField, &tableReq->gpdId, ALL_END_POINT);
          if(ACTIVE_ENTRY_AVAILABLE == entryStatus)
          {
            uint16_t entryLength = ZGP_INVALID_ENTRY_LENGTH;
            uint8_t tableEntryPayload[sizeof(ZGP_ProxyTableEntry_t) + 1/*no. of sink group for sink entry over the air*/] = {0};

            if (isProxyReq)
              entryLength = zgppBuildOTAProxyTableEntry(tableEntryPayload, (ZGP_ProxyTableEntry_t *)tableEntry);
#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC 
            else
              entryLength = zgpsBuildOTASinkTableEntry(tableEntryPayload, (ZGP_SinkTableEntry_t *)tableEntry);
#endif

            if ((entryLength > OTAEntrySize) || (ZGP_INVALID_ENTRY_LENGTH == entryLength))
            {
              // this is not expected.. So raise assert
              SYS_E_ASSERT_ERROR(false, ZGP_TABLE_ENTRY_INVALID_LENGTH);
              continue;
            }
            if(entryLength <= (maxRspPayload - otaEntriesTotalLength))
            {
              memcpy(entryPayload, tableEntryPayload, entryLength); 
              entryPayload += entryLength;
              otaEntriesTotalLength += entryLength;
              tableResp->entriesCount++;
            }
          }
          else if (ENTRY_NOT_AVAILABLE == entryStatus)
            break;          
        }
      }
    }
  }
  // initiate table response via zcl command
  req->dstAddressing.addrMode = APS_SHORT_ADDRESS;
  req->dstAddressing.addr.shortAddress = address->addr.shortAddress; // send to proxy address 
  req->dstAddressing.sequenceNumber = address->sequenceNumber;

  rspOverhead += otaEntriesTotalLength;

  req->ZCL_Notify = tableRespNotify;

  {
    uint8_t options;
    uint8_t cmdId;
    if (isProxyReq)
    {
      options = ZCL_CLUSTER_SIDE_SERVER;
      cmdId = ZCL_GP_CLUSTER_SERVER_GP_PROXY_TABLE_RESPONSE_COMMAND_ID;
    }
    else
    {
      options = ZCL_CLUSTER_SIDE_CLIENT;
      cmdId = ZCL_GP_CLUSTER_CLIENT_GP_SINK_TABLE_RESPONSE_COMMAND_ID;
    }
    zgpClusterGenericSendZgpCommand(req, (options | ZGP_RESPONSE_MASK), cmdId, \
                                    rspOverhead);
  }
  zgpFreeMemReqBuffer();
  return ZCL_SUCCESS_STATUS;
}

/**************************************************************************//**
  \brief Generic handling for proxy/sink table req

  \param[in] addressing - addressing info
             payloadlength & payload - payload info
             isProxyTableReq - true for proxy table req
                               false for sink table req

  \return status - status after processing the request
******************************************************************************/
ZCL_Status_t zgpClusterGenericTableReqIndHandling(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload,\
                                                  bool isProxyTableReq)
{
  ZGP_TableRequest_t gpTableReqCmd;
 
  if(addressing->nonUnicast)
    return ZCL_SUCCESS_STATUS;

  if(!payloadLength)
    return ZCL_MALFORMED_COMMAND_STATUS;

  memset(&gpTableReqCmd, 0x00, sizeof(ZGP_TableRequest_t));

  if(INVALID_CMD_LENGTH == gpParsingTableReq(payloadLength, payload, &gpTableReqCmd))
    return ZCL_MALFORMED_COMMAND_STATUS;

   return zgpSendTableResponse(isProxyTableReq, &gpTableReqCmd, addressing);  
}

/**************************************************************************//**
  \brief Parsing gpdId field from the rxd payload

  \param[in] appId - application id
             gpdId - gpdId to be populated
             endpoint - endpoint to be populated
             payload - rxd payload

  \return gpdId length - INVALID_CMD_LENGTH in case of invalid app. id
                         proper length otherwise
******************************************************************************/
uint8_t ZGPH_ParseGpdIdFromPayload(ZGP_ApplicationId_t appId, ZGP_GpdId_t *gpdId, uint8_t *endpoint,\
                                               uint8_t *payload)
{
  uint8_t payloadIndex =0;

  if(appId == ZGP_SRC_APPID)
  {
    uint32_t gpdSrcId;
    memcpy(&gpdSrcId, &payload[payloadIndex], sizeof(gpdSrcId));
    gpdId->gpdSrcId = gpdSrcId;
    payloadIndex += sizeof(gpdId->gpdSrcId);
  }
  else if(appId == ZGP_IEEE_ADDR_APPID)
  {
    ExtAddr_t gpdIeeeAddr;
    memcpy(&gpdIeeeAddr, &payload[payloadIndex], sizeof(ExtAddr_t));
    gpdId->gpdIeeeAddr = gpdIeeeAddr;
    payloadIndex += sizeof(gpdId->gpdIeeeAddr);
    *endpoint = payload[payloadIndex++];
  }
  else
    return INVALID_CMD_LENGTH;

  return payloadIndex;
}

/**************************************************************************//**
 \brief Add security fields to OTA payload

  \param[in] OTAPayload - payload to be populated
             tableGenericInfo - table generic info
             securityUse,  macSeqNoCapability - bit fields

  \return payload length
******************************************************************************/
uint16_t zgpGenericAddSecurityFields(uint8_t *OTAPayload, zgpTableGenericInfo_t *tableGenericInfo, \
                                             bool securityUse, bool macSeqNoCapability)
{
  uint16_t otaPayloadIndex = 0;

  // security options and key
  if(securityUse)
  {
    memcpy(&OTAPayload[otaPayloadIndex], &tableGenericInfo->securityOptions, \
		                     sizeof(tableGenericInfo->securityOptions));
    otaPayloadIndex++;
  }
  // security frame counter
  if(securityUse || (!securityUse && macSeqNoCapability))
  {
    uint32_t secFrameCounter;
    secFrameCounter = tableGenericInfo->gpdSecurityFrameCounter;
    memcpy(&OTAPayload[otaPayloadIndex], &secFrameCounter, sizeof(secFrameCounter));
    otaPayloadIndex += sizeof(secFrameCounter);
  }
  if(securityUse)
  {
    memcpy(&OTAPayload[otaPayloadIndex], tableGenericInfo->securityKey, ZGP_SECURITY_KEY_LENGTH);
    otaPayloadIndex += ZGP_SECURITY_KEY_LENGTH;
  }

  return otaPayloadIndex;
}

/**************************************************************************//**
\brief get table attrubute

\param[in] attrValuePtr - Buffer pointer where sink table attr to be loaded
           maxLength - max. payload length allowed
           isProxyTable - true for proxy table
                          false for sink table

\return   attr length
******************************************************************************/
static uint16_t zgpGetTableAttribute(uint8_t *attrValuePtr, uint16_t maxLength, bool isProxyTable)
{
  uint16_t tableAttrCurLength = 0x00;
  uint8_t tableEntryPayload[sizeof(ZGP_ProxyTableEntry_t) + 2/*length field for unicast & group cast list in proxy entry*/] = {0};
  ZGP_ProxyTableEntry_t *tableEntry = (ZGP_ProxyTableEntry_t *)zgpGetMemReqBuffer();
  ZGP_GpdId_t gpdId;
  ZGP_TableOperationField_t  tableOperationField = {.entryType = ZGP_PROXY_ENTRY, .commMode = ALL_COMMUNICATION_MODE, \
  .nonEmptyIndexForRead = 0};
  ZGP_ReadOperationStatus_t entryStatus;

  if (!isProxyTable)
    tableOperationField.entryType = ZGP_SINK_ENTRY;

  // This field won't be used when nonEmptyIndexForRead is not ZGP_ENTRY_INVALID_INDEX
  // so init. with zero
  memset(&gpdId, 0x00, sizeof(gpdId));
  while (ENTRY_NOT_AVAILABLE != (entryStatus = ZGPL_ReadTableEntryFromNvm((void *)tableEntry, tableOperationField, &gpdId, ALL_END_POINT)))
  {
    uint16_t entryLength;

    if (ACTIVE_ENTRY_AVAILABLE == entryStatus)
    {
      if (isProxyTable)
        entryLength = zgppBuildOTAProxyTableEntry((uint8_t *)&tableEntryPayload[0], tableEntry);
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC   
      else
        entryLength = zgpsBuildOTASinkTableEntry((uint8_t *)&tableEntryPayload[0], (ZGP_SinkTableEntry_t *)tableEntry);
#endif

      tableAttrCurLength += entryLength;
      if (maxLength && tableAttrCurLength > maxLength)
      {
         zgpFreeMemReqBuffer();
         return ZGP_INVALID_ATTR_LENGTH;
      }
      else
         memcpy(attrValuePtr, (void *)&tableEntryPayload[0], entryLength);

      attrValuePtr += entryLength;
    }
    tableOperationField.nonEmptyIndexForRead++;
  }
  zgpFreeMemReqBuffer();
  return tableAttrCurLength;
}

/**************************************************************************//**
\brief Add pre-commissioned group list to OTA payload

\param[in] genericInfo - pointer to table info
           OTApayload - payload to be populated

\return size of pre-commissioned group list added to the payload
******************************************************************************/
uint8_t zgpAddPreCommissionedGroupList(zgpTableGenericInfo_t *genericInfo, uint8_t *OTApayload)
{
  uint16_t otaEntryIndex = 0;
  uint8_t *noOfNonEmptySinkGroupAddr;

  noOfNonEmptySinkGroupAddr = &OTApayload[otaEntryIndex];
  *noOfNonEmptySinkGroupAddr = 0;
  otaEntryIndex += sizeof(uint8_t);
  for(uint8_t i = 0; i < ZGP_SINK_GROUP_LIST_SIZE; i++)
  {
    if(genericInfo->zgpSinkGrouplist[i].sinkGroup != ZGP_NWK_ADDRESS_GROUP_INIT)
    {
      uint16_t sinkGroup;
      uint16_t alias;
      sinkGroup = genericInfo->zgpSinkGrouplist[i].sinkGroup;
      memcpy(&OTApayload[otaEntryIndex], &sinkGroup, sizeof(sinkGroup));
      otaEntryIndex += sizeof(sinkGroup);
      alias = genericInfo->zgpSinkGrouplist[i].alias;
      memcpy(&OTApayload[otaEntryIndex], &alias , sizeof(alias));
      otaEntryIndex += sizeof(alias);
      (*noOfNonEmptySinkGroupAddr)++;
    }
  }

  return otaEntryIndex;
}
 
/**************************************************************************//**
\brief Add addr info to OTA payload

\param[in] - appId - application id
             genericInfo - generic info to be read fron
             OTApayload - OTApayload to be populated

\return size of addr info added to the payload
******************************************************************************/
uint8_t zgpAddGpdSourceAddrToOtaPayload(ZGP_ApplicationId_t appId, zgpTableGenericInfo_t *genericInfo,\
                         uint8_t *OTApayload)
{
  uint16_t otaEntryIndex = 0;

  if(ZGP_SRC_APPID == appId)
  {
    uint32_t gpdSrcId; 
    gpdSrcId = genericInfo->gpdId.gpdSrcId;
    memcpy(&OTApayload[otaEntryIndex], &gpdSrcId, sizeof(gpdSrcId));
    otaEntryIndex += sizeof(gpdSrcId);
  }
  else if(ZGP_IEEE_ADDR_APPID == appId)
  {
    ExtAddr_t gpdExtAddr; 
    gpdExtAddr = genericInfo->gpdId.gpdIeeeAddr;
    memcpy(&OTApayload[otaEntryIndex], &gpdExtAddr, sizeof(gpdExtAddr));
    otaEntryIndex += sizeof(gpdExtAddr);
    // endpoint 
    OTApayload[otaEntryIndex++] = genericInfo->endPoint;
  }

  return otaEntryIndex;
}

/**************************************************************************//**
\brief ZCL command response

\param[in] ntfy- pointer to zcl notification
\return -none
******************************************************************************/
static void writeAttributeResp(ZCL_Notify_t *ntfy)
{
  cmdprintf(" <-Green Power write attribute response: %x \r\n", ntfy->status);
}

/**************************************************************************//**
\brief This is to provide the indication for received cluster command

\param[in] zgpIndicationType- indication type
           addressing- addressing infor
           payloadLength- payloadlength
           payload- pointer to paylaod

\return  status after processing
******************************************************************************/
ZCL_Status_t zgpClusterCommandHandler(ZGP_IndicationType_t zgpIndicationType, ZCL_Addressing_t *addressing, \
                              uint8_t payloadLength, uint8_t *payload)
{
  ZGP_IndicationInfo_t indicationInfo;

  memset(&indicationInfo, 0x00, sizeof(indicationInfo));
  indicationInfo.indicationType = zgpIndicationType;
  indicationInfo.indicationData.clusterCmdInd.payloadlength = payloadLength;
  indicationInfo.indicationData.clusterCmdInd.payload = payload;
  indicationInfo.indicationData.clusterCmdInd.retStatus = ZCL_SUCCESS_STATUS;
  indicationInfo.indicationData.clusterCmdInd.srcAddress = addressing;
  SYS_PostEvent(BC_EVENT_ZGPH_INDICATION, (SYS_EventData_t)&indicationInfo);
  return indicationInfo.indicationData.clusterCmdInd.retStatus;
}

/**************************************************************************//**
\brief Raising trans remove/add events

\param[in] pairing type - comm/pairing config/none(for remove translation entry)
           sinkEntry- sink entry
           appInfo- application info
           noOfEndPoints - no. of end points(will  be zero for remove operation and
                           non-zero for add operation)
           endPointList- endPoint list for add operation

\return  None
******************************************************************************/
void zgpTransRemoveAndAddEventHandler(zgpPairingProcType_t pairingType, ZGP_SinkTableEntry_t *sinkEntry, \
                                      ZGP_GpdAppInfo_t *appInfo, uint8_t noOfEndPoints, uint8_t *endPointList)
{
  ZGP_TransTableIndicationInfo_t transTableIndicationInfo;

  memset(&transTableIndicationInfo, 0x00, sizeof(transTableIndicationInfo));
  if (NONE == pairingType)
  {
    transTableIndicationInfo.transTableIndType = REMOVE_TRANS_TABLE_ENTRY;
    transTableIndicationInfo.indicationData.removeTransTableEntry.appId = (ZGP_ApplicationId_t)sinkEntry->options.appId;
    transTableIndicationInfo.indicationData.removeTransTableEntry.endPoint = sinkEntry->tableGenericInfo.endPoint;
    transTableIndicationInfo.indicationData.removeTransTableEntry.gpdId = &sinkEntry->tableGenericInfo.gpdId;
  }
  else
  {
    // For add operation, noOfEndPoints should be non-zero
    transTableIndicationInfo.transTableIndType = ADD_TRANS_TABLE_ENTRY;
    transTableIndicationInfo.indicationData.addTransTableEntry.appInfo = appInfo;
    transTableIndicationInfo.indicationData.addTransTableEntry.endPointList = endPointList;
    transTableIndicationInfo.indicationData.addTransTableEntry.noOfEndPoints = noOfEndPoints;
    transTableIndicationInfo.indicationData.addTransTableEntry.pairingType = pairingType;
    transTableIndicationInfo.indicationData.addTransTableEntry.sinkEntry = sinkEntry;
  }

  SYS_PostEvent(BC_EVENT_ZGPH_TRANS_TABLE_INDICATION, (SYS_EventData_t)&transTableIndicationInfo);
}
#endif // APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
#endif // _GREENPOWER_SUPPORT_

// eof zgpClusterGeneric.c
