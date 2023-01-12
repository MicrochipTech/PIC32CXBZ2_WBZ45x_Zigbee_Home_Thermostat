/*******************************************************************************
  Zigbee green power low sink Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpLowSink.c

  Summary:
    This file contains the Low layer Sink Interface API.

  Description:
    This file contains the Low layer Sink Interface API.
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
#if (APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC)
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpDstub.h>
#include <zgp/GPInfrastructure/lowZgp/include/private/zgpLowGeneric.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLow.h>
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
  \brief remove the pairing information in sink table(pre-commissioned mode)

  \param[in] currentEntry - pointer to sink table entry to be updated
             groupListToBeRemoved - groupList to be removed
             commMode - communication mode to be depaired
             sendPairing - bool to enabled/disable pairing tx

  \return true - if entry becomes empty
          false otherwise
******************************************************************************/
bool zgpRemoveSinkPairingInfo(ZGP_SinkTableEntry_t *currentEntry, ZGP_SinkGroup_t *groupListToBeRemoved, uint8_t commMode)
{
  bool removeEntry = false;

  if(commMode == PRECOMMISSIONED_GROUPCAST)
  {
    for(uint8_t i = 0; i < ZGP_SINK_GROUP_LIST_SIZE  ;i++)
    {
      bool matchFound = false;

      if(groupListToBeRemoved[i].sinkGroup != ZGP_NWK_ADDRESS_GROUP_INIT)
      {
        for(uint8_t j = 0; j < ZGP_SINK_GROUP_LIST_SIZE ; j++)
        {
          if(currentEntry->tableGenericInfo.zgpSinkGrouplist[j].sinkGroup == groupListToBeRemoved[i].sinkGroup)
          {
            matchFound = true;
            currentEntry->tableGenericInfo.zgpSinkGrouplist[j].sinkGroup = ZGP_NWK_ADDRESS_GROUP_INIT;
            currentEntry->tableGenericInfo.zgpSinkGrouplist[j].alias = ZGP_NWK_ADDRESS_GROUP_INIT;
          }
        }
        if (!matchFound)
          groupListToBeRemoved[i].sinkGroup = ZGP_NWK_ADDRESS_GROUP_INIT;
      }
    }
    removeEntry = ZGPL_SinkEntryIsPairingInfoEmpty(currentEntry);
  }
  else
  {
    if (commMode == DERIVED_GROUPCAST)
      currentEntry->tableGenericInfo.gpdAssignedAlias = ZGP_NWK_ADDRESS_GROUP_INIT;
    removeEntry = true;
  }

  return removeEntry;
}

/**************************************************************************//**
\brief add or extend the sink information for an entry

\param[in]   currEntry - pointer to the entry to be added (or) extended
            sinkTableEntry  - pointer to the entry to be copied

\return   true -If Entry is Updated Successfully
          false -If Entry is not updated Successfully(Can be due to Insufficinet Space)
******************************************************************************/
bool zgpAddExtendSinkTableEntry(ZGP_SinkTableEntry_t *currEntry, ZGP_SinkTableEntry_t *sinkTableEntry)
{
  // pre-commissioned groupcast
  if(sinkTableEntry->options.communicationMode == PRECOMMISSIONED_GROUPCAST)
  {
    //copy derived sink group ID
    // TBD search the empty addr entry and copy
    uint8_t nonEmptyPairingsReceived = 0;

    for(uint8_t j = 0 ; j < ZGP_SINK_GROUP_LIST_SIZE ; j++)
    {
      if(sinkTableEntry->tableGenericInfo.zgpSinkGrouplist[j].sinkGroup != ZGP_NWK_ADDRESS_GROUP_INIT)
        nonEmptyPairingsReceived++;
    }
    
    for(uint8_t pairingIndex = 0; pairingIndex < nonEmptyPairingsReceived; pairingIndex++)
    {
      if( !ZGPL_AddSinkGroupEntry(currEntry, &sinkTableEntry->tableGenericInfo.zgpSinkGrouplist[pairingIndex]) )
        return false;
    }
  }
  else if (sinkTableEntry->options.communicationMode == DERIVED_GROUPCAST)
  {
    uint16_t aliasAddr;
    if (sinkTableEntry->options.assignedAlias)
    {
      currEntry->options.assignedAlias = true;
      currEntry->tableGenericInfo.gpdAssignedAlias = sinkTableEntry->tableGenericInfo.gpdAssignedAlias;
      aliasAddr = currEntry->tableGenericInfo.gpdAssignedAlias;
    }
    else
    {
      ZGP_GpdId_t gpdId;

      gpdId.gpdIeeeAddr = sinkTableEntry->tableGenericInfo.gpdId.gpdIeeeAddr;
      aliasAddr = ZGPL_GetAliasSourceAddr(&gpdId);
    }

    if (aliasAddr == NWK_GetShortAddr())
      NWK_ForceChangeOwnAddr();
  }
  return true;
}

/**************************************************************************//**
\brief fill GPD info of sink table entry

\param[in]   currEntry - pointer to the entry to be replaced
            sinkTableEntry  - pointer to the entry to be copied

\return none
******************************************************************************/
void zgpFillGpdInfoOfSinkTableEntry(ZGP_SinkTableEntry_t *currEntry, ZGP_SinkTableEntry_t *sinkTableEntry)
{
  if (sinkTableEntry->tableGenericInfo.groupCastRadius > currEntry->tableGenericInfo.groupCastRadius)
    currEntry->tableGenericInfo.groupCastRadius = sinkTableEntry->tableGenericInfo.groupCastRadius;

  memcpy(&currEntry->options,&sinkTableEntry->options,sizeof(sinkTableEntry->options));
  currEntry->deviceId = sinkTableEntry->deviceId;
  currEntry->tableGenericInfo.gpdSecurityFrameCounter = sinkTableEntry->tableGenericInfo.gpdSecurityFrameCounter;
  // copy Security related parameters
  if(currEntry->options.securityUse)
  {
    memcpy(&currEntry->tableGenericInfo.securityOptions, &sinkTableEntry->tableGenericInfo.securityOptions, sizeof(uint8_t));
    memcpy(&currEntry->tableGenericInfo.securityKey, &sinkTableEntry->tableGenericInfo.securityKey, ZGP_SECURITY_KEY_LENGTH);
  }
}
#endif //APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
#endif // _GREENPOWER_SUPPORT_

// eof zgpLowSink.c
