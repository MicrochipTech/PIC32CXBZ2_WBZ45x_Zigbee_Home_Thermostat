/**************************************************************************//**
  \file zgpMiddleProxy.h

  \brief
    The header file describes the ZGP_ZCL Interface
  \author
    Microchip Corporation: http://ww.microchip.com \n
    Support email: support@microchip.com

  Copyright (c) 2008-2015, Microchip Corporation. All rights reserved.
  Licensed under Microchip's Limited License Agreement.

  \internal
    History:
     09.11.17 Agasthian - Created.
******************************************************************************/
#ifndef _ZGPMIDDLEPROXY_H
#define _ZGPMIDDLEPROXY_H

#ifdef _GREENPOWER_SUPPORT_
#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zgp/include/zgpCommon.h>

#include <zcl/include/zcl.h>
#include <zcl/include/clusters.h>
#include <zllplatform/ZLL/N_Zcl_Framework/include/N_Zcl_Framework_Bindings.h>
#include <zllplatform/ZLL/N_Zcl_Framework/include/N_Zcl_Framework.h>

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
  uint8_t             radius;         //!< Forwarding radius
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
#endif // APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
#endif // _ZGPMIDDLEPROXY_H

// eof zgpMiddleProxy.h
