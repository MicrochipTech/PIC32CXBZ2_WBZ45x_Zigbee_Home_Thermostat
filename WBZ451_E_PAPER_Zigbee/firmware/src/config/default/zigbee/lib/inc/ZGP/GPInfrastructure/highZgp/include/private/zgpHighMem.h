/*******************************************************************************
  Zigbee green power higher layer memory Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpHighMem.h

  Summary:
    This file contains the ZGP memory types.

  Description:
    This file contains the ZGP memory types.
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

#ifndef _ZGPMEM_H
#define _ZGPMEM_H

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
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowGpdf.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighGeneric.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpInfraDevice.h>
#include <zgp/include/zgpDbg.h>
#include <systemenvironment/include/sysAssert.h>

/******************************************************************************
                        Define section.
******************************************************************************/
typedef enum
{
  REQ_BUFFER_IDLE = 0x0,
  REQ_BUFFER_BUSY = 0x1
} zgpReqBufferState_t;

/******************************************************************************
                        Types section.
******************************************************************************/
/** Internal variables of the zgp packet manager. */
typedef struct
{
  union
  {
    ZGP_GpdfDataReq_t dstubDataReq;
    ZGP_ProxyTableEntry_t proxyTableEntry;
    ZGP_SinkTableEntry_t sinkTableEntry;
    uint8_t gpResponse[sizeof(ZGP_GpResp_t)];
    uint8_t cmdPayload[ZGP_MAX_GSDU_SIZE];
    ZGP_IndicationInfo_t indicationInfo;
    zgpCommReq_t commReq;
  } buffer;
  zgpReqBufferState_t bufferState;
} zgpReqBufer_t;

/******************************************************************************
                        Global varible section.
******************************************************************************/
extern zgpReqBufer_t reqBuffer;

/******************************************************************************
                        inline function section.
******************************************************************************/
/**************************************************************************//**
  \brief To get zgp memory
******************************************************************************/
void* zgpGetMemReqBuffer(void);

/**************************************************************************//**
  \brief To free zgp memory
******************************************************************************/
void zgpFreeMemReqBuffer(void);

#endif // _GREENPOWER_SUPPORT_
#endif //_ZGPMEM_H

// eof zgpHighMem.h
