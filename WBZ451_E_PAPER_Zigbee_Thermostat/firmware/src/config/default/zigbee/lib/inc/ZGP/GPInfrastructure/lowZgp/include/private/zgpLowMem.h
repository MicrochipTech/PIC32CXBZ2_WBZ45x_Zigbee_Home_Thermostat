/*******************************************************************************
  Zigbee green power low layer memory Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpLowMem.h

  Summary:
    This file contains the ZGP lower layer memory types.

  Description:
    This file contains the ZGP lower layer memory types.
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

#ifndef _ZGPLOWMEM_H
#define _ZGPLOWMEM_H

#ifdef _GREENPOWER_SUPPORT_
/******************************************************************************
                        Includes  section.
******************************************************************************/
#include <systemenvironment/include/sysQueue.h>
#include <hal/include/appTimer.h>
#ifdef ZGP_SECURITY_ENABLE
#include <security/serviceprovider/include/sspSfp.h>
#include <security/serviceprovider/include/sspHash.h>
#endif
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpDstub.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpPacket.h>
#include <zgp/include/zgpDbg.h>
#include <systemenvironment/include/sysAssert.h>

/******************************************************************************
                        Define section.
******************************************************************************/

/******************************************************************************
                        Types section.
******************************************************************************/
/** Internal variables of the zgp packet manager. */
typedef struct PACK
{
  zgpPacketManager_t        zgpPacketManager;
#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC
  SSP_KeyedHashMacReq_t zgpDerivedKeyHmacReq;
  void (*zgpKeyedHashMacCb)(void);
#endif
} ZGP_Mem_t;

/******************************************************************************
                        Global varible section.
******************************************************************************/
extern ZGP_Mem_t zgpMem;

/******************************************************************************
                        inline function section.
******************************************************************************/
/**************************************************************************//**
  \brief To get zgp packet manager memory
******************************************************************************/
INLINE zgpPacketManager_t* zgpMemPacketManager(void)
{
  return &zgpMem.zgpPacketManager;
}

#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC
/**************************************************************************//**
  \brief To get zgp key derivation req memory
******************************************************************************/
INLINE SSP_KeyedHashMacReq_t* zgpGetMemKeyedHmacReq(void)
{
  return &zgpMem.zgpDerivedKeyHmacReq;
}
#endif // APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC

/**************************************************************************//**
  \brief To get zgp memory
******************************************************************************/
INLINE ZGP_Mem_t* zgpGetMem(void)
{
  return &zgpMem;
}

#endif // _GREENPOWER_SUPPORT_
#endif //_ZGPLOWMEM_H

// eof zgpLowMem.h
