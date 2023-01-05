/*******************************************************************************
  Thermostat Source File

  Company:
    Microchip Technology Inc.

  File Name:
    thermostat.c

  Summary:
    This file contains the Thermostat implementation.

  Description:
    This file contains the Thermostat implementation.
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
#include <z3device/thermostat/include/thClusters.h>
#include <z3device/clusters/include/basicCluster.h>
#include <zcl/clusters/include/identifyCluster.h>
#include <z3device/thermostat/include/thThermostatCluster.h>
#include <z3device/thermostat/include/thThermostatUiConfCluster.h>
#include <z3device/thermostat/include/thAlarmsCluster.h>
#include <z3device/common/include/z3Device.h>
#include <zcl/include/zclSecurityManager.h>
#include <zcl/include/zclCommandManager.h>
#include <app_zigbee/zigbee_console/console.h>
#include <pds/include/wlPdsMemIds.h>
#include <z3device/clusters/include/haClusters.h>
#include <z3device/common/include/otauService.h>
#include <z3device/thermostat/include/thOccupancySensingCluster.h>
#include <z3device/thermostat/include/thFanControlCluster.h>
#include <z3device/thermostat/include/thHumidityMeasurementCluster.h>
#include <z3device/thermostat/include/thGroupsCluster.h>
#include <z3device/thermostat/include/thScenesCluster.h>
#include <z3device/thermostat/include/thTemperatureMeasurementCluster.h>
#include <z3device/thermostat/include/thIdentifyCluster.h>
#include <z3device/thermostat/include/thBasicCluster.h>
#include <z3device/thermostat/include/thTimeCluster.h>
#ifdef OTAU_CLIENT
#include <zcl/include/zclOtauManager.h>
#endif

/******************************************************************************
                   Define(s) section
******************************************************************************/
#define UPDATING_PERIOD                    20000UL

/******************************************************************************
                   type(s) section
******************************************************************************/
/*******************************************************************************
                    Static functions section
*******************************************************************************/
static void updateSensorsAttributeValues(void);
static void thFindingBindingFinishedForACluster(Endpoint_t ResponentEp, ClusterId_t id);
static void thConfigureReportingResp(ZCL_Notify_t *ntfy);

#ifdef OTAU_CLIENT 
static void thermostatAddOTAUClientCluster(void);
static void configureImageKeyDone(void);
#endif
/******************************************************************************
                    Local variables section
******************************************************************************/
static ZCL_DeviceEndpoint_t thEndpoint =
{
  .simpleDescriptor =
  {
    .endpoint            = APP_ENDPOINT_THERMOSTAT,
    .AppProfileId        = PROFILE_ID_HOME_AUTOMATION,
    .AppDeviceId         = APP_Z3DEVICE_ID,
    .AppInClustersCount  = ARRAY_SIZE(thServerClusterIds),
    .AppInClustersList   = thServerClusterIds,
    .AppOutClustersCount = ARRAY_SIZE(thClientClusterIds),
    .AppOutClustersList  = thClientClusterIds,
  },
  .serverCluster = thServerClusters,
  .clientCluster = thClientClusters,
};

static ClusterId_t thClientClusterToBindIds[] =
{
  IDENTIFY_CLUSTER_ID,
  OCCUPANCY_SENSING_CLUSTER_ID,
  TIME_CLUSTER_ID,
  HUMIDITY_MEASUREMENT_CLUSTER_ID,
  TEMPERATURE_MEASUREMENT_CLUSTER_ID,
};

static ClusterId_t thServerClusterToBindIds[] =
{
  BASIC_CLUSTER_ID,
  IDENTIFY_CLUSTER_ID,
  THERMOSTAT_CLUSTER_ID,
  THERMOSTAT_UI_CONF_CLUSTER_ID,
  OCCUPANCY_SENSING_CLUSTER_ID,
  ALARMS_CLUSTER_ID,
  HUMIDITY_MEASUREMENT_CLUSTER_ID,
  FAN_CONTROL_CLUSTER_ID,
  GROUPS_CLUSTER_ID,
  SCENES_CLUSTER_ID,
  TEMPERATURE_MEASUREMENT_CLUSTER_ID,
};

static AppBindReq_t thBindReq =
{
  .srcEndpoint       = APP_ENDPOINT_THERMOSTAT,
  .remoteServersCnt  = ARRAY_SIZE(thClientClusterToBindIds),
  .remoteClientsCnt  = ARRAY_SIZE(thServerClusterToBindIds),
  .groupId           = 0xffff, 
  .remoteServers     = thClientClusterToBindIds,
  .remoteClients     = thServerClusterToBindIds,
  .callback          = thFindingBindingFinishedForACluster,
  .startIdentifyingFn= thIdetifyStartIdentifyingCb
};

static ZCL_LinkKeyDesc_t thermostatKeyDesc = {APS_UNIVERSAL_EXTENDED_ADDRESS  /*addr*/,
                                         HA_LINK_KEY /*key*/};

static HAL_AppTimer_t sensorAttributeUpdateTimer =
{
  .interval = UPDATING_PERIOD,
  .mode     = TIMER_REPEAT_MODE,
  .callback = updateSensorsAttributeValues,
};

/******************************************************************************
                    Implementation section
******************************************************************************/

/**************************************************************************//**
\brief Device initialization routine
******************************************************************************/
void appDeviceInit(void)
{
#if APP_ENABLE_CONSOLE == 1
  initConsole();
#endif
  if (!APP_RegisterEndpoint(&thEndpoint, &thBindReq))
   return;

  LCD_INIT();
  LCD_PRINT(0, 1, "Thermostat");
  ZCL_CommandManagerInit();
  thBasicClusterInit();
  thIdentifyClusterInit();
  thThermostatClusterInit();
  thAlarmsClusterInit();
  thThermostatUiConfClusterInit();
  thOccupancySensingClusterInit();
  thFanControlClusterInit();
  thHumidityMeasurementClusterInit();
  thGroupsClusterInit();
  thScenesClusterInit();
  thTemperatureMeasurementClusterInit();
  thTimeCluserInit();
  #ifdef OTAU_CLIENT
    thermostatAddOTAUClientCluster();
  #endif //OTAU_CLIENT
  if (PDS_IsAbleToRestore(Z3DEVICE_APP_MEMORY_MEM_ID))
    PDS_Restore(Z3DEVICE_APP_MEMORY_MEM_ID);

  /* Timer update the attribute values of various sensor types */
#if ZB_COMMISSIONING_ON_STARTUP == 1  
  HAL_StartAppTimer(&sensorAttributeUpdateTimer);
#endif  
}

/**************************************************************************//**
\breif Performs security initialization actions
******************************************************************************/
void appSecurityInit(void)
{
  ZCL_Set_t zclSet;

  ZCL_ResetSecurity();
  zclSet.attr.id = ZCL_LINK_KEY_DESC_ID;
  zclSet.attr.value.linkKeyDesc = &thermostatKeyDesc;
  ZCL_Set(&zclSet);
}

/**************************************************************************//**
\brief Callback about confguring image key on EEPROM
******************************************************************************/
#ifdef OTAU_CLIENT
static void configureImageKeyDone(void)
{
  return;
}
#endif

/**************************************************************************//**
\brief Device common task handler
******************************************************************************/
void appDeviceTaskHandler(void)
{
  switch (appDeviceState) // Actual device state when one joined network
  {
    case DEVICE_INITIAL_STATE:
      {
        appDeviceState = DEVICE_ACTIVE_IDLE_STATE;
      }
#ifdef OTAU_CLIENT
      startOtauClient();
#endif
      break;
    case DEVICE_ACTIVE_IDLE_STATE:
    default:
      break;
  }
}

/**************************************************************************//**
\brief Indication of configure reporting response

\param[in] resp - pointer to response
******************************************************************************/
static void thConfigureReportingResp(ZCL_Notify_t *ntfy)
{
#ifdef _ZCL_REPORTING_SUPPORT_
  ZCL_StartReporting();
#endif
  (void)ntfy;

}

#if ZB_COMMISSIONING_ON_STARTUP == 1  
/**************************************************************************//**
\brief Periodic update of various attributes of different sensors
*****************************************************************************/
static void updateSensorsAttributeValues(void)
{
//  humidityMeasurementUpdateMeasuredValue();
//  occupancySensingToggleOccupancy(); 
}
#endif

/*******************************************************************************
\brief callback called on the finishing of binding of one cluster
********************************************************************************/
static void thFindingBindingFinishedForACluster(Endpoint_t ResponentEp, ClusterId_t clusterId)
{
  ZCL_Cluster_t *serverCluster;
  switch(clusterId)
  {
    case THERMOSTAT_CLUSTER_ID:
      serverCluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, THERMOSTAT_CLUSTER_ID, ZCL_CLUSTER_SIDE_SERVER);
      if (serverCluster)
        sendConfigureReportingToNotify(APP_ENDPOINT_THERMOSTAT, 0, THERMOSTAT_CLUSTER_ID,
                                     ZCL_THERMOSTAT_CLUSTER_LOCAL_TEMPERATURE_SERVER_ATTRIBUTE_ID, THERMOSTAT_LOCAL_TEMPERATURE_MAX_REPORT_PERIOD, 
                                     thConfigureReportingResp);
      break;
    case OCCUPANCY_SENSING_CLUSTER_ID:
       serverCluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, OCCUPANCY_SENSING_CLUSTER_ID, ZCL_CLUSTER_SIDE_SERVER);
       if (serverCluster)
         sendConfigureReportingToNotify(APP_ENDPOINT_THERMOSTAT, 0, 
                                      OCCUPANCY_SENSING_CLUSTER_ID, ZCL_OCCUPANCY_SENSING_CLUSTER_OCCUPANCY_SERVER_ATTRIBUTE_ID, 
                                      OCCUPANCY_SENSING_VAL_MAX_REPORT_PERIOD, thConfigureReportingResp);
      break;
  case HUMIDITY_MEASUREMENT_CLUSTER_ID:
       serverCluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, HUMIDITY_MEASUREMENT_CLUSTER_ID, ZCL_CLUSTER_SIDE_SERVER);
       if (serverCluster)
         sendConfigureReportingToNotify(APP_ENDPOINT_THERMOSTAT, 0, 
                                      HUMIDITY_MEASUREMENT_CLUSTER_ID, ZCL_HUMIDITY_MEASUREMENT_CLUSTER_SERVER_MEASURED_VALUE_ATTRIBUTE_ID, 
                                      HUMIDITY_MEASUREMENT_VAL_MAX_REPORT_PERIOD, thConfigureReportingResp);
      break; 
  case  TEMPERATURE_MEASUREMENT_CLUSTER_ID:
       serverCluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, TEMPERATURE_MEASUREMENT_CLUSTER_ID, ZCL_CLUSTER_SIDE_SERVER);
       if (serverCluster)
         sendConfigureReportingToNotify(APP_ENDPOINT_THERMOSTAT, 0, 
                                      TEMPERATURE_MEASUREMENT_CLUSTER_ID, ZCL_TEMPERATURE_MEASUREMENT_CLUSTER_SERVER_MEASURED_VALUE_ATTRIBUTE_ID, 
                                      TEMPERATURE_MEASUREMENT_VAL_MAX_REPORT_PERIOD, thConfigureReportingResp);
      break;   
  }
}

/**************************************************************************//**
\brief Stops identifying on endpoints
******************************************************************************/
void appIdentifyStart(uint16_t identifyTime, bool colorEffect, uint16_t enhancedHue)
{
  thIdentifyStart(identifyTime);
  (void)colorEffect,(void)enhancedHue;
}

/**************************************************************************//**
\brief Stops identifying on endpoints
******************************************************************************/
void appIdentifyStop(void)
{
  thIdentifyStop();
}


#ifdef OTAU_CLIENT 
/**************************************************************************//**
\brief Adds OTAU client cluster to list of clients clusters of light devices
******************************************************************************/
static void thermostatAddOTAUClientCluster(void)
{
  thClientClusters[TH_CLIENT_CLUSTERS_COUNT - 1U] = ZCL_GetOtauClientCluster();
}
#endif
#endif // APP_DEVICE_TYPE_THERMOSTAT
// eof thermostat.c