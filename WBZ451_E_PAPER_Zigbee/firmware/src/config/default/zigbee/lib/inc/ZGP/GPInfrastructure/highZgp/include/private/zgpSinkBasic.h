/*******************************************************************************
  Zigbee green power sink basic Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpSinkBasic.h

  Summary:
    This file contains the zgp sink basic interface.

  Description:
    This file contains the zgp sink basic interface.
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

#ifndef _ZGPSINKBASIC_H
#define _ZGPSINKBASIC_H

#ifdef _GREENPOWER_SUPPORT_
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcl.h>
#include <zcl/include/zclGreenPowerCluster.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighGeneric.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighMem.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpInfraDevice.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowGpdf.h>
/******************************************************************************
                    Defines
******************************************************************************/
//GPS commissioning exit mode values
#define GPS_EXIT_ON_COMM_WINDOW_EXPIRATION   0x01
#define GPS_EXIT_ON_FIRST_PAIRING_SUCCESS    0x02
#define GPS_EXIT_ON_PROXY_COMM_MODE_EXIT     0x04

//The default secuirty level as per spec: 0x06
//Setting minGpdSecurityLevel to (0x02), protectionWithGpLinkKey enabled(0x01)
#define ZGP_SINK_DEFAULT_SECURITY_LEVEL_ATTR_VALUE 0x06
/******************************************************************************
                    Types
******************************************************************************/
typedef enum PACK
{
  COMM_IDLE = 0x00,
  COMM_RXD_GPD_KEY_DECRYPTION,
  COMM_WAITING_FOR_COMPLETE_COM_REQ_INFO,
  COMM_PROCESS_COMPLETE_COMM_INFO,
  COMM_COMM_REPLY_HANDLING,
  COMM_GPD_KEY_DERIVATION,
  COMM_TX_GPD_KEY_ENCRYPTION,
  COMM_FRAME_AND_SEND_COMM_REPLY,
  COMM_WAIT_FOR_SUCCCESS_IND,
  COMM_SUCCESSFUL_SESSION,
  COMM_COMPLETE,
  COMM_WAIT_FOR_PAIRING_CONFIG_TX
} sinkCommProcState_t;

typedef struct PACK
{
  uint8_t rxChannelInNextAttempt      : 4;
  uint8_t rxChannelInSecondAttempt    : 4;
} zgpChannelReq_t;

typedef struct PACK
{
  uint8_t operationalChannel    : 4;
  uint8_t basic                 : 1;
  uint8_t reserved              : 3;
} zgpChannelConfig_t;

typedef struct PACK
{
  uint8_t panidPresent        : 1;
  uint8_t gpdSecurityKeyPresent  : 1;
  uint8_t gpdKeyEncryption    : 1;
  uint8_t securityLevel       : 2;
  uint8_t keyType             : 3;
} zgpCommReplyOptions_t;

typedef struct PACK
{
  zgpCommReplyOptions_t options;
  uint16_t panId;
  uint8_t gpdSecurityKey[ZGP_SECURITY_KEY_LENGTH];
  uint32_t mic;
  uint32_t frameCounter;
} zgpCommReply_t;

typedef enum _zgpSinkState_t
{
  IDLE_STATE = 0x00,
  RX_GPDF_DECRYPTION_STATE = 0x01,
  RX_GPDF_PARSING_STATE = 0x02,
  TX_RESPONSE_ENCRYPTION_STATE = 0x03,
  TX_RESPONSE_SENDING_STATE = 0x04,
  GPDF_PROCESSING_COMPLETE = 0x05
} zgpSinkState_t;

typedef struct PACK
{
  uint8_t commissioningGpdEp;
} zgpSinkBasicModeInfo_t;

/******************************************************************************
                    Externals
******************************************************************************/

/******************************************************************************
                    Prototypes
******************************************************************************/
/**************************************************************************//**
\brief Handling incoming frame.
******************************************************************************/
void zgpSinkDstubDataInd(const ZGP_LowDataInd_t *const ind);

/**************************************************************************//**
\brief Handling GPDF receiving from cluster client.
******************************************************************************/
void zgpSinkGpdfHandling(ZGP_LowDataInd_t *gpdfDataInd, bool responseToGpd, uint16_t gppShortAddr);

#endif // APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
#endif // _GREENPOWER_SUPPORT_
#endif // _ZGPSINKBASIC_H

// eof zgpSinkBasic.h
