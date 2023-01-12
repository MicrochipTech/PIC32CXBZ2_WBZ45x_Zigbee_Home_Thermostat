/*******************************************************************************
  Thermostat Alarm cluster Source File

  Company:
    Microchip Technology Inc.

  File Name:
    thAlarmsCluster.c

  Summary:
    This file contains the Thermostat Alarm cluster interface.

  Description:
    This file contains the Thermostat Alarm cluster interface.
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
#include <z3device/thermostat/include/thAlarmsCluster.h>
#include <z3device/clusters/include/haClusters.h>
#include <app_zigbee/zigbee_console/console.h>
#include <zcl/include/zclCommandManager.h>
#include <pds/include/wlPdsMemIds.h>
#include <z3device/common/include/z3Device.h>
#include <app_zigbee/app_zigbee_handler.h>

/******************************************************************************
                    Definition(s) section
******************************************************************************/


/******************************************************************************
                    Prototypes section
******************************************************************************/
static ZCL_Status_t resetAlarmInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_ResetAlarm_t *payload);
static ZCL_Status_t resetAllAlarmsInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
static ZCL_Status_t getAlarmInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
static ZCL_Status_t resetAlarmLogInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);

static void getAlarmIndRespCB(ZCL_Notify_t *ntfy);
static void removeAlarm(uint8_t alarmIndex);

/******************************************************************************
                    Global variables section
******************************************************************************/
ZCL_AlarmsClusterServerAttributes_t thAlarmsClusterServerAttributes =
{
  ZCL_DEFINE_ALARMS_CLUSTER_SERVER_ATTRIBUTES()
};

ZCL_AlarmsClusterClientAttributes_t thAlarmsClusterClientAttributes =
{
  ZCL_DEFINE_ALARMS_CLUSTER_CLIENT_ATTRIBUTES()
};

PROGMEM_DECLARE (ZCL_AlarmsClusterServerCommands_t thAlarmsClusterServerCommands)=
{
  ZCL_DEFINE_ALARMS_CLUSTER_SERVER_COMMANDS(resetAlarmInd, resetAllAlarmsInd, getAlarmInd, resetAlarmLogInd)
};

/*Device Alarm Log*/
ZCL_AlarmEntry_t thAlarmTable[ALARM_TABLE_SIZE];

/******************************************************************************
                    Static variables section
******************************************************************************/

/******************************************************************************
                    Implementation section
******************************************************************************/
/**************************************************************************//**
\brief Initializes Identify cluster
******************************************************************************/
void thAlarmsClusterInit(void)
{
  ZCL_Cluster_t *cluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, ALARMS_CLUSTER_ID, ZCL_CLUSTER_SIDE_SERVER);

  if (cluster)
  {
    thAlarmsClusterServerAttributes.alarmCount.value = 0;
    thAlarmsClusterServerAttributes.clusterVersion.value = ALARM_CLUSTER_VERSION;
    cluster->ZCL_DefaultRespInd = ZCL_CommandZclDefaultResp;
  }
}
/**************************************************************************//**
\brief Callback on receiving resetAllAlarm command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t resetAllAlarmsInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  BcZCLActionReq_t resetAllAlarmActionReq;
  ZCL_ResetAlarmNotification_t resetAllAlarmsNotification;
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_RESET_ALL_ALARMS;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;

  LOG_STRING(resetAllAlarmStr, "<-resetAllAlarm\r\n");

  resetAllAlarmsNotification.commandId = ZCL_ALARMS_CLUSTER_SERVER_RESET_ALL_ALARMS_COMMAND_ID;
  resetAllAlarmsNotification.clusterIdentifier = 0x00;
  resetAllAlarmsNotification.alarmCode = 0x00;

  // Raise event to the subscribers. They will reset all their alarms
  // If alarm is still active, they may initiate notification
  resetAllAlarmActionReq.action = ZCL_ACTION_RESET_ALARM_REQUEST;
  resetAllAlarmActionReq.context = (const void *)&resetAllAlarmsNotification;
  SYS_PostEvent(BC_ZCL_EVENT_ACTION_REQUEST, (SYS_EventData_t)&resetAllAlarmActionReq);


  APP_Zigbee_Handler(event);
  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
\brief Callback on receiving resetAlarm command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t resetAlarmInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_ResetAlarm_t *payload)
{
  BcZCLActionReq_t resetAlarmActionReq;
  ZCL_ResetAlarmNotification_t resetAlarmNotification;

  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_RESET_ALARM;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;
  LOG_STRING(resetAlarmStr, "<-resetAlarm clusterId = 0x%x alarmCode = %d\r\n");

  resetAlarmNotification.commandId = ZCL_ALARMS_CLUSTER_SERVER_RESET_ALARM_COMMAND_ID;
  resetAlarmNotification.clusterIdentifier = payload->clusterIdentifier;
  resetAlarmNotification.alarmCode = payload->alarmCode;

  // Raise event to the subscribers. They will reset the alarm
  // If alarm is still active, they may initiate notification
  resetAlarmActionReq.action = ZCL_ACTION_RESET_ALARM_REQUEST;
  resetAlarmActionReq.context = (const void *)&resetAlarmNotification;

  SYS_PostEvent(BC_ZCL_EVENT_ACTION_REQUEST, (SYS_EventData_t)&resetAlarmActionReq);

  APP_Zigbee_Handler(event);
  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
\brief Callback on receiving getAlarm command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t getAlarmInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  ZCL_Request_t *req;
  ZCL_getAlarmResponse_t *getAlarmResponse;
  uint8_t reqLength;

  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_GET_ALARM;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;

  LOG_STRING(resetGetAlarmStr, "<-getAlarm\r\n");
  APP_Zigbee_Handler(event);
  if (addressing->nonUnicast)
    return ZCL_SUCCESS_STATUS;

  if (!(req = getFreeCommand()))
    return ZCL_INSUFFICIENT_SPACE_STATUS;

  getAlarmResponse = (ZCL_getAlarmResponse_t *)req->requestPayload;

  if (thAlarmsClusterServerAttributes.alarmCount.value)
  {
    reqLength = sizeof(ZCL_getAlarmResponse_t);
    getAlarmResponse->status = ZCL_SUCCESS_STATUS;
    getAlarmResponse->alarmCode = thAlarmTable[0].alarmCode;
    getAlarmResponse->clusterIdentifier = thAlarmTable[0].clusterIdentifier;
    getAlarmResponse->timeStamp = thAlarmTable[0].timeStamp;

  }
  else
  {
    reqLength = 1;
    getAlarmResponse->status = ZCL_NOT_FOUND_STATUS;
  }
  req->ZCL_Notify = getAlarmIndRespCB;
  fillCommandRequest(req, ZCL_ALARMS_CLUSTER_CLIENT_GET_ALARM_RESPONSE_COMMAND_ID, reqLength, APP_ENDPOINT_THERMOSTAT);
  fillDstAddressingViaSourceAddressing(&req->dstAddressing, addressing, ZCL_CLUSTER_SIDE_CLIENT);
  ZCL_CommandManagerSendCommand(req);

  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
\brief Callback on receiving resetAlarmLog command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
ZCL_Status_t resetAlarmLogInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_RESET_ALARM_LOG;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;

  LOG_STRING(resetAlarmLogStr, "<-resetAlarmLog\r\n");

  thAlarmsClusterServerAttributes.alarmCount.value = 0;

  APP_Zigbee_Handler(event);
  return ZCL_SUCCESS_STATUS;
}


static void getAlarmIndRespCB(ZCL_Notify_t *ntfy)
{
  if(ZCL_SUCCESS_STATUS == ntfy->status)
  {
    /*Remove corresponding entry for the Alarm table*/
    removeAlarm(0);
  }
}
/**************************************************************************//**
\brief adding entry to alarm table

\param[in] alarmCode - alarm code;
\param[in] clusterId - cluster identifier;
******************************************************************************/
void addAlarmEntry(uint8_t alarmCode, ClusterId_t clusterId)
{
  if (thAlarmsClusterServerAttributes.alarmCount.value >= ALARM_TABLE_SIZE)
    removeAlarm(0);

  thAlarmTable[thAlarmsClusterServerAttributes.alarmCount.value].alarmCode = alarmCode;
  thAlarmTable[thAlarmsClusterServerAttributes.alarmCount.value].clusterIdentifier = clusterId;
  thAlarmTable[thAlarmsClusterServerAttributes.alarmCount.value].timeStamp = HAL_GetSystemTime() / 1000; // this needs to be linked with time cluster UTC
  thAlarmsClusterServerAttributes.alarmCount.value++;
}

/**************************************************************************//**
\brief remove the first entry(earliest timestamp)from alarm table

******************************************************************************/
static void removeAlarm(uint8_t alarmIndex)
{
  /*Remove the earliest entry and move up the remaining entries*/
  for (uint8_t i = alarmIndex; i < (thAlarmsClusterServerAttributes.alarmCount.value-1); i++)
  {
    memcpy(&thAlarmTable[i], &thAlarmTable[i+1], sizeof(ZCL_AlarmEntry_t));
  }
  // Decrement the alarm count by 1
  thAlarmsClusterServerAttributes.alarmCount.value--;
}

/**************************************************************************//**
\brief remove the id and cluster from the alarm entry

******************************************************************************/
void removeAlarmID(uint8_t alarmCode, ClusterId_t clusterId)
{
  for (uint8_t index = 0; index < thAlarmsClusterServerAttributes.alarmCount.value; ++index)
  {
    if (thAlarmTable[index].alarmCode == alarmCode && thAlarmTable[index].clusterIdentifier == clusterId)
    {
      /*Alarm code and Cluster matches,  move up the remaining entries*/
      removeAlarm(index);
      break;
    }
  }
}

#endif // APP_DEVICE_TYPE_THERMOSTAT

// eof thAlarmsCluster.c
