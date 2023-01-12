/*******************************************************************************
  Thermostat Scenes cluster Source File

  Company:
    Microchip Technology Inc.

  File Name:
    thScenesCluster.c

  Summary:
    This file contains the Thermostat Scenes cluster interface.

  Description:
    This file contains the Thermostat Scenes cluster interface.
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

#if (APP_Z3_DEVICE_TYPE == APP_DEVICE_TYPE_THERMOSTAT)

/******************************************************************************
                    Includes section
******************************************************************************/
#include <z3device/thermostat/include/thScenesCluster.h>
#include <z3device/thermostat/include/thThermostatCluster.h>
#include <z3device/clusters/include/haClusters.h>
#include <app_zigbee/zigbee_console/console.h>
#include <zcl/include/zclCommandManager.h>
#include <pds/include/wlPdsMemIds.h>
#include <z3device/clusters/include/scenesCluster.h>
#include <zcl/clusters/include/scenes.h>
#include <zcl/clusters/include/groupsCluster.h>
#include <z3device/common/include/appConsts.h>
#include <app_zigbee/app_zigbee_handler.h>
/******************************************************************************
                    Definitions section
******************************************************************************/
#define GLOBAL_SCENE_SCENE_ID 0u
#define GLOBAL_SCENE_GROUP_ID 0u

/******************************************************************************
                    Prototypes section
******************************************************************************/
static ZCL_Status_t addSceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_AddScene_t *payload);
static ZCL_Status_t viewSceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_ViewScene_t *payload);
static ZCL_Status_t removeSceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_RemoveScene_t *payload);
static ZCL_Status_t removeAllScenesInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_RemoveAllScenes_t *payload);
static ZCL_Status_t storeSceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_StoreScene_t *payload);
static ZCL_Status_t recallSceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_RecallScene_t *payload);
static ZCL_Status_t getSceneMembershipInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_GetSceneMembership_t *payload);
static ZCL_Status_t enhancedAddSceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_EnhancedAddScene_t *payload);
static ZCL_Status_t enhancedViewSceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_EnhancedViewScene_t *payload);
static ZCL_Status_t copySceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_CopyScene_t *payload);


/******************************************************************************
                    Global variables section
******************************************************************************/
ZCL_SceneClusterServerAttributes_t thScenesClusterServerAttributes =
{
  ZCL_DEFINE_SCENES_CLUSTER_SERVER_ATTRIBUTES()
};

PROGMEM_DECLARE(ZCL_ScenesClusterCommands_t thScenesCommands) =
{
  ZCL_DEFINE_SCENES_CLUSTER_SERVER_COMMANDS(addSceneInd, viewSceneInd, removeSceneInd, removeAllScenesInd,
                                     storeSceneInd, recallSceneInd, getSceneMembershipInd,
                                     enhancedAddSceneInd, enhancedViewSceneInd, copySceneInd)
};

Scene_t thermostatSceneTable[MAX_SCENES_AMOUNT];
/******************************************************************************
                    Implementation section
******************************************************************************/
/**************************************************************************//**
\brief Initializes Scenes cluster
******************************************************************************/
void thScenesClusterInit(void)
{
  memset(thermostatSceneTable, 0, sizeof(Scene_t) * MAX_SCENES_AMOUNT);

  for (uint8_t i = 0; i < MAX_SCENES_AMOUNT; i++)
      thermostatSceneTable[i].free = true;

  thScenesClusterServerAttributes.sceneCount.value   = 0;
  thScenesClusterServerAttributes.currentScene.value = 0;
  thScenesClusterServerAttributes.currentGroup.value = 0;
  thScenesClusterServerAttributes.sceneValid.value   = true;
  thScenesClusterServerAttributes.nameSupport.value  = 0;
  thScenesClusterServerAttributes.clusterVersion.value = SCENES_CLUSTER_VERSION;
  /* Allocate space for global scene*/
  {
    Scene_t *scene = allocateScene(thermostatSceneTable, &thScenesClusterServerAttributes.sceneCount.value);

    scene->sceneId = GLOBAL_SCENE_SCENE_ID;
    scene->groupId = GLOBAL_SCENE_GROUP_ID;
    thScenesClusterServerAttributes.sceneCount.value--; // to pass the certification
  }
}

/**************************************************************************//**
\brief Callback on receiving Add Scene command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t addSceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_AddScene_t *payload)
{
  ZCL_Status_t status;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_ADD_SCENE;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;
  status = processAddSceneCommand(false, addressing, payloadLength, (ZCL_AddScene_t *)payload, APP_ENDPOINT_THERMOSTAT, thermostatSceneTable, &thScenesClusterServerAttributes.sceneCount.value);
  if (ZCL_SUCCESS_STATUS == status)
  {
    PDS_Store(APP_TH_SCENES_MEM_ID);
  }
  APP_Zigbee_Handler(event);
  return status;
}
/**************************************************************************//**
\brief Callback on receiving View Scene command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t viewSceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_ViewScene_t *payload)
{
  ZCL_Status_t status;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_VIEW_SCENE;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;
  status = processViewSceneCommand(false, addressing, payload, APP_ENDPOINT_THERMOSTAT, thermostatSceneTable);

  APP_Zigbee_Handler(event);
  return status;
}

/**************************************************************************//**
\brief Callback on receiving Remove Scene command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t removeSceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_RemoveScene_t *payload)
{
  ZCL_Status_t status;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_REMOVE_SCENE;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;

  status = processRemoveSceneCommand(addressing, payload, APP_ENDPOINT_THERMOSTAT, thermostatSceneTable, &thScenesClusterServerAttributes.sceneCount.value);

  if (ZCL_SUCCESS_STATUS == status)
  {
    PDS_Store(APP_TH_SCENES_MEM_ID);
  }

  APP_Zigbee_Handler(event);
  return status;
}

/**************************************************************************//**
\brief Callback on receiving Remove All Scenes command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t removeAllScenesInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_RemoveAllScenes_t *payload)
{
  ZCL_Status_t status;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_REMOVE_ALL_SCENES;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;
  status = processRemoveAllScenesCommand(addressing, payload, APP_ENDPOINT_THERMOSTAT, thermostatSceneTable, &thScenesClusterServerAttributes.sceneCount.value);

  // Update scenes in non-volatile memory
  if (ZCL_SUCCESS_STATUS == status)
    PDS_Store(APP_TH_SCENES_MEM_ID);

  APP_Zigbee_Handler(event);
  return status;
}

/**************************************************************************//**
\brief Callback on receiving Store Scene command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t storeSceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_StoreScene_t *payload)
{
  ZCL_Status_t status;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_STORE_SCENE;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;
  status = processStoreSceneCommand(addressing, payload, APP_ENDPOINT_THERMOSTAT, thermostatSceneTable, &thScenesClusterServerAttributes);

  // Update scenes in non-volatile memory
  if (ZCL_SUCCESS_STATUS == status)
    PDS_Store(APP_TH_SCENES_MEM_ID);
  APP_Zigbee_Handler(event);
  return status;
}

/**************************************************************************//**
\brief Callback on receiving Recall Scene command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t recallSceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_RecallScene_t *payload)
{
  ZCL_Status_t status;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_RECALL_SCENE;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;
  status = recallScene(payload, APP_ENDPOINT_THERMOSTAT, thermostatSceneTable, &thScenesClusterServerAttributes);
  
  if (ZCL_SUCCESS_STATUS == status)
    PDS_Store(APP_TH_SCENES_MEM_ID);

  APP_Zigbee_Handler(event);
  return status;
}

/**************************************************************************//**
\brief Callback on receiving Get Scenes Membership command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t getSceneMembershipInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_GetSceneMembership_t *payload)
{
  ZCL_Status_t status;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_GET_SCENE_MEMBERSHIP;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;

  status = processGetSceneMembershipCommand(addressing, payload, APP_ENDPOINT_THERMOSTAT, thermostatSceneTable, thScenesClusterServerAttributes.sceneCount.value);
  APP_Zigbee_Handler(event);
  return status;
}

/**************************************************************************//**
\brief Callback on receive of Enhanced Add Scene command
******************************************************************************/
static ZCL_Status_t enhancedAddSceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_EnhancedAddScene_t *payload)
{
  ZCL_Status_t status;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_ENHANCED_ADD_SCENE;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;
  status =  processAddSceneCommand(true, addressing, payloadLength, (ZCL_AddScene_t *)payload, APP_ENDPOINT_THERMOSTAT, thermostatSceneTable, &thScenesClusterServerAttributes.sceneCount.value);

  if (ZCL_SUCCESS_STATUS == status)
  {
     PDS_Store(APP_TH_SCENES_MEM_ID);
  }
  APP_Zigbee_Handler(event);
  return status;
}

/**************************************************************************//**
\brief Callback on receive of Enhanced View Scene command
******************************************************************************/
static ZCL_Status_t enhancedViewSceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_EnhancedViewScene_t *payload)
{
  ZCL_Status_t status;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_ENHANCED_VIEW_SCENE;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;
  status = processViewSceneCommand(true, addressing, (ZCL_ViewScene_t *)payload, APP_ENDPOINT_THERMOSTAT, thermostatSceneTable);
  APP_Zigbee_Handler(event);
  return status;
}

/**************************************************************************//**
\brief Callback on receive of Copy Scene command
******************************************************************************/
static ZCL_Status_t copySceneInd(ZCL_Addressing_t *addressing, uint8_t payloadLength,
                                 ZCL_CopyScene_t *payload)
{
  ZCL_Status_t status;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_COPY_SCENE;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;
  status = processCopySceneCommand(addressing, payload, APP_ENDPOINT_THERMOSTAT,
                                   thermostatSceneTable, &thScenesClusterServerAttributes);

  // Update scenes in non-volatile memory
  if (ZCL_SUCCESS_STATUS == status)
    PDS_Store(APP_TH_SCENES_MEM_ID);
  APP_Zigbee_Handler(event);
  return status;
}

/**************************************************************************//**
\brief Stores scene to scene table

\param[in] storeScene - the pointer to Store Scene request

\returns status of scene storing
******************************************************************************/
ZCL_Status_t storeScene(ZCL_StoreScene_t *storeScene, Endpoint_t srcEp, Scene_t* scenePool, ZCL_SceneClusterServerAttributes_t* scenesAttributes)
{
    if (groupsIsValidGroup(storeScene->groupId, APP_ENDPOINT_THERMOSTAT))
  {
    Scene_t *scene;

    scene = findSceneBySceneAndGroup(storeScene->groupId, storeScene->sceneId, scenePool);
    if (!scene)
    {
      scene = allocateScene(scenePool, &scenesAttributes->sceneCount.value);
    }
    else
    {
      thScenesClusterServerAttributes.currentScene.value = scene->sceneId;
      thScenesClusterServerAttributes.currentGroup.value = scene->groupId;
      thScenesClusterServerAttributes.sceneValid.value   = true;

      /* scene alraedy exists; update with current extension field sets */
      return getExtensionFieldInfo(scene);
    }

    if (scene)
    {
      thScenesClusterServerAttributes.sceneCount.value++;

      scene->transitionTime = 0;
      scene->groupId        = storeScene->groupId;
      scene->sceneId        = storeScene->sceneId;

      thScenesClusterServerAttributes.currentScene.value = scene->sceneId;
      thScenesClusterServerAttributes.currentGroup.value = scene->groupId;
      thScenesClusterServerAttributes.sceneValid.value   = true;

      return getExtensionFieldInfo(scene);
    }
    else
      return ZCL_INSUFFICIENT_SPACE_STATUS;
  }

  return ZCL_INVALID_FIELD_STATUS;
}

/**************************************************************************//**
\brief gets the scene extensions info from other clusterson teh devices

\param[in] scene - Scene to be updated
******************************************************************************/
ZCL_Status_t getExtensionFieldInfo(Scene_t* scene)
{
  ZCL_Cluster_t *cluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, THERMOSTAT_CLUSTER_ID, ZCL_CLUSTER_SIDE_SERVER);

  if (cluster)
  {
    scene->occupiedCoolingSetpoint = thThermostatClusterServerAttributes.occupiedCoolingSetpoint.value;
    scene->occupiedHeatingSetpoint = thThermostatClusterServerAttributes.occupiedHeatingSetpoint.value;
    scene->systemMode              = thThermostatClusterServerAttributes.systemMode.value;

    return ZCL_SUCCESS_STATUS;
  }
  else
    return ZCL_INVALID_FIELD_STATUS;
}

/**************************************************************************//**
\brief sets attributes of thermostat cluster

\param[in] scene - Scene to be updated
******************************************************************************/
void thermostatClusterSetExtensionField(Scene_t* scene)
{
  thThermostatClusterServerAttributes.occupiedCoolingSetpoint.value = scene->occupiedCoolingSetpoint;
  thThermostatClusterServerAttributes.occupiedHeatingSetpoint.value = scene->occupiedHeatingSetpoint;
  thThermostatClusterServerAttributes.systemMode.value = scene->systemMode;
}
#endif // APP_DEVICE_TYPE_THERMOSTAT
//eof thScenesCluster.c
