/*******************************************************************************
  Zigbee green power Common Stub Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpCstub.c

  Summary:
    This file contains the commong GP stub implementation.

  Description:
    This file contains the commong GP stub implementation.
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
                   Includes section
******************************************************************************/

#include <zgp/include/zgpCommon.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpCstub.h>
#include <zgp/include/zgpDbg.h>
#ifdef _GREENPOWER_SUPPORT_
#include <nwk/include/nldeData.h>
#endif
#include <systemenvironment/include/sysAssert.h>

/******************************************************************************
                            Definitions section.
******************************************************************************/

/******************************************************************************
                            Types section
******************************************************************************/

/******************************************************************************
                   Static functions prototype section
******************************************************************************/
#ifdef _GREENPOWER_SUPPORT_
static void macDataInd(MAC_DataInd_t *indParams);
#endif

/******************************************************************************
                   Global variables section
******************************************************************************/


/******************************************************************************
                   Static variables section
******************************************************************************/
static zgpCstubDataIndCallback_t zgpCstubGpDataIndCallback;

/******************************************************************************
                   Implementation section
******************************************************************************/
/**************************************************************************//**
  \brief Init. cGP stub.

  \param none.
  \return none.
******************************************************************************/
void zgpCstubInit(void)
{
#ifdef _GREENPOWER_SUPPORT_
  // Registering callback for macDataInd to NWK layer
  NWK_GreenPowerPacketIndRegisterCallback(macDataInd);
#endif
}

/**************************************************************************//**
  \brief MAC data indication to cGP stub

  \param[in] macDataInd - MCPS-DATA indication primitive's parameters, see
      IEEE 802.15.4-2006, 7.1.1.3 MCPS-DATA.indication.
  \return None.
 ******************************************************************************/
static void macDataInd(MAC_DataInd_t *indParams)
{
  ZGP_NwkFrameControl_t *cstubNwkFrameControl = (ZGP_NwkFrameControl_t *)(indParams->msdu);

  if (cstubNwkFrameControl->isExtNwkFcf)
  {
    // received length less than FCF + EXT_FCF, the drop the packet
    if (indParams->msduLength < (ZGP_EXT_NWK_FCF_LENGTH + ZGP_NWK_FCF_LENGTH))
      return;
    // Extracting the extended network frame control field.
    ZGP_ExtNwkFrameControl_t *cstubExtNwkFrameControl = (ZGP_ExtNwkFrameControl_t *)(indParams->msdu+1);

    if (!(((ZGP_SRC_APPID == cstubExtNwkFrameControl->appId) || \
	     (ZGP_IEEE_ADDR_APPID == cstubExtNwkFrameControl->appId)) && \
	     (ZGP_TX_BY_ZGPD == cstubExtNwkFrameControl->direction)))
      return;
    // Forwarding to LPED will be handled here.
    // else {}
  }

  if (zgpCstubGpDataIndCallback)
    zgpCstubGpDataIndCallback(indParams);

}

#ifndef _GREENPOWER_SUPPORT_
/**************************************************************************//**
  \brief Indication from MAC-layer about receiving a new frame.

  \param[in] macDataInd - MCPS-DATA indication primitive's parameters, see
      IEEE 802.15.4-2006, 7.1.1.3 MCPS-DATA.indication.
  \return None.
 ******************************************************************************/
void MAC_DataInd(MAC_DataInd_t *indParams)
{
  macDataInd(indParams);
}
#endif

/**************************************************************************//**
  \brief Subscribes for cStub Indication callback

  It is intended to notify subcribed Layer about incoming data packet.

  \param[in] callback - Callback The callback from subscriber
  \return None.
 ******************************************************************************/
void zgpCstubGpDataIndRegisterCallback(zgpCstubDataIndCallback_t callback)
{
  SYS_E_ASSERT_ERROR(callback && !zgpCstubGpDataIndCallback, ZGP_CSTUB_DATAINDALREADYREGISTERED);
  zgpCstubGpDataIndCallback = callback;
}

#endif //#if (APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC)
//eof zgpCstub.c
