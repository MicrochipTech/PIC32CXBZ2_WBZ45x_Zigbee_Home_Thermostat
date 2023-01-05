/*******************************************************************************
  Thermostat clusters Header File

  Company:
    Microchip Technology Inc.

  File Name:
   thClusters.h

  Summary:
    This file contains the Thermostat clusters interface.

  Description:
    This file contains the Thermostat clusters interface.
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

#ifndef _THCLUSTERS_H
#define _THCLUSTERS_H

/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zcl.h>

/******************************************************************************
                    Definitions section
******************************************************************************/
#define TH_SERVER_CLUSTERS_COUNT     11
#ifdef OTAU_CLIENT
  #define TH_CLIENT_CLUSTERS_COUNT   8
#else
  #define TH_CLIENT_CLUSTERS_COUNT   7 
#endif
#define TH_CLIENT_CLUSTER_INIT_COUNT TH_CLIENT_CLUSTERS_COUNT
#define TH_SERVER_CLUSTER_INIT_COUNT TH_SERVER_CLUSTERS_COUNT

/******************************************************************************
                    Externals
******************************************************************************/
extern ZCL_Cluster_t thServerClusters[TH_SERVER_CLUSTERS_COUNT];
extern ClusterId_t thServerClusterIds[TH_SERVER_CLUSTERS_COUNT];
extern void (*thServerClusterInitFunctions[TH_SERVER_CLUSTER_INIT_COUNT])();

extern ZCL_Cluster_t thClientClusters[TH_CLIENT_CLUSTERS_COUNT];
extern ClusterId_t thClientClusterIds[TH_CLIENT_CLUSTERS_COUNT];
extern void (*thClientClusterInitFunctions[TH_CLIENT_CLUSTER_INIT_COUNT])();
#endif // _THCLUSTERS_H

// eof thClusters.h
