/*******************************************************************************
  Zigbee green power middle proxy Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpMiddleProxy.c

  Summary:
    This file contains the header file describes the ZGP_ZCL Interface.

  Description:
    This file contains the header file describes the ZGP_ZCL Interface.
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
#include <zgp/GPInfrastructure/highZgp/include/zgpMiddleProxy.h>
#include <zdo/include/zdo.h>
#include <security/serviceprovider/include/sspHash.h>
#include <zcl/include/zclCommandManager.h>

/******************************************************************************
                    Defines section
******************************************************************************/


/******************************************************************************
                    Prototypes section
******************************************************************************/


/******************************************************************************
                   Static variables section
******************************************************************************/

/******************************************************************************
                    Implementations section
******************************************************************************/
bool ZGP_sendGpNotification(ZGP_Addressing_t *addressing, uint8_t *payload, uint8_t payloadLength)
{
  ZCL_Request_t *req;
  uint8_t dir;
  
  if (!(req = ZCL_CommandManagerAllocCommand()))
    return false;

  dir = addressing->cmdDir;
  req->dstAddressing.addrMode = addressing->addrMode;
  req->dstAddressing.addr.shortAddress = addressing->addr.shortAddress;
  req->dstAddressing.radius = addressing->radius;
  req->dstAddressing.aliasSrcAddr = addressing->aliasSrcAddr;
  req->dstAddressing.aliasSeqNumber = addressing->aliasSeqNumber;
  req->dstAddressing.endpointId = GREEN_POWER_ENDPOINT;
  req->dstAddressing.manufacturerSpecCode = false;
  req->dstAddressing.profileId = GREEN_POWER_PROFILE_ID;
  req->dstAddressing.clusterId = GREEN_POWER_CLUSTER_ID;
  req->dstAddressing.clusterSide = (dir & DIR_MASK)? 1 : 0;
  req->endpointId = GREEN_POWER_ENDPOINT;
  req->requestLength = payloadLength;
  req->id = addressing->cmdId;
  req->defaultResponse = ZCL_FRAME_CONTROL_DISABLE_DEFAULT_RESPONSE;
  
  memcpy((void *)req->requestPayload, payload, req->requestLength);

  if(!(dir & ZGP_RESPONSE_MASK))
    req->dstAddressing.sequenceNumber = ZGP_GETNEXTSEQNUMBER();

#ifndef DISABLE_ZCL_ATTRIBUTE_REQ_SUPPORT
  if (dir & ATTRIBUTE_CMD_MASK)
    ZCL_CommandManagerSendAttribute(req);
  else
#endif
    ZCL_CommandManagerSendCommand(req);

  return true;
}
#endif //APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
#endif // _GREENPOWER_SUPPORT_

// eof zgpMiddleProxy.c
