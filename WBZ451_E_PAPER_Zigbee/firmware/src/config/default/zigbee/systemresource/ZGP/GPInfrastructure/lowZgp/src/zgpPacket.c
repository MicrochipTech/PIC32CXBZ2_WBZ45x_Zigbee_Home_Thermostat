/*******************************************************************************
  Zigbee green power packet Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpPacket.c

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

#if (APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC)
/******************************************************************************
                        Includes  section.
******************************************************************************/
#include <systemenvironment/include/sysQueue.h>
#ifdef ZGP_SECURITY_ENABLE
#include <security/serviceprovider/include/sspSfp.h>
#endif
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpDstub.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpPacket.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpLowMem.h>
#include <zgp/include/zgpDbg.h>
#include <configserver/include/configserver.h>
#include <systemenvironment/include/sysAssert.h>

/******************************************************************************
                          Global variables section
 ******************************************************************************/
static const uint8_t zgpBufferQuota[ZGP_BUFFER_TYPE_LAST-1] = \
                 {ZGP_INPUT_PACKET_COUNT, ZGP_OUTPUT_PACKET_COUNT};

/******************************************************************************
                             Local functions section
 ******************************************************************************/
static bool zgpCheckPacketQuotas(const zgpPacketType_t type, const uint8_t typeCount);

/******************************************************************************
                           Implementations section
 ******************************************************************************/

/**************************************************************************//**
  \brief Checking quotas for packet with given type.

  \param[in] type - type of input packet.
  \param[in] typeCount - current number of packet with given type.

  \return 'true' if quotas are not exceeded otherwise 'false'.
 ******************************************************************************/
static bool zgpCheckPacketQuotas(const zgpPacketType_t type, const uint8_t typeCount)
{
  if (typeCount < zgpBufferQuota[type-1])
  	return true;

  return false;
}

/**************************************************************************//**
  \brief To allocate zgp packet

  \param[in] type - type of packet.
  \return Pointer to packet or NULL.
 ******************************************************************************/
zgpPacket_t* zgpAllocPacket(const zgpPacketType_t type)
{
  zgpPacketManager_t *manager = zgpMemPacketManager();
  uint8_t typeCount = 0U;
  zgpPacket_t *freePkt = NULL;

  for (uint8_t index = 0; index < ZGP_DSTUB_BUFFERS_AMOUNT; index++)
  {
    if (ZGP_UNKNOWN_PACKET == manager->zgpPacketBuffer[index].type)
      freePkt = &manager->zgpPacketBuffer[index];
    else if (type == manager->zgpPacketBuffer[index].type)
     ++typeCount;
  }

  if (freePkt && zgpCheckPacketQuotas(type, typeCount))
  {
    memset((void*)freePkt,0x00u, sizeof(zgpPacket_t));
    freePkt->type = type;
    freePkt->state = ZGP_PACKET_IDLE;
    return freePkt;
  }
  return NULL;
}

/**************************************************************************//**
  \brief Release zgp Input/Output packet.

  \param[in] inPkt - pointer to a packet.

  \return None.
 ******************************************************************************/
void zgpFreeInOutPacket(void *const inPkt)
{
  zgpPacket_t *packet = GET_PARENT_BY_FIELD(zgpPacket_t,
	  pkt.in, inPkt);
 
  SYS_E_ASSERT_FATAL(((ZGP_UNKNOWN_PACKET != packet->type) && (ZGP_PACKET_IDLE == packet->state)), ZGP_DSTUB_PACKET_ZGPFREEBUFFER0);

  packet->type = ZGP_UNKNOWN_PACKET;
}

/**************************************************************************//**
  \brief Clear internal variables of the zgp packet manager.

  \param[in] None
  \return None.
 ******************************************************************************/
void zgpResetPacketManager(void)
{
  zgpPacketManager_t *const manager = zgpMemPacketManager();
  uint8_t i;

  for (i = 0U; i < ZGP_DSTUB_BUFFERS_AMOUNT; ++i)
  {
    manager->zgpPacketBuffer[i].type = ZGP_UNKNOWN_PACKET;
    manager->zgpPacketBuffer[i].state = ZGP_PACKET_IDLE;
  }

}
#endif //#if (APP_ZGP_DEVICE_TYPE != APP_ZGP_DEVICE_TYPE_NONE)
/** eof zgpPacket.c */

