/*******************************************************************************
  Thermostat Alarm cluster Header File

  Company:
    Microchip Technology Inc.

  File Name:
    thAlarmsCluster.h

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

#ifndef _THALARMSCLUSTER_H
#define _THALARMSCLUSTER_H

/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zclAlarmsCluster.h>

/******************************************************************************
                    Defines section
******************************************************************************/
#define ALARM_TABLE_SIZE 5

/******************************************************************************
                    External variables section
******************************************************************************/
extern ZCL_AlarmsClusterServerAttributes_t thAlarmsClusterServerAttributes;
extern ZCL_AlarmsClusterClientAttributes_t thAlarmsClusterClientAttributes;
extern PROGMEM_DECLARE (ZCL_AlarmsClusterServerCommands_t   thAlarmsClusterServerCommands);

/******************************************************************************
                    Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Initializes Identify cluster
******************************************************************************/
void thAlarmsClusterInit(void);

/**************************************************************************//**
\brief alarm notification raised by other clusters

\param[in] alarmCode - alarm code;
\param[in] clusterId - cluster identifier;
******************************************************************************/
void ZCL_AlarmNotification(ZCL_Alarm_t *alarm, Endpoint_t srcEndPoint, bool masked);

/**************************************************************************//**
\brief Add alarm entry

\param[in] alarmCode - alarm code;
\param[in] clusterId - cluster identifier;
******************************************************************************/
void addAlarmEntry(uint8_t alarmCode, ClusterId_t clusterId);

/**************************************************************************//**
\brief Removes the corresponding entry from the alarm log

\param[in] alarmCode - alarm code;
\param[in] clusterId - cluster identifier;
******************************************************************************/
void removeAlarmID(uint8_t alarmCode, ClusterId_t clusterId);



#endif // _THALARMSCLUSTER_H

// eof thAlarmsCluster.h
