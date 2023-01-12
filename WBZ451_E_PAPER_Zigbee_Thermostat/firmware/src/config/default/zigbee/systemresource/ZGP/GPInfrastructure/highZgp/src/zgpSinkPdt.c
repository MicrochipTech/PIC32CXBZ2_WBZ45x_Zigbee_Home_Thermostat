/*******************************************************************************
  Zigbee green power sink persistent data table Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpSinkPdt.c

  Summary:
    This file contains the sink persistent data table definition.

  Description:
    This file contains the sink persistent data table definition.
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
/*******************************************************************************
                            Includes section
*******************************************************************************/
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterServer.h>
#include <pds/include/wlPdsMemIds.h>
#include <systemenvironment/include/sysUtils.h>


/*******************************************************************************
                            extern variables section
*******************************************************************************/
extern ZGP_SinkGroup_t zgpSinkGroupEntry;
extern ZCL_GreenPowerClusterServerAttributes_t zgpClusterServerAttributes;

/*******************************************************************************
                            Declaration section
*******************************************************************************/
#define SINK_TABLE_ITEM_SIZE         (ZGP_SINK_TABLE_SIZE * sizeof(ZGP_SinkTableEntry_t))

PDS_DECLARE_FILE(ZGP_SINK_ATTR_MEM_ID, sizeof(zgpClusterServerAttributes), &zgpClusterServerAttributes, NO_FILE_MARKS);
PDS_DECLARE_FILE(ZGP_SINK_GROUP_ENTRY_MEM_ID, sizeof(zgpSinkGroupEntry), &zgpSinkGroupEntry, NO_FILE_MARKS);
#endif //#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
#endif // _GREENPOWER_SUPPORT_
//eof zgpSinkPdt.c
