/*******************************************************************************
  Thermostat Occupancy Sensing cluster Source File

  Company:
    Microchip Technology Inc.

  File Name:
   thOccupancySensingCluster.c

  Summary:
    This file contains the Thermostat Occupancy Sensing cluster interface.

  Description:
    This file contains the Thermostat Occupancy Sensing cluster interface.
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
#include <z3device/thermostat/include/thOccupancySensingCluster.h>
#include <z3device/thermostat/include/thFanControlCluster.h>
#include <zcl/include/zclCommandManager.h>
#include <app_zigbee/zigbee_console/console.h>
#include <pds/include/wlPdsMemIds.h>
#include <z3device/common/include/z3Device.h>

/******************************************************************************
                             Defines section
******************************************************************************/
#define MOVEMENT_DETECTION_PERIOD                       2000UL
#define AMOUNT_MSEC_IN_SEC                              1000UL
#define NO_OF_MOVEMENT_DETECTION_EVENTS_ALLOWED         7U

/******************************************************************************
                    Global variables
******************************************************************************/
ZCL_OccupancySensingClusterServerAttributes_t thOccupancySensingClusterServerAttributes =
{
  ZCL_DEFINE_OCCUPANCY_SENSING_CLUSTER_SERVER_ATTRIBUTES(OCCUPANCY_SENSING_VAL_MIN_REPORT_PERIOD, OCCUPANCY_SENSING_VAL_MAX_REPORT_PERIOD)
};

ZCL_OccupancySensingClusterClientAttributes_t thOccupancySensingClusterClientAttributes =
{
  ZCL_DEFINE_OCCUPANCY_SENSING_CLUSTER_CLIENT_ATTRIBUTES()
};

/*******************************************************************************
                   Types section
*******************************************************************************/
typedef enum
{
  OCCUPANCY_CHANGE_STATE_IDLE,
  OCCUPANCY_CHANGE_STATE_OCCUPIED_TO_UNOCCUPIED_IN_PROGRESS,
  OCCUPANCY_CHANGE_STATE_UNOCCUPIED_TO_OCCUPIED_IN_PROGRESS,
} OccupancyChangeState_t;

/******************************************************************************
                    Static functions
******************************************************************************/
static void occupancySensingSetOccupancyState(void);
static void thOccupancySensorReportInd(ZCL_Addressing_t *addressing, uint8_t reportLength, uint8_t *reportPayload);
static void occupiedToUnoccupiedHandler(void);
static void unoccupiedToOccupiedHandler(void);
static void movementDetected(void);
static bool verifyOccupancySensorDetection(bool state);
static void thOccupancyAttributeEventInd(ZCL_Addressing_t *addressing, ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event);

/******************************************************************************
                    Static variables
******************************************************************************/
static HAL_AppTimer_t sensorAttributeUpdateTimer;
static uint8_t occupancyChangeState = OCCUPANCY_CHANGE_STATE_IDLE;
static uint64_t delayStartTime = 0;
static uint8_t eventCount = 0;
static uint8_t threshold;
static uint16_t delay;

static HAL_AppTimer_t movementEventTimer =
{
  .interval = MOVEMENT_DETECTION_PERIOD,
  .mode     = TIMER_REPEAT_MODE,
  .callback = movementDetected,
};

/******************************************************************************
                    Implementation section
******************************************************************************/
/**************************************************************************//**
\brief Initializes Occupancy Sensing cluster
******************************************************************************/
void thOccupancySensingClusterInit(void)
{
  ZCL_Cluster_t *cluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, OCCUPANCY_SENSING_CLUSTER_ID, ZCL_CLUSTER_SIDE_CLIENT);

  if (cluster)
  {
    thOccupancySensingClusterClientAttributes.clusterVersion.value = OCCUPANCY_CLUSTER_VERSION;
    cluster->ZCL_ReportInd = thOccupancySensorReportInd;
    cluster->ZCL_DefaultRespInd = ZCL_CommandZclDefaultResp;
  }
  cluster->ZCL_DefaultRespInd = ZCL_CommandZclDefaultResp;
  cluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, OCCUPANCY_SENSING_CLUSTER_ID, ZCL_CLUSTER_SIDE_SERVER);
  
  if (cluster)
  {
    thOccupancySensingClusterServerAttributes.clusterVersion.value = OCCUPANCY_CLUSTER_VERSION;
    thOccupancySensingClusterServerAttributes.occupancySensorType.value = OCCUPANYC_SENSOR_TYPE_ATTRIBUTE_VALUE_PIR;
    thOccupancySensingClusterServerAttributes.occupancySensorTypeBitmap.value = OCCUPANYC_SENSOR_TYPE_BITMAP_ATTRIBUTE_VALUE_PIR;
    thOccupancySensingClusterServerAttributes.occupancy.value = OCCUPANCY_ATTRIBUTE_VALUE_UNOCCUPIED;

    ZCL_ReportOnChangeIfNeeded(&thOccupancySensingClusterServerAttributes.occupancy);
    thOccupancySensingClusterServerAttributes.PIROccupiedToUnoccupiedDelay.value = ZCL_OCCUPANCY_SENSING_CL_PIR_OCCUPIED_TO_UNOCCUPIED_DELAY_SER_ATTR_DEFAULT_VAL;
    thOccupancySensingClusterServerAttributes.PIRUnoccupiedToOccupiedDelay.value = ZCL_OCCUPANCY_SENSING_CL_PIR_UNOCCUPIED_TO_OCCUPIED_DELAY_SER_ATTR_DEFAULT_VAL;
    thOccupancySensingClusterServerAttributes.PIRUnoccupiedToOccupiedThreshold.value = ZCL_OCCUPANCY_SENSING_CL_PIR_UNOCCUPIED_TO_OCCUPIED_THRESHOLD_SER_ATTR_DEFAULT_VAL;
    thOccupancySensingClusterServerAttributes.UltrasonicOccupiedToUnoccupiedDelay.value = ZCL_OCCUPANCY_SENSING_CL_ULTRASONIC_OCCUPIED_TO_UNOCCUPIED_DELAY_SER_ATTR_DEFAULT_VAL;
    thOccupancySensingClusterServerAttributes.UltrasonicUnoccupiedToOccupiedDelay.value = ZCL_OCCUPANCY_SENSING_CL_ULTRASONIC_UNOCCUPIED_TO_OCCUPIED_DELAY_SER_ATTR_DEFAULT_VAL;
    thOccupancySensingClusterServerAttributes.UltrasonicUnoccupiedToOccupiedThreshold.value = ZCL_OCCUPANCY_SENSING_CL_ULTRASONIC_UNOCCUPIED_TO_OCCUPIED_THRESHOLD_SER_ATTR_DEFAULT_VAL;
    thOccupancySensingClusterServerAttributes.PhysicalContactOccupiedToUnoccupiedDelay.value = ZCL_OCCUPANCY_SENSING_CL_PHYSICAL_CONTACT_OCCUPIED_TO_UNOCCUPIED_DELAY_SER_ATTR_DEFAULT_VAL;
    thOccupancySensingClusterServerAttributes.PhysicalContactUnoccupiedToOccupiedDelay.value = ZCL_OCCUPANCY_SENSING_CL_PHYSICAL_CONTACT_UNOCCUPIED_TO_OCCUPIED_DELAY_SER_ATTR_DEFAULT_VAL;
    thOccupancySensingClusterServerAttributes.PhysicalContactUnoccupiedToOccupiedThreshold.value = ZCL_OCCUPANCY_SENSING_CL_PHYSICAL_CONTACT_UNOCCUPIED_TO_OCCUPIED_THRESHOLD_SER_ATTR_DEFAULT_VAL;
    cluster->ZCL_AttributeEventInd = thOccupancyAttributeEventInd;
    
    if (PDS_IsAbleToRestore(APP_TH_OCCUPANCY_MEM_ID))
      PDS_Restore(APP_TH_OCCUPANCY_MEM_ID);
  }
}


/**************************************************************************//**
\brief Toggles occupancy
******************************************************************************/
void occupancySensingToggleOccupancy(void)
{
  if (OCCUPANCY_ATTRIBUTE_VALUE_UNOCCUPIED == thOccupancySensingClusterServerAttributes.occupancy.value)
    thOccupancySensingClusterServerAttributes.occupancy.value = OCCUPANCY_ATTRIBUTE_VALUE_OCCUPIED;
  else
    thOccupancySensingClusterServerAttributes.occupancy.value = OCCUPANCY_ATTRIBUTE_VALUE_UNOCCUPIED;
  
  PDS_Store(APP_TH_OCCUPANCY_MEM_ID);
#ifdef _ZCL_REPORTING_SUPPORT_
  ZCL_ReportOnChangeIfNeeded(&thOccupancySensingClusterServerAttributes.occupancy);
#endif
}

/**************************************************************************//**
\brief Initiates occupancy to Occupied state or Unoccupied state or vice versa
******************************************************************************/
void occupancySensingInitiateSetOccupancyState(bool state)
{
  if (OCCUPANCY_CHANGE_STATE_IDLE == occupancyChangeState)
  {
    if (OCCUPANCY_ATTRIBUTE_VALUE_UNOCCUPIED == state)
      occupiedToUnoccupiedHandler();
    else
      unoccupiedToOccupiedHandler();
  }
  else
    /* Consider this as movement event detetced from sensor */
    verifyOccupancySensorDetection(state);
  
  PDS_Store(APP_TH_OCCUPANCY_MEM_ID);
}

/***************************************************************************//**
\brief checks the detected movement whether Occupied to Unoccupied or Unoccupied to Occupied
******************************************************************************/
static bool verifyOccupancySensorDetection(bool state)
{
  /* check the occupancy state detected */
  switch(occupancyChangeState)
  {
    case OCCUPANCY_CHANGE_STATE_OCCUPIED_TO_UNOCCUPIED_IN_PROGRESS:
      if (state)
      {
        HAL_StopAppTimer(&sensorAttributeUpdateTimer);
        occupancyChangeState = OCCUPANCY_CHANGE_STATE_IDLE;
      }
      break;
    case OCCUPANCY_CHANGE_STATE_UNOCCUPIED_TO_OCCUPIED_IN_PROGRESS:
      if (!state)
      {
        HAL_StopAppTimer(&movementEventTimer);
        HAL_StopAppTimer(&sensorAttributeUpdateTimer);
        occupancyChangeState = OCCUPANCY_CHANGE_STATE_IDLE;
      }
      else
        return true;
      break;
  }
  return false;
}

/**************************************************************************//**
\brief Handler for changing the occupancy state from occupied to unoccupied
******************************************************************************/
static void occupiedToUnoccupiedHandler(void)
{
  occupancyChangeState = OCCUPANCY_CHANGE_STATE_OCCUPIED_TO_UNOCCUPIED_IN_PROGRESS;

  if (OCCUPANYC_SENSOR_TYPE_ATTRIBUTE_VALUE_PIR == thOccupancySensingClusterServerAttributes.occupancySensorType.value ||
      OCCUPANYC_SENSOR_TYPE_BITMAP_ATTRIBUTE_VALUE_PIR == thOccupancySensingClusterServerAttributes.occupancySensorTypeBitmap.value)
  {
    if (!thOccupancySensingClusterServerAttributes.PIROccupiedToUnoccupiedDelay.value)
    {
      occupancySensingSetOccupancyState();
      return;
    }
    else
      sensorAttributeUpdateTimer.interval = thOccupancySensingClusterServerAttributes.PIROccupiedToUnoccupiedDelay.value * AMOUNT_MSEC_IN_SEC;//msec
  }
  else 
      if (OCCUPANYC_SENSOR_TYPE_ATTRIBUTE_VALUE_ULTRASONIC == thOccupancySensingClusterServerAttributes.occupancySensorType.value ||
           OCCUPANYC_SENSOR_TYPE_BITMAP_ATTRIBUTE_VALUE_ULTRASONIC == thOccupancySensingClusterServerAttributes.occupancySensorTypeBitmap.value)
  {
    if (!thOccupancySensingClusterServerAttributes.UltrasonicOccupiedToUnoccupiedDelay.value)
      {
        occupancySensingSetOccupancyState();
        return;
      }
      else
        sensorAttributeUpdateTimer.interval = thOccupancySensingClusterServerAttributes.UltrasonicOccupiedToUnoccupiedDelay.value * AMOUNT_MSEC_IN_SEC;//msec
  }
  else if (OCCUPANYC_SENSOR_TYPE_ATTRIBITE_VALUE_PHYSICAL_CONTACT == thOccupancySensingClusterServerAttributes.occupancySensorType.value ||
           OCCUPANYC_SENSOR_TYPE_BITMAP_ATTRIBUTE_VALUE_PHYSICAL_CONTACT_PIR == thOccupancySensingClusterServerAttributes.occupancySensorTypeBitmap.value)
  {
    if (!thOccupancySensingClusterServerAttributes.PhysicalContactOccupiedToUnoccupiedDelay.value)
      {
        occupancySensingSetOccupancyState();
        return;
      }
      else
        sensorAttributeUpdateTimer.interval = thOccupancySensingClusterServerAttributes.PhysicalContactOccupiedToUnoccupiedDelay.value * AMOUNT_MSEC_IN_SEC;//msec
  }
  sensorAttributeUpdateTimer.mode     = TIMER_ONE_SHOT_MODE,
  sensorAttributeUpdateTimer.callback = occupancySensingSetOccupancyState,
  HAL_StartAppTimer(&sensorAttributeUpdateTimer);
}

/**************************************************************************//**
\brief Handler for changing the occupancy state from unoccupied to occupied 
******************************************************************************/
static void unoccupiedToOccupiedHandler(void)
{
  occupancyChangeState = OCCUPANCY_CHANGE_STATE_UNOCCUPIED_TO_OCCUPIED_IN_PROGRESS;

  if (OCCUPANYC_SENSOR_TYPE_ATTRIBUTE_VALUE_PIR == thOccupancySensingClusterServerAttributes.occupancySensorType.value ||
      OCCUPANYC_SENSOR_TYPE_BITMAP_ATTRIBUTE_VALUE_PIR == thOccupancySensingClusterServerAttributes.occupancySensorTypeBitmap.value)
  {
    if (!thOccupancySensingClusterServerAttributes.PIRUnoccupiedToOccupiedDelay.value && !thOccupancySensingClusterServerAttributes.PIRUnoccupiedToOccupiedThreshold.value)
    {
      occupancySensingSetOccupancyState();
      return;
    }
    else
    {
      delay = thOccupancySensingClusterServerAttributes.PIRUnoccupiedToOccupiedDelay.value;
      threshold = thOccupancySensingClusterServerAttributes.PIRUnoccupiedToOccupiedThreshold.value;
    }
  }
  else if (OCCUPANYC_SENSOR_TYPE_ATTRIBUTE_VALUE_ULTRASONIC == thOccupancySensingClusterServerAttributes.occupancySensorType.value ||
           OCCUPANYC_SENSOR_TYPE_BITMAP_ATTRIBUTE_VALUE_ULTRASONIC == thOccupancySensingClusterServerAttributes.occupancySensorTypeBitmap.value)
  {
    if (!thOccupancySensingClusterServerAttributes.UltrasonicUnoccupiedToOccupiedDelay.value)
    {
      occupancySensingSetOccupancyState();
      return;
    }
    else
    {
      delay = thOccupancySensingClusterServerAttributes.UltrasonicUnoccupiedToOccupiedDelay.value;
      threshold = thOccupancySensingClusterServerAttributes.UltrasonicUnoccupiedToOccupiedThreshold.value;
    }
  }
  else if (OCCUPANYC_SENSOR_TYPE_ATTRIBITE_VALUE_PHYSICAL_CONTACT == thOccupancySensingClusterServerAttributes.occupancySensorType.value ||
           OCCUPANYC_SENSOR_TYPE_BITMAP_ATTRIBUTE_VALUE_PHYSICAL_CONTACT_PIR == thOccupancySensingClusterServerAttributes.occupancySensorTypeBitmap.value)
  {
    if (!thOccupancySensingClusterServerAttributes.PhysicalContactUnoccupiedToOccupiedDelay.value)
    {
      occupancySensingSetOccupancyState();
      return;
    }
    else
    {
      delay = thOccupancySensingClusterServerAttributes.PhysicalContactUnoccupiedToOccupiedDelay.value;
      threshold = thOccupancySensingClusterServerAttributes.PhysicalContactUnoccupiedToOccupiedThreshold.value;
    }
  }
  eventCount++; //this is considered as first movement detetcted
  delayStartTime = HAL_GetSystemTime();
  HAL_StartAppTimer(&movementEventTimer);
}

/**************************************************************************//**
\brief Simulation of occupied movement detection events (for every 2secs )

\param[in] resp - pointer to response
******************************************************************************/
static void movementDetected(void)
{
  uint64_t currentTime = 0;
  /* In general , this should be called on any kind of movement unoccupied to occupied
     or occcupied to unoccupied , but here only called on unoccipied to occupied movement 
     detection event only */
  /* API can added here to read sensor detection event - 0 to 1 or 1 to 0 */
  if (!verifyOccupancySensorDetection(1))
    return;

  if (++eventCount > (threshold & NO_OF_MOVEMENT_DETECTION_EVENTS_ALLOWED) - 1 )
  {
    HAL_StopAppTimer(&movementEventTimer);
    HAL_StopAppTimer(&sensorAttributeUpdateTimer);
    eventCount = 0;
    currentTime = HAL_GetSystemTime();
    if ((currentTime - delayStartTime)/AMOUNT_MSEC_IN_SEC >= delay )
    {
      occupancySensingSetOccupancyState();
    }
    else
    {
      /* remaining time before occupancy delay expires */
      sensorAttributeUpdateTimer.interval = delay * AMOUNT_MSEC_IN_SEC - (currentTime - delayStartTime);
      sensorAttributeUpdateTimer.mode     = TIMER_ONE_SHOT_MODE;
      sensorAttributeUpdateTimer.callback = occupancySensingSetOccupancyState;

      HAL_StartAppTimer(&sensorAttributeUpdateTimer);
    }
  }
}
/**************************************************************************//**
\brief Sets occupancy to Occupied state or Unoccupied state
******************************************************************************/
static void occupancySensingSetOccupancyState(void)
{
  thOccupancySensingClusterServerAttributes.occupancy.value = !(thOccupancySensingClusterServerAttributes.occupancy.value & 0x01);
  ZCL_ReportOnChangeIfNeeded(&thOccupancySensingClusterServerAttributes.occupancy);
  occupancyChangeState = OCCUPANCY_CHANGE_STATE_IDLE;
  eventCount = 0;
}
/**************************************************************************//**
\brief Sets occupancy to Occupied state or Unoccupied state
******************************************************************************/
void occupancySensingSetSensorType(uint8_t sensorType)
{
  if (thOccupancySensingClusterServerAttributes.occupancySensorType.value != sensorType)
  {
    thOccupancySensingClusterServerAttributes.occupancySensorType.value = sensorType;
    thOccupancySensingClusterServerAttributes.occupancy.value = OCCUPANCY_ATTRIBUTE_VALUE_UNOCCUPIED;
    HAL_StopAppTimer(&movementEventTimer);
    HAL_StopAppTimer(&sensorAttributeUpdateTimer);
    eventCount = 0;
  }
}
/**************************************************************************//**
\brief Sets occupancysensingTypeBitmap attribute
******************************************************************************/
void occupancySensingSetSensorTypeBitmap(uint8_t sensorType)
{
  if (thOccupancySensingClusterServerAttributes.occupancySensorTypeBitmap.value != sensorType)
  {
    thOccupancySensingClusterServerAttributes.occupancySensorTypeBitmap.value = sensorType;
    thOccupancySensingClusterServerAttributes.occupancy.value = OCCUPANCY_ATTRIBUTE_VALUE_UNOCCUPIED;
    HAL_StopAppTimer(&movementEventTimer);
    HAL_StopAppTimer(&sensorAttributeUpdateTimer);
    eventCount = 0;
  }
}
/**************************************************************************//**
\brief Report attribute indication handler

\param[in] addressing - pointer to addressing information;
\param[in] reportLength - data payload length;
\param[in] reportPayload - data pointer
******************************************************************************/
static void thOccupancySensorReportInd(ZCL_Addressing_t *addressing, uint8_t reportLength, uint8_t *reportPayload)
{
  ZCL_Report_t *rep = (ZCL_Report_t *)reportPayload;
  APP_Zigbee_Event_t eventItem;
  eventItem.eventGroup = EVENT_CLUSTER;
  eventItem.eventId = CMD_ZCL_REPORTING_OCCUPANCY;
  eventItem.eventData.zclEventData.addressing = addressing;
  eventItem.eventData.zclEventData.payloadLength = reportLength;
  eventItem.eventData.zclEventData.payload = reportPayload;

  thFanControlOccupancyNotify((bool)rep->value[0]);
  APP_Zigbee_Handler(eventItem);
}

/**************************************************************************//**
\brief Attribute event (writing/reading) callback.

\param[in] addressing - incoming request addressing information.
\param[in] attributeId - attribute identifier.
\param[in] event - attribute event (read/write).
******************************************************************************/
static void thOccupancyAttributeEventInd(ZCL_Addressing_t *addressing, ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event)
{
  APP_Zigbee_Event_t eventItem;
  eventItem.eventGroup = EVENT_CLUSTER;
  eventItem.eventId = CMD_ZCL_ATTR_OCCUPANCY;
  eventItem.eventData.zclAttributeData.addressing = addressing;
  eventItem.eventData.zclAttributeData.attributeId = attributeId;
  eventItem.eventData.zclAttributeData.event = event;
  APP_Zigbee_Handler(eventItem);

  if (((ZCL_CONFIGURE_ATTRIBUTE_REPORTING_EVENT == event) || \
      (ZCL_CONFIGURE_DEFAULT_ATTRIBUTE_REPORTING_EVENT == event)) && \
      ((ZCL_OCCUPANCY_SENSING_CLUSTER_OCCUPANCY_SERVER_ATTRIBUTE_ID == attributeId))
      )
  {
    PDS_Store(APP_TH_OCCUPANCY_MEM_ID);
  }  
}

#endif // APP_DEVICE_TYPE_THERMOSTAT

// eof thOccupancySensingCluster.c 
