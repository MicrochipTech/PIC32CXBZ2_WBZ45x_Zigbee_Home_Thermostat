/*******************************************************************************
  Thermostat Basic cluster Header File

  Company:
    Microchip Technology Inc.

  File Name:
    thBasicCluster.h

  Summary:
    This file contains Thermostat Basic cluster interface.

  Description:
    This file contains Thermostat Basic cluster interface.
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

#ifndef _THBASICCLUSTER_H
#define _THBASICCLUSTER_H

/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zclZllBasicCluster.h>

/******************************************************************************
                    External variables section
******************************************************************************/
extern ZCL_BasicClusterServerAttributes_t thBasicClusterServerAttributes;
extern ZCL_BasicClusterClientAttributes_t thBasicClusterClientAttributes;
extern PROGMEM_DECLARE (ZCL_BasicClusterServerCommands_t thBasicClusterServerCommands);

/******************************************************************************
                    Prototypes
******************************************************************************/
/**************************************************************************//**
/brief Initializes Basic cluster
******************************************************************************/
void thBasicClusterInit(void);

/**************************************************************************//**
\brief Sends resetToFactoryDefaults command unicastly

\param[in] mode - address mode;
\param[in] addr - short address of destination node;
\param[in] ep   - destination endpoint;
******************************************************************************/
void basicResetToFactoryDefaultsCommand(APS_AddrMode_t mode,ShortAddr_t addr, Endpoint_t ep, Endpoint_t srcEp);

#endif // _THBASICCLUSTER_H

// eof thBasicCluster.h