/*******************************************************************************
  Zigbee green power lower layer proxy Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpLowProxy.c

  Summary:
    This file contains the Low Proxy Interface API.

  Description:
    This file contains the Low Proxy Interface API.
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
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpDstub.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpLowGeneric.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLow.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpLowProxy.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowGpdf.h>
#include <zdo/include/zdo.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpLowMem.h>
#include <security/serviceprovider/include/sspHash.h>
/******************************************************************************
                    Defines section
******************************************************************************/

/******************************************************************************
                    Prototypes section
******************************************************************************/

/******************************************************************************
                   Static variables section
******************************************************************************/

/******************************************************************************
                    Implementations section
******************************************************************************/
/**************************************************************************//**
\brief checks if Complete SinkInfo is empty

\param[in] Ptr to ProxyTableEntry

\return   true  - if entry is empty
          false - otherwise
******************************************************************************/
bool zgpCheckSinkInfoEmpty(ZGP_ProxyTableEntry_t *gpEntry)
{
  for(uint8_t i = 0; i<ZGP_LIGHT_WEIGHT_SINK_ADDRESS_LIST_SIZE; i++)
  {
    if (gpEntry->zgpProxyLightWeightSinkAddrlist[i].sinkNwkAddr != ZGP_NWK_ADDRESS_GROUP_INIT)
      return false;
  }
#if ZGP_PROXY_ADVANCED == 1
  for(uint8_t i = 0; i<ZGP_FULL_UNICAST_SINK_ADDRESS_LIST_SIZE; i++)
  {
    if (gpEntry->zgpProxyFullUnicastSinkAddrlist[i].sinkNwkAddr != ZGP_NWK_ADDRESS_GROUP_INIT)
      return false;
  }
#endif
  for(uint8_t i = 0; i<ZGP_SINK_GROUP_LIST_SIZE; i++)
  {
    if(gpEntry->tableGenericInfo.zgpSinkGrouplist[i].sinkGroup != ZGP_NWK_ADDRESS_GROUP_INIT)
      return false;
  }
  if (gpEntry->options.derivedGroupGps)
    return false; 

  return true;
}
/**************************************************************************//**
\brief Removes proxy table entry pairing info

\param[in] removeEntry - actual entry pairing info to be removed from
           rxdEntryInfo - reference entry(rxd in gp pairing cmd)

\return   true - after removing pairing info, entry becomes empty
          false - after removing pairing info, entry doesn't become empty
******************************************************************************/
bool zgpRemoveProxyPairingInfo(ZGP_ProxyTableEntry_t *removeEntry, ZGP_ProxyTableEntry_t *rxdEntyrInfo)
{
  if(rxdEntyrInfo->options.lightWeightUnicastGps)
  {
    for(uint8_t i = 0; i<ZGP_LIGHT_WEIGHT_SINK_ADDRESS_LIST_SIZE; i++)
    {
      if(removeEntry->zgpProxyLightWeightSinkAddrlist[i].sinkNwkAddr == rxdEntyrInfo->zgpProxyLightWeightSinkAddrlist[0].sinkNwkAddr &&
           removeEntry->zgpProxyLightWeightSinkAddrlist[i].sinkIeeeAddr == rxdEntyrInfo->zgpProxyLightWeightSinkAddrlist[0].sinkIeeeAddr)
      {
        removeEntry->zgpProxyLightWeightSinkAddrlist[i].sinkNwkAddr = ZGP_NWK_ADDRESS_GROUP_INIT;
        removeEntry->zgpProxyLightWeightSinkAddrlist[i].sinkIeeeAddr = 0x00;
      }
    }
  }
  if(rxdEntyrInfo->options.commissionedGroupGps)
  {
    for(uint8_t i = 0; i<ZGP_SINK_GROUP_LIST_SIZE; i++)
    {
      if(removeEntry->tableGenericInfo.zgpSinkGrouplist[i].sinkGroup == rxdEntyrInfo->tableGenericInfo.zgpSinkGrouplist[0].sinkGroup)
      {
        removeEntry->tableGenericInfo.zgpSinkGrouplist[i].sinkGroup = ZGP_NWK_ADDRESS_GROUP_INIT;
        removeEntry->tableGenericInfo.zgpSinkGrouplist[i].alias = ZGP_NWK_ADDRESS_GROUP_INIT;
      }
    }
  }
  if (rxdEntyrInfo->options.derivedGroupGps)
  {
    removeEntry->options.derivedGroupGps = false;
    removeEntry->options.assignedAlias = false;
    removeEntry->tableGenericInfo.gpdAssignedAlias = ZGP_NWK_ADDRESS_GROUP_INIT;
  }
#if ZGP_PROXY_ADVANCED == 1
  if(gpPairingCmd->options.communicationMode == FULL_UNICAST)
  {
    for(uint8_t i = 0; i<ZGP_FULL_UNICAST_SINK_ADDRESS_LIST_SIZE; i++)
    {
      if(removeEntry->zgpProxyFullUnicastSinkAddrlist[i].sinkNwkAddr == gpPairingCmd->sinkNwkAddr &&
           removeEntry->zgpProxyLightWeightSinkAddrlist[i].sinkIeeeAddr == gpPairingCmd->sinkIeeeAddr)
      {
        removeEntry->zgpProxyFullUnicastSinkAddrlist[i].sinkNwkAddr = ZGP_NWK_ADDRESS_GROUP_INIT;
        removeEntry->zgpProxyFullUnicastSinkAddrlist[i].sinkIeeeAddr = 0x00;
      }
    }
  }
#endif
  if (zgpCheckSinkInfoEmpty(removeEntry))
    return true;
  else
    return false;
}
/**************************************************************************//**
\brief To update proxy entry

\param[in] - currentEntry - entry to be updated
             entryInfo - reference entry

\return   true -If Entry is Updated Successfully
          false -If Entry is not updated Successfully(Can be due to Insufficinet Space)
******************************************************************************/
bool zgpUpdateProxyEntry(ZGP_ProxyTableEntry_t *currentEntry, ZGP_ProxyTableEntry_t *entryInfo)
{
  uint16_t derivedGrpId = 0;
  //uint16_t gpdGroupId = ZGP_NWK_ADDRESS_GROUP_INIT;

  // copy sink nwk and ieee address
  if(entryInfo->options.lightWeightUnicastGps) // lightweight unicast
  {
    uint8_t availableIndex = ZGP_ENTRY_INVALID_INDEX;
    uint8_t i = 0;

    for(i = 0; i<ZGP_LIGHT_WEIGHT_SINK_ADDRESS_LIST_SIZE; i++)
    {
      if(currentEntry->zgpProxyLightWeightSinkAddrlist[i].sinkNwkAddr == ZGP_NWK_ADDRESS_GROUP_INIT)
      {
        if(availableIndex == ZGP_ENTRY_INVALID_INDEX)
          availableIndex = i;
      }
      else if (currentEntry->zgpProxyLightWeightSinkAddrlist[i].sinkNwkAddr == entryInfo->zgpProxyLightWeightSinkAddrlist[0].sinkNwkAddr)
        break;
    }
    if (i == ZGP_LIGHT_WEIGHT_SINK_ADDRESS_LIST_SIZE)
    {
      if(availableIndex != ZGP_ENTRY_INVALID_INDEX)//ZGP_ENTRY_INVALID_INDEX
      {
        currentEntry->zgpProxyLightWeightSinkAddrlist[availableIndex].sinkNwkAddr = entryInfo->zgpProxyLightWeightSinkAddrlist[0].sinkNwkAddr;
        currentEntry->zgpProxyLightWeightSinkAddrlist[availableIndex].sinkIeeeAddr = entryInfo->zgpProxyLightWeightSinkAddrlist[0].sinkIeeeAddr;
        currentEntry->options.lightWeightUnicastGps = true;
      }
      else
        return false;
    }
  }
  else
  {
    ZGP_GpdId_t gpdId;

    gpdId.gpdIeeeAddr = entryInfo->tableGenericInfo.gpdId.gpdIeeeAddr;
    derivedGrpId = ZGPL_GetAliasSourceAddr(&gpdId);
  }
  if(entryInfo->options.commissionedGroupGps) // pre-commissioned groupcast
  {
    //copy derived sink group ID
    uint8_t availableIndex = ZGP_ENTRY_INVALID_INDEX;
    uint8_t i = 0;
    for(i = 0; i<ZGP_SINK_GROUP_LIST_SIZE; i++)
    {
      if(currentEntry->tableGenericInfo.zgpSinkGrouplist[i].sinkGroup == ZGP_NWK_ADDRESS_GROUP_INIT)
      {
        if(availableIndex == ZGP_ENTRY_INVALID_INDEX)
          availableIndex = i;
      }
      else if (currentEntry->tableGenericInfo.zgpSinkGrouplist[i].sinkGroup == entryInfo->tableGenericInfo.zgpSinkGrouplist[0].sinkGroup)
      {
        //gpdGroupId = entryInfo->tableGenericInfo.zgpSinkGrouplist[0].sinkGroup;
        break;
      }
    }
    if (i == ZGP_SINK_GROUP_LIST_SIZE)
    {
      if(availableIndex != ZGP_ENTRY_INVALID_INDEX)
      {
        currentEntry->tableGenericInfo.zgpSinkGrouplist[availableIndex].sinkGroup = entryInfo->tableGenericInfo.zgpSinkGrouplist[0].sinkGroup;
        //gpdGroupId = entryInfo->tableGenericInfo.zgpSinkGrouplist[0].sinkGroup;
        if (ZGP_NWK_ADDRESS_GROUP_INIT != entryInfo->tableGenericInfo.zgpSinkGrouplist[0].alias)
        {
          currentEntry->tableGenericInfo.zgpSinkGrouplist[availableIndex].alias = entryInfo->tableGenericInfo.zgpSinkGrouplist[0].alias;
          // Need to raise address conflict incase of matching with own network address
          if (currentEntry->tableGenericInfo.zgpSinkGrouplist[availableIndex].alias == NWK_GetShortAddr())
            NWK_ForceChangeOwnAddr();
        }
        else
          currentEntry->tableGenericInfo.zgpSinkGrouplist[availableIndex].alias = derivedGrpId;
      }
      else
        return false;

      currentEntry->options.commissionedGroupGps = true;
    }
  }
  if (entryInfo->options.derivedGroupGps)
  {
    currentEntry->options.derivedGroupGps =  true;

    if (entryInfo->options.assignedAlias)
    {
      currentEntry->options.assignedAlias = true;
      currentEntry->tableGenericInfo.gpdAssignedAlias = entryInfo->tableGenericInfo.gpdAssignedAlias;
    }
  }


  currentEntry->options.gpdFixed  = entryInfo->options.gpdFixed;
  currentEntry->options.macSeqNumCapability = entryInfo->options.macSeqNumCapability;
  currentEntry->options.firstToFoward = entryInfo->options.firstToFoward;
  currentEntry->options.inRange = entryInfo->options.inRange;

  // copy Security related parameters
  if(entryInfo->options.securityUse)
  {
    currentEntry->options.securityUse = true;
    currentEntry->tableGenericInfo.securityOptions.securityLevel = entryInfo->tableGenericInfo.securityOptions.securityLevel;
    currentEntry->tableGenericInfo.securityOptions.securityKeyType = entryInfo->tableGenericInfo.securityOptions.securityKeyType;
  }
  if(entryInfo->tableGenericInfo.gpdSecurityFrameCounter != 0xFFFFFFFF)
  {
    currentEntry->tableGenericInfo.gpdSecurityFrameCounter = entryInfo->tableGenericInfo.gpdSecurityFrameCounter;
  }
  if(ZGPL_IskeyValid(&entryInfo->tableGenericInfo.securityKey[0]))
  {
    memcpy(&currentEntry->tableGenericInfo.securityKey[0], &entryInfo->tableGenericInfo.securityKey[0], ZGP_SECURITY_KEY_LENGTH);
  }

  if(entryInfo->tableGenericInfo.groupCastRadius)
  {
    if (entryInfo->tableGenericInfo.groupCastRadius > currentEntry->tableGenericInfo.groupCastRadius)
      currentEntry->tableGenericInfo.groupCastRadius = entryInfo->tableGenericInfo.groupCastRadius;
  }
  return true;
}
#endif //APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
#endif // _GREENPOWER_SUPPORT_

// eof zgpLowProxy.c
