/*******************************************************************************
  Thermostat Relative Humidity Measurement cluster Header File

  Company:
    Microchip Technology Inc.

  File Name:
    thHumidityMeasurementCluster.h

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

#ifndef _THHUMIDITYMEASUREMENTCLUSTER_H
#define _THHUMIDITYMEASUREMENTCLUSTER_H

/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zclHumidityMeasurementCluster.h>
#include <z3device/clusters/include/haClusters.h>

/******************************************************************************
                    Definition(s) section
******************************************************************************/
#define HUMIDITY_MEASUREMENT_VAL_MIN_REPORT_PERIOD 25
#define HUMIDITY_MEASUREMENT_VAL_MAX_REPORT_PERIOD 95

/* Some default values for attributes */
#define APP_HUMIDITY_MEASUREMENT_MEASURED_VALUE_ATTRIBUTE_VALUE       0x1000
#define APP_HUMIDITY_MEASUREMENT_MIN_MEASURED_VALUE_ATTRIBUTE_VALUE   0x0000 //range 0x0 ? 0x270f
#define APP_HUMIDITY_MEASUREMENT_MAX_MEASURED_VALUE_ATTRIBUTE_VALUE   0x2710 //range 0x954e ? 0x2710
#define APP_HUMIDITY_MEASUREMENT_TOLERANCE_ATTRIBUTE_VALUE            0x0100 //0x0000 ? 0x0800

#define APP_HUMIDITY_MEASUREMENT_MEASURED_VALUE_PERIODIC_CHANGE       1000 //coresponding to 10%
/******************************************************************************
                    Externals
******************************************************************************/
extern ZCL_HumidityMeasurementClusterServerAttributes_t thHumidityMeasurementClusterServerAttributes;
extern ZCL_HumidityMeasurementClusterClientAttributes_t thHumidityMeasurementClusterClientAttributes;

/******************************************************************************
                    Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Initializes Humidity Measurement cluster
******************************************************************************/
void thHumidityMeasurementClusterInit(void);

/**************************************************************************//**
\brief Update the measured Value
******************************************************************************/
void humidityMeasurementUpdateMeasuredValue(void);

#endif // _THHUMIDITYMEASUREMENTCLUSTER_H

// eof thHumidityMeasurementCluster.h