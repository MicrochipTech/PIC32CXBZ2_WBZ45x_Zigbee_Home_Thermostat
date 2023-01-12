/*******************************************************************************
  Zigbee Cluster Library Time Cluster Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zclTimeCluster.h

  Summary:
    The header file describes the ZCL Time Cluster and its interface.

  Description:
    The file describes the types and interface of the ZCL Time Cluster.
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

#ifndef _ZCLTIMECLUSTER_H
#define _ZCLTIMECLUSTER_H

/*!
This cluster provides a basic interface to a real-time clock. The clock time may be
read and also written, in order to synchronize the clock (as close as practical) to a
time standard. This time standard is the number of seconds since 0 hrs 0 mins 0
sec on 1st January 2000 UTC (Universal Coordinated Time).
*/

/*******************************************************************************
                   Includes section
*******************************************************************************/

#include <zcl/include/zcl.h>
#include <zcl/include/clusters.h>


/*******************************************************************************
                   Define(s) section
*******************************************************************************/
#define ZCL_TIME_CLUSTER_DEF_ZERO_VALUE     0x00000000
#define ZCL_TIME_CLUSTER_DEF_INVALID_VALUE  0xffffffff
#define ZCL_TIME_ZONE_MIN_VALUE             -86400
#define ZCL_TIME_ZONE_MAX_VALUE             86400
#define ZCL_DST_SHIFT_MIN_VALUE             -86400
#define ZCL_DST_SHIFT_MAX_VALUE             86400
/***************************************************************************//**
  \brief ZCL Time Cluster server side commands amount
*******************************************************************************/
#define ZCL_TIME_CLUSTER_SERVER_ATTRIBUTES_AMOUNT      11
//The Time and TimeStatus attributes

/***************************************************************************//**
  \brief ZCL Time Cluster client side commands amount
*******************************************************************************/
#define ZCL_TIME_CLUSTER_CLIENT_ATTRIBUTES_AMOUNT      1
//There is no any attributes at client cluster side

/***************************************************************************//**
  \brief ZCL Time Cluster commands amount
*******************************************************************************/
#define ZCL_TIME_CLUSTER_COMMANDS_AMOUNT        0
//There is no any commands at client cluster side

//!ZCL Time Cluster server side Time attribute id
#define ZCL_TIME_CLUSTER_SERVER_TIME_ATTRIBUTE_ID             CCPU_TO_LE16(0x0000)
//!ZCL Time Cluster server side TimeStatus attribute id
#define ZCL_TIME_CLUSTER_SERVER_TIME_STATUS_ATTRIBUTE_ID      CCPU_TO_LE16(0x0001)
//!ZCL Time Cluster server side TimeZone attribute id
#define ZCL_TIME_CLUSTER_SERVER_TIME_ZONE_ATTRIBUTE_ID        CCPU_TO_LE16(0x0002)
//!ZCL Time Cluster server side DstStart attribute id
#define ZCL_TIME_CLUSTER_SERVER_DST_START_ATTRIBUTE_ID        CCPU_TO_LE16(0x0003)
//!ZCL Time Cluster server side DstEnd attribute id
#define ZCL_TIME_CLUSTER_SERVER_DST_END_ATRIBUTE_ID           CCPU_TO_LE16(0x0004)
//!ZCL Time Cluster server side DstShift attribute id
#define ZCL_TIME_CLUSTER_SERVER_DST_SHIFT_ATTRIBUTE_ID        CCPU_TO_LE16(0x0005)
//!ZCL Time Cluster server side StandardTime attribute id
#define ZCL_TIME_CLUSTER_SERVER_STANDARD_TIME_ATTRIBUTE_ID    CCPU_TO_LE16(0x0006)
//!ZCL Time Cluster server side LocalTime attribute id
#define ZCL_TIME_CLUSTER_SERVER_LOCAL_TIME_ATTRIBUTE_ID       CCPU_TO_LE16(0x0007)
//!ZCL Time Cluster server side lastSetTime attribute id
#define ZCL_TIME_CLUSTER_SERVER_LAST_SET_TIME_ATTRIBUTE_ID    CCPU_TO_LE16(0x0008)
//!ZCL Time Cluster server side validUntilTime attribute id
#define ZCL_TIME_CLUSTER_SERVER_VALID_UNTIL_TIME_ATTRIBUTE_ID CCPU_TO_LE16(0x0009)

//Attribute Ids of Basic Cluster Information
#define ZCL_TIME_CLUSTER_GLOBAL_CLUSTER_VERSION_ATTRIBUTE_ID                          CCPU_TO_LE16(0xfffd)

/***************************************************************************//**
  \brief ZCL Time Cluster server side attributes defining macros

  This macros should be used for ZCL Time Cluster server side attributes defining.

  \return None

  \internal
  //The typical usage is:
  //Time Cluster server side related attributes
  ZCL_TimeClusterServerAttributes_t timeClusterAttributes = ZCL_DEFINE_TIME_CLUSTER_SERVER_ATTRIBUTES();
*******************************************************************************/
#define ZCL_DEFINE_TIME_CLUSTER_SERVER_ATTRIBUTES() 
    
#define ZCL_DEFINE_TIME_CLUSTER_CLIENT_ATTRIBUTES() \
  DEFINE_ATTRIBUTE(clusterVersion, ZCL_READONLY_ATTRIBUTE, ZCL_TIME_CLUSTER_GLOBAL_CLUSTER_VERSION_ATTRIBUTE_ID, ZCL_U16BIT_DATA_TYPE_ID)     
/***************************************************************************//**
  \brief ZCL Time Cluster server side defining macros

  This macros should be used with #DEFINE_ZCL_TIME_CLUSTER_SERVER_ATTRIBUTES for
  ZCL Time Cluster server side defining in application.

  \param attributes - pointer to cluster server attributes (ZCL_TimeClusterServerAttributes_t)

  \return None

  \internal
  //The typical code is:
  //Time Cluster server side related attributes
  ZCL_TimeClusterServerAttributes_t timeClusterServerAttributes = DEFINE_ZCL_TIME_CLUSTER_SERVER_ATTRIBUTES();
  ZCL_Cluster_t myClusters[] =
  {
    ZCL_DEFINE_TIME_CLUSTER_SERVER(&timeClusterServerAttributes),
    //... Any other cluster defining ...
  }
*******************************************************************************/
#define TIME_CLUSTER_ZCL_SERVER_CLUSTER_TYPE(clattributes, clcommands)                                                    \
  {                                                                                                                       \
    .id = TIME_CLUSTER_ID,                                                                                                \
    .options = {.type = ZCL_SERVER_CLUSTER_TYPE, .security = ZCL_NETWORK_KEY_CLUSTER_SECURITY},                  \
    .attributesAmount = ZCL_TIME_CLUSTER_SERVER_ATTRIBUTES_AMOUNT,                                                        \
    .attributes = (uint8_t *) clattributes,                                                                               \
    .commandsAmount = ZCL_TIME_CLUSTER_COMMANDS_AMOUNT,                                                                   \
    .commands = (uint8_t *) clcommands                                                                                    \
  }

#define TIME_CLUSTER_ZCL_SERVER_CLUSTER_TYPE_FLASH(clattributes, clcommands)                                              \
  {                                                                                                                       \
    .id = TIME_CLUSTER_ID,                                                                                                \
    .options = {.type = ZCL_SERVER_CLUSTER_TYPE, .security = ZCL_NETWORK_KEY_CLUSTER_SECURITY},                  \
    .attributesAmount = ZCL_TIME_CLUSTER_SERVER_ATTRIBUTES_AMOUNT,                                                        \
    .attributes = (uint8_t *) clattributes,                                                                               \
    .commandsAmount = ZCL_TIME_CLUSTER_COMMANDS_AMOUNT,                                                                   \
    .commands = (FLASH_PTR uint8_t *) clcommands                                                                                    \
  }

/***************************************************************************//**
  \brief ZCL Time Cluster client side defining macros

  This macros should be used for ZCL Time Cluster client side defining in application.

  \return None

  \internal
  //The typical code is:
  ZCL_Cluster_t myClusters[] =
  {
    ZCL_DEFINE_TIME_CLUSTER_CLIENT(),
    //... Any other cluster defining ...
  }
*******************************************************************************/
#define TIME_CLUSTER_ZCL_CLIENT_CLUSTER_TYPE(clattributes, clcommands)                                                    \
  {                                                                                                                       \
    .id = TIME_CLUSTER_ID,                                                                                                \
    .options = {.type = ZCL_CLIENT_CLUSTER_TYPE, .security = ZCL_NETWORK_KEY_CLUSTER_SECURITY },                 \
    .attributesAmount = ZCL_TIME_CLUSTER_CLIENT_ATTRIBUTES_AMOUNT,                                                        \
    .attributes = (uint8_t *) clattributes,                                                                               \
    .commandsAmount = ZCL_TIME_CLUSTER_COMMANDS_AMOUNT,                                                                   \
    .commands = (uint8_t *) clcommands                                                                                    \
  }

#define TIME_CLUSTER_ZCL_CLIENT_CLUSTER_TYPE_FLASH(clattributes, clcommands)                                              \
  {                                                                                                                       \
    .id = TIME_CLUSTER_ID,                                                                                                \
    .options = {.type = ZCL_CLIENT_CLUSTER_TYPE, .security = ZCL_NETWORK_KEY_CLUSTER_SECURITY },                 \
    .attributesAmount = ZCL_TIME_CLUSTER_CLIENT_ATTRIBUTES_AMOUNT,                                                        \
    .attributes = (uint8_t *) clattributes,                                                                               \
    .commandsAmount = ZCL_TIME_CLUSTER_COMMANDS_AMOUNT,                                                                   \
    .commands = (FLASH_PTR uint8_t *) clcommands                                                                                    \
  }

#define DEFINE_TIME_CLUSTER(cltype, clattributes, clcommands) TIME_CLUSTER_##cltype(clattributes, clcommands)
#define DEFINE_TIME_CLUSTER_FLASH(cltype, clattributes, clcommands) TIME_CLUSTER_##cltype##_FLASH(clattributes, clcommands)

/*******************************************************************************
                   Types section
*******************************************************************************/

BEGIN_PACK
/***************************************************************************//**
  \brief
  ZCL Time Cluster attributes
*******************************************************************************/
typedef struct PACK
{
} ZCL_TimeClusterServerAttributes_t;

typedef struct PACK
{
  struct PACK
  {
    ZCL_AttributeId_t id;
    uint8_t type;
    uint8_t properties;
    uint16_t value;
  } clusterVersion;
} ZCL_TimeClusterClientAttributes_t;

END_PACK
#endif // #ifndef _ZCLTIMECLUSTER_H

//eof zclTimeCluster.h
