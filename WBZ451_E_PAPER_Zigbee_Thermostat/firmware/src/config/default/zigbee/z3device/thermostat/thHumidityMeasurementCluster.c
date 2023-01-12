/*******************************************************************************
  Thermostat Relative Humidity Measurement cluster Source File

  Company:
    Microchip Technology Inc.

  File Name:
    thHumidityMeasurementCluster.c

  Summary:
    This file contains the Thermostat Relative Humidity Measurement cluster interface.

  Description:
    This file contains the Thermostat Relative Humidity Measurement cluster interface.
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
#include <z3device/clusters/include/haClusters.h>
#include <z3device/thermostat/include/thHumidityMeasurementCluster.h>
#include <pds/include/wlPdsMemIds.h>
#include <z3device/common/include/z3Device.h>
#include <app_zigbee/zigbee_console/console.h>
#include <zcl/include/zclCommandManager.h>
#include <app_zigbee/app_zigbee_handler.h>

/******************************************************************************
                    Global variables
******************************************************************************/
ZCL_HumidityMeasurementClusterServerAttributes_t thHumidityMeasurementClusterServerAttributes =
{
  ZCL_DEFINE_HUMIDITY_MEASUREMENT_CLUSTER_SERVER_ATTRIBUTES(HUMIDITY_MEASUREMENT_VAL_MIN_REPORT_PERIOD, HUMIDITY_MEASUREMENT_VAL_MAX_REPORT_PERIOD)
};

ZCL_HumidityMeasurementClusterClientAttributes_t thHumidityMeasurementClusterClientAttributes =
{
  ZCL_DEFINE_HUMIDITY_MEASUREMENT_CLUSTER_CLIENT_ATTRIBUTES()
};

/******************************************************************************
                    Prototypes section
******************************************************************************/
static void thHumidityMeasurementReportInd(ZCL_Addressing_t *addressing, uint8_t reportLength, uint8_t *reportPayload);
static void thHumidityAttributeEventInd(ZCL_Addressing_t *addressing, ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event);
/******************************************************************************
                    Implementation section
******************************************************************************/
/**************************************************************************//**
\brief Initializes Humidity Measurement cluster
******************************************************************************/
void thHumidityMeasurementClusterInit(void)
{
  ZCL_Cluster_t *cluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, HUMIDITY_MEASUREMENT_CLUSTER_ID, ZCL_CLUSTER_SIDE_CLIENT);

  if (cluster)
  {
    thHumidityMeasurementClusterClientAttributes.clusterVersion.value = HUMIDITY_MEASUREMENT_CLUSTER_VERSION;
    cluster->ZCL_ReportInd = thHumidityMeasurementReportInd;
    cluster->ZCL_DefaultRespInd = ZCL_CommandZclDefaultResp;
  }
  cluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, HUMIDITY_MEASUREMENT_CLUSTER_ID, ZCL_CLUSTER_SIDE_SERVER);

  if (cluster)
  {
    thHumidityMeasurementClusterServerAttributes.clusterVersion.value = HUMIDITY_MEASUREMENT_CLUSTER_VERSION;
    thHumidityMeasurementClusterServerAttributes.measuredValue.value = APP_HUMIDITY_MEASUREMENT_MEASURED_VALUE_ATTRIBUTE_VALUE;
    thHumidityMeasurementClusterServerAttributes.minMeasuredValue.value = APP_HUMIDITY_MEASUREMENT_MIN_MEASURED_VALUE_ATTRIBUTE_VALUE;
    thHumidityMeasurementClusterServerAttributes.maxMeasuredValue.value = APP_HUMIDITY_MEASUREMENT_MAX_MEASURED_VALUE_ATTRIBUTE_VALUE;
    thHumidityMeasurementClusterServerAttributes.tolerance.value = APP_HUMIDITY_MEASUREMENT_TOLERANCE_ATTRIBUTE_VALUE;//to be updated
    ZCL_ReportOnChangeIfNeeded(&thHumidityMeasurementClusterServerAttributes.measuredValue);
    cluster->ZCL_AttributeEventInd = thHumidityAttributeEventInd;
  }

  if (PDS_IsAbleToRestore(APP_TH_HUMIDITY_MEASURED_VALUE_MEM_ID))
    PDS_Restore(APP_TH_HUMIDITY_MEASURED_VALUE_MEM_ID);

}

/**************************************************************************//**
\brief Update the measured Value
******************************************************************************/
void humidityMeasurementUpdateMeasuredValue(void)
{
  uint16_t measuredValue ;

  measuredValue = thHumidityMeasurementClusterServerAttributes.measuredValue.value + APP_HUMIDITY_MEASUREMENT_MEASURED_VALUE_PERIODIC_CHANGE;

  if (measuredValue > thHumidityMeasurementClusterServerAttributes.maxMeasuredValue.value)
    thHumidityMeasurementClusterServerAttributes.measuredValue.value =  thHumidityMeasurementClusterServerAttributes.minMeasuredValue.value;
  else
    thHumidityMeasurementClusterServerAttributes.measuredValue.value = measuredValue;

  PDS_Store(APP_TH_HUMIDITY_MEASURED_VALUE_MEM_ID);
#ifdef _ZCL_REPORTING_SUPPORT_
  ZCL_ReportOnChangeIfNeeded(&thHumidityMeasurementClusterServerAttributes.measuredValue);
#endif
}

/**************************************************************************//**
\brief Report attribute indication handler

\param[in] addressing - pointer to addressing information;
\param[in] reportLength - data payload length;
\param[in] reportPayload - data pointer
******************************************************************************/
static void thHumidityMeasurementReportInd(ZCL_Addressing_t *addressing, uint8_t reportLength, uint8_t *reportPayload)
{
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_REPORTING_HUMIDITY_MEASUREMENT;
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
static void thHumidityAttributeEventInd(ZCL_Addressing_t *addressing, ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event)
{
  APP_Zigbee_Event_t eventItem;
  eventItem.eventGroup = EVENT_CLUSTER;
  eventItem.eventId = CMD_ZCL_ATTR_HUMIDITY_MEASUREMENT;
  eventItem.eventData.zclAttributeData.addressing = addressing;
  eventItem.eventData.zclAttributeData.attributeId = attributeId;
  eventItem.eventData.zclAttributeData.event = event;

  APP_Zigbee_Handler(eventItem);
  if (((ZCL_CONFIGURE_ATTRIBUTE_REPORTING_EVENT == event) || \
       (ZCL_CONFIGURE_DEFAULT_ATTRIBUTE_REPORTING_EVENT == event)) && \
      (ZCL_HUMIDITY_MEASUREMENT_CLUSTER_SERVER_MEASURED_VALUE_ATTRIBUTE_ID == attributeId))      
  {
    PDS_Store(APP_TH_HUMIDITY_MEASURED_VALUE_MEM_ID);
  }

}
#endif // APP_DEVICE_TYPE_THERMOSTAT

// eof thHumidityMeasuementCluster.c
