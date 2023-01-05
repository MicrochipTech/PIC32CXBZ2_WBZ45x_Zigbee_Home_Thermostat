/*******************************************************************************
  Zigbee green power proxy table Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpProxyTable.c

  Summary:
    This file contains the zgp infrastructure proxy table implementation.

  Description:
    This file contains the zgp infrastructure proxy table implementation.
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
#include <zcl/include/zcl.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighMem.h>
#include <zcl/include/zclGreenPowerCluster.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterClient.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighGeneric.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowNvmTable.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpProxyTable.h>
#include <pds/include/wlPdsMemIds.h>

/******************************************************************************
                    Implementations section
******************************************************************************/
/**************************************************************************//**
\brief Creates New  proxy table entry

\param[in] gpPairingCmd - pairing command info
          isUpdateRequested - status of requested update
             
\return  ZGP_ENTRY_ADDED - if Entry added Successfully
         ZGP_PROXY_TABLE_FULL - if Proxy Table is full
******************************************************************************/
zgpProxyTableStatus_t zgpProxyTableCreateOrUpdateEntry(ZGP_GpPairing_t *gpPairingCmd, bool isUpdateRequested)
{
  ZGP_ProxyTableEntry_t *proxyTableEntry = (ZGP_ProxyTableEntry_t *)zgpGetMemReqBuffer();
  uint16_t gpdGroupId = ZGP_NWK_ADDRESS_GROUP_INIT;
  uint16_t derivedGrpId = ZGP_NWK_ADDRESS_GROUP_INIT;
  ZGP_TableUpdateAction_t tableAction = UPDATE_ENTRY;
  zgpProxyTableStatus_t status = ZGP_PROXY_TABLE_SUCCESS;

  if (!isUpdateRequested)
  {
    tableAction = REMOVE_PAIRING_ENTRY;
  }

  ZGPL_ResetTableEntry((void *)proxyTableEntry, ZGP_PROXY_ENTRY);
  // copy sink nwk and ieee address
  if(gpPairingCmd->options.communicationMode == LIGHTWEIGHT_UNICAST) // lightweight unicast
  {
      proxyTableEntry->zgpProxyLightWeightSinkAddrlist[0].sinkNwkAddr = gpPairingCmd->sinkNwkAddr;
      proxyTableEntry->zgpProxyLightWeightSinkAddrlist[0].sinkIeeeAddr = gpPairingCmd->sinkIeeeAddr;
      proxyTableEntry->options.lightWeightUnicastGps = true;  
  }
  else
  {
    ZGP_GpdId_t gpdId;

    gpdId.gpdIeeeAddr = gpPairingCmd->gpdId.gpdIeeeAddr;
    derivedGrpId = ZGPL_GetAliasSourceAddr(&gpdId);
  }
  if(PRECOMMISSIONED_GROUPCAST == gpPairingCmd->options.communicationMode) // pre-commissioned groupcast
  {
    proxyTableEntry->tableGenericInfo.zgpSinkGrouplist[0].sinkGroup = gpPairingCmd->sinkGroupId;
    gpdGroupId = gpPairingCmd->sinkGroupId;
    if (gpPairingCmd->options.assignedAliasPresent)
    {
      proxyTableEntry->tableGenericInfo.zgpSinkGrouplist[0].alias = gpPairingCmd->assignedAlias;
    }
    else
      proxyTableEntry->tableGenericInfo.zgpSinkGrouplist[0].alias = derivedGrpId;

    // Need to raise address conflict incase of matching with own network address
    if (proxyTableEntry->tableGenericInfo.zgpSinkGrouplist[0].alias == NWK_GetShortAddr())
        NWK_ForceChangeOwnAddr();

    proxyTableEntry->options.commissionedGroupGps = true;

  }
  if (DERIVED_GROUPCAST == gpPairingCmd->options.communicationMode)
  {
    proxyTableEntry->options.derivedGroupGps =  true;

    {
      uint16_t aliasAddr;

      gpdGroupId = derivedGrpId;
      if (gpPairingCmd->options.assignedAliasPresent)
      {
        proxyTableEntry->options.assignedAlias = true;
        proxyTableEntry->tableGenericInfo.gpdAssignedAlias = gpPairingCmd->assignedAlias;
        aliasAddr = proxyTableEntry->tableGenericInfo.gpdAssignedAlias;
      }
      else
      {
        aliasAddr = gpdGroupId;
      }

      if (aliasAddr == NWK_GetShortAddr())
        NWK_ForceChangeOwnAddr();
    }
  }

  if ((DERIVED_GROUPCAST == gpPairingCmd->options.communicationMode) || \
      (PRECOMMISSIONED_GROUPCAST == gpPairingCmd->options.communicationMode))
  {
    if (!NWK_IsGroupMember(gpdGroupId, GREEN_POWER_ENDPOINT))
    {
      if (!NWK_AddGroup(gpdGroupId, GREEN_POWER_ENDPOINT))
      {
        // To be decided on handling this negative scenario
      }
    }
  }

  proxyTableEntry->options.gpdFixed  = gpPairingCmd->options.gpdFixed;
  proxyTableEntry->options.macSeqNumCapability = gpPairingCmd->options.gpdMacSeqNumCapability;

  // copy Security related parameters
  if(gpPairingCmd->options.securityLevel != ZGP_SECURITY_LEVEL_0)
  {
    proxyTableEntry->options.securityUse = true;
    proxyTableEntry->tableGenericInfo.securityOptions.securityLevel = gpPairingCmd->options.securityLevel;
    proxyTableEntry->tableGenericInfo.securityOptions.securityKeyType = gpPairingCmd->options.securityKeyType;
  }
  if(gpPairingCmd->options.gpdSecurityFrameCounterPresent)
  {
    proxyTableEntry->tableGenericInfo.gpdSecurityFrameCounter = gpPairingCmd->gpdSecurityFrameCounter;
  }
  if(gpPairingCmd->options.gpdSecurityKeyPresent)
  {
    memcpy(&proxyTableEntry->tableGenericInfo.securityKey, &gpPairingCmd->gpdKey, ZGP_SECURITY_KEY_LENGTH);
  }

  proxyTableEntry->options.appId = gpPairingCmd->options.appId;

  if(gpPairingCmd->options.appId == ZGP_SRC_APPID)
    proxyTableEntry->tableGenericInfo.gpdId.gpdSrcId = gpPairingCmd->gpdId.gpdSrcId;
  else if(gpPairingCmd->options.appId == ZGP_IEEE_ADDR_APPID)
  {
    proxyTableEntry->tableGenericInfo.gpdId.gpdIeeeAddr = gpPairingCmd->gpdId.gpdIeeeAddr;
    proxyTableEntry->tableGenericInfo.endPoint = gpPairingCmd->endpoint;
  }

  // copy groupcast radius
  proxyTableEntry->tableGenericInfo.groupCastRadius = 0x00; // As per spec. A.3.3.5.2
  if(gpPairingCmd->options.groupCastRadiusPresent)
  {
     proxyTableEntry->tableGenericInfo.groupCastRadius = gpPairingCmd->groupCastRadius;
  }

  proxyTableEntry->options.entryActive = true;
  proxyTableEntry->options.entryValid = true;

  if (ZGPL_AddOrUpdateTableEntryOnNvm((void *)proxyTableEntry, tableAction, ZGP_PROXY_ENTRY) == false)
  {
    if(tableAction != UPDATE_ENTRY)
      status = ZGP_PROXY_TABLE_ENTRY_NOT_AVAILABLE;
    else
      status = ZGP_PROXY_TABLE_FULL;
  }
  
  zgpFreeMemReqBuffer();
  return status;
}

#endif // #if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
#endif // _GREENPOWER_SUPPORT_

// eof zgpProxyTable.c
