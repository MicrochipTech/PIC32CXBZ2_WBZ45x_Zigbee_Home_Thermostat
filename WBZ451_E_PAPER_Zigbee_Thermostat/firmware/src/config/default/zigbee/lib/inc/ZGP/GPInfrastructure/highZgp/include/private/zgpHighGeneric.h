/*******************************************************************************
  Zigbee green power high generic Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpHighGeneric.h

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

#ifndef _ZGPHIGHGENERIC_H
#define _ZGPHIGHGENERIC_H

#ifdef _GREENPOWER_SUPPORT_
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcl.h>
#include <zcl/include/zclGreenPowerCluster.h>
#include <zgp/include/zgpCommon.h>

/******************************************************************************
                           Definitions section
******************************************************************************/
// Boundary check for received frames

#define ZGP_FRAME_COMM_REQ_BASIC_FRAME_LENGTH  2// device id & options
#define ZGP_FRAME_COMM_REQ_EXT_OPTION_LENGTH 1

/******************************************************************************
                    Types section
******************************************************************************/

typedef enum _ZGP_ProxyDataFrameType_t
{
  GPDF_DATA_FRAME = 0x00,
  GPDF_UNPROCESSED_DATA_FRAME = 0x01,
  GPDF_COMMISSIONING_FRAME = 0xE0,
  GPDF_DECOMMISSIONING_FRAME = 0xE1,
  GPDF_COMMISSIONING_SUCCESS_FRAME = 0xE2,
  GPDF_CHANNEL_REQUEST_FRAME = 0xE3,
  GPDF_APPLICATION_DESCR_FRAME = 0xE4,
  GPDF_RESERVED_CMD1 = 0xE5,
  GPDF_RESERVED_CMD2 = 0xE6,
  GPDF_RESERVED_CMD3 = 0xE7,
  GPDF_RESERVED_CMD4 = 0xE8,
  GPDF_RESERVED_CMD5 = 0xE9,
  GPDF_RESERVED_CMD6 = 0xEA,
  GPDF_RESERVED_CMD7 = 0xEB,
  GPDF_RESERVED_CMD8 = 0xEC,
  GPDF_RESERVED_CMD9 = 0xED,
  GPDF_RESERVED_CMDA = 0xEE,
  GPDF_RESERVED_CMDB = 0xEF,
  GPDF_MANUFAC_SPECIFIC_CMD0_FRAME = 0xB0,
  GPDF_MANUFAC_SPECIFIC_CMD1_FRAME = 0xB1,
  GPDF_MANUFAC_SPECIFIC_CMD2_FRAME = 0xB2,
  GPDF_MANUFAC_SPECIFIC_CMD3_FRAME = 0xB3,
  GPDF_MANUFAC_SPECIFIC_CMD4_FRAME = 0xB4,
  GPDF_MANUFAC_SPECIFIC_CMD5_FRAME = 0xB5,
  GPDF_MANUFAC_SPECIFIC_CMD6_FRAME = 0xB6,
  GPDF_MANUFAC_SPECIFIC_CMD7_FRAME = 0xB7,
  GPDF_MANUFAC_SPECIFIC_CMD8_FRAME = 0xB8,
  GPDF_MANUFAC_SPECIFIC_CMD9_FRAME = 0xB9,
  GPDF_MANUFAC_SPECIFIC_CMDA_FRAME = 0xBA,
  GPDF_MANUFAC_SPECIFIC_CMDB_FRAME = 0xBB,
  GPDF_MANUFAC_SPECIFIC_CMDC_FRAME = 0xBC,
  GPDF_MANUFAC_SPECIFIC_CMDD_FRAME = 0xBD,
  GPDF_MANUFAC_SPECIFIC_CMDE_FRAME = 0xBE,
  GPDF_MANUFAC_SPECIFIC_CMDF_FRAME = 0xBF,
  ZDO_DEVICE_ANNOUNCE_FRAME = 0x04,
  GP_CLUSTER_GP_PAIRING_CMD = 0x05,
  GP_CLUSTER_GP_PROXY_COMMISSIONING_CMD = 0x06,
  GP_CLUSTER_GP_RESPONSE_CMD = 0x07
} zgpProxyDataFrameType_t;

typedef enum _zgpCommModeType_t
{
  NO_COMM_MODE = 0x00,
  COMM_MODE_WAIT_FOR_PAIRING_SUCCESS = 0x01,
  COMM_MODE_WAIT_FOR_TIMEOUT         = 0x02,
  COMM_MODE_WAIT_FOR_EXIT            = 0x04
} zgpCommModeType_t;

typedef struct PACK
{
  MAC_SetReq_t         channelSet;
  uint8_t              operationalChannel;
  bool                 inTransmitChannel;
} zgp_MacReq_t;

/******************************************************************************
                    Externals
******************************************************************************/


/******************************************************************************
                    Prototypes
******************************************************************************/
/**************************************************************************//**
\brief Initialize High zgp generic feature.
******************************************************************************/
void zgpHighGenericInit(void);

/**************************************************************************//**
\brief Handling channel change request from proxy/sink basic
******************************************************************************/
bool zgpGenericChannelChangeHandling(uint8_t tempChannel);

/**************************************************************************//**
\brief Transmit channel check - To Check proxy/Sink is in transmit channel
******************************************************************************/
bool zgpGenericNotInTransmitChannnel(void);
#endif // _GREENPOWER_SUPPORT_

#endif // _ZGPGENERIC_H

// eof zgpHighGeneric.h
