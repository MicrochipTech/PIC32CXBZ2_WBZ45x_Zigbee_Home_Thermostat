/*******************************************************************************
  Zigbee green power cluster structure Header File

  Company:
    Microchip Technology Inc.

  File Name:
    zgpClusterStructure.h

  Summary:
    This file contains the header file describes the ZGP Structures involved.

  Description:
    This file contains the header file describes the ZGP Structures involved.
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

#ifndef _ZGPCLUSTERSTRUCTURE_H
#define _ZGPCLUSTERSTRUCTURE_H

/*!
Attributes and commands for determining basic information about a device,
setting user device information such as location, enabling a device and resetting it
to factory defaults.
*/
#ifdef _GREENPOWER_SUPPORT_
/*******************************************************************************
                   Includes section
*******************************************************************************/

#include <zcl/include/zcl.h>
#include <zcl/include/clusters.h>
#include <configserver/include/csDefaults.h>
#include <zgp/GPInfrastructure/lowZgp/include/zgpLowNvmTable.h>
#include <zgp/GPInfrastructure/highZgp/include/zgpMiddleProxy.h>

/*******************************************************************************
                   Define(s) section
*******************************************************************************/
#define NO_OF_SINK     1
#define PROXY_BLOCKEDID_TABLE_SIZE 0
#define DMIN_U 5u
#define DMIN_B 32u
#define DMAX   100u

#ifndef ZGP_ATTRIBUTE_MAX_SIZE
#define ZGP_ATTRIBUTE_MAX_SIZE       4 // bytes
#endif

#ifndef ZGP_MAX_ATTR_RECORD_IN_A_DATA_POINTER
#define ZGP_MAX_ATTR_RECORD_IN_A_DATA_POINTER  2
#endif

BEGIN_PACK
/***************************************************************************//**
  \brief
    Describes the greenPower Cluster client side generated commands
*******************************************************************************/

typedef enum _ZGP_SinkTableActions_t
{
  NO_ACTION = 0,
  EXTEND_SINKTABLE_ENTRY = 1,
  REPLACE_SINKTABLE_ENTRY = 2,
  REMOVE_PAIRING = 3,
  REMOVE_GPD = 4,
  APPLICATION_DESCRIPTION = 5
} ZGP_SinkTableActions_t;

typedef enum PACK _ZGP_SinkFunctionality_t
{
  GPS_FEATURE                        = 0x000001,
  GPS_STUB                           = 0x000002,
  GPS_DERIVED_GROUP_COMM             = 0x000004,
  GPS_PRE_COMM_GROUP_COMM            = 0x000008,
  GPS_FULL_UNICAST_COMM              = 0x000010,
  GPS_LIGHTWEIGHT_UNICAST_COMM       = 0x000020,
  GPS_BIDIRECTIONAL_OPERATION        = 0x000080,
  GPS_PROXY_TABLE_MAINTENANCE        = 0x000100,
  GPS_PROXIMITY_COMMISSIONING        = 0x000200,
  GPS_MULTIHOP_COMMISSIONING         = 0x000400,
  GPS_CT_BASED_COMMISSIONING         = 0x000800,
  GPS_GPD_MAINTENANCE                = 0x001000,
  GPS_SECURITY_LEVEL_0               = 0x002000,
  GPS_SECURITY_LEVEL_1               = 0x004000,
  GPS_SECURITY_LEVEL_2               = 0x008000,
  GPS_SECURITY_LEVEL_3               = 0x010000,
  GPS_TRANSLATION_TABLE              = 0x040000,
  GPS_IEEE_ADDR                      = 0x080000,
  GPS_COMPACT_ATTRIBUTE_REPORTING    = 0x100000,
}ZGP_SinkFunctionality_t;

typedef enum PACK _ZGP_SinkActiveFunctionality_t
{
  GPS_GP_FUNCTIONALITY               = 0x000001,
}ZGP_SinkActiveFunctionality_t;

typedef enum PACK _ZGP_ProxyFunctionality_t
{
  GPP_FEATURE                        = 0x000001,
  GPP_STUB                           = 0x000002,
  GPP_DERIVED_GROUP_COMM             = 0x000004,
  GPP_PRE_COMM_GROUP_COMM            = 0x000008,
  GPP_FULL_UNICAST_COMM              = 0x000010,
  GPP_LIGHTWEIGHT_UNICAST_COMM       = 0x000020,
  GPP_BIDIRECTIONAL_OPERATION        = 0x000080,
  GPP_PROXY_TABLE_MAINTENANCE        = 0x000100,
  GPP_COMMISSIONING                  = 0x000400,
  GPP_CT_BASED_COMMISSIONING         = 0x000800,
  GPP_GPD_MAINENANCE                 = 0x001000,
  GPP_SECURITY_LEVEL_0               = 0x002000,
  GPP_SECURITY_LEVEL_1               = 0x004000,
  GPP_SECURITY_LEVEL_2               = 0x008000,
  GPP_SECURITY_LEVEL_3               = 0x010000,
  GPP_IEEE_ADDR                      = 0x080000,
}ZGP_ProxyFunctionality_t;

typedef enum PACK _ZGP_ProxyActiveFunctionality_t
{
  GPP_GP_FUNCTIONALITY               = 0x000001,
}ZGP_ProxyActiveFunctionality_t;

// GP pairing search command
typedef struct PACK
{
  uint16_t appId                         : 3;
  uint16_t reqUnicastSinks               : 1;
  uint16_t reqDerivedGrpCastSinks        : 1;
  uint16_t reqCommissionedGrpCastSinks   : 1;
  uint16_t reqGpdSecurityFrameCounter    : 1;
  uint16_t reqGpdSecKey                  : 1;
  uint16_t reserved                      : 8;
}ZGP_GpPairingSearchOptions_t;

typedef struct PACK
{
  ZGP_GpPairingSearchOptions_t options;
  ZGP_GpdId_t gpdId;
  uint8_t endpoint;
} ZGP_GpPairingSearch_t;

// GP tunneling stop command
typedef struct PACK
{
  uint8_t appId                         : 3;
  uint8_t alsoDerivedGroup              : 1;
  uint8_t alsoCommissionedGroup         : 1;
  uint8_t unidirectionalOnly            : 1;
  uint8_t reserved                      : 2;
}zgpTunnelingOptions_t;

typedef struct PACK
{
  zgpTunnelingOptions_t options;
  ZGP_GpdId_t  gpdId;
  uint8_t  endpoint;
  uint32_t gpdSecurityFrameCounter;
  uint16_t gpdShortAddress;
  uint8_t  gppDistance;
} ZGP_TunnelingStop_t;
// GP commissioning notification command


// GP proxy table response command
typedef struct PACK
{
  ZCL_Status_t status;
  uint8_t noOfNonEmptyTableEntries;
  uint8_t startIndex;
  uint8_t entriesCount;
  uint8_t tableEntries[ZCL_MAX_TX_ZSDU_SIZE];
  // length byte of proxy table entry + length byte for LW unicast list + Length byte for sink group list
} ZGP_TableResp_t;

//GP proxy table request
typedef enum PACK
{
  GPD_ID_REF = 0x00,
  INDEX_REF = 0x01
}zgpTableReqType_t;

// GP proxy/sink table request options
typedef struct PACK
{
  uint8_t appId                         : 3;
  zgpTableReqType_t requestType      : 2;
  uint8_t reserved                      : 3;
}zgpTableReqOptions_t;

typedef struct PACK
{
  zgpTableReqOptions_t options;
  ZGP_GpdId_t gpdId;
  uint8_t ep;
  uint8_t index;
} ZGP_TableRequest_t;

// GP sink commissioning mode options
typedef struct PACK
{
  uint8_t action                        : 1;
  uint8_t involveGPMinSecurity          : 1;
  uint8_t involveGPMinPairing           : 1;
  uint8_t involveProxies                : 1;
  uint8_t reserved                      : 4;
}ZGP_SinkCommissioningModeOptions_t;

typedef struct PACK
{
  ZGP_SinkCommissioningModeOptions_t options;
  uint16_t gpmAddrForSec;
  uint16_t gpmAddrForPairing;
  uint8_t sinkEndPoint;
} ZGP_SinkCommissioningMode_t;

typedef struct PACK
{
  uint8_t minGpdSecurityLevel      : 2;
  uint8_t protectionWithGpLinkKey  : 1;
  uint8_t involveTc                : 1;
  uint8_t reserver                 : 4;
} ZGP_GpsSecurityLevelAttribute_t;

/***************************************************************************//**
  \brief
    Describes the GreenPower Cluster server side generated commands
*******************************************************************************/
// GP notification response command
typedef struct PACK
{
  uint8_t appId             : 3;
  uint8_t firstToForward    : 1;
  uint8_t noPairing         : 1;
  uint8_t reserved          : 3;
}zgpGpNotifyRespOptions_t;

typedef struct PACK
{
  // 1 - Options, 4/8 - Src ID/IEEE addr, 1 - Endpoint, 4 - GPD secuiryt frame counter
  zgpGpNotifyRespOptions_t options;
  ZGP_GpdId_t gpdId;
  uint8_t endpoint;
  uint32_t gpdSecurityFrameCounter;
} ZGP_GpNotificationResp_t;

typedef struct
{
  uint32_t appId                    : 3;
  uint32_t addSink                  : 1;
  uint32_t removeGpd                : 1;
  uint32_t communicationMode        : 2;
  uint32_t gpdFixed                 : 1;
  uint32_t gpdMacSeqNumCapability   : 1;
  uint32_t securityLevel            : 2;
  uint32_t securityKeyType          : 3;
  uint32_t gpdSecurityFrameCounterPresent  : 1;
  uint32_t gpdSecurityKeyPresent       : 1;
  uint32_t assignedAliasPresent     : 1;
  uint32_t groupCastRadiusPresent  : 1;
  uint32_t reserved                 : 6;
  uint32_t reserved_padding         : 8;  
}zgpGpPairingOptions_t;

typedef struct PACK
{
  ZGP_GpdId_t   gpdId;
  uint64_t  sinkIeeeAddr;
  zgpGpPairingOptions_t options;
  uint32_t  gpdSecurityFrameCounter;
  uint16_t  sinkNwkAddr;
  uint16_t  sinkGroupId;
  uint16_t  assignedAlias;
  uint8_t   endpoint;
  uint8_t   deviceId;
  uint8_t   groupCastRadius;
  uint8_t   gpdKey[16];
} ZGP_GpPairing_t;

//GP proxy commissioning mode
typedef enum PACK
{
  GPP_EXIT_ON_FIRST_PAIRING_SUCCESS = 0x01,
  GPP_EXIT_ON_EXIT_PROXY_COMM_MODE = 0x02
}zgpGpProxyCommModeExitMode_t;

typedef struct PACK
{
  uint8_t action               : 1;
  uint8_t commWindowPresent    : 1;
  zgpGpProxyCommModeExitMode_t exitMode  : 2;
  uint8_t channelPresent       : 1;
  uint8_t unicastCommunication : 1;
  uint8_t reserved             : 2;
}zgpGpProxyCommModeOptions_t;

typedef struct PACK
{
  // 1 - Options, 2 - Commissioning window, 1 -channel
  uint16_t commWindow;
  zgpGpProxyCommModeOptions_t options;
  uint8_t channel;
  uint16_t srcSinkNwkAddress;
} ZGP_GpProxyCommMode_t;

//GP response
typedef struct PACK
{
  uint8_t appId               : 3;
  uint8_t txOnEndpointMatch   : 1;
  uint8_t reserved            : 4;
}zgpGpRespOptions_t;

typedef struct PACK
{
  zgpGpRespOptions_t options;
  uint16_t tempMasterShortAddr;
  uint8_t tempMasterTxChannel;
  ZGP_GpdId_t gpdId;
  uint8_t endpoint;
  uint8_t gpdCmdId;
  uint8_t gpdCmdPayloadLength;
  uint8_t gpdCmdPayload[MAX_PAYLOAD_BY_GPD];
  bool nonUnicast;
} ZGP_GpResp_t;

//GP pairing Configuration Actions command
typedef struct PACK
{
  uint8_t action        :3;
  uint8_t sendGpPairing :1;
  uint8_t reserved      :4;
}ZgpGpPairingConfigActions_t;

//GP pairing Configuration Options command
typedef struct PACK
{
  uint16_t appId                         :3;
  uint16_t communicationMode             :2;
  uint16_t seqNumCapabilities            :1;
  uint16_t rxOnCapability                :1;
  uint16_t fixedLocation                 :1;
  uint16_t assignedAlias                 :1;
  uint16_t securityUse                   :1;
  uint16_t applicationInfoPresent        :1;
  uint16_t reserved                      :5;
}ZgpGpPairingConfigOptions_t;

//GP pairing Configuration App information
typedef struct PACK
{
  uint8_t manufacturerIdPresent          :1;
  uint8_t modelIdPresent                 :1;
  uint8_t gpdCommandspresent             :1;
  uint8_t clusterListPresent             :1;
  uint8_t reserved                       :4;
}ZgpGpAppInfo_t;

typedef struct PACK _ZgpGppGpdLink_t
{
  uint8_t rssi        :6;
  uint8_t linkQuality :2;
}zgpGppGpdLink_t;

//GP pairing Configuration
typedef struct PACK
{
  //ID of the GPD
  union
  {
    uint32_t srcId;
    uint8_t  srcExtAddress[8];
  } addrinfo;

  //The incoming security frame counter for the GPD
  uint32_t gpdSecurityFrameCounter;
  //The security key for the GPD
  uint8_t securityKey[SECURITY_KEY_SIZE];

  //The 16-bit GroupID and alias for the group communication.
  uint8_t *groupList;
  uint8_t *gpdCmdList;
  uint8_t *clusterlist;
  uint8_t *pairedEndpoints;

  ZgpGpPairingConfigOptions_t options;
  //The commissioned 16-bit ID to be used as alias for this GPD
  uint16_t gpdAssignedAlias;

  uint16_t manufacturerId;
  uint16_t modelId;
  ZgpGpPairingConfigActions_t actions;

  //GPD endpoint
  uint8_t endPoint;
  //To limit the range of the group-cast
  uint8_t groupCastRadius;
  //The security options
  zgpSecurityOptions_t securityOptions;
  uint8_t deviceId;
  //indicates the number of endpoints listed in the Paired endpoints field
  uint8_t noOfPairedEndPoints;
  ZgpGpAppInfo_t appInformation;
  uint8_t noOfGpdCmds;
}ZGP_GpPairingConfig_t;

// Attribute record in data point descriptor of application description command
typedef struct PACK
{
  uint16_t attrId;
  uint8_t attrDataType;
  uint8_t attrOptions;
  uint8_t attrOffsetWithinReport;
  uint8_t attrValue[ZGP_ATTRIBUTE_MAX_SIZE];
}ZGP_AttributeRecord_t;

// Data point descriptor of application description command
typedef struct PACK
{
  ZGP_AttributeRecord_t attrRecord[ZGP_MAX_ATTR_RECORD_IN_A_DATA_POINTER]; // array of attribute records
  void *nextDataPointDescriptor;
  uint16_t clusterId;
  uint16_t manufacturerId;
  uint8_t dataPointOptions;
  bool busy;
}ZGP_DataPointDescriptor_t;

// Report descriptor of application description command
typedef struct PACK
{
  ZGP_DataPointDescriptor_t *dataPointDescriptor;
  void *nextReportDescriptor;
  uint16_t timeoutPeriod;
  uint8_t reportId; // if it is free, then the value would be 0xFF
  uint8_t reportOptions;
  uint8_t noOfDatapointDescriptor;
} ZGP_ReportDescriptor_t;

END_PACK
#endif /* _ZGPCLUSTERSTRUCTURE_H */
#endif
