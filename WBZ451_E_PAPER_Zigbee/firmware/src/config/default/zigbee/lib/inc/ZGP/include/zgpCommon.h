/*******************************************************************************
   Zigbee green power Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpCommon.h

  Summary:
    This file contains the header file describes the common definition of GP.

  Description:
    This file contains the public interface of GP
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

#ifndef _ZGPCOMMON_H
#define _ZGPCOMMON_H

/******************************************************************************
                           Includes section
******************************************************************************/
#include <systemenvironment/include/sysTypes.h>
#include <mac_phy/include/macData.h>

/******************************************************************************
                          Defines section
******************************************************************************/
#define NO_OF_BITS_IN_BYTE  8U
#define ZGP_SECURITY_KEY_LENGTH                 16U

/******************************************************************************
                                ZGP Default values
******************************************************************************/
#define GREENPOWER_PROTOCOL_VERSION         0x03u     //ZGP Protocol ID
#define ZGP_INVALID_SRC_ID                  0x00000000
#define ZGP_MAINTENANCE_FRAME_SRC_ID        0x00000000
#define ZGP_BTX_SRC_ID                      0xFFFFFFFF
#define ZGP_ALL_SRC_ID                      0xFFFFFFFF
#define ZGP_FIRST_RESERVED_SRC_ID           0xFFFFFFF9
#define ZGP_LAST_RESERVED_SRC_ID            0xFFFFFFFE
#define ZGP_DEVICE_ANNCE_EXT_ADDR           0xFFFFFFFFFFFFFFFF
#define ZGP_ALL_IEEE_ADDR                   0xFFFFFFFFFFFFFFFF

#define ZGP_CMD_ID_LENGTH                   1
#define ZGP_SRC_ID_LENGTH                   4
#define ZGP_ENDPOINT_LENGTH                 1
#define ZGP_NO_DATA                         0
#define ZGP_EXT_NWK_FCF_LENGTH              1
#define ZGP_NWK_FCF_LENGTH                  1
#define ZGP_SEC_FRAME_CTR_LENGTH            4
#define ZGP_SEC_TWOBYTE_MIC                 2
#define ZGP_SEC_FOURBYTE_MIC                4
#define ZGP_OPTIONS_FIELD_LENGTH            1
#define ZGP_BROADCAST_ADDR                  0xFFFF
#define ZGP_BROADCAST_PANID                 0xFFFF
#define ZGP_COMM_KEY_HEADER_LEN             4

#define ZGP_FCF_SECLVL_POS                3
#define ZGP_FCF_SECKEY_POS                5
#define ZGP_FCF_RXAFTX_POS                6
#define ZGP_FCF_DIR_POS                   7

#define ZGP_FCF_VER_POS                   2
#define ZGP_FCF_AUTOCOM_POS               6
#define ZGP_FCF_ISEXT_POS                 7


#define ZGP_AUTO_COMMISSIONING            0   //Auto Commissioning
#define ZGP_UNIDIRECTIONAL_COMMISSIONING  1   //Unidirectional Commissioning
#define ZGP_BIDIRECTIONAL_COMMISSIONING   2   //Bidirectional Commissioning

#define ZGP_RX_OFFSET_MICROSEC            15000
// since the rxoffset timer is started at the end of transmission instead of
// doing at the start of transmission,subtracting the transmission time(with jitter).
//#define ZGP_RX_OFFSET_MICROSEC            (20000 - 1600)

#define ALL_END_POINT             0xff
#define GPD_PROCESSING_IN_APPLICATION  0xFC
#define APP_INDEPENDENT_END_POINT 0x00


// Checks and configures ZGP duplicate timeout
#ifdef  ZGP_DUPLICATE_TIMEOUT
#if (ZGP_DUPLICATE_TIMEOUT > 32760)
#error the maximum duplicate timeout should not be greater than 32760
#endif //(ZGP_DUPLICATE_TIMEOUT > 32760)
#else
#define ZGP_DUPLICATE_TIMEOUT 2000
#endif //ZGP_DUPLICATE_TIMEOUT

#define DEF_QUEUE_ENTRY_LIFETIME  0xFFFFFFFF

/** ZGP Stub NWK header*/
#define ZGP_MAX_HEADER_LENGTH                            10
/** ZGP Stub NWK header*/
#define ZGP_MAX_HEADER_LENGTH_FOR_IEEE_APPID              7
/** MIC length*/
#define ZGP_MAX_FOOTER_LENGTH                            4
/** Length of the reserved part in the data frame. */
#define ZGP_MAX_AFFIX_LENGTH  (MAC_AFFIX_LENGTH + \
   ZGP_MAX_HEADER_LENGTH + ZGP_MAX_FOOTER_LENGTH)

/** Length of the reserved header in the data frame. */
#define ZGP_MAX_ASDU_OFFSET (MAC_MSDU_OFFSET + ZGP_MAX_HEADER_LENGTH)

#define ZGP_MAX_ASDU_SIZE (MAC_MAX_MSDU_SIZE \
   - ZGP_MAX_HEADER_LENGTH - ZGP_MAX_FOOTER_LENGTH)

#define NO_OF_ATTRIBUTE_REPORT_FIELD      1
#define NO_OF_ATTR_RECORDS                1

#define ZGP_MAX_GSDU_SIZE 64U

#define MAX_ZGP_PAYLOAD (ZGP_MAX_HEADER_LENGTH + ZGP_MAX_GSDU_SIZE + ZGP_SEC_FOURBYTE_MIC + ZGP_CMD_ID_LENGTH)
#define MAX_ZGP_MSDU_SIZE (MAX_ZGP_PAYLOAD + MAC_MAX_MPDU_UNSECURED_OVERHEAD_NO_SRC_ADDR_IEEE_DST_ADDR_MODE)

#define MAX_COMM_REPLY_LENGTH (1/* options */ + 2/* PAN id */  + 16/* GPD key */ + 4/*GPD key MIC */  + 4 /*FrameCOunter*/)
/******************************************************************************
                              ZGP cluster related
******************************************************************************/
#define ZGP_TEMP_CLUSTER_ID            0x0402
#define ZGP_TEMP_ATTRIBUTE_ID          0x0000

#define ZGP_TEMP_ATTRIBUTE_DATA_TYPE   0x29

/******************************************************************************
                              ZGP Command IDs
******************************************************************************/
//Identify Command ID'S
#define ZGP_IDENTIFY_CMD_ID_IDENTIFY                         0x00
// Scene cluster command IDs
#define ZGP_SCENE_CMD_ID_RECALLSCENE0                        0x10
#define ZGP_SCENE_CMD_ID_RECALLSCENE1                        0x11
#define ZGP_SCENE_CMD_ID_RECALLSCENE2                        0x12
#define ZGP_SCENE_CMD_ID_RECALLSCENE3                        0x13
#define ZGP_SCENE_CMD_ID_RECALLSCENE4                        0x14
#define ZGP_SCENE_CMD_ID_RECALLSCENE5                        0x15
#define ZGP_SCENE_CMD_ID_RECALLSCENE6                        0x16
#define ZGP_SCENE_CMD_ID_RECALLSCENE7                        0x17

#define ZGP_SCENE_CMD_ID_STORESCENE0                        0x18
#define ZGP_SCENE_CMD_ID_STORESCENE1                        0x19
#define ZGP_SCENE_CMD_ID_STORESCENE2                        0x1A
#define ZGP_SCENE_CMD_ID_STORESCENE3                        0x1B
#define ZGP_SCENE_CMD_ID_STORESCENE4                        0x1C
#define ZGP_SCENE_CMD_ID_STORESCENE5                        0x1D
#define ZGP_SCENE_CMD_ID_STORESCENE6                        0x1E
#define ZGP_SCENE_CMD_ID_STORESCENE7                        0x1F

// OnOff cluster command IDs - doesn't have command payload
#define ZGP_ONOFF_CMD_ID_OFF                                 0x20
#define ZGP_ONOFF_CMD_ID_ON                                  0x21
#define ZGP_ONOFF_CMD_ID_TOGGLE                              0x22
#define ZGP_ATTRIBUTE_REPORT_CMD_ID                          0xA0
#define ZGP_MANU_SPEC_ATTR_REPORT_CMD_ID                     0xA1
#define ZGP_MULTI_CLUSTER_ATTR_REPORT_CMD_ID                 0xA2
#define ZGP_MANU_SPEC_MULTI_CLUSTER_ATTR_REPORT_CMD_ID       0xA3
#define ZGP_ZCL_TUNNELING_CMD_ID                             0xA6
#define ZGP_COMPACT_ATTR_REPORT_CMD_ID                       0xA8
#define ZGP_REQUEST_ATTRIBUTE_CMD_ID                         0xA4
#define ZGP_READ_ATTRIBUTE_RESP_CMD_ID                       0xA5
#define ZGP_WRITE_ATTRIBUTE_CMD_ID                           0xF1
#define ZGP_READ_ATTRIBUTE_CMD_ID                            0xF2

// Level Control cluster command IDs
#define ZGP_LC_CMD_ID_MOVE_UP                                0x30
#define ZGP_LC_CMD_ID_MOVE_DOWN                              0x31
#define ZGP_LC_CMD_ID_STEP_UP                                0x32
#define ZGP_LC_CMD_ID_STEP_DOWN                              0x33
#define ZGP_LC_CMD_ID_LEVEL_CONTROL_STOP                     0x34
#define ZGP_LC_CMD_ID_MOVE_UP_WITH_ONOFF                     0x35
#define ZGP_LC_CMD_ID_MOVE_DOWN_WITH_ONOFF                   0x36
#define ZGP_LC_CMD_ID_STEP_UP_WITH_ONOFF                     0x37
#define ZGP_LC_CMD_ID_STEP_DOWN_WITH_ONOFF                   0x38

// Color Control cluster command IDs
#define ZGP_CC_CMD_ID_MOVE_HUE_STOP                          0x40
#define ZGP_CC_CMD_ID_MOVE_HUE_UP                            0x41
#define ZGP_CC_CMD_ID_MOVE_HUE_DOWN                          0x42
#define ZGP_CC_CMD_ID_STEP_HUE_UP                            0x43
#define ZGP_CC_CMD_ID_STEP_HUE_DOWN                          0x44
#define ZGP_CC_CMD_ID_MOVE_SATURATION_STOP                   0x45
#define ZGP_CC_CMD_ID_MOVE_SATURATION_UP                     0x46
#define ZGP_CC_CMD_ID_MOVE_SATURATION_DOWN                   0x47
#define ZGP_CC_CMD_ID_STEP_SATURATION_UP                     0x48
#define ZGP_CC_CMD_ID_STEP_SATURATION_DOWN                   0x49
#define ZGP_CC_CMD_ID_MOVE_COLOR                             0x4A
#define ZGP_CC_CMD_ID_STEP_COLOR                             0x4B

#define ZGP_8_BIT_VECTOR_PRESS                               0x69
#define ZGP_8_BIT_VECTOR_RELEASE                             0x6A

#define ZGP_MANUFAC_SPECIFIC_CMD_ID_START                    0xB0
#define ZGP_MANUFAC_SPECIFIC_CMD_ID_END                      0xBF
/******************************************************************************
                              ZGP Supported Channels
******************************************************************************/
#define ZGP_CHANNEL_11                   0x00000800UL
#define ZGP_CHANNEL_12                   0x00001000UL
#define ZGP_CHANNEL_13                   0x00002000UL
#define ZGP_CHANNEL_14                   0x00004000UL
#define ZGP_CHANNEL_15                   0x00008000UL
#define ZGP_CHANNEL_16                   0x00010000UL
#define ZGP_CHANNEL_17                   0x00020000UL
#define ZGP_CHANNEL_18                   0x00040000UL
#define ZGP_CHANNEL_19                   0x00080000UL
#define ZGP_CHANNEL_20                   0x00100000UL
#define ZGP_CHANNEL_21                   0x00200000UL
#define ZGP_CHANNEL_22                   0x00400000UL
#define ZGP_CHANNEL_23                   0x00800000UL
#define ZGP_CHANNEL_24                   0x01000000UL
#define ZGP_CHANNEL_25                   0x02000000UL
#define ZGP_CHANNEL_26                   0x04000000UL

#define ZGP_CHANNEL_OFFSET                11U
/******************************************************************************
                           Types section
******************************************************************************/

/******************************************************************************
                              ZGP primitive return code
******************************************************************************/
typedef enum
{
  ZGP_SUCCESS_STATUS                    = 0x00,
  /** ZGP Data request queue is full */
  ZGP_TX_QUEUE_FULL                     = 0x70,
  /*  previous GPDF is overwritten */
  ZGP_ENTRY_REPLACED                    = 0x71,
  /*  GPDF is added to the zgp TxQueue */
  ZGP_ENTRY_ADDED                       = 0x72,
  /* zgpTxQueueEntryLifetime timeout */
  ZGP_ENTRY_EXPIRED                     = 0x73,
  ZGP_ENTRY_REMOVED                     = 0x74,
  ZGP_GPDF_SENDING_FINALIZED            = 0x75,
  /* No reply received for commissioning request */
  ZGP_NO_COMMISSIONING_REPLY            = 0x76,
  /* No confirmation for commissioning request */
  ZGP_COMMISSIONING_TIMEOUT             = 0x77,
  /* Commissioning reply has invalid parameter */
  ZGP_COMMISSIONING_REPLY_FAILURE       = 0x78,
  /* Commissioning/Decommissioning invalid request */
  ZGP_COMMISSIONING_REQ_INVALID         = 0x79,
  /* ZGP key unavailable*/
  ZGP_UNAVAILABLE_KEY_STATUS            = 0x7A,
  /* ZGP bad state */
  ZGP_BAD_STATE                         = 0x7B,
  /* Invalid parameter */
  ZGP_INVALID_PARAMETER                 = 0x7C,
  /* Invalid parameter */
  ZGP_NO_CHANNEL_MASK                   = 0x7D,
  /* ZGP_SECURITY_LEVEL_NOT_MATCHED */
  ZGP_SECURITY_LEVEL_NOT_MATCHED        = 0x7E,
  /* No reply received for channel request*/
  ZGP_NO_CHANNEL_CONFIG_REPLY           = 0x7F,
  /** IEEE 802.15.4-2006, Table 78 MAC enumerations description. */
  /** The frame counter purportedly applied by the originator of the
   * received frame is invalid. */
  ZGP_MAC_COUNTER_ERROR_STATUS       = 0xDB,
  /** The key purportedly applied by the originator of the received frame is
   * not allowed to be used with that frame type according to the key usage
   * policy of the recipient. */
  ZGP_MAC_IMPROPER_KEY_TYPE_STATUS   = 0xDC,
  /** The security level purportedly applied by the originator of the received
   * frame does not meet the minimum security level required/expected by
   * the recipient for that frame type. */
  ZGP_MAC_IMPROPER_SECURITY_LEVEL_STATUS = 0xDD,
  /** The received frame was purportedly secured using security based on
   * IEEE Std 802.15.4-2003, and such security is not supported by this standard.
   **/
  ZGP_MAC_UNSUPPORTED_LEGACY_STATUS   = 0xDE,
  /** The security purportedly applied by the originator of the received frame
   * is not supported. */
  ZGP_MAC_UNSUPPORTED_SECURITY_STATUS = 0xDF,
  /** The beacon was lost following a synchronization request. */
  ZGP_MAC_BEACON_LOSS_STATUS          = 0xE0,
  /** A transmission could not take place due to activity on the channel,
   * i.e., the CSMA-CA mechanism has failed. */
  ZGP_MAC_CHANNEL_ACCESS_FAILURE_STATUS = 0xE1,
  /** The GTS request has been denied by the PAN coordinator. */
  ZGP_MAC_DENIED_STATUS               = 0xE2,
  /** The attempt to disable the transceiver has failed. */
  ZGP_MAC_DISABLE_TRX_FAILURE_STATUS  = 0xE3,
  /** Either a frame resulting from processing has a length that is
   * greater than aMaxPHYPacketSize or a requested transaction is
   * too large to fit in the CAP or GTS. */
  ZGP_MAC_FRAME_TOO_LONG_STATUS       = 0xE5,
  /** The requested GTS transmission failed because the specified
   * GTS either did not have a transmit GTS direction or was not defined. */
  ZGP_MAC_INVALID_GTS_STATUS          = 0xE6,
  /** A request to purge an MSDU from the transaction queue was made using
   * an MSDU handle that was not found in the transaction table. */
  ZGP_MAC_INVALID_HANDLE_STATUS       = 0xE7,
  /** A parameter in the primitive is either not supported or is out of
   * the valid range. */
  ZGP_MAC_INVALID_PARAMETER_STATUS    = 0xE8,
  /** No acknowledgment was received after macMaxFrameRetries. */
  ZGP_MAC_NO_ACK_STATUS               = 0xE9,
  /** A scan operation failed to find any network beacons. */
  ZGP_MAC_NO_BEACON_STATUS            = 0xEA,
  /** No response data were available following a request. */
  ZGP_MAC_NO_DATA_STATUS              = 0xEB,
  /** The operation failed because a 16-bit short address was not allocated. */
  ZGP_MAC_NO_SHORT_ADDRESS_STATUS     = 0xEC,
  /** A receiver enable request was unsuccessful because it could not be
   * completed within the CAP. */
  ZGP_MAC_OUT_OF_CAP_STATUS           = 0xED,
  /** A PAN identifier conflict has been detected and communicated
   * to the PAN coordinator. */
  ZGP_MAC_PAN_ID_CONFLICT_STATUS      = 0xEE,
  /** A coordinator realignment command has been received. */
  ZGP_MAC_REALIGNMENT_STATUS          = 0xEF,
  /** The transaction has expired and its information was discarded. */
  ZGP_MAC_TRANSACTION_EXPIRED_STATUS  = 0xF0,
  /** There is no capacity to store the transaction. */
  ZGP_MAC_TRANSACTION_OVERFLOW_STATUS = 0xF1,
  /** The transceiver was in the transmitter enabled state when the receiver
   * was requested to be enabled. */
  ZGP_MAC_TX_ACTIVE_STATUS            = 0xF2,
  /** The key purportedly used by the originator of the received frame is
   * not available or, if available, the originating device is not known
   * or is blacklisted with that particular key. */
  ZGP_MAC_UNAVAILABLE_KEY_STATUS      = 0xF3,
  /** A SET/GET request was issued with the identifier of a PIB
   * attribute that is not supported. */
  ZGP_MAC_UNSUPPORTED_ATTRIBUTE_STATUS = 0xF4,
  /** A request to send data was unsuccessful because neither the source address
   * parameters nor the destination address parameters were present. */
  ZGP_MAC_INVALID_ADDRESS_STATUS      = 0xF5,
  /** A receiver enable request was unsuccessful because it specified a number
   * of symbols that was longer than the beacon interval. */
  ZGP_MAC_ON_TIME_TOO_LONG_STATUS     = 0xF6,
  /** A receiver enable request was unsuccessful because it could not be
   * completed within the current superframe and was not permitted to be
   * deferred until the next superframe. */
  ZGP_MAC_PAST_TIME_STATUS            = 0xF7,
  /** The device was instructed to start sending beacons based on the
   * timing of the beacon transmissions of its coordinator, but the device
   * is not currently tracking the beacon of its coordinator. */
  ZGP_MAC_TRACKING_OFF_STATUS         = 0xF8,
  /** An attempt to write to a MAC PIB attribute that is in a table failed
   * because the specified table index was out of range. */
  ZGP_MAC_INVALID_INDEX_STATUS        = 0xF9,
  /** There are some unscanned channels yet, but there is no memory */
  ZGP_MAC_LIMIT_REACHED_STATUS        = 0xFA,
  /** A SET/GET request was issued with the identifier of an attribute
   * that is read only. */
  ZGP_MAC_READ_ONLY_STATUS            = 0xFB,
  /** A request to perform a scan operation failed because the MLME was
   * in the process of performing a previously initiated scan operation. */
  ZGP_MAC_SCAN_IN_PROGRESS_STATUS     = 0xFC,
  /** The device was instructed to start sending beacons based on the timing of
   * the beacon transmissions of its coordinator, but the instructed start time
   * overlapped the transmission time of the beacon of its coordinator. */
  ZGP_MAC_SUPERFRAME_OVERLAP_STATUS   = 0xFD
}ZGP_Status_t;

/******************************************************************************
                                ZGP Application IDs
******************************************************************************/
typedef enum PACK _ZGP_ApplicationId_t
{
  ZGP_SRC_APPID       =  0x00,
  ZGP_LPED_APPID      =  0x01,
  ZGP_IEEE_ADDR_APPID =  0x02
} ZGP_ApplicationId_t;

/******************************************************************************
                                ZGP Command IDs
******************************************************************************/
typedef enum _ZGP_CommandId_t
{
  ZGP_OFF_CMD_ID                      = 0x20,
  ZGP_ON_CMD_ID                       = 0x21,
  ZGP_MANUFAC_SPECIFIC_CMD0_FRAME     = 0xB0,
  ZGP_MANUFAC_SPECIFIC_CMD1_FRAME     = 0xB1,
  ZGP_MANUFAC_SPECIFIC_CMD2_FRAME     = 0xB2,
  ZGP_MANUFAC_SPECIFIC_CMD3_FRAME     = 0xB3,
  ZGP_MANUFAC_SPECIFIC_CMD4_FRAME     = 0xB4,
  ZGP_MANUFAC_SPECIFIC_CMD5_FRAME     = 0xB5,
  ZGP_MANUFAC_SPECIFIC_CMD6_FRAME     = 0xB6,
  ZGP_MANUFAC_SPECIFIC_CMD7_FRAME     = 0xB7,
  ZGP_MANUFAC_SPECIFIC_CMD8_FRAME     = 0xB8,
  ZGP_MANUFAC_SPECIFIC_CMD9_FRAME     = 0xB9,
  ZGP_MANUFAC_SPECIFIC_CMDA_FRAME     = 0xBA,
  ZGP_MANUFAC_SPECIFIC_CMDB_FRAME     = 0xBB,
  ZGP_MANUFAC_SPECIFIC_CMDC_FRAME     = 0xBC,
  ZGP_MANUFAC_SPECIFIC_CMDD_FRAME     = 0xBD,
  ZGP_MANUFAC_SPECIFIC_CMDE_FRAME     = 0xBE,
  ZGP_MANUFAC_SPECIFIC_CMDF_FRAME     = 0xBF,
  ZGP_COMMISSIONING_CMD_ID            = 0xE0,
  ZGP_DECOMMISSIONING_CMD_ID          = 0xE1,
  ZGP_COMMISSIONING_SUCCESS_CMD_ID    = 0xE2,
  ZGP_CHANNEL_REQUEST_CMD_ID          = 0xE3,
  ZGP_APPLICATION_DESCRIPTION_CMD_ID  = 0xE4,
  ZGP_RESERVED_CMD1 = 0xE5,
  ZGP_RESERVED_CMD2 = 0xE6,
  ZGP_RESERVED_CMD3 = 0xE7,
  ZGP_RESERVED_CMD4 = 0xE8,
  ZGP_RESERVED_CMD5 = 0xE9,
  ZGP_RESERVED_CMD6 = 0xEA,
  ZGP_RESERVED_CMD7 = 0xEB,
  ZGP_RESERVED_CMD8 = 0xEC,
  ZGP_RESERVED_CMD9 = 0xED,
  ZGP_RESERVED_CMDA = 0xEE,
  ZGP_RESERVED_CMDB = 0xEF,
  ZGP_COMMISSIONING_REPLY_CMD_ID      = 0xF0,
  ZGP_CHANNEL_CONFIG_CMD_ID           = 0xF3
} ZGP_CommandId_t;

/******************************************************************************
                                ZGP Frame Types
******************************************************************************/
typedef enum _ZgpFrameType_t
{
  ZGP_FRAME_DATA             = 0x00,
  ZGP_FRAME_MAINTENANCE      = 0x01
}ZGP_FrameType_t;

/******************************************************************************
                                ZGP Direction
******************************************************************************/
typedef enum _ZgpFrameDir_t
{
  ZGP_TX_BY_ZGPD             =0x00,
  ZGP_TX_TO_ZGPD             =0x01
}ZGP_FrameDir_t;

/******************************************************************************
                               ZGP Security Level
Different level of security is used in ZGP. Please refer A.1.4.1.3 for details.
******************************************************************************/
typedef enum _ZgpSecLevel_t
{
  ZGP_SECURITY_LEVEL_0,
  ZGP_SECURITY_LEVEL_1,
  ZGP_SECURITY_LEVEL_2,
  ZGP_SECURITY_LEVEL_3
}ZGP_SecLevel_t;

/******************************************************************************
                               ZGP Security Key Type
Different types of security keys are available in ZGP. Please refer A.1.5.3.3
for details.
******************************************************************************/
typedef enum _ZgpSecKeyType_t
{
  ZGP_KEY_TYPE_NO_KEY = 0x00,
  ZGP_KEY_TYPE_NWK_KEY = 0x01,
  ZGP_KEY_TYPE_ZGPD_GROUP_KEY = 0x02,
  ZGP_KEY_TYPE_NWKKEY_DERIVED_GROUP_KEY = 0x03,
  ZGP_KEY_TYPE_OOB_ZGPD_KEY = 0x04,
  ZGP_KEY_TYPE_RESERVED1 = 0x05,
  ZGP_KEY_TYPE_RESERVED2 = 0x06,
  ZGP_KEY_TYPE_DERIVED_INDIVIDUAL_ZGPD_KEY = 0x07
}ZGP_SecKeyType_t;

/******************************************************************************
                                 ZGP Security Key
Different security keys are used in ZGP. Please refer A.1.4.1.3 / A.3.9.3 for details.
******************************************************************************/
typedef enum _zgpSecKey_t
{
  ZGP_SECURITY_KEY_NONE               = 0x00,
  ZGP_SECURITY_KEY_SHARED             = 0x00,
  ZGP_SECURITY_KEY_INDIVIDUAL         = 0x01
}ZGP_SecKey_t;

/******************************************************************************
                              ZGP Generic Device IDs
******************************************************************************/
typedef enum _ZGP_DeviceId_t
{
  ZGP_SIMPLE_GENERIC_1_STATE_SWITCH = 0x00,
  ZGP_SIMPLE_GENERIC_2_STATE_SWITCH,
  ZGP_ON_OFF_SWITCH,
  ZGP_LEVEL_CONTROL_SWITCH,
  ZGP_SIMPLE_SENSOR,
  ZGP_ADVANCED_GENERIC_1_STATE_SWITCH,
  ZGP_ADVANCED_GENERIC_2_STATE_SWITCH,
  ZGP_GENERIC_8_CONTACT_SWITCH,
  ZGP_COLOR_DIMMER_SWITCH = 0x10,
  ZGP_LIGHT_SENSOR,
  ZGP_OCCUPANCY_SENSOR,
  ZGP_DOOR_LOCK_CONTROLLER = 0x20,
  ZGP_TEMPERATURE_SENSOR = 0x30,
  ZGP_PRESSUR_SENSOR,
  ZGP_FLOW_SENSOR,
  ZGP_INDOOR_MOVEMENT_SENSOR,
  ZGP_UNSPECIFIED_DEVICE_ID = 0xFE
}ZGP_DeviceId_t;

/******************************************************************************
                               ZGP Nwk FCF
Different fields of network fcf in GPDF
******************************************************************************/
BEGIN_PACK
typedef struct _ZgpNwkFrameControl_t
{
  ZGP_FrameType_t frametype  : 2;
  uint8_t zigbeeProtVersion : 4;
  uint8_t autoCommissioning : 1;
  uint8_t isExtNwkFcf       : 1;
}ZGP_NwkFrameControl_t;
END_PACK

/******************************************************************************
                               ZGP Nwk FCF
Different fields of Extended network fcf in GPDF
******************************************************************************/
BEGIN_PACK
typedef struct _ZgpExtNwkFrameControl_t
{
  uint8_t appId             : 3;
  uint8_t secLevel          : 2;
  ZGP_SecKey_t secKey       : 1;
  uint8_t rxAfterTx         : 1;
  ZGP_FrameDir_t direction  : 1;
}ZGP_ExtNwkFrameControl_t;
END_PACK

typedef uint32_t ZgpSourceId_t;
#endif //_ZGPCOMMON_H

//eof zgpCommon.h
