/*******************************************************************************
  Application Zigbee Handler Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_zigbee_handler.c

  Summary:
    This file contains the Application Zigbee functions for this project.

  Description:
    This file contains the Application Zigbee functions for this project.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <app_zigbee/app_zigbee_handler.h>
#include <app_zigbee/zigbee_console/console.h>
#include <zcl/include/zclAlarmsCluster.h>
#include <zcl/include/zclZllIdentifyCluster.h>
#include <zcl/include/zclZllGroupsCluster.h>
#include <zcl/include/zclZllScenesCluster.h>
#include <zcl/include/zclFanControlCluster.h>
#include <zcl/include/zclThermostatCluster.h>
#include <zcl/include/zclTemperatureMeasurementCluster.h>
#include <zcl/include/zclTimeCluster.h>
#include <zcl/include/zclThermostatUiConfCluster.h>
#include <zcl/include/zcloccupancysensingcluster.h>
#include <z3device/thermostat/include/thOccupancySensingCluster.h>
#include <z3device/thermostat/include/thThermostatCluster.h>
#include <z3device/thermostat/include/thTemperatureMeasurementCluster.h>
#include <z3device/thermostat/include/thThermostatUiConfCluster.h>
#include <z3device/thermostat/include/thHumidityMeasurementCluster.h>
#include <z3device/thermostat/include/thAlarmsCluster.h>
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************
extern void local_temp_update(int16_t temp_value);
void BSP_Event_Handler(APP_Zigbee_Event_t event);
void Cluster_Event_Handler(APP_Zigbee_Event_t event);
void Zigbee_Event_Handler(APP_Zigbee_Event_t event);

#define MIN_COLOR_LEVEL           0
#define MAX_COLOR_LEVEL           0xfeff

#define MIN_SATURATION_LEVEL      0
#define MAX_SATURATION_LEVEL      0xfe
// *****************************************************************************
// *****************************************************************************
// Section: Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Zigbee_Handler(APP_Zigbee_Event_t event)

  Remarks:
    See prototype in app_zigbee_handler.h.
******************************************************************************/

void APP_Zigbee_Handler(APP_Zigbee_Event_t event)
{
    switch(event.eventGroup)
    {
        case EVENT_BSP:
        {
            /* BSP Events Dispatcher */
            BSP_Event_Handler(event);
        }
        break;

        case EVENT_ZIGBEE:
        {
            /* Zigbee Events Dispatcher */
            Zigbee_Event_Handler(event);
        }
        break;

        case EVENT_CLUSTER:
        {
            /* Cluster Events Dispatcher */
            Cluster_Event_Handler(event);
        }
        break;

        default:
        break;
    }
}

/*******************************************************************************
  Function:
    void BSP_Event_Handler(APP_Zigbee_Event_t event)

  Remarks:
    See prototype in app_zigbee_handler.h.
******************************************************************************/

void BSP_Event_Handler(APP_Zigbee_Event_t event)
{
    // User to handle  board Support package events
    switch(event.eventId)
    {
        case CMD_LED_OPEN:
        {
            /* Init/Open LEDs */
            //appSnprintf("Init /Open LEDs\r\n");
        }
        break;

        case CMD_LED_ON:
        {
            /* Turn on the LED */
            //Access - > event.eventData.value;
            //appSnprintf("On\r\n");
        }
        break;

        case CMD_LED_OFF:
        {
            /* Turn off the LED */
            //Access - > event.eventData.value;
            //appSnprintf("Off\r\n");
        }
        break;

        case CMD_LED_TOGGLE:
        {
            /* Toggle the LED */
            //Access - > event.eventData.value;
            //appSnprintf("Toggle\r\n");
        }
        break;
        case CMD_LED_BRIGHTNESS:
        {
            /* Set the given LED brightness */
            //Access - > event.eventData.value;
            //appSnprintf("Led Brightness \r\n");
        }
        break;

        case CMD_LED_COLOR_HS:
        {
            /* LED Hue , Saturation */
            //Access - > event.eventData.colorHS.hue;
            //Access - > event.eventData.colorHS.saturation;
            //appSnprintf("LED Hue , Saturation \r\n");
        }
        break;

        case CMD_LED_COLOR_XY:
        {
            /* Set the LED Color X Y */
            //Access - > event.eventData.colorXY.x;
            //Access - > event.eventData.colorXY.y;
            //appSnprintf("LED X,Y Color \r\n");
        }
        break;

        case CMD_BUTTON_OPEN:
        {
            /* Button Init */
            //appSnprintf("Button Init/Open \r\n");
        }
        break;

        case CMD_BUTTON_READ:
        {
            /* Button Press */
            //Access - > event.eventData.state;
            //appSnprintf("Button Read \r\n");
        }
        break;

        case CMD_SENSOR_OPEN:
        {
            /* Sensor Data */
            //appSnprintf("Sensor Open /Init Event \r\n");
        }        
        break;

        case CMD_SENSOR_READ:
        {
            /* Sensor Data */
            //Access - > event.eventData.data;
            //appSnprintf("Sensor Read Event \r\n");
        }        
        break;

        default:
        break;
    }
}

/*******************************************************************************
  Function:
    void Zigbee_Event_Handler(APP_Zigbee_Event_t event)

  Remarks:
    See prototype in app_zigbee_handler.h.
******************************************************************************/

void Zigbee_Event_Handler(APP_Zigbee_Event_t event)
{
    // User to handle all zigbee stack events  
    switch(event.eventId)
    {
        case EVENT_NETWORK_ESTABLISHED:
        {
            appSnprintf("Network Established\r\n");
            LCD_PRINT(0,9,"NTW EST");
        }
        break;
        case EVENT_DISCONNECTED:
        {
            appSnprintf("Disconnected from the Network\r\n");
            LCD_PRINT(0,9,"DISCONNECTED");
        }
        break;
        case EVENT_COMMISSIONING_STARTED:
        {
            appSnprintf("Commissioning Procedure Started - 180 Seconds \r\n");
            LCD_PRINT(0,9,"Comm Started");
            appSnprintf("Commissioning Sequence: ");
            appSnprintf("Formation->");
            appSnprintf("Steering->");
            appSnprintf("Find & Bind->");
            LCD_PRINT(0,9,"Find & Bind");
            appSnprintf("\r\n");
        }
        break;

        case EVENT_COMMISSIONING_COMPLETE:
        {
            appSnprintf("Commissioning Procedure Complete \r\n");
        }
        break;
        case EVENT_COMMISSIONING_FORMATION_COMPLETE:
        {
            appSnprintf("Nwk Formation: ");
            if(event.eventData.value == BDB_COMMISSIONING_SUCCESS)
                appSnprintf("Success\r\n");
            else //BDB_COMMISSIONING_FORMATION_FAILURE
                appSnprintf("Failed\r\n");
        }
        break;
        case EVENT_COMMISSIONING_STEERING_COMPLETE:
        {
            appSnprintf("Steering: ");
            if(event.eventData.value == BDB_COMMISSIONING_NO_NETWORK)
                appSnprintf("No networks found to join\r\n");
            else if(event.eventData.value == BDB_COMMISSIONING_SUCCESS)
                appSnprintf("Success\r\n");
            else
                appSnprintf("Failed\r\n");
        }
        break;
        case EVENT_COMMISSIONING_TOUCHLINK_COMPLETE:
        {
            appSnprintf("Touchlink: Attempt: ");
            if(event.eventData.value == BDB_COMMISSIONING_NO_SCAN_RESPONSE)
                appSnprintf("No scan response\r\n");
            else if(event.eventData.value == BDB_COMMISSIONING_SUCCESS)
                appSnprintf("Success\r\n");
            else
                appSnprintf("Failed\r\n");
        }
        break;
        case EVENT_COMMISSIONING_FINDBIND_COMPLETE:
        {
            appSnprintf("Finding & Binding: ");
            if(event.eventData.value == BDB_COMMISSIONING_NO_IDENTIFY_QUERY_RESPONSE)
              appSnprintf("No identify Query Response\r\n");
            else if(event.eventData.value == BDB_COMMISSIONING_BINDING_TABLE_FULL)
              appSnprintf("Binding table full\r\n");
            else if(event.eventData.value == BDB_COMMISSIONING_SUCCESS)
              appSnprintf("Success\r\n");
            else
              appSnprintf("Failed\r\n");
        }
        break;
        case EVENT_COMMISSIONING_FAILURE:
        {
          switch(event.eventData.value)
          {
              case BDB_COMMISSIONING_NO_NETWORK:
              {
                  //appSnprintf("No network found in search\r\n");
              }
              break;
              case BDB_COMMISSIONING_NOT_SUPPORTED:
              {
                  appSnprintf("Commissioning: One of the BDB commissioning procedure not supported\r\n");
              }
              break;
              case BDB_COMMISSIONING_NO_SCAN_RESPONSE:
              break;
              case BDB_COMMISSIONING_NO_IDENTIFY_QUERY_RESPONSE:
              break;
              default:
              break;
          }
        }
        break;

        case EVENT_STARTED_CENTRALIZED_NETWORK:
        {
            appSnprintf("Started Centralized Network\r\n");
        }
        break;
        case EVENT_STARTED_DISTRIBUTED_NETWORK:
        {
            appSnprintf("Started Distributed Network\r\n");
            LCD_PRINT(0,9,"DIST NTW");
        }
        break;
        case EVENT_JOINED_TO_AN_EXISTING_NETWORK:
        {
            appSnprintf("Network Search: Complete: Joined to a Network \r\n");
            LCD_PRINT(0,9,"Joined Ntw");
            appSnprintf("Joined to: Address 0x%04x  MACID 0x%08x%08x ExtendedPANID 0x%08x%08x\r\n", event.eventData.ParentChildInfo.shortAddress, (uint32_t)(event.eventData.ParentChildInfo.extendedAddress >> 32), (uint32_t)(event.eventData.ParentChildInfo.extendedAddress & 0xFFFFFFFF), (uint32_t)(event.eventData.ParentChildInfo.extendedPanId >> 32), (uint32_t)(event.eventData.ParentChildInfo.extendedPanId & 0xFFFFFFFF));
        }
        break;

        case EVENT_WAKEUP:
        {
            //appSnprintf("Wake up Indication \r\n");
        }
        break;

        case EVENT_LEFT_FROM_NETWORK:
        {
            //appSnprintf("Left from the Network \r\n");
        }
        break;

        case EVENT_CHILD_JOINED:
        {
            appSnprintf("Device joined: Address 0x%04x  MACID 0x%08x%08x ExtendedPANID 0x%08x%08x\r\n", event.eventData.ParentChildInfo.shortAddress, (uint32_t)(event.eventData.ParentChildInfo.extendedAddress >> 32), (uint32_t)(event.eventData.ParentChildInfo.extendedAddress & 0xFFFFFFFF), (uint32_t)(event.eventData.ParentChildInfo.extendedPanId >> 32), (uint32_t)(event.eventData.ParentChildInfo.extendedPanId & 0xFFFFFFFF));
        }
        break;

        case EVENT_CHILD_REMOVED:
        {
            appSnprintf("Child Left\r\n");
        }
        break;

        case EVENT_NWK_UPDATE:
        {
            //appSnprintf("Network Information updated \r\n");
        }
        break;

        case EVENT_RESET_TO_FACTORY_DEFAULTS:
        {
            //appSnprintf("Reset To Factory New\r\n");
        }
        break;

        case EVENT_NWK_ADDRESS_RESPONSE:
        {
            if(event.eventData.ParentChildInfo.status == ZCL_SUCCESS_STATUS)
                appSnprintf( "->NwkAddrResponse, status = %d, address = %04x\r\n" ,event.eventData.ParentChildInfo.status, event.eventData.ParentChildInfo.shortAddress);
            else
                appSnprintf( "->NwkAddrResponse, status = %d \r\n" ,event.eventData.ParentChildInfo.status);
        }
        break;

        case EVENT_IEEE_ADDRESS_RESPONSE:
        {
            if(event.eventData.ParentChildInfo.status == ZCL_SUCCESS_STATUS)
                appSnprintf("->IeeeAddrResponse, status = %d, address = 0x%04x \r\n", event.eventData.ParentChildInfo.status, (uint32_t)(event.eventData.ParentChildInfo.extendedAddress >> 32), (uint32_t)(event.eventData.ParentChildInfo.extendedAddress & 0xFFFFFFFF));
            else
                appSnprintf( "->IeeeAddrResponse, status = %d, address = 0x%04x \r\n", event.eventData.ParentChildInfo.status);
        }
        break;

        case EVENT_SIMPLE_DESCRIPTOR_RESPONSE:
        {
            appSnprintf( "->SimpleDescResponse, status = %d \r\n" ,event.eventData.ParentChildInfo.status);
        }
        break;

        case EVENT_MATCH_DESCRIPTOR_RESPONSE:
        {
            if(event.eventData.ParentChildInfo.status == ZCL_SUCCESS_STATUS)
                appSnprintf( "->MatchDescResponse, status = %d, MatchedEpCount = %d\r\n" ,event.eventData.ParentChildInfo.status, event.eventData.ParentChildInfo.ep);
            else
                appSnprintf( "->MatchDescResponse, status = %d \r\n" ,event.eventData.ParentChildInfo.status);
        }
        break;

        case EVENT_ACTIVE_EP_RESPONSE:
        {
            if(event.eventData.ParentChildInfo.status == ZCL_SUCCESS_STATUS)
                appSnprintf( "->ActiveEpResponse, status = %d, EpCount = %d\r\n" ,event.eventData.ParentChildInfo.status, event.eventData.ParentChildInfo.ep);
            else
                appSnprintf( "->ActiveEpResponse, status = %d \r\n" ,event.eventData.ParentChildInfo.status);
        }
        break;

        case EVENT_NODE_DESCRIPTOR_RESPONSE:
        {
            appSnprintf( "->NodeDescResponse, status = %d \r\n" ,event.eventData.ParentChildInfo.status);
        }
        break;

        case EVENT_LEAVE_RESPONSE:
        {
            appSnprintf( "->LeaveRsp, status = %d \r\n" ,event.eventData.ParentChildInfo.status);
        }
        break;

        case EVENT_MANAGEMENT_BIND_RESPONSE:
        {
            appSnprintf( "MgmtBindRsp %d\r\n", event.eventData.ParentChildInfo.status); 
        }
        break;

        case EVENT_LQI_RESPONSE:
        {
            appSnprintf( "->MgmtLqiRsp, status = %d \r\n" ,event.eventData.ParentChildInfo.status);
        }
        break;

        case EVENT_BIND_RESPONSE:
        {
            appSnprintf( "->BindRsp, status = %d \r\n" ,event.eventData.ParentChildInfo.status);
        }
        break;

        case EVENT_UNBIND_RESPONSE:
        {
            appSnprintf( "->UnBindRsp, status = %d \r\n" ,event.eventData.ParentChildInfo.status);
        }
        break;

        default:
        break;
    }
}

/*******************************************************************************
  Function:
    void Cluster_Event_Handler(APP_Zigbee_Event_t event)

  Remarks:
    See prototype in app_zigbee_handler.h.
******************************************************************************/
APP_Zigbee_Event_t event_test;
void Cluster_Event_Handler(APP_Zigbee_Event_t event)
{
    switch(event.eventId)
    {
        case CMD_ZCL_RESET_TO_FACTORY_DEFAULTS:
        {
            /* Command ZCL ResetToFactoryDefaults */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            //appSnprintf("ZCL ResetToFactoryDefaults\r\n");
        }
        break;
        case CMD_ZCL_IDENTIFY:
        {
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            appSnprintf( "->Identify\r\n");
        }
        break;
        case CMD_ZCL_IDENTIFY_QUERY:
        {
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            //appSnprintf("->IdentifyQuery\r\n");
        }
        break;
        case CMD_ZCL_TRIGGER_EFFECT:
        {
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_TriggerEffect_t *payload = (ZCL_TriggerEffect_t *)event.eventData.zclEventData.payload;
            appSnprintf("->TriggerEffect 0x%x\r\n",  payload->effectIdentifier);
        }
        break;
        case CMD_ZCL_IDENTIFY_QUERY_RESPONSE:
        {
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            //appSnprintf("->IdentifyQueryResponse, addr = 0x%04x, timeout = 0x%04x\r\n", addressing->addr.shortAddress, payload->timeout);
        }
        break;
        case CMD_ZCL_ADD_GROUP:
        {
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_AddGroup_t *payload = (ZCL_AddGroup_t *)event.eventData.zclEventData.payload;
            appSnprintf("addGroupInd(): 0x%04x\r\n", payload->groupId);
        }
        break;
        case CMD_ZCL_VIEW_GROUP:
        {
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_ViewGroup_t *payload = (ZCL_ViewGroup_t *)event.eventData.zclEventData.payload;
            appSnprintf("viewGroupInd(): 0x%04x\r\n", payload->groupId);
        }
        break;
        case CMD_ZCL_GET_GROUP_MEMBERSHIP:
        {
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload; 
            appSnprintf("getGroupMembershipInd()\r\n");
        }
        break;
        case CMD_ZCL_REMOVE_GROUP:
        {
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_RemoveGroup_t *payload = (ZCL_RemoveGroup_t *)event.eventData.zclEventData.payload;
            appSnprintf("removeGroupInd(): 0x%04x\r\n", payload->groupId);
        }
        break;
        case CMD_ZCL_REMOVE_ALL_GROUP:
        {
            //Access - > event.eventData.zclAttributeData.addressing;
            //Access - > event.eventData.zclAttributeData.attributeId;
            //Access - > event.eventData.zclAttributeData.event;
            appSnprintf( "removeAllGroupsInd()\r\n");
        }
        break;
        case CMD_ZCL_ADD_GROUP_IF_IDENTIFYING:
        {
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_AddGroupIfIdentifying_t *payload = (ZCL_AddGroupIfIdentifying_t *)event.eventData.zclEventData.payload;
            appSnprintf("addGroupIfIdentifyingInd(): 0x%04x\r\n", payload->groupId);
        }
        break;
        case CMD_ZCL_ADD_GROUP_RESPONSE:
        {
            //Access - > event.eventData.zclAttributeData.addressing;
            //Access - > event.eventData.zclAttributeData.attributeId;
            //Access - > event.eventData.zclAttributeData.event;
            appSnprintf("addGroupResponseInd()\r\n");
        }
        break;
        case CMD_ZCL_VIEW_GROUP_RESPONSE:
        {
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_ViewGroupResponse_t *payload = (ZCL_ViewGroupResponse_t *)event.eventData.zclEventData.payload;
            appSnprintf("viewGroupResponse(): status = 0x%02x\r\n", payload->status);
            appSnprintf("groupId = 0x%04x\r\n", payload->groupId);
        }
        break;
        case CMD_ZCL_GET_GROUP_MEMBERSHIP_RESPONSE:
        {
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_GetGroupMembershipResponse_t *payload = (ZCL_GetGroupMembershipResponse_t *)event.eventData.zclEventData.payload;
            appSnprintf("getGroupMembershipResponse()\r\n");
            appSnprintf("groupCount = %d\r\n", payload->groupCount);
            for (uint8_t i = 0; i < payload->groupCount; i++)
              appSnprintf("groupId = 0x%04x\r\n", payload->groupList[i]);
        }
        break;
        case CMD_ZCL_REMOVE_GROUP_RESPONSE:
        {
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_RemoveGroupResponse_t *payload = (ZCL_RemoveGroupResponse_t *)event.eventData.zclEventData.payload;
            appSnprintf("removeGroupResponseInd()\r\n");
            appSnprintf("groupId = 0x%04x\r\n", payload->groupId);

        }
        break;
    case CMD_ZCL_RESET_ALL_ALARMS:
        {
            /* Command ZCL OnOffAttributeEventInd */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;            
            appSnprintf("<-resetAllAlarm\r\n");
        }
        break;
    case CMD_ZCL_RESET_ALARM:
        {
            /* Command ZCL OnOffAttributeEventInd */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_ResetAlarm_t *payload = (ZCL_ResetAlarm_t *)event.eventData.zclEventData.payload;        
            appSnprintf("<-resetAlarm clusterId = 0x%x alarmCode = 0x%x\r\n", payload->clusterIdentifier, payload->alarmCode);
        }
        break;
    case CMD_ZCL_GET_ALARM:
        {
            /* Command ZCL OnOffAttributeEventInd */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload; 
            appSnprintf("<-getAlarm\r\n");
        }
        break;
    case CMD_ZCL_RESET_ALARM_LOG:
        {
            /* Command ZCL OnOffAttributeEventInd */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload; 
            appSnprintf("<-resetAlarmLog\r\n");
        }
        break;
        case CMD_ZCL_SETPOINT:
        {
            /* Command ZCL TH HumidityMeasurementReportInd Prints Report Value */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;    
           //appSnprintf("ZCL SetEndpoint Ind");
        }
        break;
        case CMD_ZCL_ADD_SCENE:
        {
            /* ZCL Command add scene received */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_AddScene_t *cmd =((ZCL_AddScene_t*)event.eventData.zclEventData.payload);
            appSnprintf("addSceneInd(): 0x%04x, 0x%02x\r\n", cmd->groupId, cmd->sceneId);
        }
        break;
        case CMD_ZCL_VIEW_SCENE:
        {
            /* ZCL Command view scene received */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_ViewScene_t *cmd = ((ZCL_ViewScene_t*)event.eventData.zclEventData.payload);
            appSnprintf("viewSceneInd(): 0x%04x, 0x%02x\r\n", cmd->groupId, cmd->sceneId);
        }
        break;
        case CMD_ZCL_ENHANCED_ADD_SCENE:
        {
            /* ZCL Command enhanced add scene received */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_EnhancedAddScene_t *cmd = ((ZCL_EnhancedAddScene_t*)event.eventData.zclEventData.payload);
            appSnprintf("enhancedAddSceneInd(): 0x%04x, 0x%02x\r\n", cmd->groupId, cmd->sceneId);
        }
        break;
        case CMD_ZCL_ENHANCED_VIEW_SCENE:
        {
            /* ZCL Command enhanced view scene received */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_EnhancedViewScene_t *cmd = ((ZCL_EnhancedViewScene_t*)event.eventData.zclEventData.payload);
            appSnprintf("enhancedViewSceneInd(): 0x%04x, 0x%02x\r\n", cmd->groupId, cmd->sceneId);
        }
        break;
        case CMD_ZCL_REMOVE_SCENE:
        {
            /* ZCL Command remove scene received */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_RemoveScene_t *cmd = ((ZCL_RemoveScene_t*)event.eventData.zclEventData.payload);
            appSnprintf("removeSceneInd(): 0x%04x, 0x%02x\r\n", cmd->groupId, cmd->sceneId);
        }
        break;
        case CMD_ZCL_REMOVE_ALL_SCENES:
        {
            /* ZCL Command remove all scenes received */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_RemoveAllScenes_t *cmd = ((ZCL_RemoveAllScenes_t*)event.eventData.zclEventData.payload);
            appSnprintf("removeAllScenesInd(): 0x%04x\r\n", cmd->groupId);
        }
        break;
        case CMD_ZCL_STORE_SCENE:
        {
            /* ZCL Command store scene received */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_StoreScene_t *cmd = ((ZCL_StoreScene_t*)event.eventData.zclEventData.payload);
            appSnprintf("storeSceneInd(): 0x%04x, 0x%x\r\n", cmd->groupId, cmd->sceneId);
        }
        break;
        case CMD_ZCL_RECALL_SCENE:
        {
            /* ZCL Command recall scene received */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_RecallScene_t *cmd = ((ZCL_RecallScene_t *)event.eventData.zclEventData.payload);
            appSnprintf("recallSceneInd(): 0x%04x, 0x%x\r\n", cmd->groupId, cmd->sceneId);
        }
        break;
        case CMD_ZCL_GET_SCENE_MEMBERSHIP:
        {
            /* ZCL Command get scene membership received */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_GetSceneMembership_t *cmd = ((ZCL_GetSceneMembership_t *)event.eventData.zclEventData.payload);
            appSnprintf("getSceneMembershipInd(): 0x%04x\r\n", cmd->groupId);
        }
        break;
        case CMD_ZCL_COPY_SCENE:
        {
            /* ZCL Command get scene membership received */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;
            ZCL_GetSceneMembership_t *cmd = ((ZCL_GetSceneMembership_t *)event.eventData.zclEventData.payload);
            appSnprintf("copySceneInd()\r\n");
        }
        break;
        case CMD_ZCL_ATTR_IDENTIFY:
        {
            /* Command ZCL IdentifyAttributeEventInd */ 
            //Access - > event.eventData.zclAttributeData.addressing;
            //Access - > event.eventData.zclAttributeData.attributeId;
            //Access - > event.eventData.zclAttributeData.event;
            //appSnprintf("ZCL IdentifyAttributeEventInd\r\n");
        }
        break;
        case CMD_ZCL_ATTR_OCCUPANCY:
        {
            /* Command ZCL OccupancyAttributeEventInd */
            //Access - > event.eventData.zclAttributeData.addressing;
            //Access - > event.eventData.zclAttributeData.attributeId;
            //Access - > event.eventData.zclAttributeData.event;
            ZCL_AttributeEvent_t eventzcl = event.eventData.zclAttributeData.event;
            ZCL_AttributeId_t attributeId = event.eventData.zclAttributeData.attributeId;
            if ((ZCL_CONFIGURE_DEFAULT_ATTRIBUTE_REPORTING_EVENT == eventzcl) && \
                (ZCL_OCCUPANCY_SENSING_CLUSTER_OCCUPANCY_SERVER_ATTRIBUTE_ID == attributeId))
            {
                thOccupancySensingClusterServerAttributes.occupancy.minReportInterval = OCCUPANCY_SENSING_VAL_MIN_REPORT_PERIOD;
                thOccupancySensingClusterServerAttributes.occupancy.maxReportInterval = OCCUPANCY_SENSING_VAL_MAX_REPORT_PERIOD;
                thOccupancySensingClusterServerAttributes.occupancy.reportableChange = 0;     
            }
        }
        break;
        case CMD_ZCL_ATTR_HUMIDITY_MEASUREMENT:
        {
            /* Command ZCL CustomHumidityMeasurementReportInd Prints Report Value */
            //Access - > event.zclAttributeData.zclEventData.addressing;
            //Access - > event.zclAttributeData.zclEventData.attributeId;
            //Access - > event.zclAttributeData.zclEventData.event;           
            //appSnprintf("ZCL HS Humidity Measurement Attr Ind\r\n");
            ZCL_AttributeEvent_t eventzcl = event.eventData.zclAttributeData.event;
            ZCL_AttributeId_t attributeId = event.eventData.zclAttributeData.attributeId;
            if ((ZCL_CONFIGURE_DEFAULT_ATTRIBUTE_REPORTING_EVENT == eventzcl) && \
                (ZCL_HUMIDITY_MEASUREMENT_CLUSTER_SERVER_MEASURED_VALUE_ATTRIBUTE_ID == attributeId))
            {
              thHumidityMeasurementClusterServerAttributes.measuredValue.minReportInterval = HUMIDITY_MEASUREMENT_VAL_MIN_REPORT_PERIOD;
              thHumidityMeasurementClusterServerAttributes.measuredValue.maxReportInterval = HUMIDITY_MEASUREMENT_VAL_MAX_REPORT_PERIOD;
              thHumidityMeasurementClusterServerAttributes.measuredValue.reportableChange  = 0;
            }
        }
        break;
        case CMD_ZCL_ATTR_TIME:
        {
            /* Command ZCL Time attribute indication */         
            if(event.eventData.zclAttributeData.attributeId == ZCL_TIME_CLUSTER_SERVER_TIME_ATTRIBUTE_ID && 
               event.eventData.zclAttributeData.event == ZCL_WRITE_ATTRIBUTE_EVENT)
            {
	          appSnprintf("Standard and local time updated by client\r\n");
            }
        }
        break;
        case CMD_ZCL_ATTR_THERMOSTAT_UI_CONF:
        {
            /* Command ZCL Thermostat UI conf attribute indication*/
            if (ZCL_WRITE_ATTRIBUTE_EVENT == event.eventData.zclAttributeData.event)
            {
              switch(event.eventData.zclAttributeData.attributeId)
              {
                case ZCL_THERMOSTAT_UI_CONF_CLUSTER_TEMPERATURE_DISPLAY_MODE_SERVER_ATTRIBUTE_ID:
                {
                  if(thThermostatUiConfClusterServerAttributes.temperatureDisplayMode.value == ZCL_TEMPERATURE_IN_CELSIUS)
                  {          
                    appSnprintf(" Temp in Celcius = %d\r\n",thThermostatClusterServerAttributes.localTemperature.value);           
                  }
                  else
                  {
                    appSnprintf(" Temp in Fahr = %d\r\n",thTranslateZclCelciusTemptoFahr(thThermostatClusterServerAttributes.localTemperature.value));
                  }
                }
                break;
                case ZCL_THERMOSTAT_UI_CONF_CLUSTER_KEYPAD_LOCKOUT_SERVER_ATTRIBUTE_ID:
                {
                  appSnprintf(" Keypad Lock Attribute = 0x%x\r\n",thThermostatUiConfClusterServerAttributes.keypadLockOut.value);
                }
                break;
                case ZCL_THERMOSTAT_UI_CONF_CLUSTER_SCHEDULE_PROGRAMMING_VISIBILITY_SERVER_ATTRIBUTE_ID:
                {
                  appSnprintf(" Schedule Programming Visibility Attribute = 0x%x\r\n",thThermostatUiConfClusterServerAttributes.scheduleProgVisibility.value);
                }
                break;
          default:
                  break;
              }
            }


        }
        break;
        case CMD_ZCL_ATTR_THERMOSTAT:
        {
          /* Command ZCL Thermostat Attribute Indication */
          //Access - > eventItem.eventData.zclAttributeData.addressing;
          //Access - > eventItem.eventData.zclAttributeData.attributeId;
          //Access - > eventItem.eventData.zclAttributeData.event;  
          ZCL_AttributeId_t attributeId;
          ZCL_AttributeEvent_t zclEvent;
          attributeId = event.eventData.zclAttributeData.attributeId;
          zclEvent = event.eventData.zclAttributeData.event;
          if(ZCL_CONFIGURE_DEFAULT_ATTRIBUTE_REPORTING_EVENT == zclEvent)
          {
           switch(attributeId)
           {
                case ZCL_THERMOSTAT_CLUSTER_LOCAL_TEMPERATURE_SERVER_ATTRIBUTE_ID:
                  thThermostatClusterServerAttributes.localTemperature.minReportInterval = THERMOSTAT_LOCAL_TEMPERATURE_MIN_REPORT_PERIOD;
                  thThermostatClusterServerAttributes.localTemperature.maxReportInterval = THERMOSTAT_LOCAL_TEMPERATURE_MAX_REPORT_PERIOD;
                  thThermostatClusterServerAttributes.localTemperature.reportableChange = 0;    
                break;

                case ZCL_THERMOSTAT_CLUSTER_PI_COOLING_DEMAND_SERVER_ATTRIBUTE_ID:
                  thThermostatClusterServerAttributes.PICoolingDemand.minReportInterval = THERMOSTAT_PI_CO0LING_DEMAND_MIN_REPORT_PERIOD;
                  thThermostatClusterServerAttributes.PICoolingDemand.maxReportInterval = THERMOSTAT_PI_CO0LING_DEMAND_MAX_REPORT_PERIOD;
                  thThermostatClusterServerAttributes.PICoolingDemand.reportableChange = 0;
                break;
                case ZCL_THERMOSTAT_CLUSTER_PI_HEATING_DEMAND_SERVER_ATTRIBUTE_ID:
                  thThermostatClusterServerAttributes.PIHeatingDemand.minReportInterval = THERMOSTAT_PI_HEATING_DEMAND_MIN_REPORT_PERIOD;
                  thThermostatClusterServerAttributes.PIHeatingDemand.maxReportInterval = THERMOSTAT_PI_HEATING_DEMAND_MAX_REPORT_PERIOD;
                  thThermostatClusterServerAttributes.PIHeatingDemand.reportableChange = 0;
                break;
        
            default :
            break;
            }
           }
        }
        break;
        case CMD_ZCL_ATTR_TEMPERATURE_MEASUREMENT:
        {
            /* Command ZCL Temperature measurement Attribute Indication */
            //Access - > eventItem.eventData.zclAttributeData.addressing;
            //Access - > eventItem.eventData.zclAttributeData.attributeId;
            //Access - > eventItem.eventData.zclAttributeData.event;
           if ((ZCL_CONFIGURE_DEFAULT_ATTRIBUTE_REPORTING_EVENT == event.eventData.zclAttributeData.event) && \
           (ZCL_TEMPERATURE_MEASUREMENT_CLUSTER_SERVER_MEASURED_VALUE_ATTRIBUTE_ID == event.eventData.zclAttributeData.attributeId))
           {    
             thTemperatureMeasurementClusterServerAttributes.measuredValue.minReportInterval = TEMPERATURE_MEASUREMENT_VAL_MIN_REPORT_PERIOD;//User can change the min report period
             thTemperatureMeasurementClusterServerAttributes.measuredValue.maxReportInterval = TEMPERATURE_MEASUREMENT_VAL_MAX_REPORT_PERIOD;//User can change the max report period
             thTemperatureMeasurementClusterServerAttributes.measuredValue.reportableChange = 0;     
           }
        }
        break;
        case CMD_ZCL_ATTR_FANCONTROL:
        {   
            //Access - > event.eventData.zclAttributeData.addressing;
            //Access - > event.eventData.zclAttributeData.attributeId;
            //Access - > event.eventData.zclAttributeData.event;   		
            //appSnprintf("<-Attr ID 0x%x event 0x%x\r\n",event.eventData.zclAttributeData.attributeId, event.eventData.zclAttributeData.event);
        }
        break;
        case CMD_ZCL_REPORTING_OCCUPANCY:
        {
            /* Command ZCL LevelReportInd( */
            //Access - > event.eventData.zclAttributeData.addressing;
            //Access - > event.eventData.zclAttributeData.payloadLength;
            //Access - > event.eventData.zclAttributeData.payload;
            ZCL_Report_t *rep = (ZCL_Report_t *)event.eventData.zclEventData.payload;
            appSnprintf("<-Occupancy Sensor Attr Report: Value = 0x%x\r\n", (int)rep->value[0]);
        }
        break;
        case CMD_ZCL_REPORTING_THERMOSTAT:
        {
            /* Command ZCL Thermostat Report Indication */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;                
            //appSnprintf("ZCL Report TH Thermostat Ind\r\n");
            ZCL_Report_t *rep = (ZCL_Report_t *)event.eventData.zclEventData.payload;
            int16_t reportValue = 0;
            #if defined(THERMOSTAT_EXTERNAL_TEMPERATURE_SENSOR_NODE_AVAILABLE)
             if(rep->id == ZCL_THERMOSTAT_CLUSTER_LOCAL_TEMPERATURE_SERVER_ATTRIBUTE_ID)
             {
               memcpy(&reportValue, &rep->value[0], sizeof(int16_t));
               thermostatUpdateServerAttributes(reportValue);
               #ifdef ZCL_THERMOSTAT_CLUSTER_INCLUDE_OPTIONAL_ATTRIBUTES  
                 thermostatUpdateThRunningMode(reportValue);
               #endif
               appSnprintf("<-Thermostat Attr (0x%x) Report: Value = %d.%dC\r\n", (unsigned)rep->id, (int)(reportValue/THERMOSTAT_LOCAL_TEMPERATURE_SCALE),(int)(reportValue%THERMOSTAT_LOCAL_TEMPERATURE_SCALE));
              }
              else if ((rep->id == ZCL_THERMOSTAT_CLUSTER_PI_COOLING_DEMAND_SERVER_ATTRIBUTE_ID) || (rep->id == ZCL_THERMOSTAT_CLUSTER_PI_HEATING_DEMAND_SERVER_ATTRIBUTE_ID))
              {
                memcpy(&reportValue, &rep->value[0], sizeof(uint8_t));
                appSnprintf("<-Thermostat Attr (0x%x) Report: Value = 0x%x\r\n", rep->id, (uint8_t)reportValue);
              }
             #endif
        }
        break;
        case CMD_ZCL_REPORTING_TEMPERATURE_MEASUREMENT:
        {
            /* Command ZCL Thermostat Report Indication */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;              
            ZCL_Report_t *rep = (ZCL_Report_t *)event.eventData.zclEventData.payload;
            int16_t reportValue;
            memcpy(&reportValue, &rep->value[0], sizeof(int16_t));
            thermostatUpdateServerAttributes(reportValue);
            local_temp_update(reportValue);
            appSnprintf( "<-Temperature Measurement Attr Report: Value = %d\r\n", reportValue);
        }
        break;
        case CMD_ZCL_REPORTING_HUMIDITY_MEASUREMENT:
        {
            /* Command ZCL Humidity Measurement Report Indication */
            //Access - > event.eventData.zclEventData.addressing;
            //Access - > event.eventData.zclEventData.payloadLength;
            //Access - > event.eventData.zclEventData.payload;              
            ZCL_Report_t *rep = (ZCL_Report_t *)event.eventData.zclEventData.payload;
            uint16_t reportValue;
            memcpy(&reportValue, &rep->value[0], sizeof(uint16_t));
            appSnprintf( "<-Relative Humidity Measurement Attr Report: Value = 0x%x\r\n", reportValue);
        }
        break;
        default:
        break;
    }
}