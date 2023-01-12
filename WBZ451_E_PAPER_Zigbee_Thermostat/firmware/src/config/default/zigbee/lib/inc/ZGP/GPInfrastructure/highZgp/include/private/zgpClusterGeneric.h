/*******************************************************************************
  Zigbee greenpower cluster generic Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpClusterGeneric.h

  Summary:
    This file contains the green power cluster generic interface.

  Description:
    This file contains the green power cluster generic interface.
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

#ifndef _ZGPCLUSTERGENERIC_H
#define _ZGPCLUSTERGENERIC_H

#ifdef _GREENPOWER_SUPPORT_
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcl.h>
#include <zgp/include/zgpCommon.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpClusterStructure.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpInfraDevice.h>
#if MICROCHIP_APPLICATION_SUPPORT != 1
#include <zllplatform/ZLL/N_Zcl_Framework/include/N_Zcl_Framework_Bindings.h>
#include <zllplatform/ZLL/N_Zcl_Framework/include/N_Zcl_Framework.h>
#endif
/******************************************************************************
                    Defines
******************************************************************************/
#if (MICROCHIP_APPLICATION_SUPPORT == 1)
#define ZGP_GETNEXTSEQNUMBER() ZCL_GetNextSeqNumber()
#else
#define ZGP_GETNEXTSEQNUMBER() N_Zcl_Framework_GetNextSequenceNumber()
#endif

#define ZGP_INVALID_ATTR_LENGTH 0xFFFF

/******************************************************************************
                    Prototypes
******************************************************************************/
/**************************************************************************//**
  \brief send zgp cluster command

  \param[in] req - pointer to ZCL request
             dir - cluster direction
             cmdId - command id of the request
             payLoadLength - length of the command payload length

  \return none
******************************************************************************/
void zgpClusterGenericSendZgpCommand(ZCL_Request_t *req, uint8_t options, uint8_t cmdId, \
                                      uint8_t payLoadLength);

/**************************************************************************//**
  \brief Parsing the received proxy/sink Table request

  \param[in] payloadlength & payload - payload info
             gpTableReq - request structure to be populated

  \return commandLength - INVALID_CMD_LENGTH - invalid command frame
                          otherwise valid commandLength
******************************************************************************/
uint8_t gpParsingTableReq(uint8_t payloadLength, uint8_t *payload, \
                                          ZGP_TableRequest_t *gpTableReq);

/**************************************************************************//**
  \brief Send GP proxy/sink table response

  \param[in] addressing - addressing info
             payloadlength & payload - payload info

  \return status - status of the response.
******************************************************************************/
ZCL_Status_t zgpSendTableResponse(bool isProxyReq, ZGP_TableRequest_t *tableReq, ZCL_Addressing_t *address);

/**************************************************************************//**
\brief Initialize Green power cluster generic interface.
******************************************************************************/
void zgpClusterGenericInit(void);

/**************************************************************************//**
  \brief Generic handling for proxy/sink table req

  \param[in] addressing - addressing info
             payloadlength & payload - payload info
             isProxyTableReq - true for proxy table req
                               false for sink table req

  \return status - status after processing the request
******************************************************************************/
ZCL_Status_t zgpClusterGenericTableReqIndHandling(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload,\
                                                  bool isProxyTableReq);

/**************************************************************************//**
 \brief Add security fields to OTA payload

  \param[in] OTAPayload - payload to be populated
             genericInfo - table generic info
             securityUse,  macSeqNoCapability - bit fields

  \return payload length
******************************************************************************/
uint16_t zgpGenericAddSecurityFields(uint8_t *OTAPayload, zgpTableGenericInfo_t *genericInfo, \
                                             bool securityUse, bool macSeqNoCapability);

/**************************************************************************//**
\brief Add pre-commissioned group list to OTA payload

\param[in] - genericInfo - contains group list
             OTApayload  - payload to be populated

\return size of pre-commissioned group list added to the payload
******************************************************************************/
uint8_t zgpAddPreCommissionedGroupList(zgpTableGenericInfo_t *genericInfo, uint8_t *OTApayload);

/**************************************************************************//**
\brief Add addr infor to OTA payload

\param[in] - appId - application id
             genericInfo - generic info to be read fron
             OTApayload - OTApayload to be populated

\return size of addr info added to the payload
******************************************************************************/
uint8_t zgpAddGpdSourceAddrToOtaPayload(ZGP_ApplicationId_t appId, zgpTableGenericInfo_t *genericInfo,\
                         uint8_t *OTApayload);

/**************************************************************************//**
\brief This is to provide the indication for received cluster command

\param[in] zgpIndicationType- indication type
           addressing- addressing infor
           payloadLength- payloadlength
           payload- pointer to paylaod

\return  status after processing
******************************************************************************/
ZCL_Status_t zgpClusterCommandHandler(ZGP_IndicationType_t zgpIndicationType, ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);

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
                                      ZGP_GpdAppInfo_t *appInfo, uint8_t noOfEndPoints, uint8_t *endPointList);
#endif // _GREENPOWER_SUPPORT_

#endif // _ZGPCLUSTERGENERIC_H

// eof zgpClusterGeneric.h
