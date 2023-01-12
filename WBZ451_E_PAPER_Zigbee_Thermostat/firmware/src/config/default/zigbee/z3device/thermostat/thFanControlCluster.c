/*******************************************************************************
  Thermostat Fan Control cluster Source File

  Company:
    Microchip Technology Inc.

  File Name:
    thFanControlCluster.c

  Summary:
    This file contains the Thermostat Fan Control cluster interface.

  Description:
    This file contains the Thermostat Fan Control cluster interface.
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
#include <z3device/thermostat/include/thFanControlCluster.h>
#if APP_ENABLE_CONSOLE == 1
#include <app_zigbee/zigbee_console/console.h>
#endif
#include <zcl/include/zclCommandManager.h>
#include <zcl/include/zclAttributes.h>
#include <z3device/clusters/include/haClusters.h>
#include <z3device/common/include/appConsts.h>
#include <app_zigbee/app_zigbee_handler.h>
/******************************************************************************
                    Global variables
******************************************************************************/
ZCL_FanControlClusterServerAttributes_t thFanControlClusterServerAttributes =
{
  ZCL_DEFINE_FAN_CONTROL_CLUSTER_SERVER_ATTRIBUTES()
};

ZCL_FanControlClusterClientAttributes_t thFanControlClusterClientAttributes =
{
  ZCL_DEFINE_FAN_CONTROL_CLUSTER_CLIENT_ATTRIBUTES()
};

/******************************************************************************
                    Prototypes section
******************************************************************************/

static void thFanControlAttrEventInd(ZCL_Addressing_t *addressing, ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event);
static void thFanControlEventListener(SYS_EventId_t eventId, SYS_EventData_t data);
static void thFanControlsetFanOnOff(ZCL_FanControlFanMode_t fanState);

/******************************************************************************
                    Local variables
******************************************************************************/
static SYS_EventReceiver_t thFanControlEvent = { .func = thFanControlEventListener};
static ZCL_FanControlFanMode_t fanMotorControl = ZCL_FC_FAN_MODE_OFF;
/*****************************************************************************/

/******************************************************************************
                    Implementation section
******************************************************************************/
/**************************************************************************//**
\brief Initializes Fan Control cluster
******************************************************************************/
void thFanControlClusterInit(void)
{
  ZCL_Cluster_t *cluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, FAN_CONTROL_CLUSTER_ID, ZCL_CLUSTER_SIDE_SERVER);  
  ZCL_Cluster_t *fanControlcluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, FAN_CONTROL_CLUSTER_ID, ZCL_CLUSTER_SIDE_CLIENT);  
  
  if (cluster)
  {
    cluster->ZCL_AttributeEventInd = thFanControlAttrEventInd;
  }
  thFanControlClusterServerAttributes.fanMode.value = ZCL_FAN_CONTROL_CL_FAN_MODE_SER_ATTR_DEFAULT_VAL;
  thFanControlClusterServerAttributes.fanModeSequence.value = ZCL_FAN_CONTROL_CL_FAN_SEQUENCE_OPERATION_SER_ATTR_DEFAULT_VAL;
  thFanControlClusterServerAttributes.clusterVersion.value = FAN_CONTROL_CLUSTER_VERSION;
  SYS_SubscribeToEvent(BC_ZCL_EVENT_ACTION_REQUEST, &thFanControlEvent);
  thFanControlClusterClientAttributes.clusterVersion.value = FAN_CONTROL_CLUSTER_VERSION;
  if (fanControlcluster)
    fanControlcluster->ZCL_DefaultRespInd = ZCL_CommandZclDefaultResp;
}

/**************************************************************************//**
\brief Attribute Event indication handler(to indicate when attr values have
        read or written)

\param[in] addressing - pointer to addressing information;
\param[in] reportLength - data payload length;
\param[in] reportPayload - data pointer
******************************************************************************/
static void thFanControlAttrEventInd(ZCL_Addressing_t *addressing, ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event)
{
  APP_Zigbee_Event_t eventItem;
  if ( (attributeId == ZCL_FAN_CONTROL_CLUSTER_FAN_MODE_SERVER_ATTRIBUTE_ID) 
           && (event == ZCL_WRITE_ATTRIBUTE_EVENT) )
  {
    switch(thFanControlClusterServerAttributes.fanMode.value)
    {
      case ZCL_FC_FAN_MODE_LOW:
      case ZCL_FC_FAN_MODE_MEDIUM:
      case ZCL_FC_FAN_MODE_HIGH:
        fanMotorControl = (ZCL_FanControlFanMode_t)thFanControlClusterServerAttributes.fanMode.value;
        break;
      case ZCL_FC_FAN_MODE_AUTO:
        /*Specification does not define this case, hence setting it to medium
        User may change for actual fan control*/
        fanMotorControl = ZCL_FC_FAN_MODE_MEDIUM;
        break;
      case ZCL_FC_FAN_MODE_OFF:
      case ZCL_FC_FAN_MODE_ON:
        thFanControlsetFanOnOff((ZCL_FanControlFanMode_t)thFanControlClusterServerAttributes.fanMode.value);
        break;
    }
  }
  eventItem.eventGroup = EVENT_CLUSTER;
  eventItem.eventId = CMD_ZCL_ATTR_FANCONTROL;
  eventItem.eventData.zclAttributeData.addressing = addressing;
  eventItem.eventData.zclAttributeData.attributeId = attributeId;
  eventItem.eventData.zclAttributeData.event = event;
  APP_Zigbee_Handler(eventItem);
}

/**************************************************************************//**
  \brief  ZCL action request event handler, 
          handles the ZCL_ACTION_WRITE_ATTR_REQUEST for attribute specific validation

  \param[in] ev - must be BC_ZCL_EVENT_ACTION_REQUEST.
  \param[in] data - this field must contain pointer to the BcZCLActionReq_t structure,

  \return None.
 ******************************************************************************/
static void thFanControlEventListener(SYS_EventId_t eventId, SYS_EventData_t data)
{
  BcZCLActionReq_t *const actionReq = (BcZCLActionReq_t*)data;  
  ZCL_FanControlFanMode_t requestedValue = (ZCL_FanControlFanMode_t)0;

  if (BC_ZCL_EVENT_ACTION_REQUEST != eventId)
    return;
  
  if (ZCL_ACTION_WRITE_ATTR_REQUEST != actionReq->action)
    return;

  ZCLActionWriteAttrReq_t *const zclWriteAttrReq = (ZCLActionWriteAttrReq_t*)actionReq->context;
  if( (FAN_CONTROL_CLUSTER_ID != zclWriteAttrReq->clusterId) || 
          (ZCL_CLUSTER_SIDE_SERVER != zclWriteAttrReq->clusterSide))
    return;

  requestedValue = (ZCL_FanControlFanMode_t)(*((uint8_t*)(zclWriteAttrReq->attrValue)));
  if(ZCL_FAN_CONTROL_CLUSTER_FAN_MODE_SERVER_ATTRIBUTE_ID != zclWriteAttrReq->attrId)
    return;
  switch(thFanControlClusterServerAttributes.fanModeSequence.value)
  {
    case ZCL_FC_FAN_SEQUENCE_OPERATION_LOW_MED_HIGH:
      if(ZCL_FC_FAN_MODE_AUTO == requestedValue)
        actionReq->denied = 1U;
      break;
    case ZCL_FC_FAN_SEQUENCE_OPERATION_LOW_HIGH:
      if((ZCL_FC_FAN_MODE_AUTO == requestedValue) || (ZCL_FC_FAN_MODE_MEDIUM == requestedValue))
        actionReq->denied = 1U;
      break;
    case ZCL_FC_FAN_SEQUENCE_OPERATION_LOW_MED_HIGH_AUTO:
      actionReq->denied = 0U;
      break;
    case ZCL_FC_FAN_SEQUENCE_OPERATION_LOW_HIGH_AUTO:
      if(ZCL_FC_FAN_MODE_MEDIUM == requestedValue)
        actionReq->denied = 1U;
      break;
    case ZCL_FC_FAN_SEQUENCE_OPERATION_ON_AUTO:
      if(ZCL_FC_FAN_MODE_AUTO != requestedValue)
        actionReq->denied = 1U;
      break;
    default:
      actionReq->denied = 0U;
      break;
  }
}

/**************************************************************************//**
\brief Occupancy notification from Occupancy (client/server) cluster

\param[in] occupied - 0 - Not occupied, 1- occupied
\param[out] - None
******************************************************************************/
void thFanControlOccupancyNotify(bool occupied)
{
  if(ZCL_FC_FAN_MODE_SMART == thFanControlClusterServerAttributes.fanMode.value)
  {
    if(occupied)    thFanControlsetFanOnOff(ZCL_FC_FAN_MODE_ON);
    else    thFanControlsetFanOnOff(ZCL_FC_FAN_MODE_OFF);
  }
}

/**************************************************************************//**
\brief Set the fan to ON or OFF (need to modified by the user)

\param[in] fanState : 0 - off, non zero - ON
\param[out] - None
******************************************************************************/
static void thFanControlsetFanOnOff(ZCL_FanControlFanMode_t fanState)
{ 
  /*The user has to implement the functionality to control the actual fan*/
  if(fanState)
  {
#if APP_ENABLE_CONSOLE == 1
    appSnprintf("Fan is turned ON at speed 0x%x\r\n",fanMotorControl);
#else
    (void)fanMotorControl;
#endif
  }
#if APP_ENABLE_CONSOLE == 1
  else
    appSnprintf("Fan is turned OFF\r\n");
#endif
}
#endif // APP_DEVICE_TYPE_THERMOSTAT

// eof thFanControlCluster.c
