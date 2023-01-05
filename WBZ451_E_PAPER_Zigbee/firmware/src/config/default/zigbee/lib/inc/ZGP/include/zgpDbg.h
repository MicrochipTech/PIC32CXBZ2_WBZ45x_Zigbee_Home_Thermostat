/*******************************************************************************
  zigbee green power debug Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpDbg.h

  Summary:
    This file contains the zgp debug codes

  Description:
    This file contains the zgp debug codes
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

#ifndef _ZGPDBG_H
#define _ZGPDBG_H

/******************************************************************************
                           Includes section
******************************************************************************/
#include <systemenvironment/include/sysTypes.h>
#include <configserver/include/csDefaults.h>
#include <systemenvironment/include/dbg.h>

/******************************************************************************
                           Definitions section
******************************************************************************/

/******************************************************************************
                           Types section
******************************************************************************/
/** GP assert code. Range 0xC000 - 0xCFFF. */
typedef enum _ZgpDbgCode_t
{
  ZGP_CSTUB_DATAINDALREADYREGISTERED     = 0xC001,
  ZGP_DSTUB_DATAINDALREADYREGISTERED     = 0xC002,
  ZGP_DSTUB_CHANNELCONFIGTXALREADYREG    = 0xC003,
  ZGP_TASKMANAGER_TASKHANDLER_0          = 0xC004,
  ZGP_DSTUB_PACKET_ZGPFREEBUFFER0        = 0xC005,
  ZGP_DSTUB_RXDATARESP0                  = 0xC006,
  ZGP_DSTUB_TXDATACONF0                  = 0xC007,
  ZGP_LOW_TXCONFIRMCALLBACKALREADYREG    = 0xC008,
  ZGP_PROXYBASIC_DATAFRAMETYPE0          = 0xC009,
  ZGP_TABLE_ENTRY_INVALID_LENGTH         = 0xC00A,
  ZGP_CLIENT_INVALID_GP_BUFFER_LENGTH    = 0xC00B,
  ZGP_TABLE_ENTRY_NOT_FOUND_ON_NVM_1     = 0xC00C,
  ZGP_TABLE_ENTRY_NOT_FOUND_ON_NVM_2     = 0xC00D,
  ZGP_TABLE_ENTRY_NOT_FOUND_ON_NVM_3     = 0xC00E,
  ZGP_TABLE_INACTIVE_ENTRY_FOUND         = 0xC00F,
  ZGP_TABLE_ENTRY_NOT_INIT_1             = 0xC010,
  ZGP_TABLE_ENTRY_NOT_INIT_2             = 0xC011,
  ZGP_TABLE_ENTRY_NOT_INIT_3             = 0xC012,
  ZGP_TABLE_ENTRY_NOT_INIT_4             = 0xC013,
  ZGP_TABLE_ENTRY_NOT_INIT_5             = 0xC014,
  ZGP_TABLE_ENTRY_NOT_INIT_6             = 0xC015,
  ZGP_TABLE_ENTRY_WRITE_FAILURE_1        = 0xC016,
  ZGP_TABLE_ENTRY_WRITE_FAILURE_2        = 0xC017,
  ZGP_INVALID_TABLE_INDEX                = 0xC018,
  ZGP_INVALID_BANK_NO_IN_BANK_SCANNING   = 0xC019,
  ZGP_MEM_REQ_BUFFER_NOT_RELEASED        = 0xC01A,
  ZGP_DSTUB_SECURED_FRAME_INVALID_STATE  = 0xC01B,
  ZGP_TABLE_ENTRY_NOT_FOUND              = 0xC01C,
  ZGP_SINK_ENTRY_NO_VALID_COMM_MODE      = 0xC01D,
  ZGP_SINK_INVALID_REPORT_DESC_BUFFER    = 0xC01E,
  ZGP_SINK_REPORT_DESC_INVALID_PTR       = 0xC01F,
  ZGP_SINK_DATA_POINT_DESC_INVALID_PTR   = 0xC020,
  ZGP_NVM_TABLE_HEADER_WRITE_FAILURE_1   = 0xC021,
  ZGP_NVM_TABLE_HEADER_WRITE_FAILURE_2   = 0xC022,
  ZGP_NVM_TABLE_HEADER_INIT_FAILURE_1    = 0xC023,
  ZGPDCOMMISSIONING_CALLBACKMISSING0     = 0xD001,
  ZGPD_INVALIDCALLBACK                   = 0xD002,
  ZGPD_INVALIDREQUESTID                  = 0xD003,
} ZgpDbgCode_t;

#define FLAG1     1
#define FLAG2     2
#define FLAG3     4
#define FLAG4     8
#define FLAG5     16
#define FLAG6     32
#define FLAG7     64
#define FLAG8     128
#define FLAG9     256
#define FLAG10    512
#define FLAG11    1024
#define FLAG12    2048
#define FLAG13    4096
#define FLAG14    8192
#define FLAG15    16384
#define FLAG16    32768

#endif //_ZGPDBG_H

//eof zgpDbg.h
