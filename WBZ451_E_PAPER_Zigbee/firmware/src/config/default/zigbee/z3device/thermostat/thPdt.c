/*******************************************************************************
  Thermostat Persistent Data Table Source File

  Company:
    Microchip Technology Inc.

  File Name:
    thPdt.c

  Summary:
    This file contains the Thermostat Persistent Data Table implementation.

  Description:
    This file contains the Thermostat Persistent Data Table implementation.
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
#include <pds/include/wlPdsMemIds.h>
#include <z3device/thermostat/include/thThermostatCluster.h>
#include <z3device/thermostat/include/thOccupancySensingCluster.h>
#include <z3device/thermostat/include/thHumidityMeasurementCluster.h>
#include <z3device/thermostat/include/thTemperatureMeasurementCluster.h>
#include <z3device/thermostat/include/thScenesCluster.h>
#include <z3device/common/include/appConsts.h>

/******************************************************************************
                    PDT definitions
******************************************************************************/
#ifdef _ENABLE_PERSISTENT_SERVER_
/* Thermostat application data file descriptors.
   Shall be placed in the PDS_FF code segment. */
PDS_DECLARE_FILE(APP_TH_LOCAL_TEMPERATURE_MEM_ID,                 sizeof(thThermostatClusterServerAttributes.localTemperature),               &thThermostatClusterServerAttributes.localTemperature,              NO_FILE_MARKS);
PDS_DECLARE_FILE(APP_TH_OCCUPIED_COOLING_SETPOINT_MEM_ID,         sizeof(thThermostatClusterServerAttributes.occupiedCoolingSetpoint),        &thThermostatClusterServerAttributes.occupiedCoolingSetpoint,       NO_FILE_MARKS);
PDS_DECLARE_FILE(APP_TH_OCCUPIED_HEATING_SETPOINT_MEM_ID,         sizeof(thThermostatClusterServerAttributes.occupiedHeatingSetpoint),        &thThermostatClusterServerAttributes.occupiedHeatingSetpoint,       NO_FILE_MARKS);
PDS_DECLARE_FILE(APP_TH_OCCUPANCY_MEM_ID,                         sizeof(thOccupancySensingClusterServerAttributes.occupancy),                &thOccupancySensingClusterServerAttributes.occupancy,               NO_FILE_MARKS);
PDS_DECLARE_FILE(APP_TH_HUMIDITY_MEASURED_VALUE_MEM_ID,           sizeof(thHumidityMeasurementClusterServerAttributes.measuredValue),         &thHumidityMeasurementClusterServerAttributes.measuredValue,        NO_FILE_MARKS);
PDS_DECLARE_FILE(APP_TH_SCENES_MEM_ID,                            MAX_SCENES_AMOUNT * sizeof(Scene_t),                                        thermostatSceneTable,                                               NO_FILE_MARKS);
PDS_DECLARE_FILE(APP_TH_TEMPERATURE_MEASURED_VALUE_MEM_ID,        sizeof(thTemperatureMeasurementClusterServerAttributes.measuredValue),      &thTemperatureMeasurementClusterServerAttributes.measuredValue,     NO_FILE_MARKS);
PDS_DECLARE_FILE(APP_TH_PICOOLING_DEMAND_MEM_ID,                  sizeof(thThermostatClusterServerAttributes.PICoolingDemand),          &thThermostatClusterServerAttributes.PICoolingDemand,         NO_FILE_MARKS);
PDS_DECLARE_FILE(APP_TH_PIHEATING_DEMAND_MEM_ID,                  sizeof(thThermostatClusterServerAttributes.PIHeatingDemand),          &thThermostatClusterServerAttributes.PIHeatingDemand,         NO_FILE_MARKS);

/* Thermostat application data file identifiers list.
   Will be placed in flash. */
PROGMEM_DECLARE(PDS_MemId_t appThMemoryIdsTable[]) =
{
  APP_TH_LOCAL_TEMPERATURE_MEM_ID,
  APP_TH_OCCUPIED_COOLING_SETPOINT_MEM_ID,
  APP_TH_OCCUPIED_HEATING_SETPOINT_MEM_ID,
  APP_TH_OCCUPANCY_MEM_ID,
  APP_TH_HUMIDITY_MEASURED_VALUE_MEM_ID,
  APP_TH_SCENES_MEM_ID,
  APP_TH_TEMPERATURE_MEASURED_VALUE_MEM_ID,
  APP_TH_PICOOLING_DEMAND_MEM_ID,
  APP_TH_PIHEATING_DEMAND_MEM_ID
};

/* Thermostat application directory descriptor.
   Shall be placed in the PDS_FD code segment. */
PDS_DECLARE_DIR(PDS_DirDescr_t appEsiMemoryDirDescr) =
{
  .list       = appThMemoryIdsTable,
  .filesCount = ARRAY_SIZE(appThMemoryIdsTable),
  .memoryId   = Z3DEVICE_APP_MEMORY_MEM_ID
};

#endif // _ENABLE_PERSISTENT_SERVER_
#endif // APP_DEVICE_TYPE_THERMOSTAT

// eof thPdt.c
