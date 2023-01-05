/*******************************************************************************
  Zigbee green power Cstub Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpCstub.h

  Summary:
    This file contains the cGP stub interface.

  Description:
    This file contains the interface and types of cGP stub.
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

#ifndef _ZGPCSTUB_H
#define _ZGPCSTUB_H

#ifdef _GREENPOWER_SUPPORT_
/******************************************************************************
                           Includes section
******************************************************************************/
#include <zgp/include/zgpCommon.h>
#include <systemenvironment/include/sysQueue.h>

/******************************************************************************
                           Types section
******************************************************************************/
/**//**
 * \brief Parameter of NWK_GreenPowerPacketIndRegisterCallback
 */
typedef void (*zgpCstubDataIndCallback_t)(MAC_DataInd_t *ind);

/******************************************************************************
                           Functions prototypes section
******************************************************************************/

/**************************************************************************//**
  \brief Init. cGP stub.

  \param none.
  \return none.
******************************************************************************/
void zgpCstubInit(void);

/**************************************************************************//**
  \brief To subscribe for cStub Indication

  It is intended to notify subcribed Layer about incoming data packet.

  \param[in] callback - Callback The callback from subscriber
  \return None.
 ******************************************************************************/
void zgpCstubGpDataIndRegisterCallback(zgpCstubDataIndCallback_t callback);

#endif
#endif //_ZGPCSTUB_H

//eof zgpCstub.h
