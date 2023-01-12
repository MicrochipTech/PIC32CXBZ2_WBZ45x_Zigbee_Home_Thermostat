/*******************************************************************************
  Zigbee green power Dstub Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpDstub.h

  Summary:
    This file contains the dGP stub interface.

  Description:
    This file contains the dGP stub interface.
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

#ifndef _ZGPDSTUB_H
#define _ZGPDSTUB_H

#ifdef _GREENPOWER_SUPPORT_
/******************************************************************************
                           Includes section
******************************************************************************/
#include <zgp/include/zgpCommon.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpCstub.h>
#include <nwk/include/nwkAttributes.h>

/******************************************************************************
                           Definitions section
******************************************************************************/
// Max. payload is 64 bytes in case of src ID
// and 59 bytes in case of IEEE address
#define DSTUB_PAYLOAD_MAXSIZE 64

#define ZGP_BASIC_FRAME_LENGTH  2 // FCF(1 bytes) + 1(commmand id)
#define ZGP_MAINTENACE_FRAME_CMD_LENGTH  2 // 1(commmand id) + payload
#define ZGP_FRAME_MIN_SECURITY_HEADER 8 // 4 -FC 4 - MIC
/******************************************************************************
                           Types section
******************************************************************************/
typedef uint8_t ZGP_Security_Key_t[16];

typedef enum _zgpDstubSecResponseStatusCode_t
{
  ZGP_DSTUB_MATCH,
  ZGP_DSTUB_DROP_FRAME,
  ZGP_DSTUB_PASS_UNPROCESSED,
  ZGP_DSTUB_TX_THEN_DROP
} zgpDstubSecResponseStatusCode_t;

typedef struct PACK
{
  uint32_t gpdSecurityFrameCounter;
  zgpDstubSecResponseStatusCode_t status;
  ZGP_SecKeyType_t gpdfKeyType;
  ZGP_Security_Key_t gpdKey;
} zgpDstubSecResponse_t;


/******************************************************************************
                           Functions prototypes section
******************************************************************************/

/**************************************************************************//**
  \brief Init. Dstub.

  \param none.
  \return none.
******************************************************************************/
void zgpDstubInit(void);


#endif // _GREENPOWER_SUPPORT_
#endif //_ZGPDSTUB_H

//eof zgpDstub.h
