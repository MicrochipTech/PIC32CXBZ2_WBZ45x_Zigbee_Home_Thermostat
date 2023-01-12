/*******************************************************************************
  Zigbee green power packet Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpPacket.h

  Summary:
    This file contains the ZGP packet handler.

  Description:
    This file contains the ZGP packet handler.
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

#ifndef _ZGPPACKET_H
#define _ZGPPACKET_H
#ifdef _GREENPOWER_SUPPORT_

/******************************************************************************
                        Includes  section.
******************************************************************************/
#include <systemenvironment/include/sysQueue.h>
#ifdef ZGP_SECURITY_ENABLE
#include <security/serviceprovider/include/sspSfp.h>
#endif
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowGpdf.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpDstub.h>
#include <mac_phy/include/mac.h>
/******************************************************************************
                                 Macros
******************************************************************************/
#ifndef ZGP_DSTUB_BUFFERS_AMOUNT
#define ZGP_DSTUB_BUFFERS_AMOUNT                                 3
#endif

#define ZGP_INPUT_PACKET_COUNT (ZGP_DSTUB_BUFFERS_AMOUNT / 2)
#define ZGP_OUTPUT_PACKET_COUNT  (ZGP_DSTUB_BUFFERS_AMOUNT - ZGP_INPUT_PACKET_COUNT)

/******************************************************************************
                        Types section.
******************************************************************************/
/** Input packet meta information. */
typedef struct
{
  ZGP_GpdfDataReq_t zgpDstubDataReq;
  MAC_DataReq_t macDataReq;
#ifdef ZGP_SECURITY_ENABLE
  ZGP_Security_Key_t secKey;
  SSP_ZgpEncryptFrameReq_t zgpEncryptReq;
#endif /* ZGP_SECURITY_ENABLE */
} zgpOutputPacket_t;

/** Input packet meta information. */
typedef struct
{
  struct
  {
    uint8_t *rxdFramePtr;
    uint8_t rxdFrameLength;
  } service;
  ZGP_LowDataInd_t zgpDataInd;
  zgpDstubSecResponseStatusCode_t secRspCode;
#ifdef ZGP_SECURITY_ENABLE
  ZGP_Security_Key_t secKey;
  SSP_ZgpDecryptFrameReq_t zgpDecryptReq;
#endif /* ZGP_SECURITY_ENABLE */
} zgpInputPacket_t;

typedef enum _ZgpPacketType_t
{
  ZGP_UNKNOWN_PACKET = 0x0,
  /** Data packet from Dstub input component. */
  ZGP_DSTUB_INPUT_DATA_PACKET = 0x1,
  /** Data packet from Dstub input component. */
  ZGP_DSTUB_OUTPUT_DATA_PACKET = 0x2,
  ZGP_BUFFER_TYPE_LAST
} zgpPacketType_t;

typedef enum _ZgpPacketState_t
{
  ZGP_PACKET_IDLE = 0x0,
  ZGP_PACKET_INPKT_MAINTENANCE_FRAME_PROCESSING = 0x1,
  ZGP_PACKET_INPKT_DATA_FRAME_PROCESSING = 0x2,
  ZGP_PACKET_INPKT_TO_PROCESS_FOR_RESPONSE = 0x3,
  ZGP_PACKET_INPKT_RESPONSE_SENDING_IN_PROCESS = 0x4,
  ZGP_PACKET_INPKT_INDICATION_TO_HIGHER_LAYER = 0x5,
  ZGP_PAKCET_INPKT_INDICATION_ON_HOLD = 0x6,
  ZGP_PACKET_INPKT_SECURITY_IN_PROCESS = 0x7,
  ZGP_PACKET_OUTPKT_IN_TXQUEUE = 0x8,
  ZGP_PACKET_OUTPKT_TX_IN_PROCESS = 0x9
} zgpPacketState_t;

/** Network packet type. */
typedef struct
{
  struct
  {
    void *next;
  }service;
  /** ZGP packet type. */
  zgpPacketType_t type;
  zgpPacketState_t state;
  uint8_t macPayload[MAX_ZGP_MSDU_SIZE];
  union
  {
    /** Service information for incoming packet from MAC-layer. */
    zgpInputPacket_t in;
    zgpOutputPacket_t out;
  } pkt;
} zgpPacket_t;

/** Internal variables of the zgp packet manager. */
typedef struct PACK
{
  zgpPacket_t zgpPacketBuffer[ZGP_DSTUB_BUFFERS_AMOUNT];
} zgpPacketManager_t;


/******************************************************************************
                        Prototypes section.
******************************************************************************/
/**************************************************************************//**
  \brief To allocate ZGP packet

  \param[in] type - packet type
  \return ZgpPacket_t * - Pointer to allocated memory.
 ******************************************************************************/
zgpPacket_t* zgpAllocPacket(const zgpPacketType_t type);

/**************************************************************************//**
  \brief To free ZGP input/output packet

  \param[in] packet - pointer to input/output packet not the ZgpPacket_t
  \return None.
 ******************************************************************************/
void zgpFreeInOutPacket(void *const packet);

/**************************************************************************//**
  \brief To reset zgp packet manager

  \param[in] dstubDataReq - request parameters
  \return None.
 ******************************************************************************/
void zgpResetPacketManager(void);


#endif // _GREENPOWER_SUPPORT_
#endif //_ZGPPACKET_H

// eof zgpPacket.h
