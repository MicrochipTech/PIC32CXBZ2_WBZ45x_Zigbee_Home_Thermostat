/*******************************************************************************
  Zigbee green power NVM table Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpNvmTable.c

  Summary:
    This file contains the zgp infrastructure generic handling for proxy/Sink table.

  Description:
    This file contains the zgp infrastructure generic handling for proxy/Sink table.
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

#ifdef _GREENPOWER_SUPPORT_

#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC

/******************************************************************************
                    Includes section
******************************************************************************/
#include <pds/include/wlPdsMemIds.h>
#if MICROCHIP_APPLICATION_SUPPORT == 1
#include <zllplatform/ZLL/N_DeviceInfo/include/N_DeviceInfo.h>
#endif
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpLowGeneric.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpNvmTable.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLow.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowNvmTable.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpLowSink.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpLowProxy.h>
#include <zgp/include/zgpDbg.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpLowMem.h>
#include <zllplatform/infrastructure/N_ErrH/include/N_ErrH.h>
#include <systemenvironment/include/sysAssert.h>

/******************************************************************************
                    Defines section
******************************************************************************/
typedef struct {
  uint8_t indexNo;
  uint8_t bankNo;
  uint8_t commModeMaskPos;
} indexSearchResult_t;

/******************************************************************************
                    externs section
******************************************************************************/
extern OSAL_SEM_HANDLE_TYPE semZgpInternalHandler;
/******************************************************************************
                    Internal variables
******************************************************************************/
static PDS_Operation_Offset_t pdsItem;
static zgpBankInfo_t zgpBankInfo;
static zgpMappingTable_t zgpMappingTableActiveBank[ZGP_MAX_ENTRIES_PER_BANK];
static zgpEntryNvmFormat_t cachedEntry;
static zgpNvmTableHeader_t nvmTableHeader;
static uint32_t deviceSecFrameCounter;
/*Device entry mapping table*/
PDS_DECLARE_ITEM(ZGP_MAPPING_TABLE_MEM_ID, \
                              sizeof(zgpMappingTableActiveBank) * ZGP_MAPPING_TABLE_BANK_COUNT, \
                              zgpMappingTableActiveBank, NULL, NO_ITEM_FLAGS);
/*Device Entry tabel*/
PDS_DECLARE_ITEM(ZGP_ENTRY_STARTING_ITEM_ID, (sizeof(cachedEntry) * ZGP_TOTAL_TABLE_ENTRIES), \
                                          &cachedEntry, NULL, NO_ITEM_FLAGS);

/*Device Frame counter - kept as separate item as this will be frequently accessed*/
PDS_DECLARE_ITEM(ZGP_FRAME_COUNTER_ITEM_ID, (sizeof(uint32_t) * ZGP_TOTAL_TABLE_ENTRIES), \
                                            &deviceSecFrameCounter, NULL, NO_ITEM_FLAGS);

/* NVM table header item having table size, bank size and version info */
PDS_DECLARE_ITEM(ZGP_NVM_TABLE_HEADER_MEM_ID, sizeof(nvmTableHeader), \
                                          &nvmTableHeader, NULL, NO_ITEM_FLAGS);

/******************************************************************************
                    Local functions
******************************************************************************/
static void scanTableEntriesOnNvm(void);
static bool checkBankStatus(uint8_t bankNo, zgpMappingTableStatus_t bankStatus);
static void updateBankStatus(uint8_t bankNo, zgpMappingTableStatus_t bankStatus, bool set);
static void tableEntryNvmOperation(uint8_t index, uint32_t *frameCounter, zgpNvmOperationType_t nvmOperation, uint8_t fieldMask);
static void updateNvmMappingTable(uint8_t bankNo);
static void updateMappingTableToCache(uint8_t bankNo);
static indexSearchResult_t getMappingTableIndex(ZGP_TableOperationField_t  tableOperationField ,\
                                                ZGP_GpdId_t *gpdId, uint8_t endPoint);
static bool checkCacheEntry(ZGP_TableOperationField_t filterField, ZGP_GpdId_t *gpdId, uint8_t endPoint);
static uint8_t getNextBankToBeScanned(bool *bankScanned);
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC 
static uint8_t getLogicalEntryCount(uint8_t commModeMask);
#endif
static void tableEntryMigration(zgpNvmTableHeader_t *nvmHeader);
static void nvmTableLowInit(void);

static void pdsNvmEntryStoreCallback(PDS_MemId_t itemID);

/*Externing required functions from PDS library*/
extern bool PDS_StoreItem(PDS_Operation_Offset_t *pdsItem);
extern bool PDS_Restore_Offset(PDS_Operation_Offset_t *item);

/**************************************************************************//**
                  Implementations section
******************************************************************************/
/**************************************************************************//**
\brief Initialize zgp table
******************************************************************************/
void ZGPL_NvmTableInit(void)
{
  // Resetting the cache entry
  memset((void *)&cachedEntry, 0x00, sizeof(zgpEntryNvmFormat_t));
  cachedEntry.entryType = ZGP_UNINIT_TABLE_TYPE;
  PDS_RegisterWriteCompleteCallback(pdsNvmEntryStoreCallback);
  nvmTableLowInit();
  PDS_InitItems(ZGP_MAPPING_TABLE_MEM_ID,ZGP_FRAME_COUNTER_ITEM_ID);
  scanTableEntriesOnNvm();
}

static void pdsNvmEntryStoreCallback(PDS_MemId_t itemID)
{
  uint8_t countEvents;
#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
  if(itemID == ZGP_ENTRY_STARTING_ITEM_ID || itemID == ZGP_FRAME_COUNTER_ITEM_ID
          || ZGP_MAPPING_TABLE_MEM_ID)
  {
    OSAL_SEM_Post(&semZgpInternalHandler);
    return;
  }
#endif
}
/**************************************************************************//**
\brief Low level Initialize for nvm table handling compatibility
******************************************************************************/
static void nvmTableLowInit(void)
{
  if (!PDS_IsAbleToRestore(ZGP_NVM_TABLE_HEADER_MEM_ID))
  {
    // no header available so initialize it
    nvmTableHeader.version        = ZGP_NVM_TABLE_VERSION;
    nvmTableHeader.sinkTableSize  = SINK_TABLE_SIZE;
    nvmTableHeader.proxyTableSize = ZGP_PROXY_TABLE_SIZE;
    nvmTableHeader.noOfBanks      = ZGP_MAPPING_TABLE_BANK_COUNT;

    PDS_Store(ZGP_NVM_TABLE_HEADER_MEM_ID);
  }
  else
  {
    if (ZGP_NVM_TABLE_VERSION == nvmTableHeader.version)
    {
      if ((ZGP_PROXY_TABLE_SIZE != nvmTableHeader.proxyTableSize)
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
           || (SINK_TABLE_SIZE != nvmTableHeader.sinkTableSize)
#endif
          )
      {
        //tableEntryMigration(&nvmTableHeader);
        nvmTableHeader.sinkTableSize  = SINK_TABLE_SIZE;
        nvmTableHeader.proxyTableSize = ZGP_PROXY_TABLE_SIZE;
        nvmTableHeader.noOfBanks      = ZGP_MAPPING_TABLE_BANK_COUNT;
        PDS_Store(ZGP_NVM_TABLE_HEADER_MEM_ID);
      }
    }
    else
    {
      // Needs to be handled later when version updated because table structure changes
    }
  }
}
#if 0
/**************************************************************************//**
\brief Migrating the table with different size
******************************************************************************/
static void tableEntryMigration(zgpNvmTableHeader_t *nvmHeader)
{
  uint8_t proxyEntryCount = 0;
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC 
  uint8_t sinkEntryCount = 0;
#endif

  for (uint8_t bankCount = 0; bankCount < nvmHeader->noOfBanks; bankCount++)
  {
    bool bankUpdated = false;

    updateMappingTableToCache(bankCount);

    for (uint8_t j = 0; j < ZGP_MAX_ENTRIES_PER_BANK; j++)
    {
      if (ZGP_PROXY_ENTRY == zgpMappingTableActiveBank[j].filterField.entryType)
      {
        proxyEntryCount++;
        if (proxyEntryCount > ZGP_PROXY_TABLE_SIZE)
        {
          memset(&zgpMappingTableActiveBank[j], 0xFF, sizeof(zgpMappingTable_t));
          bankUpdated = true;
        }
      }
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC       
      else if (ZGP_SINK_ENTRY == zgpMappingTableActiveBank[j].filterField.entryType)
      {
        sinkEntryCount++;
        if (sinkEntryCount > SINK_TABLE_SIZE)
        {
          memset(&zgpMappingTableActiveBank[j], 0xFF, sizeof(zgpMappingTable_t));
          bankUpdated = true;
        }
      }
#endif
    }
    if (bankUpdated)
    {
      updateNvmMappingTable(bankCount);
    }
  }
  scanTableEntriesOnNvm();
  // moving entries from additional bank to current bank
  if (nvmHeader->noOfBanks > ZGP_MAPPING_TABLE_BANK_COUNT)
  {
    for (uint8_t bankCount = ZGP_MAPPING_TABLE_BANK_COUNT; bankCount <= (nvmHeader->noOfBanks - ZGP_MAPPING_TABLE_BANK_COUNT); bankCount++)
    {
      updateMappingTableToCache(bankCount);
      for (uint8_t entryInBank = 0; entryInBank < ZGP_MAX_ENTRIES_PER_BANK; entryInBank++)
      {
        if (ZGP_UNINIT_TABLE_TYPE != zgpMappingTableActiveBank[entryInBank].filterField.entryType)
        {
          uint8_t entryIndex = bankCount * ZGP_MAX_ENTRIES_PER_BANK+ entryInBank;
          uint32_t securityFrameCounter;
          zgpEntryNvmFormat_t tempEntry;

          tableEntryNvmOperation(entryIndex, &securityFrameCounter, \
                               NVM_READ, ENTRY_OPERATION | SECURITY_FRAMECOUNTER_OPERATION);
          memcpy(&tempEntry, &cachedEntry, sizeof(zgpEntryNvmFormat_t));
          updateMappingTableToCache(0);
          ZGPL_AddOrUpdateTableEntryOnNvm((void *)&tempEntry.tableEntry, REPLACE_ENTRY, tempEntry.entryType);
          updateMappingTableToCache(bankCount);
          memset(&zgpMappingTableActiveBank[entryInBank], 0xFF, sizeof(zgpMappingTableActiveBank[entryInBank]));
          updateNvmMappingTable(bankCount);
        }

        // Delete the item once last entry in the item reached
        if ((entryInBank % ZGP_TABLE_ENTRIES_PER_ITEM) == (ZGP_TABLE_ENTRIES_PER_ITEM - 1))
        {
          uint8_t entryIndex = (bankCount * ZGP_MAX_ENTRIES_PER_BANK+ entryInBank);
          uint16_t itemId = getEntryItemId(entryIndex, false);
          PDS_Delete(itemId);
          itemId = getEntryItemId(entryIndex, true);
          PDS_Delete(itemId);
        }
      }
    }
  }
}
#endif
/**************************************************************************//**
\brief Finds the Total no of Non Empty Sink Table Entries

\param[in] - isProxyTable - true for proxy table
                            false for sink table

\return   No of Non Empty Sink Table Entries
******************************************************************************/
uint8_t ZGPL_TotalNonEmptyEntries(bool isProxyTable)
{
  if (isProxyTable)
    return zgpBankInfo.noOfProxyTableEntries;
  else
    return zgpBankInfo.noOfLogicalSinkTableEntries;
}


/**************************************************************************//**
\brief To check different bank status(PROXY/SINK PRESENT, FREE ENTRY)

\param[in] - bankNo - bank no.
             bankStatus - status to be checked

\return   true if the status is set
          false otherwise
******************************************************************************/
static bool checkBankStatus(uint8_t bankNo, zgpMappingTableStatus_t bankStatus)
{
  if (zgpBankInfo.bankStatus[bankStatus][bankNo/8] & (1 << (bankNo % 8)))
    return true;
  return false;
}

/**************************************************************************//**
\brief To update bank status(PROXY/SINK PRESENT, FREE ENTRY)

\param[in] - bankNo - bank no.
             bankStatus - status to be updated

\return   None
******************************************************************************/
static void updateBankStatus(uint8_t bankNo, zgpMappingTableStatus_t bankStatus, bool set)
{
  if (set)
    zgpBankInfo.bankStatus[bankStatus][bankNo/8] |= (1 << (bankNo % 8));
  else
    zgpBankInfo.bankStatus[bankStatus][bankNo/8] &= ~(1 << (bankNo % 8));
}

/**************************************************************************//**
\brief To do NVM operation on entry info/security framecounter item id

\param[in] - index - entry index
             frameCounter - pointer to framecounter
             nvmOperation - NVM operation to be carried out
             fieldMask - bitmask for NVM operation on entryinfo/framecounter

\return   None
******************************************************************************/
static void tableEntryNvmOperation(uint8_t itemStartIndex, uint32_t *frameCounter, 
                          zgpNvmOperationType_t nvmOperation, uint8_t fieldMask)
{ 
  if (NVM_DELETE == nvmOperation)
  {
    memset(&cachedEntry, 0x00, sizeof(zgpEntryNvmFormat_t));
    cachedEntry.entryType = ZGP_UNINIT_TABLE_TYPE;
    memset((void *)&cachedEntry, 0x00, sizeof(zgpEntryNvmFormat_t));
    cachedEntry.entryType = ZGP_UNINIT_TABLE_TYPE;
  }
  if (NVM_WRITE == nvmOperation || NVM_DELETE == nvmOperation)
  {
    if (fieldMask & ENTRY_OPERATION)
    {
      if (NVM_WRITE == nvmOperation)
      {
        pdsItem.id     = ZGP_ENTRY_STARTING_ITEM_ID;
        pdsItem.offset = itemStartIndex * sizeof (cachedEntry);
        pdsItem.size   = sizeof(cachedEntry);
        pdsItem.ramAddr = (uint8_t *)&cachedEntry;
        bool retvVal = PDS_StoreItem(&pdsItem);
       
        SYS_E_ASSERT_FATAL(retvVal, ZGP_TABLE_ENTRY_WRITE_FAILURE_1);
      }
      
    }
    if (fieldMask & SECURITY_FRAMECOUNTER_OPERATION)
    {
      deviceSecFrameCounter = *frameCounter;
      if ((NVM_WRITE == nvmOperation) && (ZGP_SEC_FRAME_COUNTER_DEFAULT == deviceSecFrameCounter))
        return;
      if (NVM_DELETE == nvmOperation)
        deviceSecFrameCounter = ZGP_SEC_FRAME_COUNTER_DEFAULT;

      // Reading from S_Nv & update the current bank & cachedBankNo
      pdsItem.id       = ZGP_FRAME_COUNTER_ITEM_ID;
      pdsItem.offset   = itemStartIndex * sizeof (deviceSecFrameCounter);
      pdsItem.size     = sizeof (deviceSecFrameCounter);
      pdsItem.ramAddr  = (uint8_t *)&deviceSecFrameCounter;
      bool retVal = PDS_StoreItem(&pdsItem);
	 
      SYS_E_ASSERT_FATAL(retVal, ZGP_TABLE_ENTRY_WRITE_FAILURE_2);
    }
  }
  else if (NVM_READ == nvmOperation)
  {
    if (fieldMask & ENTRY_OPERATION)
    {
      bool retVal = false;

      if(PDS_IsAbleToRestore(ZGP_ENTRY_STARTING_ITEM_ID))
      {
        pdsItem.id     = ZGP_ENTRY_STARTING_ITEM_ID;
        pdsItem.offset = itemStartIndex * sizeof (cachedEntry);
        pdsItem.size   = sizeof(cachedEntry);
        pdsItem.ramAddr = (uint8_t *)&cachedEntry;
        retVal = PDS_Restore_Offset(&pdsItem);
      }
      SYS_E_ASSERT_FATAL(retVal, ZGP_TABLE_ENTRY_NOT_FOUND_ON_NVM_2);
    }
    if (fieldMask & SECURITY_FRAMECOUNTER_OPERATION)
    {
      bool retVal = false;
      pdsItem.id       = ZGP_FRAME_COUNTER_ITEM_ID;
      pdsItem.offset   = itemStartIndex * sizeof (uint32_t);
      pdsItem.size     = sizeof(uint32_t);
      pdsItem.ramAddr  = (uint8_t *)frameCounter;

      
      if(PDS_IsAbleToRestore(ZGP_FRAME_COUNTER_ITEM_ID))
      {
         retVal = PDS_Restore_Offset(&pdsItem);
      }
      
      SYS_E_ASSERT_FATAL(retVal, ZGP_TABLE_ENTRY_NOT_FOUND_ON_NVM_2);
    }
  }
}

/**************************************************************************//**
\brief To update the current bank on PDS

\param[in] - bankNo - to be updated

\return   None
******************************************************************************/
static void updateNvmMappingTable(uint8_t bankNo)
{
  pdsItem.id = ZGP_MAPPING_TABLE_MEM_ID;
  pdsItem.ramAddr = (uint8_t *)&zgpMappingTableActiveBank;
  pdsItem.offset = bankNo * sizeof(zgpMappingTableActiveBank);
  pdsItem.size = sizeof(zgpMappingTableActiveBank);
  bool retvVal = PDS_StoreItem(&pdsItem);
 
  SYS_E_ASSERT_FATAL(retvVal, ZGP_TABLE_ENTRY_WRITE_FAILURE_1);
}

/**************************************************************************//**
\brief To update cache from NVM

\param[in] - bankNo - bank no. to be cached

\return   None
******************************************************************************/
static void updateMappingTableToCache(uint8_t bankNo)
{
  // Reading from S_Nv & update the current bank & cachedBankNo
  pdsItem.id = ZGP_MAPPING_TABLE_MEM_ID;
  pdsItem.ramAddr = (uint8_t *)&zgpMappingTableActiveBank;
  pdsItem.offset = bankNo * sizeof(zgpMappingTableActiveBank);
  pdsItem.size = sizeof(zgpMappingTableActiveBank);
  if(PDS_IsAbleToRestore(ZGP_MAPPING_TABLE_MEM_ID))
  {
    if(!PDS_Restore_Offset(&pdsItem))
    {
      SYS_E_ASSERT_FATAL(false, ZGP_TABLE_ENTRY_NOT_INIT_6);
    }
  }
  else
  { /*Initilalize all banks*/
    memset(zgpMappingTableActiveBank, 0xFF, sizeof(zgpMappingTableActiveBank));

    for(uint8_t bankcount =0; bankcount < ZGP_MAPPING_TABLE_BANK_COUNT; bankcount++)
    {
      pdsItem.offset = bankcount * sizeof(zgpMappingTableActiveBank);
      bool retvVal = PDS_StoreItem(&pdsItem);

      SYS_E_ASSERT_FATAL(retvVal, ZGP_TABLE_ENTRY_WRITE_FAILURE_1);
    }
  }
  zgpBankInfo.cachedBankNo = bankNo;
}

/**************************************************************************//**
\brief To get the next bank to be scanned

\param[in] - bankScanned - pointer to the scanned bank info

\return   next bank index
******************************************************************************/
static uint8_t getNextBankToBeScanned(bool *bankScanned)
{
  uint8_t bankIndex = 0;
  while(bankIndex < ZGP_MAPPING_TABLE_BANK_COUNT)
  {
    if (!bankScanned[bankIndex])
      break;
    bankIndex++;
  }
  return bankIndex;
}

/**************************************************************************//**
\brief To add/update proxy/sink entry on NVM

\param[in] - entryPtr - pointer to the entry
             action - to be performed on the entry
             entryType - proxy or sink entry

\return   true for successful operation
          false otherwise
******************************************************************************/
bool ZGPL_AddOrUpdateTableEntryOnNvm(void *entryPtr, ZGP_TableUpdateAction_t action, ZGP_EntryType_t entryType)
{
  uint8_t freeBankNo = ZGP_ENTRY_INVALID_INDEX;
  uint8_t freeIndex = ZGP_ENTRY_INVALID_INDEX;
  bool bankScanned[ZGP_MAPPING_TABLE_BANK_COUNT];
  zgpTableGenericInfo_t *tableGenericInfoPtr;
  ZGP_GpdId_t gpdId;
  uint8_t entrySize = sizeof(ZGP_ProxyTableEntry_t);
  uint32_t newSecurityFrameCounter;
  ZGP_TableOperationField_t filterField = {.entryType = entryType};
  uint8_t currentBankNo = zgpBankInfo.cachedBankNo;
  uint8_t freeEntriesAvailable = 0;

#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
  if (ZGP_SINK_ENTRY == filterField.entryType)
  {
    tableGenericInfoPtr = (zgpTableGenericInfo_t *) &(((ZGP_SinkTableEntry_t *)entryPtr)->tableGenericInfo);
    filterField.appId = ((ZGP_SinkTableEntry_t *)entryPtr)->options.appId;
    newSecurityFrameCounter = ((ZGP_SinkTableEntry_t *)entryPtr)->tableGenericInfo.gpdSecurityFrameCounter;
    entrySize = sizeof(ZGP_SinkTableEntry_t);
    filterField.commMode = ((ZGP_SinkTableEntry_t *)entryPtr)->options.communicationMode;
    // Comm. mode should be set to defined value
    if (ALL_COMMUNICATION_MODE == filterField.commMode)
      return false;
    ((ZGP_SinkTableEntry_t *)entryPtr)->commModeMask = (1u << filterField.commMode);
  }
  else
#endif
  {
    tableGenericInfoPtr = (zgpTableGenericInfo_t *) &(((ZGP_ProxyTableEntry_t *)entryPtr)->tableGenericInfo);
    filterField.appId = ((ZGP_ProxyTableEntry_t *)entryPtr)->options.appId;
    newSecurityFrameCounter = ((ZGP_ProxyTableEntry_t *)entryPtr)->tableGenericInfo.gpdSecurityFrameCounter;
  }

  gpdId.gpdIeeeAddr = tableGenericInfoPtr->gpdId.gpdIeeeAddr;

  memset(&bankScanned[0], 0x00, sizeof(bankScanned));
  for (uint8_t bankCount = 0; bankCount < ZGP_MAPPING_TABLE_BANK_COUNT; bankCount++)
  {
    //Raise an assert if the currentBankNo exceeds the bankCount
     if (currentBankNo >= ZGP_MAPPING_TABLE_BANK_COUNT)
       SYS_E_ASSERT_FATAL(false, ZGP_INVALID_BANK_NO_IN_BANK_SCANNING);

    // free entry available
    if (!checkBankStatus(currentBankNo, ZGP_PROXY_ENTRY_PRESENT) && \
        !checkBankStatus(currentBankNo, ZGP_SINK_ENTRY_PRESENT))
    {
      // no entry is available
      // so we can straight away can initialize the free index
      // Free index found out
      if (currentBankNo < freeBankNo)
      {
        freeIndex = 0;
        freeBankNo = currentBankNo;
      }
    }
    else
    {
      if (currentBankNo != zgpBankInfo.cachedBankNo)
      {
        updateMappingTableToCache(currentBankNo);
      }
      for (uint8_t entryIndex = 0; entryIndex < ZGP_MAX_ENTRIES_PER_BANK; entryIndex++)
      {
        if (ZGP_UNINIT_TABLE_TYPE == zgpMappingTableActiveBank[entryIndex].filterField.entryType)
        {
          if (currentBankNo <= freeBankNo)
          {
            if (freeBankNo != currentBankNo)
            {
              freeEntriesAvailable = 0;
              // Free index found out
              freeIndex = entryIndex;
              freeBankNo = currentBankNo;
            }
            freeEntriesAvailable++;
          }
        }
        else
        {
          if  ((filterField.entryType == zgpMappingTableActiveBank[entryIndex].filterField.entryType) && \
               (filterField.appId == zgpMappingTableActiveBank[entryIndex].filterField.appId))
          {
            if ( ((ZGP_SRC_APPID == filterField.appId) && (zgpMappingTableActiveBank[entryIndex].gpdId.gpdSrcId == gpdId.gpdSrcId)) || \
                 ((ZGP_IEEE_ADDR_APPID == filterField.appId) && (zgpMappingTableActiveBank[entryIndex].gpdId.gpdIeeeAddr == gpdId.gpdIeeeAddr) && 
                  (zgpMappingTableActiveBank[entryIndex].endPoint == tableGenericInfoPtr->endPoint)))
            {
              uint8_t entryOffsetIndex = entryIndex + (currentBankNo * ZGP_MAX_ENTRIES_PER_BANK);

              cachedEntry.entryType = (ZGP_EntryType_t)filterField.entryType;
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
              // need to handle this specific to proxy/sink
              if (ZGP_SINK_ENTRY == filterField.entryType)
              {
                // Need to update the entry
                if ((UPDATE_ENTRY == action) ||(REPLACE_ENTRY == action))
                {
                  uint8_t curentCommModeMask = zgpMappingTableActiveBank[entryIndex].filterField.commModeMask;

                  // update
                  if (REPLACE_ENTRY == action)
                  {
                    // Replacing the existing entry (with all comm. mode) with this new one
                    // Only gpd id will be retained
                    ((ZGP_SinkTableEntry_t *)entryPtr)->commModeMask = (1 << filterField.commMode);
                    memcpy(&cachedEntry.tableEntry, entryPtr, entrySize);
                    zgpMappingTableActiveBank[entryIndex].filterField.commModeMask = ((ZGP_SinkTableEntry_t *)entryPtr)->commModeMask;
                  }
                  else
                  {
                    tableEntryNvmOperation(entryOffsetIndex, NULL, NVM_READ, ENTRY_OPERATION);
                    memcpy(&cachedEntry.tableEntry.proxyEntry, &cachedEntry.tableEntry.proxyEntry, sizeof(ZGP_ProxyTableEntry_t));
                    if (!zgpAddExtendSinkTableEntry((ZGP_SinkTableEntry_t *)&cachedEntry.tableEntry.sinkEntry,(ZGP_SinkTableEntry_t *)entryPtr))
                      return false;
                    zgpFillGpdInfoOfSinkTableEntry((ZGP_SinkTableEntry_t *)&cachedEntry.tableEntry.sinkEntry,(ZGP_SinkTableEntry_t *)entryPtr);
                  }
                  tableEntryNvmOperation(entryOffsetIndex, &newSecurityFrameCounter, NVM_WRITE, \
                                           SECURITY_FRAMECOUNTER_OPERATION | ENTRY_OPERATION);

                  // Set comm. mode if it is not previosuly
                  if (!(curentCommModeMask & (1u << filterField.commMode)))
                  {
                    zgpMappingTableActiveBank[entryIndex].filterField.commModeMask |= (1u << filterField.commMode);
                    // New comm. mode is added so incrementing logical entries
                    zgpBankInfo.noOfLogicalSinkTableEntries++;
                    updateNvmMappingTable(currentBankNo);
                  }
                }
                else if (REMOVE_PAIRING_ENTRY == action)
                {
                  if (!(zgpMappingTableActiveBank[entryIndex].filterField.commModeMask & (1u << filterField.commMode)))
                    return false;

                  if (zgpRemoveSinkPairingInfo((ZGP_SinkTableEntry_t *)&cachedEntry.tableEntry.sinkEntry, ((ZGP_SinkTableEntry_t *)entryPtr)->tableGenericInfo.zgpSinkGrouplist, \
                                                filterField.commMode))
                  {
                    // Deleting the entry and frameCounter
                    zgpMappingTableActiveBank[entryIndex].filterField.commModeMask &= ~(1u << filterField.commMode);
                    // Comm. mode is depaired so decrementing logical entry count
                    zgpBankInfo.noOfLogicalSinkTableEntries--;
                    // None of communication mode supported then remove the entry  
                    if (!zgpMappingTableActiveBank[entryIndex].filterField.commModeMask)
                    {
                      filterField.commMode = ALL_COMMUNICATION_MODE;
                      ZGPL_DeleteTableEntryFromNvm(filterField, &gpdId, tableGenericInfoPtr->endPoint);
                      return true;
                    }
                    else
                      updateNvmMappingTable(currentBankNo);
                  }
                  tableEntryNvmOperation(entryOffsetIndex, NULL, NVM_WRITE, \
                                            ENTRY_OPERATION);
                }
                return true;
              }
              else
#endif
              {
                // update the entry
                // PROXY_TABLE operation 
                // - update communicationMode info & group info(addsink - 1 & removeGPD - 0
                // - addSink - 0 & removeGPD - 0
                // Need to update the entry
                tableEntryNvmOperation(entryOffsetIndex, NULL, NVM_READ, ENTRY_OPERATION);
                memcpy(&cachedEntry.tableEntry.proxyEntry, &cachedEntry.tableEntry.proxyEntry, sizeof(ZGP_ProxyTableEntry_t));
                if (REMOVE_PAIRING_ENTRY == action)
                {
                  if (zgpRemoveProxyPairingInfo((ZGP_ProxyTableEntry_t *)&cachedEntry.tableEntry.proxyEntry, (ZGP_ProxyTableEntry_t *)entryPtr))
                  {
                    // Deleting the entry and frameCounter
                    filterField.commMode = ALL_COMMUNICATION_MODE;
                    ZGPL_DeleteTableEntryFromNvm(filterField, &gpdId, tableGenericInfoPtr->endPoint);
                  }
                  else
                  {
                    tableEntryNvmOperation(entryOffsetIndex, NULL, NVM_WRITE, \
                                             ENTRY_OPERATION);
                  }
                }
                else
                {
                  // update the entries
                  if( !zgpUpdateProxyEntry(&cachedEntry.tableEntry.proxyEntry, (ZGP_ProxyTableEntry_t *)entryPtr) )
                    return false;
                  tableEntryNvmOperation(entryOffsetIndex, &newSecurityFrameCounter, NVM_WRITE, \
                                             SECURITY_FRAMECOUNTER_OPERATION | ENTRY_OPERATION);
                }
                return true;
              }
            }
          }
        }
      }
    }
    bankScanned[currentBankNo] = true;
    currentBankNo = getNextBankToBeScanned(&bankScanned[0]);
  }

  if (REMOVE_PAIRING_ENTRY == action)
    return false;
  if (((ZGP_PROXY_ENTRY == filterField.entryType) && (zgpBankInfo.noOfProxyTableEntries >= ZGP_PROXY_TABLE_SIZE)) || \
      ((ZGP_SINK_ENTRY == filterField.entryType) && (zgpBankInfo.noOfPhysicalSinkTableEntries >= ZGP_SINK_TABLE_SIZE)) || \
      ((zgpBankInfo.noOfPhysicalSinkTableEntries + zgpBankInfo.noOfProxyTableEntries) >= ZGP_TOTAL_TABLE_ENTRIES) || \
      ((ZGP_SINK_ENTRY == filterField.entryType) &&(ALL_COMMUNICATION_MODE == filterField.commMode)))
  {
    return false;    
  }
  if (freeIndex != ZGP_ENTRY_INVALID_INDEX)
  {
    zgpMappingTable_t mappingEntry;
    uint8_t entryIndex = freeBankNo * ZGP_MAX_ENTRIES_PER_BANK + freeIndex;

    if (1 == freeEntriesAvailable)
    {
      // Only one free entry which will be used for storing the current entry
      // so disabling ZGP_FREE_ENTRY_AVAILABLE status for this bank
      updateBankStatus(freeBankNo, ZGP_FREE_ENTRY_AVAILABLE, false);
    }
    // free entry available
    if (freeBankNo != zgpBankInfo.cachedBankNo)
      updateMappingTableToCache(freeBankNo);
  
    mappingEntry.endPoint = tableGenericInfoPtr->endPoint;
    memcpy(&mappingEntry.gpdId, &tableGenericInfoPtr->gpdId, sizeof(mappingEntry.gpdId));
    mappingEntry.filterField.appId = filterField.appId;
    mappingEntry.filterField.entryType = filterField.entryType;
    mappingEntry.filterField.commModeMask = (1 << filterField.commMode);
    memcpy(&zgpMappingTableActiveBank[freeIndex], &mappingEntry, sizeof(zgpMappingTable_t));
    cachedEntry.entryType = (ZGP_EntryType_t)filterField.entryType;
    if (ZGP_PROXY_ENTRY == entryType)
    {
      updateBankStatus(freeBankNo, ZGP_PROXY_ENTRY_PRESENT, true);
      zgpBankInfo.noOfProxyTableEntries++;
    }
    else
    {
      zgpBankInfo.noOfPhysicalSinkTableEntries++;
      zgpBankInfo.noOfLogicalSinkTableEntries++;
      updateBankStatus(freeBankNo, ZGP_SINK_ENTRY_PRESENT, true);
    }
    memcpy(&cachedEntry.tableEntry, entryPtr, entrySize);
    tableEntryNvmOperation(entryIndex, &newSecurityFrameCounter, NVM_WRITE, SECURITY_FRAMECOUNTER_OPERATION | ENTRY_OPERATION);
    updateNvmMappingTable(freeBankNo);
    return true;
  }
  return false;
}

#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC  
/**************************************************************************//**
\brief To get logical sink entry count

\param[in] - commModemask - communication mode mask

\return   logical entry count
******************************************************************************/
static uint8_t getLogicalEntryCount(uint8_t commModeMask)
{
  uint8_t count = 0;
  uint8_t index = 0;

  while (index < ALL_COMMUNICATION_MODE)
  {
    if (commModeMask & (1u << index))
      count++;
    index++;
  }
  return count;
}
#endif
/**************************************************************************//**
\brief To delete proxy/sink entry

\param[in] - filterField - entry filtering fields
             gpdId - address info
             endPoint - GPD end point
             sendGpPairing - if true, gpPairing cmd is sent
             deleteInvalidEntries - delete inactive entries

\return   true for successful operation
          false otherwise
******************************************************************************/
bool ZGPL_DeleteTableEntryFromNvm(ZGP_TableOperationField_t filterField, ZGP_GpdId_t *gpdId, uint8_t endPoint)
{
  zgpMappingTableStatus_t status = ZGP_PROXY_ENTRY_PRESENT;
  uint8_t j;
  bool matchFound = false;
  uint8_t noOfEntriesPresent =  0;
  bool bankScanned[ZGP_MAPPING_TABLE_BANK_COUNT];
  uint8_t currentBankNo = zgpBankInfo.cachedBankNo;

  if (ZGP_SINK_ENTRY == filterField.entryType)
    status = ZGP_SINK_ENTRY_PRESENT;

  memset(&bankScanned[0], 0x00, sizeof(bankScanned));
  for (uint8_t bankCount = 0; bankCount < ZGP_MAPPING_TABLE_BANK_COUNT ; bankCount++)
  {
    uint8_t indexNo = ZGP_ENTRY_INVALID_INDEX;

    //Raise an assert if the currentBankNo exceeds the bankCount
    if (currentBankNo >= ZGP_MAPPING_TABLE_BANK_COUNT)
      SYS_E_ASSERT_FATAL(false, ZGP_INVALID_BANK_NO_IN_BANK_SCANNING);

    noOfEntriesPresent = 0;
    indexNo = ZGP_ENTRY_INVALID_INDEX;

    if (checkBankStatus(currentBankNo, status))
    {
      // free entry available
      if (currentBankNo != zgpBankInfo.cachedBankNo)
      {
        updateMappingTableToCache(currentBankNo);
      }
      for (j = 0; j < ZGP_MAX_ENTRIES_PER_BANK; j++)
      {
        bool entryToBeDeleted = false;

        if (ZGP_UNINIT_TABLE_TYPE != zgpMappingTableActiveBank[j].filterField.entryType)
        {
          if (filterField.entryType == zgpMappingTableActiveBank[j].filterField.entryType)
          {
            noOfEntriesPresent++;

            if ((filterField.appId == zgpMappingTableActiveBank[j].filterField.appId) && \
                 (((ZGP_SRC_APPID == filterField.appId) && (zgpMappingTableActiveBank[j].gpdId.gpdSrcId == gpdId->gpdSrcId)) || \
                 ((ZGP_IEEE_ADDR_APPID == filterField.appId) && (zgpMappingTableActiveBank[j].gpdId.gpdIeeeAddr == gpdId->gpdIeeeAddr) && 
                  ((zgpMappingTableActiveBank[j].endPoint == endPoint) || (APP_INDEPENDENT_END_POINT == endPoint) || (ALL_END_POINT == endPoint) ))))
            {
              indexNo = j;

              entryToBeDeleted = true;
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC          
              if (ZGP_SINK_ENTRY == filterField.entryType)
              {
                zgpBankInfo.noOfLogicalSinkTableEntries -= getLogicalEntryCount(zgpMappingTableActiveBank[j].filterField.commModeMask);
                zgpBankInfo.noOfPhysicalSinkTableEntries --;
              }
              else
#endif
                zgpBankInfo.noOfProxyTableEntries --;
            }
          }
          if (entryToBeDeleted)
          {
            if (checkCacheEntry(filterField, gpdId, endPoint))
            {
              memset((void *)&cachedEntry, 0x00, sizeof(zgpEntryNvmFormat_t));
              cachedEntry.entryType = ZGP_UNINIT_TABLE_TYPE;
              memset((void *)&cachedEntry, 0x00, sizeof(zgpEntryNvmFormat_t));
              cachedEntry.entryType = ZGP_UNINIT_TABLE_TYPE;
            }
            memset(&zgpMappingTableActiveBank[indexNo], 0xFF, sizeof(zgpMappingTable_t));
          }
        }
      }
    }
    if (ZGP_ENTRY_INVALID_INDEX != indexNo)
    {
      matchFound = true;

      if (1 == noOfEntriesPresent)
      {
        updateBankStatus(currentBankNo, status, false);
      }
      // update in NVM
      updateNvmMappingTable(currentBankNo);
      updateBankStatus(currentBankNo, ZGP_FREE_ENTRY_AVAILABLE, true);
      break;
    }
    bankScanned[currentBankNo] = true;
    currentBankNo = getNextBankToBeScanned(&bankScanned[0]);
  }

  if (matchFound)
    return true;
  else
    return false;
}

/**************************************************************************//**
\brief To read a proxy/sink entry from NVm

\param[in] - entryPtr - entry to be filled
             filterField - filter fields for the entry
             additionalField - additiona filter fields
             nonEmptyIndexForRead - index to read the entry
             gpdId - pointer to address info
             endPoint - GPD end point

\return   read status
******************************************************************************/
ZGP_ReadOperationStatus_t ZGPL_ReadTableEntryFromNvm(void *entryPtr, ZGP_TableOperationField_t tableOperationField, ZGP_GpdId_t *gpdId, uint8_t endPoint)
{
  indexSearchResult_t indexResult;
  uint8_t entrySize = sizeof(ZGP_ProxyTableEntry_t);
  uint32_t securityFrameCounter = 0;
  ZGP_ReadOperationStatus_t readStatus = ENTRY_NOT_AVAILABLE;

#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC 
  if (ZGP_SINK_ENTRY == tableOperationField.entryType)
  {
    entrySize = sizeof(ZGP_SinkTableEntry_t);
  }
#endif

  if (tableOperationField.nonEmptyIndexForRead == ZGP_ENTRY_INVALID_INDEX)
  {
    if (checkCacheEntry(tableOperationField, gpdId, endPoint))
    {
      readStatus = ACTIVE_ENTRY_AVAILABLE;
    }
  }

  if (ENTRY_NOT_AVAILABLE == readStatus)
  {
    indexResult = getMappingTableIndex(tableOperationField, gpdId, endPoint);
    if (ZGP_ENTRY_INVALID_INDEX != indexResult.indexNo)
    {
      uint8_t entryIndex = indexResult.bankNo * ZGP_MAX_ENTRIES_PER_BANK+ indexResult.indexNo;

      tableEntryNvmOperation(entryIndex, &securityFrameCounter, \
                             NVM_READ, ENTRY_OPERATION | SECURITY_FRAMECOUNTER_OPERATION);

#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC      
      if (ZGP_SINK_ENTRY == tableOperationField.entryType)
      {
        cachedEntry.tableEntry.sinkEntry.commModeMask = zgpMappingTableActiveBank[indexResult.indexNo].filterField.commModeMask;
        ((ZGP_SinkTableEntry_t *)&cachedEntry.tableEntry)->tableGenericInfo.gpdSecurityFrameCounter = securityFrameCounter;
      }
      else
#endif
      {
        ((ZGP_ProxyTableEntry_t *)&cachedEntry.tableEntry)->tableGenericInfo.gpdSecurityFrameCounter = securityFrameCounter;
      }
      readStatus = ACTIVE_ENTRY_AVAILABLE;
    }
  }

  if (ENTRY_NOT_AVAILABLE != readStatus)
  {
    // Need to set communication mode and the corresponding information
    memcpy(entryPtr,&cachedEntry.tableEntry,entrySize);
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
    if (ZGP_SINK_ENTRY == tableOperationField.entryType)
    {
      if (ZGP_ENTRY_INVALID_INDEX != tableOperationField.nonEmptyIndexForRead)
      {
        // For index based search, taking the current communication mode mapping with the index
        ((ZGP_SinkTableEntry_t *)entryPtr)->options.communicationMode  = indexResult.commModeMaskPos;
      }
      else if (ALL_COMMUNICATION_MODE == tableOperationField.commMode)
      {
        uint8_t index = 0;

        // To look for first communication mode supported
        while (index < ALL_COMMUNICATION_MODE)
        {
          if ((1u << index) & ((ZGP_SinkTableEntry_t *)entryPtr)->commModeMask)
          {
            ((ZGP_SinkTableEntry_t *)entryPtr)->options.communicationMode = index;
            break;
          }
          index++;
        }
      }
      else
        ((ZGP_SinkTableEntry_t *)entryPtr)->options.communicationMode = tableOperationField.commMode;
    }
#endif
  }

  return readStatus;
}

/**************************************************************************//**
\brief To check whether the requested entry is present in cache

\param[in] - appId - application id
             gpdId - address info
             endPoint - GPD end point

\return   true for successful operation
          false otherwise
******************************************************************************/
static bool checkCacheEntry(ZGP_TableOperationField_t filterField, ZGP_GpdId_t *gpdId, uint8_t endPoint)
{
  if (filterField.entryType == cachedEntry.entryType)
  {
    zgpTableGenericInfo_t *tableGenericInfoPtr;
    ZGP_ApplicationId_t entryAppId;

#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
    if (ZGP_SINK_ENTRY == filterField.entryType)
    {
      tableGenericInfoPtr = &cachedEntry.tableEntry.sinkEntry.tableGenericInfo;
      entryAppId = (ZGP_ApplicationId_t)cachedEntry.tableEntry.sinkEntry.options.appId;
    }
    else
#endif
    {
      tableGenericInfoPtr = &cachedEntry.tableEntry.proxyEntry.tableGenericInfo;
      entryAppId = (ZGP_ApplicationId_t)cachedEntry.tableEntry.proxyEntry.options.appId;
    }

    if (filterField.appId == entryAppId)
    {
      if ( ((ZGP_SRC_APPID == filterField.appId) && ((tableGenericInfoPtr->gpdId.gpdSrcId == gpdId->gpdSrcId) || \
             (ZGP_ALL_SRC_ID == tableGenericInfoPtr->gpdId.gpdSrcId))) || \
           ((ZGP_IEEE_ADDR_APPID == filterField.appId) && ((tableGenericInfoPtr->gpdId.gpdIeeeAddr == gpdId->gpdIeeeAddr) || \
             (ZGP_ALL_IEEE_ADDR == tableGenericInfoPtr->gpdId.gpdIeeeAddr))&& 
            ((tableGenericInfoPtr->endPoint == endPoint) || (APP_INDEPENDENT_END_POINT == tableGenericInfoPtr->endPoint) || \
              (ALL_END_POINT == tableGenericInfoPtr->endPoint) )))
      {
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC        
        if (ZGP_SINK_ENTRY == filterField.entryType)
        {
          if ((filterField.commMode == ((ZGP_SinkTableEntry_t *)&cachedEntry.tableEntry)->options.communicationMode) || \
               (ALL_COMMUNICATION_MODE == filterField.commMode))
            return true;
        }
        else
#endif
          return true;
      }
    }
  }
  return false;
}

/**************************************************************************//**
\brief To get the bank and mapping table index for the given gpd id

\param[in] - filterField - entry filter fields
             additionalField - additional fields
             nonEmptyIndexRead - read entry by index(for table req handling)
             gpdId - address info
             endPoint - GPD end point

\return   index search result(bank no. and index no)
******************************************************************************/
static indexSearchResult_t getMappingTableIndex(ZGP_TableOperationField_t  tableOperationField ,\
                                                ZGP_GpdId_t *gpdId, uint8_t endPoint)
{
  indexSearchResult_t indexRes = {ZGP_ENTRY_INVALID_INDEX, ZGP_ENTRY_INVALID_INDEX, ZGP_ENTRY_INVALID_INDEX};
  zgpMappingTableStatus_t status = ZGP_PROXY_ENTRY_PRESENT;
  uint8_t nonEmptyIndexCount = 0;
  bool bankScanned[ZGP_MAPPING_TABLE_BANK_COUNT];
  uint8_t currentBankNo = zgpBankInfo.cachedBankNo;

#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC  
  if (ZGP_SINK_ENTRY == tableOperationField.entryType)
  {
    status = ZGP_SINK_ENTRY_PRESENT;
  }
#endif

  if (tableOperationField.nonEmptyIndexForRead != ZGP_ENTRY_INVALID_INDEX)
  {
    if (currentBankNo != 0)
    {
      updateMappingTableToCache(0);
      currentBankNo = 0;
    }
  }

  memset(&bankScanned[0], 0x00, sizeof(bankScanned));
  for (uint8_t bankCount = 0; bankCount < ZGP_MAPPING_TABLE_BANK_COUNT ; bankCount++)
  {
    //Raise an assert if the currentBankNo exceeds the bankCount
    if (currentBankNo >= ZGP_MAPPING_TABLE_BANK_COUNT)
       SYS_E_ASSERT_FATAL(false, ZGP_INVALID_BANK_NO_IN_BANK_SCANNING);

    if (checkBankStatus(currentBankNo, status))
    {
      if (currentBankNo != zgpBankInfo.cachedBankNo)
      {
        updateMappingTableToCache(currentBankNo);
      }
      for (uint8_t j = 0; j < ZGP_MAX_ENTRIES_PER_BANK; j++)
      {
        if (ZGP_UNINIT_TABLE_TYPE != zgpMappingTableActiveBank[j].filterField.entryType)
        {
          if (tableOperationField.entryType == zgpMappingTableActiveBank[j].filterField.entryType)
          {
            if (tableOperationField.nonEmptyIndexForRead != ZGP_ENTRY_INVALID_INDEX)
            {
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
              if (ZGP_SINK_ENTRY == tableOperationField.entryType)
              {
                zgpCommunicationMode_t commMode = 0;

                while(commMode < ALL_COMMUNICATION_MODE)
                {
                  if (zgpMappingTableActiveBank[j].filterField.commModeMask & (1 << commMode))
                  {
                    if (tableOperationField.nonEmptyIndexForRead == nonEmptyIndexCount)
                    {
                      indexRes.bankNo = currentBankNo;
                      indexRes.indexNo = j;
                      indexRes.commModeMaskPos = commMode;
                      return indexRes;
                    }
                    nonEmptyIndexCount++;
                  }
                  commMode++;
                }
              }
              else
#endif
              {
                if (tableOperationField.nonEmptyIndexForRead == nonEmptyIndexCount)
                {
                  indexRes.bankNo = currentBankNo;
                  indexRes.indexNo = j;
                  return indexRes;
                }
                else
                  nonEmptyIndexCount++;
              }
            }
            else if ( (tableOperationField.appId == zgpMappingTableActiveBank[j].filterField.appId) && \
                   (((ZGP_SRC_APPID == tableOperationField.appId) && ((zgpMappingTableActiveBank[j].gpdId.gpdSrcId == gpdId->gpdSrcId) || \
                     (ZGP_ALL_SRC_ID == zgpMappingTableActiveBank[j].gpdId.gpdSrcId))) || \
                   ((ZGP_IEEE_ADDR_APPID == tableOperationField.appId) && ((zgpMappingTableActiveBank[j].gpdId.gpdIeeeAddr == gpdId->gpdIeeeAddr) || \
                     (ZGP_ALL_IEEE_ADDR == zgpMappingTableActiveBank[j].gpdId.gpdIeeeAddr))&& 
                    ((zgpMappingTableActiveBank[j].endPoint == endPoint) || (APP_INDEPENDENT_END_POINT == endPoint) || (ALL_END_POINT == endPoint) || \
                      (APP_INDEPENDENT_END_POINT == zgpMappingTableActiveBank[j].endPoint) || (ALL_END_POINT == zgpMappingTableActiveBank[j].endPoint)))))
            {
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
              if (ZGP_SINK_ENTRY == tableOperationField.entryType)
              {
                if (!((tableOperationField.commMode == ALL_COMMUNICATION_MODE) || \
                     (zgpMappingTableActiveBank[j].filterField.commModeMask & (1 << tableOperationField.commMode))))
                  return indexRes;
              }
#endif   
              indexRes.bankNo = currentBankNo;
              indexRes.indexNo = j;
              return  indexRes;
            }
          }
        }
      }
    }
    bankScanned[currentBankNo] = true;
    currentBankNo = getNextBankToBeScanned(&bankScanned[0]);
  }
  return indexRes;
  
}

/**************************************************************************//**
\brief To scan the NVM for proxy/sink entries on startup

\return   None
******************************************************************************/
static void scanTableEntriesOnNvm(void)
{
  memset(&zgpBankInfo, 0x00, sizeof(zgpBankInfo));

  for (uint8_t bankNo = 0; bankNo < ZGP_MAPPING_TABLE_BANK_COUNT ; bankNo++)
  {
    updateMappingTableToCache(bankNo);

    for (uint8_t entryInBank = 0; entryInBank < ZGP_MAX_ENTRIES_PER_BANK; entryInBank++)
    {
      if (ZGP_UNINIT_TABLE_TYPE == zgpMappingTableActiveBank[entryInBank].filterField.entryType)
      {
        updateBankStatus(bankNo, ZGP_FREE_ENTRY_AVAILABLE, true);
      }
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC       
      else if (ZGP_SINK_ENTRY == zgpMappingTableActiveBank[entryInBank].filterField.entryType)
      {
        updateBankStatus(bankNo, ZGP_SINK_ENTRY_PRESENT, true);
        zgpBankInfo.noOfPhysicalSinkTableEntries++;
        zgpBankInfo.noOfLogicalSinkTableEntries += getLogicalEntryCount(zgpMappingTableActiveBank[entryInBank].filterField.commModeMask);
      }
#endif
      else if (ZGP_PROXY_ENTRY == zgpMappingTableActiveBank[entryInBank].filterField.entryType)
      {
        updateBankStatus(bankNo, ZGP_PROXY_ENTRY_PRESENT, true);
        zgpBankInfo.noOfProxyTableEntries++;
      }
    }
  }
}

/**************************************************************************//**
\brief To read/update frameCounter

\param[in] - frameCounter - frameCounter to be updated/read
             filterField - filter field to read the frame counter
             additionalField - additional fields
             gpdId - address info
             endPoint - GPD end point
             isUpdateOperation - true for update
                                 false for read

\return   true for successful operation
          false otherwise
******************************************************************************/
bool ZGPL_FrameCounterReadorUpdateOnNvm(uint32_t *frameCounter, ZGP_TableOperationField_t  tableOperationField , ZGP_GpdId_t *gpdId, uint8_t endPoint, \
                                     bool isUpdateOperation)
{
  indexSearchResult_t indexRes = {ZGP_ENTRY_INVALID_INDEX, ZGP_ENTRY_INVALID_INDEX};
  zgpNvmOperationType_t nvmOperation = NVM_READ;

  if (isUpdateOperation)
    nvmOperation = NVM_WRITE;

  indexRes = getMappingTableIndex(tableOperationField, gpdId, endPoint);
  if (ZGP_ENTRY_INVALID_INDEX != indexRes.indexNo)
  {
    uint8_t entryIndex = indexRes.bankNo * ZGP_MAX_ENTRIES_PER_BANK + indexRes.indexNo;

    if (!checkCacheEntry(tableOperationField, gpdId, endPoint))
    {
      tableEntryNvmOperation(entryIndex, NULL, \
                             NVM_READ, ENTRY_OPERATION);
    }

    tableEntryNvmOperation(entryIndex, frameCounter, nvmOperation, SECURITY_FRAMECOUNTER_OPERATION);

#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
    if (ZGP_SINK_ENTRY == tableOperationField.entryType)
    {
      ((ZGP_SinkTableEntry_t *)&cachedEntry.tableEntry)->tableGenericInfo.gpdSecurityFrameCounter = *frameCounter;
    }
    else
#endif
    {
      ((ZGP_ProxyTableEntry_t *)&cachedEntry.tableEntry)->tableGenericInfo.gpdSecurityFrameCounter = *frameCounter;
    }
    return true;
  }
  else
    return false;
}

/**************************************************************************//**
\brief To reset all table entries on NVM

\param[in] - None

\return   None
******************************************************************************/
void ZGPL_ResetTableToFN(void)
{
  memset(&zgpMappingTableActiveBank[0], 0xFF, sizeof(zgpMappingTableActiveBank));

  for (uint8_t i = 0; i < ZGP_MAPPING_TABLE_BANK_COUNT ; i++)
  {
    updateNvmMappingTable(i);
    updateBankStatus(i, ZGP_PROXY_ENTRY_PRESENT, false);
    updateBankStatus(i, ZGP_SINK_ENTRY_PRESENT, false);
    updateBankStatus(i, ZGP_FREE_ENTRY_AVAILABLE, true);
  }

  zgpBankInfo.cachedBankNo = 0; // make it invalid
  zgpBankInfo.noOfPhysicalSinkTableEntries = 0;
  zgpBankInfo.noOfLogicalSinkTableEntries = 0;
  zgpBankInfo.noOfProxyTableEntries = 0;
  // Resetting the cache entry
  memset((void *)&cachedEntry, 0x00, sizeof(zgpEntryNvmFormat_t));
  cachedEntry.entryType = ZGP_UNINIT_TABLE_TYPE;
  memset((void *)&cachedEntry, 0x00, sizeof(zgpEntryNvmFormat_t));
  cachedEntry.entryType = ZGP_UNINIT_TABLE_TYPE;
}

/**************************************************************************//**
\brief Read bank information

\param[in] None

\return   pointer to bank info
******************************************************************************/
zgpBankInfo_t * zgpTableGetBankInfo(void)
{
  return &zgpBankInfo;
}
#endif // APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
#endif // _GREENPOWER_SUPPORT_

// eof zgpNvmTable.c
