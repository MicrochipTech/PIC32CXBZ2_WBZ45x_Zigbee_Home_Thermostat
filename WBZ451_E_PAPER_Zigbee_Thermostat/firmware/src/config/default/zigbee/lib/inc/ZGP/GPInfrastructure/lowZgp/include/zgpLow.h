// DOM-IGNORE-BEGIN
/*******************************************************************************
  Zigbee green power low layer Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpLow.h

  Summary:
    This file contains the generic APIs from low zgp.

  Description:
    This file contains the generic APIs from low zgp.
 *******************************************************************************/


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

// DOM-IGNORE-BEGIN
#ifndef _ZGPLOW_H
#define _ZGPLOW_H

#ifdef _GREENPOWER_SUPPORT_
// DOM-IGNORE-END

/**************************************************************************
                           Includes section
 *************************************************************************/
 
#include <zgp/include/zgpCommon.h>
#include <systemenvironment/include/sysQueue.h>

/************************************************************************
                           Definitions section
 *************************************************************************/

#define MAX_PAYLOAD_BY_GPD   64

#define GREEN_POWER_ENDPOINT 0xF2
#define GREEN_POWER_PROFILE_ID 0xA1E0

#define DIR_MASK (1u << 0)
#define ATTRIBUTE_CMD_MASK (1u << 1)
#define ZGP_RESPONSE_MASK (1u << 2)
#define INVALID_CMD_LENGTH        0xFF
/******************************************************************************
                           Types section
 ******************************************************************************/
 
BEGIN_PACK
typedef uint32_t ZGP_SourceId_t;

typedef union PACK _ZGP_GpdId_t
{
  ExtAddr_t gpdIeeeAddr;
  uint32_t gpdSrcId;
}ZGP_GpdId_t;

typedef enum PACK _ZGP_Mode_t
{
  COMMISSIONING_MODE = 0x01,
  OPERATIONAL_MODE =0x02
} ZGP_Mode_t;
END_PACK

/******************************************************************************
                           Functions prototypes section
 ******************************************************************************/
 
/**************************************************************************//**
  \brief To check whether the SrcID is valid or not considering the Frame Type

  \param - srcId  - GPD srcId
             frameType - Maintanence Frame/Data Frame
             isPairingConfig - srcId received in PairingConfig/Commissioning

  \return   true if srcId is Valid
          false otherwise
 ******************************************************************************/
bool ZGPL_IsValidSrcId(uint32_t srcId, ZGP_FrameType_t frameType, bool isPairingConfig);

/**************************************************************************//**
  \brief To check whether the key is valid or not

  \param - key

  \return   true - valid
          false - invalid
 ******************************************************************************/
bool ZGPL_IskeyValid(uint8_t *key);

/**************************************************************************//**
  \brief To set the Device in operational/Commissioning Mode

  \param - isProxy - true for proxy, false for sink

  \return  None
 ******************************************************************************/
void ZGPL_SetDeviceMode(bool isProxy, ZGP_Mode_t mode);

/**************************************************************************//**
  \brief To get the Device Mode (operational/Commissioning Mode)

  \param - isProxy - true for proxy, false for sink

  \return CurrentMode of Device
 ******************************************************************************/
ZGP_Mode_t ZGPL_GetDeviceMode(bool isProxy);

/**************************************************************************//**
  \brief To derive alias addr for the given srcId/Ieee addr

  \param - gpdId

  \return AliasAddr
 ******************************************************************************/
uint16_t ZGPL_GetAliasSourceAddr(ZGP_GpdId_t *gpdId);

/**************************************************************************//**
  \brief Sending simple descritor request

  \param shortAddr - addr of the node
             ep - endpoint of the node

  \return None.
 ******************************************************************************/
void ZGPL_SendSimpleDescReq(ShortAddr_t addr,uint8_t ep);

/**************************************************************************//**
  \brief Sending device annce

  \param     nwkAddr - nwk addr to be placed in the device annce
             extAddr- ext addr to be placed in the device annce

  \return None.
 ******************************************************************************/
void ZGPL_SendDeviceAnnounceCmd(uint16_t nwkAddr, uint64_t extAddr);

/**************************************************************************//**
  \brief Enable/Disable direct mode to receive GPDF

  \param enabled - true to enable, false to disable

  \return None.
 ******************************************************************************/
void ZGPL_EnableDisableDirectMode(bool enabled);
#endif  //GREENPOWER_SUPPORT
#endif //_ZGPLOW_H

//eof zgpLow.h
