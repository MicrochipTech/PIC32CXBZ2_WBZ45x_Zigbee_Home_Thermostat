/*******************************************************************************
  Thermostat Ui conf cluster Source File

  Company:
    Microchip Technology Inc.

  File Name:
    thThermostatUiConfCluster.c

  Summary:
    This file contains the Thermostat Ui conf cluster interface.
	
  Description:
    This file contains the Thermostat Ui conf cluster interface.
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
#include <z3device/thermostat/include/thThermostatUiConfCluster.h>
#include <z3device/thermostat/include/thThermostatCluster.h>
#include <zcl/include/zclCommandManager.h>
#include <app_zigbee/zigbee_console/console.h>
#include <z3device/common/include/z3Device.h>
#include <app_zigbee/app_zigbee_handler.h>
#include "click_routines/eink_bundle/eink_bundle.h"
#include "click_routines/eink_bundle/eink_bundle_image.h"
#include "click_routines/eink_bundle/eink_bundle_font.h"
/*******************************************************************************
                             Defines section
*******************************************************************************/
/*******************************************************************************
                    Static functions prototypes section
*******************************************************************************/
static void ZCL_ThermostatUiConfAttributeEventInd(ZCL_Addressing_t *addressing, ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event);

/******************************************************************************
                    Global variables
******************************************************************************/
ZCL_ThermostatUiConfClusterServerAttributes_t thThermostatUiConfClusterServerAttributes =
{
  ZCL_DEFINE_THERMOSTAT_UI_CONF_CLUSTER_SERVER_ATTRIBUTES()
};

/******************************************************************************
                    Implementation section
******************************************************************************/
/**************************************************************************//**
\brief Initializes thermostat ui conf cluster
******************************************************************************/
void thThermostatUiConfClusterInit(void)
{
  ZCL_Cluster_t *cluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, THERMOSTAT_UI_CONF_CLUSTER_ID, ZCL_CLUSTER_SIDE_SERVER);

  if (cluster)
    cluster->ZCL_AttributeEventInd = ZCL_ThermostatUiConfAttributeEventInd;

  thThermostatUiConfClusterServerAttributes.temperatureDisplayMode.value = ZCL_TEMPERATURE_IN_CELSIUS;
  thThermostatUiConfClusterServerAttributes.keypadLockOut.value = ZCL_NO_LOCKOUT;
  thThermostatUiConfClusterServerAttributes.scheduleProgVisibility.value = ZCL_ENABLE_LOCAL_SCHEDULE_PROGRAMMING;
  thThermostatUiConfClusterServerAttributes.clusterVersion.value = THERMOSTAT_UI_CONF_CLUSTER_VERSION;

  // Enable LCD and Display local Temparature on it
//  LCD_PRINT(0,10,"Temp = %d deg C",thThermostatClusterServerAttributes.localTemperature.value); 
}

static void ZCL_ThermostatUiConfAttributeEventInd(ZCL_Addressing_t *addressing, ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event)
{
  APP_Zigbee_Event_t eventItem;
  eventItem.eventGroup = EVENT_CLUSTER;
  eventItem.eventId = CMD_ZCL_ATTR_THERMOSTAT_UI_CONF;
  eventItem.eventData.zclAttributeData.addressing = addressing;
  eventItem.eventData.zclAttributeData.attributeId = attributeId;
  eventItem.eventData.zclAttributeData.event = event;
  APP_Zigbee_Handler(eventItem);
}

/**************************************************************************//** 
* Function : translateZclTemp() 
* Description : Converts the temperature setpoints in ZCLto the half degF format. 
* The half degF format is 8-bit unsigned, and represents 2x temperature value in 
* Farenheit (to get 0.5 resolution).The format used in ZCL is 16-bit signed 
* in Celsius and multiplied by 100S to get 0.01 resolution. 
* e.g. 2500(25.00 deg C) ---> 0x9A (77 deg F) 
* Input Para : Temperature in ZCL (degC)format 
* Output Para: Temperature in half DegF format 
******************************************************************************/
int8_t thTranslateZclCelciusTemptoFahr(int16_t temperature) 
{ 
int32_t x = temperature; 
//rearrangement of 
// = (x * (9/5) / 100 + 32); 
// the added 250 is for proper rounding. 
// a rounding technique that only works 
// with positive numbers 

return (int8_t) ((x*9 + 250)/ (5*100) + 32); 
} 

#endif // APP_DEVICE_TYPE_THERMOSTAT
// eof thThermostatUiConfCluster.c
