/*******************************************************************************
  Thermostat console Source File

  Company:
    Microchip Technology Inc.

  File Name:
    thConsole.c

  Summary:
    This file contains the Thermostat console implementation.

  Description:
    This file contains the Thermostat console implementation.
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



/******************************************************************************
                    Includes section
******************************************************************************/
#include <app_zigbee/zigbee_console/console.h>
#include <bdb/include/bdbInstallCode.h>
#include <app_zigbee/zigbee_console/console.h>
#include <app_zigbee/zigbee_console/consoleCmds.h>
#include <systemenvironment/include/sysUtils.h>
#include <pds/include/wlPdsMemIds.h>
#include <zdo/include/zdo.h>
#include <bdb/include/bdb.h>
#include <z3device/common/include/z3Device.h>
#include <z3device/thermostat/include/thThermostatCluster.h>
#include <z3device/thermostat/include/thOccupancySensingCluster.h>
#include <z3device/thermostat/include/thHumidityMeasurementCluster.h>
#include <z3device/thermostat/include/thTemperatureMeasurementCluster.h>
#include <z3device/clusters/include/haClusters.h>
#include <z3device/thermostat/include/thClusters.h>
#include <z3device/thermostat/include/thAlarmsCluster.h>
#include <z3device/clusters/include/basicCluster.h>
#include <zcl/clusters/include/identifyCluster.h>
#include <z3device/clusters/include/onOffCluster.h>


#if (APP_Z3_DEVICE_TYPE == APP_DEVICE_TYPE_THERMOSTAT)
#if APP_ENABLE_CONSOLE == 1
/******************************************************************************
                    Defines section
******************************************************************************/
extern BDB_InvokeCommissioningReq_t AppbdbCommissioningreq;
/******************************************************************************
                    Prototypes section
******************************************************************************/
#if COMMISSIONING_COMMANDS_IN_CONSOLE == 1
static void processGetAppDeviceTypeCmd(const ScanValue_t *args);
static void processGetDeviceTypeCmd(const ScanValue_t *args);
#endif //#if COMMISSIONING_COMMANDS_IN_CONSOLE == 1

#if BDB_COMMANDS_IN_CONSOLE == 1
static void processSetInstallCodeCmd(const ScanValue_t *args);
static void processSetInstallCodeDeviceCmd(const ScanValue_t *args);
#endif // #if BDB_COMMANDS_IN_CONSOLE == 1

static void processSetOccupancyStateCmd(const ScanValue_t *args);
static void processSetSensorTypeCmd(const ScanValue_t *args);
static void processSetSensorTypeBitmapCmd(const ScanValue_t *args);
static void processSetOccupancyCmd(const ScanValue_t *args);
static void processAttrinitDefault(const ScanValue_t *args);

#if ZCL_COMMANDS_IN_CONSOLE == 1

#if ZLO_EXTRA_CLUSTERS_SUPPORT == 1
static void processResetToFactoryDefaultsCmd(const ScanValue_t *args);
#endif // ZLO_EXTRA_CLUSTERS_SUPPORT == 1
static void processIdentifyCmd(const ScanValue_t *args);
static void processIdentifyQueryCmd(const ScanValue_t *args);
static void processTriggerEffectCmd(const ScanValue_t *args);
#if ZB_COMMISSIONING_ON_STARTUP == 0 //Extra Cmds exposed for certification
static void processLocalTemperatureCmd(const ScanValue_t *args);
static void processOccupiedCoolingSetpointCmd(const ScanValue_t *args);
static void processOccupiedHeatingSetpointCmd(const ScanValue_t *args);
static void processAbsMinCoolSetpointLimitCmd(const ScanValue_t *args);
static void processPiCoolingDemandCmd(const ScanValue_t *args);
static void processPiHeatingDemandCmd(const ScanValue_t *args);
static void processTriggerAlarmCmd(const ScanValue_t *args);
static void processSetAlarmMaskCmd(const ScanValue_t *args);
static void processResetAlarms(const ScanValue_t *args);
#endif // ZB_COMMISSIONING_ON_STARTUP == 0
#endif // #if ZCL_COMMANDS_IN_CONSOLE == 1

/******************************************************************************
                    Local variables section
******************************************************************************/
const ConsoleCommand_t helpCmds[] =
{
  {"help", "",processHelpCmd, "->Show help you're reading now:  help\r\n"},
#if ZDO_COMMANDS_IN_CONSOLE == 1
  {"zdoHelp", "",processZdoHelpCmd, ""},
#endif
#if COMMISSIONING_COMMANDS_IN_CONSOLE == 1
  {"commissioningHelp", "",processCommissioningHelpCmd, "->Show commissioning help you're reading now:  help\r\n"},
#endif  
#if ZCL_COMMANDS_IN_CONSOLE == 1
  {"zclHelp", "",processZclHelpCmd, "->Show zcl help you're reading now:  help\r\n"},
#endif
#ifdef OTAU_CLIENT
  {"otauHelp", "",processOtauHelpCmd, "->Show OTAU help you're reading now:  help\r\n"},
#endif
   {0,0,0,0},
};

PROGMEM_DECLARE(ConsoleCommand_t commissioningHelpCmds)[]=
{
#if COMMISSIONING_COMMANDS_IN_CONSOLE == 1
  {"invokeCommissioning", "dd", processInvokeCommissioningCmd, "-> invokes commissioning [mode: 1-Touchlink: 2-Steering: 4-Formation: 8-F&B][gid]Invoke one Commissioning method at a time\r\n"}, 
  {"getAppDeviceType", "", processGetAppDeviceTypeCmd, "-> Request for device type: getAppDeviceType\r\n"},
  {"getDeviceType", "", processGetDeviceTypeCmd, "-> Request for device type: getDeviceType\r\n"},
  {"getExtAddr", "", processGetExtAddrCmd, "-> Gets ExtAddr: GetExtAddr\r\n"},
  {"getNetworkAddress", "", processGetNetworkAddressCmd, "-> Returns network address: getNetworkAddress\r\n"},
  {"getChannel", "", processGetChannelCmd, "-> Returns current channel: getChannel\r\n"},
  {"powerOff", "", processPseudoPowerOffCmd, "-> Powers off device: powerOff\r\n"},
  {"reset", "", processResetCmd, "->Reset device\r\n"},
  {"resetToFN", "", processResetToFactoryFreshCmd, "->Reset to factory fresh settings: resetToFN\r\n"},
  {"setExtAddr", "dd",processSetExtAddr, "->Set Ext Address:[upper][lower]\r\n"},
  {"setPrimaryChannelMask", "d", processSetPrimaryChannelMaskCmd, "-> Sets primary channel mask: setPrimaryChannelMask [mask]\r\n"},
  {"setSecondaryChannelMask", "d", processSetSecondaryChannelMaskCmd, "-> Sets secondary channel mask: setSecondaryChannelMask [mask]\r\n"},
  {"setcca", "dd", processSetCCAModeAndThresholdCmd, "-> Sets CCA Mode and Threshold [cca mode] [threshold]\r\n"},
  {"getcca", "", processGetCCAModeAndThresholdCmd, "-> Gets CCA Mode and Threshold\r\n"},
#if BDB_COMMANDS_IN_CONSOLE == 1
  {"formAndSteer", "", processFormAndSteerCmd, "-> forms network and steers\r\n"},
  {"formSteerAndFB", "", processFormSteerAndFBCmd, "-> forms network ,steers and FB\r\n"},
  {"setAllowSteal", "d", processSetAllowStealCmd, "-> Sets setAllowSteal: 0 or 1 <type>\r\n"},
  {"SetInstallCodeDevice", "s", processSetInstallCodeDeviceCmd, "-> Sets IC [code]\r\n"},
  {"SetInstallCode", "dds", processSetInstallCodeCmd, "-> Sets IC [extAddr][code]\r\n"},
#if BDB_TOUCHLINK_SUPPORT == 1
  {"setAllowTLResetToFN", "d", processSetAllowTLResetToFNCmd, "-> Sets setAllowTLResetToFN: 0 or 1 <type>\r\n"},
#if BDB_TOUCHLINK_INITIATOR_SUPPORT == 1
  {"SetTLRole", "d", processSetTLRole, "-> Sets TouchLink Role [initiator]\r\n"},
#endif
#endif  
#if ZB_COMMISSIONING_ON_STARTUP == 0
  {"SetFBRole", "d", processSetFBRole, "-> Sets FB Role [initiator]\r\n"},
#endif
  {"setTCLKExchangeMethod", "d", processSetTCLKExchangeMethodCmd, "-> Sets TCLK Exchange Method [method]\r\n"},
  {"setTCLKMaxRetryAttempts", "d", processSetTCLKMaxRetryAttemptsCmd, "-> Sets TCLK Max Retry Attempts [attempt]\r\n"},
  {"setGlobalKey", "d", processsetGlobalKeyCmd, "-> Sets Key for negative testing[Option]\r\n"},
  {"setPermitJoin", "d", processSetPermitJoinCmd, "-> Sets Permit Join: setPermitJoin [dur]\r\n"},
#endif
#endif
   {0,0,0,0},
};

PROGMEM_DECLARE(ConsoleCommand_t zclHelpCmds)[]=
{
#if ZCL_COMMANDS_IN_CONSOLE == 1 
  {"readAttribute", "sdddd", processReadAttrCmd, "->Read Attribute for specified cluster: readAttribute [addrMode][addr][ep][clusterId][attrId]\r\n"},
  {"writeAttribute", "sddddddd", processWriteAttrCmd, "->Write Attribute for specified cluster: writeAttribute [addrMode][addr][ep][clusterId][attrId][type][attrValue][attrSize]\r\n"},
  {"writeAttributeNoResp", "sddddddd", processWriteAttrNoRespCmd, "->Write Attribute No response for specified cluster: writeAttributeNoResp [addrMode][addr][ep][clusterId][attrId][type][attrValue][attrSize]\r\n"},
  {"configureReporting", "sddddddd", processConfigureReportingCmd, "->Sends configure reporting to specified cluster server: configureReporting [addrMode][addr][ep][clusterId][attrId][type][min][max]\r\n"},
  {"configureReportingWRC", "sdddddddd", processConfigureReportingWithReportableChangeCmd, "->Sends configure reporting with reportable Change to specified cluster server: configureReportingWRC [addrMode][addr][ep][clusterId][attrId][type][min][max][repChange]\r\n"},
  {"startReporting", "", processStartReportingCmd, "Triggers Reporting\r\n"},
  {"setOccupancyState", "d", processSetOccupancyStateCmd, "-> Sets the Occupancy state [state]- occupiad - 1,unoccupiad -0\r\n "},
  {"setOccupancySensorType", "d", processSetSensorTypeCmd, "-> Sets the OccupancySensor Type [sensorType]- PIR - 0,Ultrasonic-1, PIR + Ultrasonic-2, Physical Contact-3\r\n "},
  {"setOccupancySensorTypeBitmap", "d", processSetSensorTypeBitmapCmd, "-> Sets the OccupancySensor TypeBitmap [sensorType]- PIR - Bit0,Ultrasonic- Bit1,Physical Contact- Bit2\r\n "},
#if ZB_COMMISSIONING_ON_STARTUP == 0 //Extra Cmds exposed for certification
  {"setLocalTemperature", "d",processLocalTemperatureCmd, "->Set local temperature value [value]"}, 
  {"setOccupiedCoolingsp", "d",processOccupiedCoolingSetpointCmd, "->Set Occupied Cooling setpoint value [value]"},
  {"setOccupiedHeatingsp", "d",processOccupiedHeatingSetpointCmd, "->Set Occupied Heating setpoint value [value]"},
  {"setAbsMinCoolspLimit", "d",processAbsMinCoolSetpointLimitCmd, "->Set Absolute MinCoolSetpointLimit value [value]"},
  {"setPiCoolingDemand", "d",processPiCoolingDemandCmd, "->Set Pi CoolingDemaand value [value]"},
  {"setPiHeatingDemand", "d",processPiHeatingDemandCmd, "->Set Pi HeatingDemaand value [value]"},
  {"triggerAlarm","ddd", processTriggerAlarmCmd, "-> Triggers alarm condition: triggerAlarm [clusterId][alarmCode][0- inactive 1- set active]\r\n"},
  {"setAlarmMask","dd", processSetAlarmMaskCmd, "-> Set Alarm Mask: setAlarmMask [clusterId][alarmMask]\r\n"},
  {"resetAlarms", "", processResetAlarms, "-> Reset alarms\r\n"},
#endif // ZB_COMMISSIONING_ON_STARTUP == 0
  {"setOccupancy", "d", processSetOccupancyCmd, "-> Sets Occupancy [0- UnOccupied/1- Occupied]\r\n"},
  {"clusterAttrInitDefault", "d", processAttrinitDefault, "-> Initializes all attributes to default values [clusterID]\r\n"},
#if BDB_COMMANDS_IN_CONSOLE == 1
#if BDB_TOUCHLINK_SUPPORT == 1  
#if BDB_TOUCHLINK_INITIATOR_SUPPORT == 1
  {"resetTargetToFN", "", processTargetToFnCmd, "Reset device to FN: resetDeviceToFN\r\n"},
#endif // #if BDB_TOUCHLINK_INITIATOR_SUPPORT == 1
  {"setTargetType", "d", processSetTargetTypeCmd, "-> Sets target type: setTargetType <type>\r\n"},
#endif // #if BDB_TOUCHLINK_SUPPORT == 1
#endif // #if BDB_COMMANDS_IN_CONSOLE == 1 
#if ZLO_EXTRA_CLUSTERS_SUPPORT == 1
  {"resetToFactoryDefaults", "sdd", processResetToFactoryDefaultsCmd, "-> reset all cluster attributes to factory defaults command:resetToFactoryDefaults [addrMode][addr][ep]\r\n"},
#endif // #if ZLO_EXTRA_CLUSTERS_SUPPORT == 1
  {"identify", "sddd", processIdentifyCmd, "->Send Identify command: identify [addrMode][addr][ep][idTime]\r\n"},
  {"identifyQuery", "sdd", processIdentifyQueryCmd, "->Send Identify Query command: identifyQuery [addrMode][addr][ep]\r\n"},
  {"triggerEffect", "sdddd", processTriggerEffectCmd, "->Send TriggerEffect command: triggerEffect [addrMode][addr][ep][effectId][effectVariant]"},
#endif // #if ZCL_COMMANDS_IN_CONSOLE == 1
  {0,0,0,0},
};

/******************************************************************************
                   Global variables section
******************************************************************************/
bool fbRole = false;
ScanValue_t local;
Endpoint_t srcEp = APP_ENDPOINT_THERMOSTAT;

/******************************************************************************
                    Implementation section
******************************************************************************/

#if COMMISSIONING_COMMANDS_IN_CONSOLE == 1
/**************************************************************************//**
\brief Processes request for device type obtaining

\param[in] args - array of command arguments
******************************************************************************/
static void processGetDeviceTypeCmd(const ScanValue_t *args)
{
  appSnprintf("DeviceType = %d\r\n", TEST_DEVICE_TYPE_ZIGBEE_ROUTER);
  (void)args;
}

/**************************************************************************//**
\brief Processes request for HA device type obtaining

\param[in] args - array of command arguments
+******************************************************************************/
static void processGetAppDeviceTypeCmd(const ScanValue_t *args)
{
  appSnprintf("Z3DeviceType = 0x%04x\r\n", APP_Z3DEVICE_ID);
  (void)args;
}
#endif //#if COMMISSIONING_COMMANDS_IN_CONSOLE == 1
#if ZCL_COMMANDS_IN_CONSOLE == 1 
/**************************************************************************//**
\brief Processes Set Occupancy command

\param[in] args - array of command arguments
******************************************************************************/
static void processSetOccupancyCmd(const ScanValue_t *args)
{ 
  if (ZCL_SUCCESS_STATUS == thermostatSetOccupancy((ZCL_ThOccupancy_t)args[0].uint8))
    appSnprintf("Occupancy set to :%d\r\n",(ZCL_ThOccupancy_t)args[0].uint8);
  else
    appSnprintf("Occupancy not set: Invalid value\r\n");
}

/**************************************************************************//**
\brief Processes SetOccupancyState command

\param[in] args - array of command arguments
******************************************************************************/
static void processSetOccupancyStateCmd(const ScanValue_t *args)
{
  occupancySensingInitiateSetOccupancyState(args[0].uint8);
}

/**************************************************************************//**
\brief Processes SetOccupancySensorType command

\param[in] args - array of command arguments
******************************************************************************/
static void processSetSensorTypeCmd(const ScanValue_t *args)
{
  occupancySensingSetSensorType(args[0].uint8);
}


/**************************************************************************//**
\brief Processes SetOccupancySensorType command

\param[in] args - array of command arguments
******************************************************************************/
static void processSetSensorTypeBitmapCmd(const ScanValue_t *args)
{
  occupancySensingSetSensorTypeBitmap(args[0].uint8);
}


/**************************************************************************//**
\brief Processes cluster attributes initialization to default

\param[in] args - array of command arguments
******************************************************************************/
static void processAttrinitDefault(const ScanValue_t *args)
{ 
  if(THERMOSTAT_CLUSTER_ID == args[0].uint16)
    thermostatClusterInitAttributes();
  else
    appSnprintf("Invalid Cluster ID\r\n");
}

/*****************************************************************************/
//                      BASIC CLUSTER COMMANDS
/*****************************************************************************/
#if ZLO_EXTRA_CLUSTERS_SUPPORT == 1

/**************************************************************************//**
\brief Processes reset To factory defaults command

\param[in] args - array of command arguments
******************************************************************************/
static void processResetToFactoryDefaultsCmd(const ScanValue_t *args)
{
  basicResetToFactoryDefaultsCommand(determineAddressMode(args), args[1].uint16,
    args[2].uint8, srcEp);
}

#endif // #if ZLO_EXTRA_CLUSTERS_SUPPORT == 1

#endif // #if ZCL_COMMANDS_IN_CONSOLE == 1 

#if BDB_COMMANDS_IN_CONSOLE == 1
/**************************************************************************//**
\brief InstallCode callback

\param[in] status - status of action
******************************************************************************/
void myICCallback(InstallCode_Configuration_Status_t status)
{
  appSnprintf("Status = %d\r\n", status);
}

/**************************************************************************//**
\brief Processes InstallCode command

\param[in] args - array of command arguments
******************************************************************************/
void processSetInstallCodeCmd(const ScanValue_t *args)
{
  ExtAddr_t devAddr = 0xFFFFFFFFFFFFFFFF;
  uint8_t icode[18];
  hexStrTouint8array(args[0].str, icode, 18U);
  BDB_ConfigureInstallCode(devAddr, icode, myICCallback);
  (void)args;
}

/**************************************************************************//**
\brief Processes InstallCode command

\param[in] args - array of command arguments
******************************************************************************/
void processSetInstallCodeDeviceCmd(const ScanValue_t *args)
{
  ExtAddr_t devAddr = 0xFFFFFFFFFFFFFFFF;
  uint8_t icode[18];
  hexStrTouint8array(args[0].str, icode, 18U);
  BDB_ConfigureInstallCode(devAddr, icode, myICCallback);
  (void)args;
}

#endif // #if BDB_COMMANDS_IN_CONSOLE == 1

#if ZCL_COMMANDS_IN_CONSOLE == 1  
/**************************************************************************//**
\brief Processes Identify command

\param[in] args - array of command arguments
******************************************************************************/
static void processIdentifyCmd(const ScanValue_t *args)
{
  identifySendIdentify(determineAddressMode(args), args[1].uint16, args[2].uint8, srcEp,
    args[3].uint16);
}

/**************************************************************************//**
\brief Processes Identify Query command

\param[in] args - array of command arguments
******************************************************************************/
static void processIdentifyQueryCmd(const ScanValue_t *args)
{
  identifySendIdentifyQuery(determineAddressMode(args), args[1].uint16, args[2].uint8, srcEp);
}

/**************************************************************************//**
\brief Processes Trigger Effect command

\param[in] args - array of command arguments
******************************************************************************/
static void processTriggerEffectCmd(const ScanValue_t *args)
{
  identifySendTriggerEffect(determineAddressMode(args), args[1].uint16, args[2].uint8, srcEp,
    args[3].uint8, args[4].uint8);
}
#if ZB_COMMISSIONING_ON_STARTUP == 0
/**************************************************************************//**
\brief Processes Trigger Alarm command

\param[in] args - array of command arguments
******************************************************************************/
static void processTriggerAlarmCmd(const ScanValue_t *args)
{
  // alarmCode,  clusterId,  alarmActive
  ZCL_Alarm_t alarm;

  thSetAlarmActive((ZclThermostatAlarmCode_t)args[1].uint8,(bool)args[2].uint8);
  alarm.alarmCode = args[1].uint8;  // ALarm code
  alarm.clusterIdentifier = args[0].uint16; // Cluster ID

  ZCL_AlarmNotification(&alarm, srcEp, isAlarmMasked((ZclThermostatAlarmCode_t)alarm.alarmCode));
}

/**************************************************************************//**
\brief Processes set alarm mask command

\param[in] args - array of command arguments
******************************************************************************/
static void processSetAlarmMaskCmd(const ScanValue_t *args)
{
  if (THERMOSTAT_CLUSTER_ID == args[0].uint16)
    thermostatSetAlarmMaskAttr(args[1].uint8);
}

/**************************************************************************//**
\brief Processes adding alarms to device

\param[in] args - array of command arguments
******************************************************************************/
static void processResetAlarms(const ScanValue_t *args)
{
  thAlarmsClusterServerAttributes.alarmCount.value = 0;
}

/**************************************************************************//**
\brief Processes Local temperature command

\param[in] args - array of command arguments
******************************************************************************/
static void processLocalTemperatureCmd(const ScanValue_t *args)
{
  setLocalTemperature(args[0].int16); 
}

/**************************************************************************//**
\brief Processes Occupied Cooling Setpoint command

\param[in] args - array of command arguments
******************************************************************************/
static void processOccupiedCoolingSetpointCmd(const ScanValue_t *args)
{
  setOccupiedCoolingSp(args[0].int16); 
}

/**************************************************************************//**
\brief Processes Occupied Heatinf Setpoint command

\param[in] args - array of command arguments
******************************************************************************/
static void processOccupiedHeatingSetpointCmd(const ScanValue_t *args)
{
  setOccupiedHeatingSp(args[0].int16); 
}

/**************************************************************************//**
\brief Processes Occupied Cooling Setpoint command

\param[in] args - array of command arguments
******************************************************************************/
static void processAbsMinCoolSetpointLimitCmd(const ScanValue_t *args)
{
  setAbsMinCoolSetpointLimit(args[0].int16); 
}

/**************************************************************************//**
\brief Processes PI Cooling Demand command

\param[in] args - array of command arguments
******************************************************************************/
static void processPiCoolingDemandCmd(const ScanValue_t *args)
{
  setPiCoolingDemand(args[0].uint8); 
}

/**************************************************************************//**
\brief Processes PI Heating Demand command

\param[in] args - array of command arguments
******************************************************************************/
static void processPiHeatingDemandCmd(const ScanValue_t *args)
{
  setPiHeatingDemand(args[0].uint8); 
}
#endif // ZB_COMMISSIONING_ON_STARTUP == 0
#endif // ZCL_COMMANDS_IN_CONSOLE == 1 

#endif // APP_ENABLE_CONSOLE == 1

#endif // APP_DEVICE_TYPE_THERMOSTAT

// eof thConsole.c
