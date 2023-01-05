/*******************************************************************************
  Zigbee green power NVM table Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpNvmTable.h

  Summary:
    This file contains the green power nvm table interface.

  Description:
    This file contains the green power nvm table interface.
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

#ifndef _ZGPNVMTABLE_H
#define _ZGPNVMTABLE_H

#ifdef _GREENPOWER_SUPPORT_
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zgp/GPInfrastructure/lowZgp/include/zgpLow.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowNvmTable.h>
#include <pds/include/wlPdsMemIds.h>
/******************************************************************************
                    Defines
******************************************************************************/
/* 
PDS item IDs
ZGP_MAPPING_TABLE_MEM_ID        - Stores the banks in PDS 
ZGP_ENTRY_STARTING_ITEM_ID      - Stores the entries for each device
ZGP_FRAME_COUNTER_ITEM_ID       - Stores the Frame counters for each device
ZGP_NVM_TABLE_HEADER_MEM_ID   - Stores the table header information

Each bank will have 16 device entries, total no of banks will be based on No of entries defined 
by SINK_TABLE_SIZE + ZGP_PROXY_TABLE_SIZE.
For eg, There will be 4 banks if the total entries is 64, i.e, 64 / 16 = 4 banks.

Out of all the banks stored in the PDS with ZGP_MAPPING_TABLE_MEM_ID,
Only one bank will be cached into RAM(zgpMappingTableActiveBank) from NVM. 
Each bank mapping table will have the information about the 16 entries in that particular bank.

*/

#ifndef ZGP_MAX_NVM_ENTRIES
#define ZGP_MAX_NVM_ENTRIES                       64
#endif

#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
#define SINK_TABLE_SIZE  ZGP_SINK_TABLE_SIZE
#else
#define SINK_TABLE_SIZE  0
#endif

#if (SINK_TABLE_SIZE + ZGP_PROXY_TABLE_SIZE) > ZGP_MAX_NVM_ENTRIES
#error configured table size(proxy & sink) is more than ZGP_MAX_NVM_ENTRIES
#else
#define ZGP_TOTAL_TABLE_ENTRIES         (SINK_TABLE_SIZE + ZGP_PROXY_TABLE_SIZE)
#endif

// By default keeping the bank size as 16 for easy backward compatibility handling
#define ZGP_MAX_ENTRIES_PER_BANK          16 //  (64 entries / 16 entries per bank = 4 banks at max)

#define ZGP_MAPPING_TABLE_BANK_COUNT      ((ZGP_TOTAL_TABLE_ENTRIES / ZGP_MAX_ENTRIES_PER_BANK) + \
                                            ((ZGP_TOTAL_TABLE_ENTRIES % ZGP_MAX_ENTRIES_PER_BANK)? 1:0))

#define ZGP_MAPPING_TABLE_INFO_MASK_SIZE  (ZGP_MAPPING_TABLE_BANK_COUNT/8 + \
                                            (ZGP_MAPPING_TABLE_BANK_COUNT % 8 ? 1 : 0))

/******************************************************************************
                    Types section
******************************************************************************/
BEGIN_PACK
typedef enum PACK _ZgpNvmOperationType_t
{
  NVM_WRITE = 0x1,
  NVM_DELETE = 0x2,
  NVM_READ = 0x3
} zgpNvmOperationType_t;

typedef enum PACK _ZgpNvmOperationBitMask_t
{
  ENTRY_OPERATION                   = (1 << 0),
  SECURITY_FRAMECOUNTER_OPERATION   = (1 << 1),
} zgpNvmOperationFieldBitMask_t;

typedef enum PACK _ZgpMappingTableStatus_t
{
  ZGP_FREE_ENTRY_AVAILABLE = 0x0,
  ZGP_PROXY_ENTRY_PRESENT = 0x1,
  ZGP_SINK_ENTRY_PRESENT = 0x2,
  ZGP_STATUS_MAX_INDEX
} zgpMappingTableStatus_t;

typedef struct PACK _ZgpNvmTableHeader_t
{
  uint16_t version;
  uint8_t proxyTableSize;
  uint8_t sinkTableSize;
  uint8_t noOfBanks;
  uint8_t reserved[6];
} zgpNvmTableHeader_t;

typedef struct PACK _ZgpBankInfo_t
{
  uint8_t cachedBankNo;
  uint8_t noOfProxyTableEntries;
  // Specific to GPDs alone
  uint8_t noOfPhysicalSinkTableEntries;
  // Specific to GPD's communication mode
  uint8_t noOfLogicalSinkTableEntries;
  uint8_t bankStatus[ZGP_STATUS_MAX_INDEX][ZGP_MAPPING_TABLE_INFO_MASK_SIZE];
} zgpBankInfo_t;

END_PACK

typedef struct _zgpEntryFilterField_t
{
  uint16_t appId :2;
  uint16_t entryType :2;
  uint16_t commModeMask :4;
  // Reserved bits for future enhancment
  uint16_t reserved :8;
} zgpEntryFilterField_t;

typedef struct _ZgpMappingTable_t
{
  zgpEntryFilterField_t filterField;
  ZGP_GpdId_t gpdId;
  uint8_t endPoint;
} zgpMappingTable_t;

typedef struct _ZgpEntryNvmFormat_t
{
  ZGP_EntryType_t entryType;
  union
  {
    ZGP_ProxyTableEntry_t proxyEntry;
    ZGP_SinkTableEntry_t sinkEntry;
  } tableEntry;
} zgpEntryNvmFormat_t;

/******************************************************************************
                    Prototypes
******************************************************************************/

/**************************************************************************//**
\brief Read bank information

\param[in] None

\return   pointer to bank info
******************************************************************************/
zgpBankInfo_t * zgpTableGetBankInfo(void);

#endif // GREENPOWER_SUPPORT
#endif // _ZGPNVMTABLE_H

// eof zgpNvmTable.h
