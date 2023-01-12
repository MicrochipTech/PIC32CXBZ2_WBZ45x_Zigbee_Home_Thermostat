/*******************************************************************************
  Zigbee green power cluster Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpCluster.h

  Summary:
    This file contains the cluster specific APIs from high layer of zgp.

  Description:
    This file contains the cluster specific APIs from high layer of zgp.
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

#ifndef _ZGPCLUSTER_H
#define _ZGPCLUSTER_H

#ifdef _GREENPOWER_SUPPORT_
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcl.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpClusterStructure.h>

/******************************************************************************
                    Externals
 ******************************************************************************/

/******************************************************************************
                    Functions prototypes section
 ******************************************************************************/
/**************************************************************************//**
  \brief Sending read attribute command

  \param[in] addr - dst addr
  \param[in] dir - server to client / client to server
  \param[in] attrId - attribute id

  \return zcl status
 ******************************************************************************/
ZCL_Status_t ZGPH_SendReadAttribute(uint16_t addr, uint8_t dir, uint16_t attrId);

/**************************************************************************//**
  \brief Send ZGP cluster command in raw mode

  \param addr - dst addr
         dir - server to client / client to server
         cmdId - cluster cmd id
         payloadLength - length of payload to be sent
         payload - payload to be sent

  \return zcl status
 ******************************************************************************/
ZCL_Status_t ZGPH_SendCmdInRawMode(ZCL_Addressing_t *dstAddr, bool dir, uint8_t cmdId, uint8_t payLoadLength, uint8_t  *payLoad);

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
                                               uint8_t *payload);

#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
/**************************************************************************//**
  \brief Send proxy commissioning mode command

  \param[in] options - options field
  \param[in] commissioningWindow - commissioningWindow field
  \param[in] channel - channel field

  \return zcl status
 ******************************************************************************/
ZCL_Status_t ZGPH_SendProxyCommissioningModeCommand(zgpGpProxyCommModeOptions_t options, uint16_t commissioningWindow, uint8_t channel);

/**************************************************************************//**
  \brief Sending proxy table request.

  \param[in] addr - dst addr
  \param[in] options - options field
  \param[in] gpdId_Ieee - gpdId / IEEE addr
  \param[in] ep - ep for IEEE addr gpd
  \param[in] index - index field

  \return zcl status
 ******************************************************************************/
ZCL_Status_t ZGPH_SendProxyTableRequest(uint16_t addr, uint8_t options, uint64_t gpdId_Ieee, uint8_t ep, uint8_t index);
#endif // APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC

/**************************************************************************//**
  \brief Sending sink table request

  \param[in] addr - dst addr
  \param[in] options - options field
  \param[in] gpdId_Ieee - gpdId / IEEE addr
  \param[in] ep - ep for IEEE addr gpd
  \param[in] index - index field

  \return zcl status
 ******************************************************************************/
ZCL_Status_t ZGPH_SendSinkTableRequest(uint16_t addr, uint8_t options, uint64_t gpdId_Ieee, uint8_t ep, uint8_t index);


#endif // _GREENPOWER_SUPPORT_
#endif // _ZGPCLUSTER_H

// eof zgpCluster.h
