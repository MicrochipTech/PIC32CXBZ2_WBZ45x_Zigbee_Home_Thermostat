/*******************************************************************************
  Zigbee green power proxy persistent data Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpProxyPdt.c

  Summary:
    This file contains the proxy persistent data table definition.

  Description:
    This file contains the proxy persistent data table definition.
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
/*******************************************************************************
                            Includes section
*******************************************************************************/
#include <zgp/GPInfrastructure/highZgp/include/private/zgpClusterClient.h>
#include <pds/include/wlPdsMemIds.h>
#include <systemenvironment/include/sysUtils.h>
/*******************************************************************************
                            Global variables section
*******************************************************************************/
extern ZCL_GreenPowerClusterClientAttributes_t zgpClusterClientAttributes;
/*******************************************************************************
                            Declaration section
*******************************************************************************/
#define PROXY_TABLE_ITEM_SIZE         (ZGP_PROXY_TABLE_SIZE * sizeof(ZGP_ProxyTableEntry_t))
#define PROXY_SHARED_KEY_ITEM_SIZE       (SECURITY_KEY_SIZE * sizeof(uint8_t))
#define PROXY_SHARED_KEY_TYPE_ITEM_SIZE  (sizeof(uint8_t))
#define PROXY_LINK_KEY_ITEM_SIZE         (SECURITY_KEY_SIZE * sizeof(uint8_t))

PDS_DECLARE_FILE(ZGP_PROXY_SHARED_KEY_TYPE_MEM_ID, PROXY_SHARED_KEY_TYPE_ITEM_SIZE, &zgpClusterClientAttributes.gpSharedSecurityKeyType.value, NO_FILE_MARKS);
PDS_DECLARE_FILE(ZGP_PROXY_SHARED_KEY_MEM_ID, PROXY_SHARED_KEY_ITEM_SIZE, &zgpClusterClientAttributes.gpSharedSecuritykey.value,     NO_FILE_MARKS);
PDS_DECLARE_FILE(ZGP_PROXY_LINK_KEY_MEM_ID, PROXY_LINK_KEY_ITEM_SIZE, &zgpClusterClientAttributes.gpLinkKey.value,               NO_FILE_MARKS);

PROGMEM_DECLARE(PDS_MemId_t zgpProxyMemoryIdsTable[]) =
{
  ZGP_PROXY_SHARED_KEY_TYPE_MEM_ID,
  ZGP_PROXY_SHARED_KEY_MEM_ID,
  ZGP_PROXY_LINK_KEY_MEM_ID
};
#endif //#if APP_ZGP_DEVICE_TYPE >= APP_ZGP_DEVICE_TYPE_PROXY_BASIC
#endif /* _GREENPOWER_SUPPORT_ */
//eof zgpProxyPdt.c
