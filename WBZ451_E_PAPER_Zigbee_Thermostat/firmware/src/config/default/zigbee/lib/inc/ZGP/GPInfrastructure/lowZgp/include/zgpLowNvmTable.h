// DOM-IGNORE-BEGIN
/*******************************************************************************
  Zigbee green power nvm table Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpLowNvmTable.h

  Summary:
    This file contains the NVM table specific APIs from low zgp.

  Description:
    This file contains the NVM table specific APIs from low zgp.
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
#ifndef _ZGPLOWNVMTABLEINTERFACE_H
#define _ZGPLOWNVMTABLEINTERFACE_H

#ifdef _GREENPOWER_SUPPORT_
// DOM-IGNORE-END

/******************************************************************************
                           Includes section
 ******************************************************************************/
 
#include <zgp/include/zgpCommon.h>
#include <systemenvironment/include/sysQueue.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLow.h>

/******************************************************************************
                           Defines section
 ******************************************************************************/
 
#define ZGP_NVM_TABLE_VERSION 0x1u
#define ZGP_GROUP_CAST_RADIUS_DEFAULT            0x00

#define ZGP_SEC_FRAME_COUNTER_DEFAULT            0xFFFFFFFF

#ifndef ZGP_SINK_GROUP_LIST_SIZE
#define ZGP_SINK_GROUP_LIST_SIZE 2
#endif

#ifndef ZGP_LIGHT_WEIGHT_SINK_ADDRESS_LIST_SIZE
#define ZGP_LIGHT_WEIGHT_SINK_ADDRESS_LIST_SIZE 2
#endif

#ifndef ZGP_FULL_UNICAST_SINK_ADDRESS_LIST_SIZE
#define ZGP_FULL_UNICAST_SINK_ADDRESS_LIST_SIZE 2
#endif

#define ZGP_NWK_ADDRESS_GROUP_INIT               0xFFFF
#define ZGP_ENTRY_INVALID_INDEX   0xFF

// Checks and configures ZGP proxy table. Minimum size should be 5.
#ifdef  ZGP_PROXY_TABLE_SIZE
#if (ZGP_PROXY_TABLE_SIZE < 5)
#warning Proxy table size shall be minimum 5as per section A.3.4.2.1 in green power spec
#endif //(ZGP_PROXY_TABLE_SIZE < 5)
#else
#define ZGP_PROXY_TABLE_SIZE    5
#endif //ZGP_PROXY_TABLE_SIZE

// Checks and configures ZGP sink table. Minimum size should be 5.
#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
#ifdef  ZGP_SINK_TABLE_SIZE
#if (ZGP_SINK_TABLE_SIZE < 5)
#warning Sink table size shall be minimum 5 as per section A.3.3.2.1 in green power spec
#endif //(ZGP_SINK_TABLE_SIZE < 5)
#else
#define ZGP_SINK_TABLE_SIZE    5
#endif // ZGP_SINK_TABLE_SIZE
#endif //APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC

/******************************************************************************
                           Types section
 ******************************************************************************/
 
typedef enum _ZgpTableType_t
{
  ZGP_PROXY_ENTRY       = 0x0,
  ZGP_SINK_ENTRY        = 0x1,
  ZGP_UNINIT_TABLE_TYPE = 0x3
} ZGP_EntryType_t;

BEGIN_PACK
typedef enum _ZgpReadOperationStatus_t
{
  ENTRY_NOT_AVAILABLE     = 0x0,
  ACTIVE_ENTRY_AVAILABLE  = 0x1,
} ZGP_ReadOperationStatus_t;

typedef enum _ZpTableUpdateAction_t
{
  UPDATE_ENTRY          = 0x1,
  REPLACE_ENTRY         = 0x2,
  REMOVE_PAIRING_ENTRY  = 0x3
} ZGP_TableUpdateAction_t;

typedef struct PACK _ZGP_SinkGroup_t
{
  uint16_t sinkGroup;
  uint16_t alias;
}ZGP_SinkGroup_t;

typedef struct PACK _ZgpSecurityOptions_t
{
  uint8_t securityLevel   :2;
  uint8_t securityKeyType :3;
  uint8_t reserved        :3;
}zgpSecurityOptions_t;

// GP pairing command
typedef enum _zgpCommunicationMode_t
{
  FULL_UNICAST = 0x00,
  DERIVED_GROUPCAST = 0x01,
  PRECOMMISSIONED_GROUPCAST = 0x02,
  LIGHTWEIGHT_UNICAST = 0x03,
  ALL_COMMUNICATION_MODE = 0x04
} zgpCommunicationMode_t;

typedef struct _ZGP_TableOperationField_t
{
  uint8_t appId;
  uint8_t entryType;
  uint8_t commMode;
  uint8_t nonEmptyIndexForRead;
} ZGP_TableOperationField_t;

typedef struct PACK _ZgpProxyTableEntryOptions_t
{
  uint16_t appId                 :3;
  uint16_t entryActive           :1;
  uint16_t entryValid            :1;
  uint16_t macSeqNumCapability   :1;
  uint16_t lightWeightUnicastGps :1;
  uint16_t derivedGroupGps       :1;
  uint16_t commissionedGroupGps  :1;
  uint16_t firstToFoward         :1;
  uint16_t inRange               :1;
  uint16_t gpdFixed              :1;
  uint16_t hasAllUnicastRoutes   :1;
  uint16_t assignedAlias         :1;
  uint16_t securityUse           :1;
  uint16_t optionsExt            :1;
}zgpProxyTableEntryOptions_t;

typedef struct PACK _ZgpTableGenericInfo_t
{
 //ID of the GPD
  ZGP_GpdId_t gpdId;
  //The security key for the GPD
  uint8_t securityKey[16];
  //The incoming security frame counter for the GPD
  uint32_t gpdSecurityFrameCounter;
  //GroupIDs and Aliases for the sinks that require the tunneling in groupcast communication mode
  ZGP_SinkGroup_t zgpSinkGrouplist[ZGP_SINK_GROUP_LIST_SIZE];
  //The commissioned 16-bit ID to be used as alias for this GPD
  uint16_t gpdAssignedAlias;

  //GPD endpoint
  uint8_t endPoint;
  //To limit the range of the group-cast
  uint8_t groupCastRadius;
  //The security options
  zgpSecurityOptions_t securityOptions;
} zgpTableGenericInfo_t;

typedef struct PACK _ZgpProxyLightWeightSinkAddr_t
{
  uint64_t sinkIeeeAddr;
  uint16_t sinkNwkAddr;
}zgpProxyLightWeightSinkAddr_t;

typedef struct PACK _ZgpExtendedOptions_t
{
  uint16_t fullUnicastGPS  :1;
  uint16_t reserved        :15;
}zgpExtendedOptions_t;

typedef struct PACK _ZGP_ProxyTableEntry_t
{
  //IEEE and short address of the sink(s) that requires tunneling in lightweight unicast communica-tion mode
  zgpProxyLightWeightSinkAddr_t zgpProxyLightWeightSinkAddrlist[ZGP_LIGHT_WEIGHT_SINK_ADDRESS_LIST_SIZE];
  // entry options
  zgpProxyTableEntryOptions_t options;
#if ZGP_PROXY_ADVANCED == 1
  //IEEE and short address of the sink(s) that requires tunneling in full unicast communication mode
  zgpProxyLightWeightSinkAddr_t zgpProxyFullUnicastSinkAddrlist[ZGP_FULL_UNICAST_SINK_ADDRESS_LIST_SIZE];
  //This parameter specifies exten-sions to the tunneling options
  zgpExtendedOptions_t extendedOptions;
  //For inactive/invalid entries, allows for Sink re-discovery when Search Counter equals 0
  uint8_t searchCounter;
#endif
  // Generic info - common to sink and proxy table
  zgpTableGenericInfo_t tableGenericInfo;
} ZGP_ProxyTableEntry_t;

//Describes whether a particular field is present in sinkTableEntry
typedef struct PACK _ZgpSinkTableEntryOptions_t
{
  uint16_t appId                 :3;
  // This is mainly used in OTA sink entry format
  // This will be derived from commModeMask in ZGP_SinkTableEntry_t
  // while sending OTA
  uint16_t communicationMode     :2;
  uint16_t seqNumCapabilities    :1;
  uint16_t rxOnCapability        :1;
  uint16_t fixedLocation         :1;
  uint16_t assignedAlias         :1;
  uint16_t securityUse           :1;
  uint16_t reserved              :6;
}zgpSinkTableEntryOptions_t;

typedef struct PACK _ZGP_SinkTableEntry_t
{
  // Generic info - common to sink and proxy table
  zgpTableGenericInfo_t tableGenericInfo;
  // entry options
  zgpSinkTableEntryOptions_t options;
  // comm mode mask
  // FULL_UNICAST ->              (1 << 0x00)
  // DERIVED_GROUPCAST ->         (1 << 0x01)
  // PRECOMMISSIONED_GROUPCAST -> (1 << 0x02)
  // LIGHTWEIGHT_UNICAST ->       (1 << 0x03)
  uint8_t commModeMask;
  uint8_t deviceId;
} ZGP_SinkTableEntry_t;
END_PACK
/******************************************************************************
                           Functions prototypes section
 ******************************************************************************/

/**************************************************************************//**
  \brief Initialize zgp NVM table

  \param None.

  \return None.
 ******************************************************************************/
void ZGPL_NvmTableInit(void);

/**************************************************************************//**
  \brief To reset all the parameters of the given entry to init value

  \param - entry - entry to be reset
             tableType - proxy/sink entry

  \return None
 ******************************************************************************/
void ZGPL_ResetTableEntry(void *entry, ZGP_EntryType_t tableType);

/**************************************************************************//**
  \brief To fetch the total no. of non-empty entries. This is mainly used by
         high sink/proxy while sending sink/proxy table response

  \param - isProxyTable - true for proxy table
                            false for sink table

  \return   No of Non Empty Sink Table Entries
 ******************************************************************************/
uint8_t ZGPL_TotalNonEmptyEntries(bool isProxyTable);

/**************************************************************************//**
  \brief To add/update proxy/sink entry on NVM

  \param - entryPtr - pointer to the entry
             action - to be performed on the entry
             tableType - proxy or sink entry

  \return   true for successful operation
          false otherwise
 ******************************************************************************/
bool ZGPL_AddOrUpdateTableEntryOnNvm(void *entryPtr, ZGP_TableUpdateAction_t action, ZGP_EntryType_t tableType);

/**************************************************************************//**
  \brief To read a proxy/sink entry from NVm

  \param - entryPtr - entry to be filled
             tableOperationField - filter fields for read operation
             gpdId - pointer to address info
             endPoint - GPD end point

  \return   read status
 ******************************************************************************/
ZGP_ReadOperationStatus_t ZGPL_ReadTableEntryFromNvm(void *entryPtr, ZGP_TableOperationField_t  tableOperationField , ZGP_GpdId_t *gpdId, uint8_t endPoint);

/**************************************************************************//**
  \brief To delete proxy/sink entry

  \param - tableOperationField - filter fields for delete operation
               gpdId - address info
               endPoint - GPD end point


  \return   true for successful operation
          false otherwise
 ******************************************************************************/
bool ZGPL_DeleteTableEntryFromNvm(ZGP_TableOperationField_t filterField, ZGP_GpdId_t *gpdId, uint8_t endPoint);

/**************************************************************************//**
  \brief To read/update frameCounter

  \param - frameCounter - frameCounter to be updated/read
               tableOperationField - filter field for frame counter operation
               gpdId - address info
               endPoint - GPD end point
               isUpdateOperation - true for update
                                   false for read

  \return   true for successful operation
          false otherwise
 ******************************************************************************/
bool ZGPL_FrameCounterReadorUpdateOnNvm(uint32_t *frameCounter, ZGP_TableOperationField_t  tableOperationField , ZGP_GpdId_t *gpdId, uint8_t endPoint, \
                                     bool isUpdateOperation);

/**************************************************************************//**
  \brief To check whether pairing info in the sink entry is empty or not

  \param currEntry - Proxy table entry address

  \return   true -  Pairing Info empty
          false - Pairing Infor non-empty
 ******************************************************************************/
bool ZGPL_SinkEntryIsPairingInfoEmpty(ZGP_SinkTableEntry_t *currEntry);

/**************************************************************************//**
  \brief add sink group entry to the sink table entry

  \return   currEntry - pointer to the entry to be added (or) extended
          groupEntry  - sink group entry to be added
 ******************************************************************************/
bool ZGPL_AddSinkGroupEntry(ZGP_SinkTableEntry_t *currEntry, ZGP_SinkGroup_t *groupEntry);

/**************************************************************************//**
  \brief To reset all table entries on NVM

  \param - None

  \return   None
 ******************************************************************************/
void ZGPL_ResetTableToFN(void);
#endif // GREENPOWER_SUPPORT
#endif //_ZGPLOWNVMTABLE_H

//eof zgpNvmTable.h
