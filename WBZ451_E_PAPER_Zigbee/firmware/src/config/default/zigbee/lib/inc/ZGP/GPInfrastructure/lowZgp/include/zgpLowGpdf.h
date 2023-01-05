// DOM-IGNORE-BEGIN
/*******************************************************************************
  Zigbee green power Low layer Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpLowGpdf.h

  Summary:
    This file contains the GPDF(dstub) specific APIs from low zgp.

  Description:
    This file contains the GPDF(dstub) specific APIs from low zgp.
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
#ifndef _ZGPLOWGPDFINTERFACE_H
#define _ZGPLOWGPDFINTERFACE_H

#ifdef _GREENPOWER_SUPPORT_
// DOM-IGNORE-END

/******************************************************************************
                            Includes section
 ******************************************************************************/
#include <zgp/include/zgpCommon.h>
#include <systemenvironment/include/sysQueue.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowNvmTable.h>

/******************************************************************************
                           Types section
 ******************************************************************************/
 
typedef struct PACK
{
  //! The status of the last MSDU transmission.
  MAC_Status_t  status;
  uint8_t cmdId;
} ZGP_GpdfTxConfirm_t;

typedef void (*ZGP_LowGpdfTxConfirmCallback)(ZGP_GpdfTxConfirm_t *confirm);

typedef struct PACK
{
  ZGP_Status_t status;
} ZGP_GpdfDataConfirm_t;

typedef enum _ZGP_GpdfIndStatusCode_t
{
  ZGP_DSTUB_SECURITY_SUCCESS,
  ZGP_DSTUB_NO_SECURITY,
  ZGP_DSTUB_COUNTER_FAILURE,
  ZGP_DSTUB_AUTH_FAILURE,
  ZGP_DSTUB_UNPROCESSED
} ZGP_GpdfIndStatusCode_t;

typedef struct PACK
{
  uint8_t   useGpTxQueue  :1;
  uint8_t   performCsma   :1;
  uint8_t   requireMacAck :1;
  uint8_t   gpdfFrameType :2;
  uint8_t   txOnMatchingEndpoint :1;
  /** Reserved. Should always be zero. */
  uint8_t   reserved :2;
} ZGP_GpdfTxOptions_t;

typedef struct PACK
{
  ExtAddr_t gpdIeeeAddress;
  uint32_t gpTxQueueEntryLifeTime;
  ZGP_FrameDir_t frameDir;
  ZGP_GpdfTxOptions_t txOptions;
  ZGP_ApplicationId_t applicationId;
  ZGP_SourceId_t srcId;
  uint8_t endpoint;
  ZGP_CommandId_t gpdCommandId;
  uint8_t gpdAsduLength;
  uint8_t *gpdAsdu;
  uint8_t gpepHandle;
  //! ZGP confirm argument structure. Shall not be filled by user.
  ZGP_GpdfDataConfirm_t confirm;
  bool action;
} ZGP_GpdfDataReq_t;

typedef struct PACK
{
  struct
  {
    ZGP_SecKey_t sharedOrIndividualkeyType;
  }service;
  MAC_Addr_t srcAddress;
  uint32_t srcId;
  uint32_t gpdSecurityFrameCounter;
  uint32_t mic;
  PanId_t srcPanId;
  ZGP_GpdfIndStatusCode_t status;
  ZGP_FrameType_t frameType;
  uint8_t linkQuality;
  int8_t rssi;
  uint8_t seqNumber;
  MAC_AddrMode_t srcAddrMode;
  ZGP_ApplicationId_t applicationId;
  ZGP_SecLevel_t gpdfSecurityLevel;
  ZGP_SecKeyType_t gpdfKeyType;
  uint8_t endPoint;
  uint8_t gpdCommandId;
  uint8_t gpdAsduLength;
  uint8_t *gpdAsdu;
  bool autoCommissioning;
  bool rxAfterTx;
} ZGP_LowDataInd_t;

/******************************************************************************
                           Functions prototypes section
 ******************************************************************************/

/**************************************************************************//**
  \brief Init. low zgp

  \param None.
  \return None.
 ******************************************************************************/
void ZGPL_Init(void);

/**************************************************************************//**
  \brief Flushout gpTxqueue

  \param None
  \return None.
 ******************************************************************************/
void ZGPL_FlushTxQueue(void);

/**************************************************************************//**
  \brief GP-Data-Request(Raising from higher layer to dStub)

  \param zgpGpdfDataReq - request parameters

  \return None.
 ******************************************************************************/
void ZGPL_GpdfDataRequest(ZGP_GpdfDataReq_t *zgpGpdfDataReq);

#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_COMBO_BASIC
/**************************************************************************//**
  \brief To derive the GPD security key from network/group key. This is mainly used by high sink.

  \param sinkTableEntry - sink entry
             derivedKey - key to be derived
             keyedHashMacCb - callback function

  \return None
 ******************************************************************************/
void ZGPL_KeyDerivationHmac(ZGP_SinkTableEntry_t* sinkTableEntry, uint8_t *derivedKey, void (*keyedHashMacCb)(void));
#endif

/**************************************************************************//**
  \brief  check for duplicate packet

  \param dstubDataInd - dstub data indication

  \return   true - non-duplicate packet
          false otherwise
 ******************************************************************************/
bool ZGPL_CheckForDuplicate(ZGP_LowDataInd_t *dStubDataInd);

#endif //GREENPOWER_SUPPORT
#endif //_ZGPLOWGPDF_H

//eof zgpLowGpdf.h
