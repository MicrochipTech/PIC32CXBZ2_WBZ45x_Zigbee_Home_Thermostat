/*******************************************************************************
  Zigbee green power generic Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpGeneric.c

  Summary:
    This file contains the generic Interface API to ZGP layer.

  Description:
    This file contains the generic Interface API to ZGP layer.
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
#if (APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC)
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zgp/GPInfrastructure/lowZgp/include/zgpLow.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowNvmTable.h>
#include <nwk/include/nwk.h>

#if (APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC)
/**************************************************************************//**
\brief returns true if Pairing Info is empty for a particular entry
                   False if Info is present

\param[in] currEntry - Proxy table entry address

\return   true -  Pairing Info empty
          false - Pairing Infor non-empty
******************************************************************************/
bool ZGPL_SinkEntryIsPairingInfoEmpty(ZGP_SinkTableEntry_t *currEntry)
{
  bool empty = true;
  for (uint8_t i = 0; i < ZGP_SINK_GROUP_LIST_SIZE; i++)
  {
    if(currEntry->tableGenericInfo.zgpSinkGrouplist[i].sinkGroup != ZGP_NWK_ADDRESS_GROUP_INIT)
    {
      empty = false;
      break;
    }
  }
  return empty;
}

/**************************************************************************//**
\brief add sink group entry to the sink table entry

\param[in]   currEntry - pointer to the entry to be added (or) extended
             groupEntry  - sink group entry to be added

return None
******************************************************************************/
bool ZGPL_AddSinkGroupEntry(ZGP_SinkTableEntry_t *currEntry, ZGP_SinkGroup_t *groupEntry)
{
  uint8_t availableIndex = ZGP_ENTRY_INVALID_INDEX;
  uint8_t i = 0;

  for(i = 0; i<ZGP_SINK_GROUP_LIST_SIZE; i++)
  {
    if(currEntry->tableGenericInfo.zgpSinkGrouplist[i].sinkGroup == ZGP_NWK_ADDRESS_GROUP_INIT)
    {
      if(availableIndex == ZGP_ENTRY_INVALID_INDEX)
        availableIndex = i;
    }
    else if (currEntry->tableGenericInfo.zgpSinkGrouplist[i].sinkGroup == groupEntry->sinkGroup)
      break;
  }
  if(i == ZGP_SINK_GROUP_LIST_SIZE)
  {
    if (availableIndex != ZGP_ENTRY_INVALID_INDEX)
    {
      currEntry->tableGenericInfo.zgpSinkGrouplist[availableIndex].sinkGroup = groupEntry->sinkGroup;
      currEntry->tableGenericInfo.zgpSinkGrouplist[availableIndex].alias = groupEntry->alias;
      if (ZGP_NWK_ADDRESS_GROUP_INIT == groupEntry->alias)
      {
        ZGP_GpdId_t gpdId;

        gpdId.gpdIeeeAddr = currEntry->tableGenericInfo.gpdId.gpdIeeeAddr;
        currEntry->tableGenericInfo.zgpSinkGrouplist[availableIndex].alias = ZGPL_GetAliasSourceAddr(&gpdId);
      }
      // Need to raise address conflict incase of matching with own network address
      if (groupEntry->alias == NWK_GetShortAddr())
        NWK_ForceChangeOwnAddr();
    }
    else
      return false;
  }

  return true;
}
#endif //APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
/**************************************************************************//**
\brief Initialize proxy/sink table

\param[in] - entry - entry to be reset
             tableType - proxy/sink entry

\return None
******************************************************************************/
void ZGPL_ResetTableEntry(void *entry, ZGP_EntryType_t tableType)
{
  zgpTableGenericInfo_t *tableGenericInfo;
  ZGP_ProxyTableEntry_t *proxyEntry;
#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
  ZGP_SinkTableEntry_t *sinkEntry;

  if (ZGP_SINK_ENTRY == tableType)
  {
    sinkEntry = (ZGP_SinkTableEntry_t *)entry;
    memset(sinkEntry,0x00,sizeof(ZGP_SinkTableEntry_t));
    tableGenericInfo = &sinkEntry->tableGenericInfo;
  }
  else
#endif
  {
    proxyEntry = (ZGP_ProxyTableEntry_t *)entry;
    memset(proxyEntry,0x00,sizeof(ZGP_ProxyTableEntry_t));
    tableGenericInfo = &proxyEntry->tableGenericInfo;
    for (uint8_t i = 0; i < ZGP_LIGHT_WEIGHT_SINK_ADDRESS_LIST_SIZE; i++)
      proxyEntry->zgpProxyLightWeightSinkAddrlist[i].sinkNwkAddr = ZGP_NWK_ADDRESS_GROUP_INIT;
#if ZGP_PROXY_ADVANCED == 1
    for (uint8_t i = 0; i < ZGP_FULL_UNICAST_SINK_ADDRESS_LIST_SIZE; i++)
      entry->zgpProxyFullUnicastSinkAddrlist[i].sinkNwkAddr = ZGP_NWK_ADDRESS_GROUP_INIT;
#endif
  }

  //Default Values for SecFrameCtr and GroupCastRadius Set to Values as mentioned in Spec
  tableGenericInfo->gpdSecurityFrameCounter = ZGP_SEC_FRAME_COUNTER_DEFAULT;
  tableGenericInfo->gpdAssignedAlias = ZGP_NWK_ADDRESS_GROUP_INIT;
  tableGenericInfo->groupCastRadius = ZGP_GROUP_CAST_RADIUS_DEFAULT;

  for (uint8_t i = 0; i < ZGP_SINK_GROUP_LIST_SIZE; i++)
  {
    tableGenericInfo->zgpSinkGrouplist[i].sinkGroup = ZGP_NWK_ADDRESS_GROUP_INIT;
    tableGenericInfo->zgpSinkGrouplist[i].alias = ZGP_NWK_ADDRESS_GROUP_INIT;
  }
}
#endif //APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
#endif // _GREENPOWER_SUPPORT_
