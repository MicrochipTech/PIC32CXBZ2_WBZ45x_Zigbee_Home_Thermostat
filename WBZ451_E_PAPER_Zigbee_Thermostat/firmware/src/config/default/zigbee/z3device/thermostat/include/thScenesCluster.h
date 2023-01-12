/*******************************************************************************
  Thermostat Scenes cluster Header File

  Company:
    Microchip Technology Inc.

  File Name:
    thScenesCluster.h

  Summary:
    This file contains the Thermostat Scenes cluster interface.

  Description:
    This file contains the Thermostat Scenes cluster interface.
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

#ifndef _THSCENESCLUSTER_H
#define _THSCENESCLUSTER_H

/******************************************************************************
                    Includes section
******************************************************************************/
#include <zcl/include/zclZllScenesCluster.h>
#include <zcl/clusters/include/scenes.h>

/******************************************************************************
                    Externals
******************************************************************************/
extern ZCL_SceneClusterServerAttributes_t thScenesClusterServerAttributes;
extern PROGMEM_DECLARE (ZCL_ScenesClusterCommands_t thScenesCommands);
extern Scene_t thermostatSceneTable[];
extern Scene_t scenePool[MAX_SCENES_AMOUNT];
/******************************************************************************
                    Prototypes section
******************************************************************************/
/**************************************************************************//**
\brief Initializes Scenes cluster
******************************************************************************/
void thScenesClusterInit(void);

/**************************************************************************//**
\brief Recalls scene from scene table

\param[in] recallScene - the pointer to Recall Scene request
******************************************************************************/
void thRecallScene(ZCL_RecallScene_t *recallScene, Endpoint_t srcEp, Scene_t* scenePool, ZCL_SceneClusterServerAttributes_t* scenesAttributes);

/**************************************************************************//**
\brief Process Add Scene and Enhanced Add Scene command
******************************************************************************/
ZCL_Status_t thAddScene(ZCL_AddScene_t *payload, uint8_t payloadLength, Scene_t* scenePool, uint8_t* currSceneCount);

/**************************************************************************//**
\brief gets the scene extensions info from other clusterson teh devices

\param[in] scene - Scene to be updated
******************************************************************************/
ZCL_Status_t getExtensionFieldInfo(Scene_t* scene);

/**************************************************************************//**
\brief Sets attributes of thermostat cluster

\param[in] scene - Scene to be updated
******************************************************************************/
void thermostatClusterSetExtensionField(Scene_t* scene);
#endif // _THSCENESCLUSTER_H

// eof thScenesCluster.h
