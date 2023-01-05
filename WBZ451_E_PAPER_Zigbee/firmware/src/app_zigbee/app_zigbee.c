/*******************************************************************************
  Application Zigbee Source File

  Company:
    Microchip Technology Inc.

  File Name:
    zigbee_app_source.c

  Summary:
    This file contains the Application Zigbee implementation for this project.

  Description:
    This file contains the Application Zigbee implementation for this project.
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

#include <z3device/stack_interface/zgb_api.h>
#include <z3device/stack_interface/bdb/include/bdb_api.h>
#include <z3device/stack_interface/zcl/include/zcl_api.h>
#include <z3device/stack_interface/configServer/include/cs_api.h>
#include <z3device/stack_interface/nwk/include/nwk_api.h>
#include <app_zigbee/zigbee_console/console.h>
#include <z3device/stack_interface/bdb/include/BDB_Unpack.h>
#include <configserver/include/configserver.h>
#include <aps/include/apsCommon.h>
#include <zcl/include/zclAttributes.h>
#include <z3device/common/include/z3Device.h>
#include <zcl/include/clusters.h>
#include <pds/include/wlPdsMemIds.h>
#include "app_zigbee/app_zigbee.h"
#include "app.h"
#include <osal/osal_freertos.h>
// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Global Variables
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_ZigbeeStackCb(ZB_AppGenericCallbackParam_t *response)

  Remarks:
    See prototype in app_zigbee.h.
******************************************************************************/

void APP_ZigbeeStackCb(ZB_AppGenericCallbackParam_t *cb)
{
  APP_Msg_T   appMsg;
  void *paramPtr = NULL;
  
  ZB_AppGenericCallbackParam_t* newCB = 
          (ZB_AppGenericCallbackParam_t*) OSAL_Malloc(sizeof(ZB_AppGenericCallbackParam_t));

  
  appMsg.msgId = APP_MSG_ZB_STACK_CB;
  
  memcpy(newCB,cb,sizeof(ZB_AppGenericCallbackParam_t));

  if(cb->paramSize != 0)
  {
    paramPtr = OSAL_Malloc(cb->paramSize);
    memcpy(paramPtr,cb->parameters, cb->paramSize);
    newCB->parameters = paramPtr;
  }
  
  memcpy(appMsg.msgData,&newCB,sizeof(newCB));
#ifdef H3_INDEPENDENT 
  OSAL_QUEUE_Send(&g_appQueue, &appMsg,0);
#else
  OSAL_QUEUE_Send(&appData.appQueue, &appMsg,0);
#endif 
}

/*******************************************************************************
  Function:
    void APP_ZigbeeStackInit()

  Remarks:
    See prototype in app_zigbee.h.
******************************************************************************/
void APP_ZigbeeStackInit(void)
{
  ZB_EventRegister(APP_ZigbeeStackCb);
}
