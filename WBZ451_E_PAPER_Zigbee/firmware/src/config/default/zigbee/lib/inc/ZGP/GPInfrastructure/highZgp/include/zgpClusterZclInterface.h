/*******************************************************************************
  Zigbee green power cluster zcl interface Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpClusterZclInterface.h

  Summary:
    This file contains the green power cluster generic feature interface.

  Description:
    This file contains the green power cluster generic feature interface.
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

#ifndef _ZGPCLUSTERZCLINTERFACE_H
#define _ZGPCLUSTERZCLINTERFACE_H

#ifdef _GREENPOWER_SUPPORT_
/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcl.h>
#include <zcl/include/zclGreenPowerCluster.h>

/******************************************************************************
                    Externals
******************************************************************************/
extern const ZCL_GreenPowerClusterCommands_t zgpClusterClientCommands;
extern ZCL_GreenPowerClusterClientAttributes_t zgpClusterClientAttributes;
extern ClusterId_t zgpClientClusterIds[];
extern ZCL_Cluster_t zgpClientClusters[1];

#if APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
extern const ZCL_GreenPowerClusterCommands_t zgpClusterServerCommands;
extern ZCL_GreenPowerClusterServerAttributes_t zgpClusterServerAttributes;
extern ClusterId_t zgpServerClusterIds[];
extern ZCL_Cluster_t zgpServerClusters[1];
#endif // APP_ZGP_DEVICE_TYPE == APP_ZGP_DEVICE_TYPE_COMBO_BASIC
/******************************************************************************
                    Prototypes
******************************************************************************/


#endif // _GREENPOWER_SUPPORT_
#endif // _ZGPCLUSTER_H

// eof zgpClusterZclInterface.h
