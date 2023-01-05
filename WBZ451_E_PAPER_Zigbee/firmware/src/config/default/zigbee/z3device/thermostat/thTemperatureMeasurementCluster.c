/*******************************************************************************
  Thermostat Temperature Measurement cluster Source File

  Company:
    Microchip Technology Inc.

  File Name:
   thTemperatureMeasurementCluster.c

  Summary:
    This file contains the Thermostat Temperature Measurement cluster interface.

  Description:
    This file contains the Thermostat Temperature Measurement cluster interface.
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
#include <z3device/thermostat/include/thTemperatureMeasurementCluster.h>
#include <pds/include/wlPdsMemIds.h>
#include <z3device/common/include/z3Device.h>
#include <app_zigbee/zigbee_console/console.h>
#include <zcl/include/zclCommandManager.h>

/******************************************************************************
                    Definition(s) section
******************************************************************************/
#define TEMPERATURE_REPORT_PERIOD   20000UL

/******************************************************************************
                    Global variables
******************************************************************************/
ZCL_TemperatureMeasurementClusterServerAttributes_t thTemperatureMeasurementClusterServerAttributes =
{
  ZCL_DEFINE_TEMPERATURE_MEASUREMENT_CLUSTER_SERVER_ATTRIBUTES(TEMPERATURE_MEASUREMENT_VAL_MIN_REPORT_PERIOD, TEMPERATURE_MEASUREMENT_VAL_MAX_REPORT_PERIOD)
};

ZCL_TemperatureMeasurementClusterClientAttributes_t thTemperatureMeasurementClusterClientAttributes =
{
  ZCL_DEFINE_TEMPERATURE_MEASUREMENT_CLUSTER_CLIENT_ATTRIBUTES()
};

/******************************************************************************
                    Local variables section
******************************************************************************/
static HAL_AppTimer_t readTemperatureimer =
{
  .interval = TEMPERATURE_REPORT_PERIOD,
  .mode     = TIMER_REPEAT_MODE,
  .callback = thTempeartureMeasurementUpdateMeasuredValue,
};

/******************************************************************************
                    Prototypes section
******************************************************************************/
static void thTemperatureMeasurementReportInd(ZCL_Addressing_t *addressing, uint8_t reportLength, uint8_t *reportPayload);
static void thTemparatureAttributeEventInd(ZCL_Addressing_t *addressing, ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event);

/******************************************************************************
                    Implementation section
******************************************************************************/
/**************************************************************************//**
\brief Initializes Temperature Measurement cluster
******************************************************************************/
void thTemperatureMeasurementClusterInit(void)
{
  ZCL_Cluster_t *cluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, TEMPERATURE_MEASUREMENT_CLUSTER_ID, ZCL_CLUSTER_SIDE_CLIENT);
  thTemperatureMeasurementClusterClientAttributes.clusterVersion.value = TEMPERATURE_MEASUREMENT_CLUSTER_VERSION;
  thTemperatureMeasurementClusterServerAttributes.clusterVersion.value = TEMPERATURE_MEASUREMENT_CLUSTER_VERSION;
  if (cluster)
  {
    cluster->ZCL_ReportInd = thTemperatureMeasurementReportInd;
    cluster->ZCL_DefaultRespInd = ZCL_CommandZclDefaultResp;
  }
  cluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, TEMPERATURE_MEASUREMENT_CLUSTER_ID, ZCL_CLUSTER_SIDE_SERVER);

  if (cluster)
  {
    cluster->ZCL_AttributeEventInd = thTemparatureAttributeEventInd;
    thTemperatureMeasurementClusterServerAttributes.measuredValue.value = APP_TEMPERATURE_MEASUREMENT_MEASURED_VALUE_ATTRIBUTE_VALUE;
    thTemperatureMeasurementClusterServerAttributes.minMeasuredValue.value = APP_TEMPERATURE_MEASUREMENT_MIN_MEASURED_VALUE_ATTRIBUTE_VALUE;
    thTemperatureMeasurementClusterServerAttributes.maxMeasuredValue.value = APP_TEMPERATURE_MEASUREMENT_MAX_MEASURED_VALUE_ATTRIBUTE_VALUE;
    thTemperatureMeasurementClusterServerAttributes.tolerance.value = APP_TEMPERATURE_MEASUREMENT_TOLERANCE_ATTRIBUTE_VALUE;//to be updated
    HAL_StartAppTimer(&readTemperatureimer);
     
    if (PDS_IsAbleToRestore(APP_TH_TEMPERATURE_MEASURED_VALUE_MEM_ID))
      PDS_Restore(APP_TH_TEMPERATURE_MEASURED_VALUE_MEM_ID);
  }
}

/**************************************************************************//**
\brief Update the measured Value
******************************************************************************/
void thTempeartureMeasurementUpdateMeasuredValue(void)
{
  int16_t measuredValue;

  measuredValue = thTemperatureMeasurementClusterServerAttributes.measuredValue.value - APP_TEMPERATURE_MEASUREMENT_MEASURED_VALUE_PERIODIC_CHANGE;

  if (measuredValue < thTemperatureMeasurementClusterServerAttributes.minMeasuredValue.value)
    thTemperatureMeasurementClusterServerAttributes.measuredValue.value =  thTemperatureMeasurementClusterServerAttributes.maxMeasuredValue.value;
  else
    thTemperatureMeasurementClusterServerAttributes.measuredValue.value = measuredValue;

  PDS_Store(APP_TH_TEMPERATURE_MEASURED_VALUE_MEM_ID);
#ifdef _ZCL_REPORTING_SUPPORT_
  ZCL_ReportOnChangeIfNeeded(&thTemperatureMeasurementClusterServerAttributes.measuredValue);
#endif
}
/**************************************************************************//**
\brief Report attribute indication handler

\param[in] addressing - pointer to addressing information;
\param[in] reportLength - data payload length;
\param[in] reportPayload - data pointer
******************************************************************************/
static void thTemperatureMeasurementReportInd(ZCL_Addressing_t *addressing, uint8_t reportLength, uint8_t *reportPayload)
{
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_REPORTING_TEMPERATURE_MEASUREMENT;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = reportLength;
  event.eventData.zclEventData.payload = reportPayload;
  
  APP_Zigbee_Handler(event);
}

/**************************************************************************//**
\brief Attribute event (writing/reading) callback.

\param[in] addressing - incoming request addressing information.
\param[in] attributeId - attribute identifier.
\param[in] event - attribute event (read/write).
******************************************************************************/
static void thTemparatureAttributeEventInd(ZCL_Addressing_t *addressing, ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event)
{
  APP_Zigbee_Event_t eventItem;
  eventItem.eventGroup = EVENT_CLUSTER;
  eventItem.eventId = CMD_ZCL_ATTR_TEMPERATURE_MEASUREMENT;
  eventItem.eventData.zclAttributeData.addressing = addressing;
  eventItem.eventData.zclAttributeData.attributeId = attributeId;
  eventItem.eventData.zclAttributeData.event = event;
 
  APP_Zigbee_Handler(eventItem);

  if (((ZCL_CONFIGURE_ATTRIBUTE_REPORTING_EVENT == event) || \
       (ZCL_CONFIGURE_DEFAULT_ATTRIBUTE_REPORTING_EVENT == event)) && \
      (ZCL_TEMPERATURE_MEASUREMENT_CLUSTER_SERVER_MEASURED_VALUE_ATTRIBUTE_ID == attributeId))
  {
    PDS_Store(APP_TH_TEMPERATURE_MEASURED_VALUE_MEM_ID);
  }
}

#endif // APP_DEVICE_TYPE_THERMOSTAT

// eof thTemperatureMeasurementCluster.c
