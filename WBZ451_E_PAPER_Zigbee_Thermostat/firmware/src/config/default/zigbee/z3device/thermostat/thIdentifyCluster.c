/*******************************************************************************
  Thermostat Identify cluster Source File

  Company:
    Microchip Technology Inc.

  File Name:
    thIdentifyCluster.c

  Summary:
    This file contains the Thermostat Identify cluster interface.

  Description:
    This file contains the Thermostat Identify cluster interface.
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
#include <z3device/thermostat/include/thIdentifyCluster.h>
#include <zcl/clusters/include/identifyCluster.h>
#include <z3device/clusters/include/haClusters.h>
#include <app_zigbee/zigbee_console/console.h>
#include <zcl/include/zclCommandManager.h>
#include <pds/include/wlPdsMemIds.h>
#include <z3device/common/include/z3Device.h>
#include <app_zigbee/app_zigbee_handler.h>

/******************************************************************************
                    Definition(s) section
******************************************************************************/
#define DEFAULT_IDENTIFY_TIME         3U
#define IDENTIFY_EFFECT_TIMER_PERIOD  500U // in milliseconds

#define BLINK_IDENTIFY_TIME           1U   // in seconds
#define BREATHE_IDENTIFY_TIME         15U  // in seconds
#define OKAY_IDENTIFY_TIME            2U   // in seconds
#define CHANNEL_CHANGE_IDENTIFY_TIME  8U   // in seconds

#define LED_MAX_BRIGHTNESS            254U
#define LED_MIN_BRIGHTNESS            2U
#define LED_NO_BRIGHTNESS             0U

/******************************************************************************
                    Prototypes section
******************************************************************************/
static ZCL_Status_t identifyInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_Identify_t *payload);
static ZCL_Status_t identifyQueryInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload);
static ZCL_Status_t identifyQueryResponseInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_IdentifyQueryResponse_t *payload);
static ZCL_Status_t triggerEffectInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_TriggerEffect_t *payload);
static void ZCL_IdentifyAttributeEventInd(ZCL_Addressing_t *addressing, ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event);
static void identifyEffectTimerFired(void);
static void thIdentifyFinish(void);
static void (*identifycb)(void);

/******************************************************************************
                    Global variables section
******************************************************************************/
ZCL_IdentifyClusterClientAttributes_t thIdentifyClusterClientAttributes =
{
  ZCL_DEFINE_IDENTIFY_CLUSTER_CLIENT_ATTRIBUTES()
};

ZCL_IdentifyClusterServerAttributes_t thIdentifyClusterServerAttributes =
{
  ZCL_DEFINE_IDENTIFY_CLUSTER_SERVER_ATTRIBUTES()
};

PROGMEM_DECLARE(ZCL_IdentifyClusterCommands_t   thIdentifyCommands) =
{
  ZCL_IDENTIFY_CLUSTER_COMMANDS(identifyInd, identifyQueryInd, triggerEffectInd , identifyQueryResponseInd)
};

/******************************************************************************
                    Static variables section
******************************************************************************/
static HAL_AppTimer_t identifyTimer;


static struct
{
  bool period        : 1;
  bool finish        : 1;
  bool chChangeEffect: 1;
  bool minTime       : 1;
} identificationStatus;

/******************************************************************************
                    Implementation section
******************************************************************************/
/**************************************************************************//**
\brief Initializes Identify cluster
******************************************************************************/
void thIdentifyClusterInit(void)
{
  thIdentifyClusterServerAttributes.identifyTime.value = 0;

  ZCL_Cluster_t *cluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, IDENTIFY_CLUSTER_ID, ZCL_CLUSTER_SIDE_SERVER);
  ZCL_Cluster_t *identifyCluster = ZCL_GetCluster(APP_ENDPOINT_THERMOSTAT, IDENTIFY_CLUSTER_ID, ZCL_CLUSTER_SIDE_CLIENT);
  
  if (cluster)
    cluster->ZCL_AttributeEventInd = ZCL_IdentifyAttributeEventInd;
  
  thIdentifyStop();
  thIdentifyClusterServerAttributes.clusterVersion.value = IDENTIFY_CLUSTER_VERSION;
  thIdentifyClusterClientAttributes.clusterVersion.value = IDENTIFY_CLUSTER_VERSION;
  LEDS_SET_COLOR_HS(255,255);
  if (identifyCluster)
    identifyCluster->ZCL_DefaultRespInd = ZCL_CommandZclDefaultResp;
}

/**************************************************************************//**
\brief Shows identification effect in way specified.

\param[in] identifyTime - identifying period in seconds.
******************************************************************************/
void thIdentifyStart(uint16_t identifyTime)
{
  HAL_StopAppTimer(&identifyTimer);

  identificationStatus.finish = false;

  thIdentifyClusterServerAttributes.identifyTime.value = identifyTime;
  
  LEDS_SET_BRIGHTNESS(LED_NO_BRIGHTNESS);

  identifyTimer.mode = TIMER_REPEAT_MODE;
  identifyTimer.interval = IDENTIFY_EFFECT_TIMER_PERIOD;
  identifyTimer.callback = identifyEffectTimerFired;

  HAL_StartAppTimer(&identifyTimer);
}

/**************************************************************************//**
\brief Timer expiry handler for identifyEffectTimer
******************************************************************************/
static void identifyEffectTimerFired(void)
{
  uint8_t level = LED_NO_BRIGHTNESS;
  /**CHANGE - Add description here- some comments*/
  if (identificationStatus.chChangeEffect)
  {
    if (identificationStatus.period)
    {
      thIdentifyClusterServerAttributes.identifyTime.value--;
      identificationStatus.period = !identificationStatus.period;
    }
    else
    {
      identificationStatus.period = !identificationStatus.period;
    }
    
    if (!identificationStatus.minTime)
    {
      level = LED_MAX_BRIGHTNESS;
      identificationStatus.minTime = !identificationStatus.minTime;
    }
    else
    {
      level = LED_MIN_BRIGHTNESS;
    }
  }
  else
  { // Effects such as Blink, Breathe and Okay or Normal Identify
    identificationStatus.period = !identificationStatus.period;

    if (identificationStatus.period)
    {
      level = LED_MAX_BRIGHTNESS;      
      thIdentifyClusterServerAttributes.identifyTime.value--;
    }    
  }

  LEDS_SET_BRIGHTNESS(level);
  (void)level;
  /**CHANGE - Need to be checked - identificationStatus.finish shall be there or not */
  if ((0 == thIdentifyClusterServerAttributes.identifyTime.value) || identificationStatus.finish)
  {
    thIdentifyStop();
  }
}


/**************************************************************************//**
\brief Finish identification routine.
******************************************************************************/
static void thIdentifyFinish(void)
{
  identificationStatus.finish = true;
}

/**************************************************************************//**
\brief Stops Identify cluster
******************************************************************************/
void thIdentifyStop(void)
{
  identificationStatus.period = 0;
  identificationStatus.minTime = false;
  identificationStatus.chChangeEffect = false;
  
  HAL_StopAppTimer(&identifyTimer);

  thIdentifyClusterServerAttributes.identifyTime.value = 0;

  if (identifycb)
    identifycb();

  LEDS_SET_BRIGHTNESS(LED_NO_BRIGHTNESS);
}

/**************************************************************************//**
\brief Checks if identification process is in progress

\returns true if it is, false otherwise
******************************************************************************/
bool thIdentifyIsIdentifying(void)
{
  return thIdentifyClusterServerAttributes.identifyTime.value > 0;
}
/**************************************************************************//**
\brief Callback on receiving Identify command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t identifyInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_Identify_t *payload)
{
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_IDENTIFY;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;

  thIdentifyClusterServerAttributes.identifyTime.value = payload->identifyTime;

  if (payload->identifyTime)
    thIdentifyStart(payload->identifyTime);
  else
    thIdentifyStop();

  RAISE_CALLBACKS_TO_IDENTIFY_SUBSCIBERS(identifySubscribers, identify);

  APP_Zigbee_Handler(event);
  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
\brief Callback on receiving Identify Query command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t identifyQueryInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, uint8_t *payload)
{
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_IDENTIFY_QUERY;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;
  APP_Zigbee_Handler(event);
  if (thIdentifyClusterServerAttributes.identifyTime.value)
    return sendIdentifyQueryResponse(addressing, APP_ENDPOINT_THERMOSTAT, thIdentifyClusterServerAttributes.identifyTime.value);
  
  RAISE_CALLBACKS_TO_IDENTIFY_SUBSCIBERS(identifySubscribers, identifyQuery);

  return ZCL_SUCCESS_WITH_DEFAULT_RESPONSE_STATUS;
}
/**************************************************************************//**
\brief Callback on receiving Identify Query Response command

\param[in] addressing - pointer to addressing information;
\param[in] payloadLength - data payload length;
\param[in] payload - data pointer

\return status of indication routine
******************************************************************************/
static ZCL_Status_t identifyQueryResponseInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_IdentifyQueryResponse_t *payload)
{
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_IDENTIFY_QUERY_RESPONSE;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;

  RAISE_CALLBACKS_TO_IDENTIFY_SUBSCIBERS(identifySubscribers, identifyQueryResponse);

  APP_Zigbee_Handler(event);
  return ZCL_SUCCESS_STATUS;
}
/**************************************************************************//**
\brief Callback on receive of Trigger Effect command
******************************************************************************/
static ZCL_Status_t triggerEffectInd(ZCL_Addressing_t *addressing, uint8_t payloadLength, ZCL_TriggerEffect_t *payload)
{
  APP_Zigbee_Event_t event;
  event.eventGroup = EVENT_CLUSTER;
  event.eventId = CMD_ZCL_TRIGGER_EFFECT;
  event.eventData.zclEventData.addressing = addressing;
  event.eventData.zclEventData.payloadLength = payloadLength;
  event.eventData.zclEventData.payload = (uint8_t*)payload;

  switch (payload->effectIdentifier)
  {
    case ZCL_EFFECT_IDENTIFIER_BLINK:
      thIdentifyStart(BLINK_IDENTIFY_TIME);
      break;

    case ZCL_EFFECT_IDENTIFIER_BREATHE:
      thIdentifyStart(BREATHE_IDENTIFY_TIME);
      break;

    case ZCL_EFFECT_IDENTIFIER_OKAY:
      thIdentifyStart(OKAY_IDENTIFY_TIME);
      break;

    case ZCL_EFFECT_IDENTIFIER_CHANNEL_CHANGE:
      identificationStatus.chChangeEffect = true;
      thIdentifyStart(CHANNEL_CHANGE_IDENTIFY_TIME);
      break;

    case ZCL_EFFECT_IDENTIFIER_FINISH_EFFECT:
      thIdentifyFinish();
      break;

    case ZCL_EFFECT_IDENTIFIER_STOP_EFFECT:
      thIdentifyStop();
      break;

    default:
      break;
  }

  APP_Zigbee_Handler(event);
  return ZCL_SUCCESS_STATUS;
}

/**************************************************************************//**
\brief Attribute event (writing/reading) callback.

\param[in] addressing - incoming request addressing information.
\param[in] attributeId - attribute identifier.
\param[in] event - attribute event (read/write).
******************************************************************************/
static void ZCL_IdentifyAttributeEventInd(ZCL_Addressing_t *addressing,
  ZCL_AttributeId_t attributeId, ZCL_AttributeEvent_t event)
{
  APP_Zigbee_Event_t eventItem;
  eventItem.eventGroup = EVENT_CLUSTER;
  eventItem.eventId = CMD_ZCL_ATTR_IDENTIFY;
  eventItem.eventData.zclAttributeData.addressing = addressing;
  eventItem.eventData.zclAttributeData.attributeId = attributeId;
  eventItem.eventData.zclAttributeData.event = event;

  if ((ZCL_WRITE_ATTRIBUTE_EVENT == event) &&
      (ZCL_IDENTIFY_CLUSTER_IDENTIFY_TIME_ATTRIBUTE_ID == attributeId))
  {
    if (thIdentifyClusterServerAttributes.identifyTime.value)
    {
      thIdentifyStart(thIdentifyClusterServerAttributes.identifyTime.value);
    }
    else
    {
      thIdentifyStop();
    }

  APP_Zigbee_Handler(eventItem);
  }
}

/**************************************************************************//**
\brief Makes device to start identify itself

\param[in] time - identifying time in seconds
******************************************************************************/
void thIdetifyStartIdentifyingCb(uint16_t time, void (*cb)(void))
{
  thIdentifyClusterServerAttributes.identifyTime.value = time;
  identifycb = cb;
  thIdentifyStart(thIdentifyClusterServerAttributes.identifyTime.value);  

  (void)cb;
}

#endif // APP_DEVICE_TYPE_THERMOSTAT

// eof thIdentifyCluster.c
