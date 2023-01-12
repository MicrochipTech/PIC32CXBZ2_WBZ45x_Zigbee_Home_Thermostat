/*******************************************************************************
  Thermostat Occupancy Sensing cluster Header File

  Company:
    Microchip Technology Inc.

  File Name:
   thOccupancySensingCluster.h

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

#ifndef _THOCCUPANCYSENSINGCLUSTER_H
#define THOCCUPANCYSENSINGCLUSTER_H

/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcloccupancysensingcluster.h>
#include <z3device/clusters/include/haClusters.h>

/******************************************************************************
                    Definition(s) section
******************************************************************************/
#define OCCUPANCY_SENSING_VAL_MIN_REPORT_PERIOD 10
#define OCCUPANCY_SENSING_VAL_MAX_REPORT_PERIOD 80

/******************************************************************************
                    Externals
******************************************************************************/
extern ZCL_OccupancySensingClusterServerAttributes_t thOccupancySensingClusterServerAttributes;
extern ZCL_OccupancySensingClusterClientAttributes_t thOccupancySensingClusterClientAttributes;
/******************************************************************************
                    Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Initializes Occupancy Sensing cluster
******************************************************************************/
void thOccupancySensingClusterInit(void);

/**************************************************************************//**
\brief Toggles occupancy
******************************************************************************/
void occupancySensingToggleOccupancy(void);

/**************************************************************************//**
\brief Sets occupancy to Occupiad state or Unoccupiad state
******************************************************************************/
void occupancySensingInitiateSetOccupancyState(bool state);

/**************************************************************************//**
\brief Sets occupancysensingType attribute
******************************************************************************/
void occupancySensingSetSensorType(uint8_t sensorType);

/**************************************************************************//**
\brief Sets occupancysensingTypeBitmap attribute
******************************************************************************/
void occupancySensingSetSensorTypeBitmap(uint8_t sensorType);

#endif // _THOCCUPANCYSENSINGCLUSTER_H

// eof thOccupancySensingCluster.h
