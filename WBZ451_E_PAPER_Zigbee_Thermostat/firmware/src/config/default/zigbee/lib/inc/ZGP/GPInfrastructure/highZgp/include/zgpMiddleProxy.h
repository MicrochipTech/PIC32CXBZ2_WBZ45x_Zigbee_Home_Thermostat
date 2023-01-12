/*******************************************************************************
  Zigbee green power proxy middle layer Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpMiddleProxy.h

  Summary:
    This file contains the ZGP_ZCL Interface.

  Description:
    This file contains the ZGP_ZCL Interface.
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

#ifndef _ZGPMIDDLEPROXY_H
#define _ZGPMIDDLEPROXY_H

#ifdef _GREENPOWER_SUPPORT_
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/clusters.h>
#include <zllplatform/ZLL/N_Zcl_Framework/include/N_Zcl_Framework_Bindings.h>
#include <zllplatform/ZLL/N_Zcl_Framework/include/N_Zcl_Framework.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLow.h>
/******************************************************************************
                           Definitions section
******************************************************************************/
#if (MICROCHIP_APPLICATION_SUPPORT == 1) 
#define ZGP_GETNEXTSEQNUMBER() ZCL_GetNextSeqNumber()
#else
#define ZGP_GETNEXTSEQNUMBER() N_Zcl_Framework_GetNextSequenceNumber()
#endif

/******************************************************************************
                    Types section
******************************************************************************/
typedef struct PACK
{
  uint16_t appId                         : 3;
  uint16_t alsoUnicast                   : 1;
  uint16_t alsoDerivedGroup              : 1;
  uint16_t alsoCommissionedGroup         : 1;
  uint16_t securityLevel                 : 2;
  uint16_t securityKeyType               : 3;
  uint16_t rxAfterTx                     : 1;
  uint16_t gpTxQueueFull                 : 1;
  uint16_t bidirectionalCapability       : 1;
  uint16_t proxyInfoPresent              : 1;
  uint16_t reserved                      : 1;
}zgpGpNotificationOptions_t;

// GP commissioning notification command
typedef struct PACK
{
  uint16_t appId                         : 3;
  uint16_t rxAfterTx                     : 1;
  uint16_t securityLevel                 : 2;
  uint16_t securityKeyType               : 3;
  uint16_t securityProcFailed            : 1;
  uint16_t bidirectionalCapability       : 1;
  uint16_t proxyInfoPresent              : 1;
  uint16_t reserved                      : 4;
}zgpGpCommNotifyOptions_t;

typedef struct PACK
{
  union
  {
    zgpGpNotificationOptions_t notifyOptions;
    zgpGpCommNotifyOptions_t commNotifyOptions;
  }options;
  ZGP_GpdId_t  gpdId;
  uint8_t  endpoint;
  uint32_t  gpdSecurityFrameCounter;
  uint8_t  gpdCmdId;
  uint8_t  gpdCmdPayload[MAX_PAYLOAD_BY_GPD];
  uint16_t gppShortAddr;
  uint8_t  gppGpdLink;
  uint32_t  mic;
} ZGP_GpNotification_t;

typedef struct
{
  APS_AddrMode_t      addrMode;     //!< Address mode indicates which type of address shall be used
  APS_Address_t       addr;         //!< Can be set to either short, extended, or group addresses, depending on the address mode
  ShortAddr_t         aliasSrcAddr;   //!< Green power alias addr
  NwkSequenceNumber_t aliasSeqNumber; //!< Green power alias seq no
  uint8_t             radius;         //!< groupCast radius
  uint8_t             cmdDir;         //Client/Server Direction
  uint8_t             cmdId;          //cmdId of ZclCmd
} ZGP_Addressing_t;

/******************************************************************************
                    Externals
******************************************************************************/
/******************************************************************************
                    Prototypes section
******************************************************************************/
/**************************************************************************//**
  \brief send GPNotification/GP Commissioning Notification Command

  \param[in] req - pointer to ZGP Addressing
             payload - pointer to Notification Payload
             payLoadLength - length of the command payload length

  \return none
******************************************************************************/
bool ZGP_sendGpNotification(ZGP_Addressing_t *addressing, uint8_t *payload, uint8_t payloadLength);

#endif // _GREENPOWER_SUPPORT_

#endif // _ZGPMIDDLEPROXY_H

// eof zgpMiddleProxy.h
