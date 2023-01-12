/*******************************************************************************
  Zigbee green power sink table Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpSinkTable.c

  Summary:
    This file contains the zgp infrastructure sink table implementation.

  Description:
    This file contains the zgp infrastructure sink table implementation.
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

#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC

/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcl.h>
#include <zcl/include/zclGreenPowerCluster.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpHighGeneric.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowNvmTable.h>
#include <zgp/GPInfrastructure/highZgp/include/private/zgpSinkTable.h>
#include <pds/include/wlPdsMemIds.h>
/******************************************************************************
                    Defines section
******************************************************************************/
/******************************************************************************
                    Implementations section
******************************************************************************/
extern void updateEntry(void);

/**************************************************************************//**
\brief Flush out in process sink table entries

\param[in] entry - entry to be populated
           index -  entry index

\return   true - entry found
          false - entry not found
******************************************************************************/
bool ZGPH_GetSinkTableEntryByIndex(ZGP_SinkTableEntry_t *entry, uint8_t index)
{
  ZGP_TableOperationField_t  tableOperationField = {.entryType = ZGP_SINK_ENTRY, .commMode = ALL_COMMUNICATION_MODE, .nonEmptyIndexForRead = index};

  if(ZGPL_TotalNonEmptyEntries(false) < (index + 1))
  {
     return false;
  }

  if ( ENTRY_NOT_AVAILABLE != ZGPL_ReadTableEntryFromNvm((void *)entry, tableOperationField,\
                                                       NULL, 0))
    return true;

  return false;
}
#endif // #if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_SINK_BASIC
#endif // _GREENPOWER_SUPPORT_

// eof zgpSinkTable.c
