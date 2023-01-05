/*******************************************************************************
  Thermostat Groups server cluster Source File

  Company:
    Microchip Technology Inc.

  File Name:
    thGroupsCluster.c

  Summary:
    This file contains the Thermostat Groups server cluster interface.

  Description:
    This file contains the Thermostat Groups server cluster interface.
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
#include <z3device/thermostat/include/thGroupsCluster.h>
#include <z3device/thermostat/include/thIdentifyCluster.h>
#include <z3device/thermostat/include/thScenesCluster.h>
#include <z3device/clusters/include/haClusters.h>
#include <app_zigbee/zigbee_console/console.h>
#include <zcl/include/zclCommandManager.h>
#include <pds/include/wlPdsMemIds.h>
#include <zcl/clusters/include/groupsCluster.h>
#include <z3device/common/include/appConsts.h>
#include <app_zigbee/app_zigbee_handler.h>

/******************************************************************************
                    Prototypes section
******************************************************************************/
static ZCL_Status_t addGroupInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_AddGroup_t *payload);
static ZCL_Status_t viewGroupInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_ViewGroup_t *payload);
static ZCL_Status_t getGroupMembershipInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_GetGroupMembership_t *payload);
static ZCL_Status_t removeGroupInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_RemoveGroup_t *payload);
static ZCL_Status_t removeAllGroupsInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, void *payload);
static ZCL_Status_t addGroupIfidentifyingInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_AddGroupIfIdentifying_t * payload);
static ZCL_Status_t addGroupIfIdentifying(uint16_t group);

static ZCL_Status_t addGroupResponseInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_AddGroupResponse_t *payload);
static ZCL_Status_t viewGroupResponseInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_ViewGroupResponse_t *payload);
static ZCL_Status_t getGroupMembershipResponseInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_GetGroupMembershipResponse_t *payload);
static ZCL_Status_t removeGroupResponseInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_RemoveGroupResponse_t *payload);

/******************************************************************************
                    Global variables section
******************************************************************************/
ZCL_GroupsClusterServerAttributes_t thGroupsClusterServerAttributes =
{
  ZCL_DEFINE_GROUPS_CLUSTER_SERVER_ATTRIBUTES()
};

ZCL_GroupsClusterClientAttributes_t thGroupsClusterClientAttributes =
{
  ZCL_DEFINE_GROUPS_CLUSTER_CLIENT_ATTRIBUTES()
};

PROGMEM_DECLARE (ZCL_GroupsClusterCommands_t thGroupsCommands) =
{
  ZCL_DEFINE_GROUPS_CLUSTER_COMMANDS(addGroupInd, viewGroupInd, getGroupMembershipInd,
                                     removeGroupInd, removeAllGroupsInd, addGroupIfidentifyingInd,
                                     addGroupResponseInd, viewGroupResponseInd, getGroupMembershipResponseInd, removeGroupResponseInd)
};

extern Scene_t thermostatSceneTable[];
extern QueueDescriptor_t groupsSubscribers;
/******************************************************************************
                    Implementation section
******************************************************************************/
/**************************************************************************//**
\brief Initializes Groups cluster
******************************************************************************/
void thGroupsClusterInit(void)
{
  ZCL_Cluster_t *cluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, GROUPS_CLUSTER_ID, ZCL_CLUSTER_SIDE_CLIENT);
  thGroupsClusterServerAttributes.nameSupport.value = 0;
  thGroupsClusterClientAttributes.clusterVersion.value = GROUPS_CLUSTER_VERSION;
  thGroupsClusterServerAttributes.clusterVersion.value = GROUPS_CLUSTER_VERSION;
  if(cluster)
    cluster->ZCL_DefaultRespInd = ZCL_CommandZclDefaultResp;
}

/**************************************************************************//**
\brief Callback on receiving Add Group command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t addGroupInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_AddGroup_t *payload)
{
  ZCL_Request_t *req;
  uint8_t status;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_ADD_GROUP;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;
  APP_Zigbee_Handler(event);
  status = addGroup(payload->groupId, APP_ENDPOINT_THERMOSTAT);

  // If received via multicast or broadcast service no response shall be given
  if (addressing->nonUnicast)
    return ZCL_SUCCESS_STATUS;

  if (!(req = getFreeCommand()))
    return ZCL_INSUFFICIENT_SPACE_STATUS;

  fillCommandRequest(req, ZCL_GROUPS_CLUSTER_ADD_GROUP_RESPONSE_COMMAND_ID,
                       sizeof(ZCL_AddGroupResponse_t), APP_ENDPOINT_THERMOSTAT);
  fillAddGroupResponsePayload((ZCL_AddGroupResponse_t *)req->requestPayload, payload->groupId, status);
  fillDstAddressingViaSourceAddressing(&req->dstAddressing, addressing, ZCL_CLUSTER_SIDE_CLIENT);
  ZCL_CommandManagerSendCommand(req);

  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
\brief Callback on receiving View Group command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t viewGroupInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_ViewGroup_t *payload)
{
  ZCL_Request_t *req;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_VIEW_GROUP;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;
  APP_Zigbee_Handler(event);
  // If received via multicast or broadcast service no response shall be given
  if (addressing->nonUnicast)
    return ZCL_SUCCESS_STATUS;

  if (!(req = getFreeCommand()))
    return ZCL_INSUFFICIENT_SPACE_STATUS;

  fillCommandRequest(req, ZCL_GROUPS_CLUSTER_VIEW_GROUP_RESPONSE_COMMAND_ID,
                       sizeof(ZCL_ViewGroupResponse_t) ,APP_ENDPOINT_THERMOSTAT);
  fillViewGroupResponsePayload((ZCL_ViewGroupResponse_t *)req->requestPayload, payload->groupId, APP_ENDPOINT_THERMOSTAT);
  fillDstAddressingViaSourceAddressing(&req->dstAddressing, addressing, ZCL_CLUSTER_SIDE_CLIENT);
  ZCL_CommandManagerSendCommand(req);

  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
\brief Callback on receiving Get Group Membership command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t getGroupMembershipInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_GetGroupMembership_t *payload)
{
  ZCL_Request_t *req;
  uint8_t size;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_GET_GROUP_MEMBERSHIP;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;
  APP_Zigbee_Handler(event);
  // If received via multicast or broadcast service no response shall be given
  if (addressing->nonUnicast)
    return ZCL_SUCCESS_STATUS;

  if (!(req = getFreeCommand()))
    return ZCL_INSUFFICIENT_SPACE_STATUS;

  size = fillGetGroupMembershipRespPayload((ZCL_GetGroupMembershipResponse_t *)req->requestPayload, payload, APP_ENDPOINT_THERMOSTAT);
  fillCommandRequest(req, ZCL_GROUPS_CLUSTER_GET_GROUP_MEMBERSHIP_RESPONSE_COMMAND_ID, size, APP_ENDPOINT_THERMOSTAT);
  fillDstAddressingViaSourceAddressing(&req->dstAddressing, addressing, ZCL_CLUSTER_SIDE_CLIENT);
  ZCL_CommandManagerSendCommand(req);

  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
\brief Callback on receiving Remove Group command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t removeGroupInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_RemoveGroup_t *payload)
{
  ZCL_Request_t *req;
  uint8_t status;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_REMOVE_GROUP;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;
  APP_Zigbee_Handler(event);
   status = removeGroup(payload->groupId, APP_ENDPOINT_THERMOSTAT, thermostatSceneTable);

  // If received via multicast or broadcast service no response shall be given
  if (addressing->nonUnicast)
    return ZCL_SUCCESS_STATUS;

  if (!(req = getFreeCommand()))
    return ZCL_INSUFFICIENT_SPACE_STATUS;

  fillCommandRequest(req, ZCL_GROUPS_CLUSTER_REMOVE_GROUP_RESPONSE_COMMAND_ID,
                       sizeof(ZCL_RemoveGroupResponse_t), APP_ENDPOINT_THERMOSTAT);
  fillRemoveGroupResponsePayload((ZCL_RemoveGroupResponse_t *)req->requestPayload, payload->groupId, status);
  fillDstAddressingViaSourceAddressing(&req->dstAddressing, addressing, ZCL_CLUSTER_SIDE_CLIENT);
  ZCL_CommandManagerSendCommand(req);

  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
\brief Callback on receiving Remove All Groups command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t removeAllGroupsInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, void *payload)
{
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_REMOVE_ALL_GROUP;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;

  removeAllGroups(APP_ENDPOINT_THERMOSTAT, thermostatSceneTable);

  APP_Zigbee_Handler(event);
  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
\brief Callback on receiving Add Group If Identifying command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t addGroupIfidentifyingInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_AddGroupIfIdentifying_t * payload)
{
  ZCL_Status_t  status;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_ADD_GROUP_IF_IDENTIFYING;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;

  status = addGroupIfIdentifying(payload->groupId);

  APP_Zigbee_Handler(event);
  return status;
}

/**************************************************************************//**
\brief Adds group to group table if device is in identifying state

\param[in] group - group id
******************************************************************************/
static ZCL_Status_t addGroupIfIdentifying(uint16_t group)
{
  ZCL_Status_t  status;
  if (thIdentifyIsIdentifying())
  {
    status = addGroup(group, APP_ENDPOINT_THERMOSTAT);
    return status;
  }
  // If received via multicast or broadcast service no response shall be given
  else
    return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
\brief Callback on receiving Add Group Response command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t addGroupResponseInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_AddGroupResponse_t *payload)
{
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_ADD_GROUP_RESPONSE;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;

  RAISE_CALLBACKS_TO_GROUPS_SUBSCIBERS(groupsSubscribers, addGroupResponse);

  APP_Zigbee_Handler(event);
  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
\brief Callback on receiving View Group Response command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t viewGroupResponseInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_ViewGroupResponse_t *payload)
{
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_VIEW_GROUP_RESPONSE;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;

  APP_Zigbee_Handler(event);
  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
\brief Callback on receiving Group Membership Response command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t getGroupMembershipResponseInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_GetGroupMembershipResponse_t *payload)
{
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_GET_GROUP_MEMBERSHIP_RESPONSE;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;

  APP_Zigbee_Handler(event);
  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
\brief Callback on receiving Remove Group Response command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t removeGroupResponseInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_RemoveGroupResponse_t *payload)
{
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_REMOVE_GROUP_RESPONSE;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;

  APP_Zigbee_Handler(event);
  return ZCL_SUCCESS_STATUS;
}
#endif // APP_DEVICE_TYPE_THERMOSTAT
// eof thGroupsCluster.c
