/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDConfig.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_diagnostic.h"
#include "zcl_power_profile.h"

#include "zcl_watermeter.h"

/*********************************************************************
 * CONSTANTS
 */

#define WATERMETER_DEVICE_VERSION     0
#define WATERMETER_FLAGS              0

#define WATERMETER_HWVERSION          1
#define WATERMETER_ZCLVERSION         1

// Basic Cluster
const uint8 zclWaterMeter_HWRevision = WATERMETER_HWVERSION;
const uint8 zclWaterMeter_ZCLVersion = WATERMETER_ZCLVERSION;
const uint8 zclWaterMeter_ManufacturerName[] = { 13 , 'O','p','e','n','S','m','a','r','t','H','o','m','e' };
const uint8 zclWaterMeter_ModelId[] = { 5, 'Z','i','g','W','M' };
const uint8 zclWaterMeter_DateCode[] = { 16, '2','0','2','0','1','0','0','1',' ',' ',' ',' ',' ',' ',' ',' ' };
const uint8 zclWaterMeter_PowerSource = POWER_SOURCE_BATTERY;

uint8 zclWaterMeter_LocationDescription[17] = { 16, ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
uint8 zclWaterMeter_PhysicalEnvironment = 0;
uint8 zclWaterMeter_DeviceEnable = DEVICE_ENABLED;

uint16 zclWaterMeter_IdentifyTime = 0;

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */
CONST zclAttrRec_t zclWaterMeter_Attrs[] =
{
  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GEN_BASIC,             // Cluster IDs - defined in the foundation (ie. zcl.h)
    {  // Attribute record
      ATTRID_BASIC_HW_VERSION,            // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT8,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                // Variable access control - found in zcl.h
      (void *)&zclWaterMeter_HWRevision  // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_ZCL_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zclWaterMeter_ZCLVersion
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclWaterMeter_ManufacturerName
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_MODEL_ID,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclWaterMeter_ModelId
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DATE_CODE,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclWaterMeter_DateCode
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zclWaterMeter_PowerSource
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_LOCATION_DESC,
      ZCL_DATATYPE_CHAR_STR,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)zclWaterMeter_LocationDescription
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_PHYSICAL_ENV,
      ZCL_DATATYPE_UINT8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclWaterMeter_PhysicalEnvironment
    }
  },
  {
    ZCL_CLUSTER_ID_GEN_BASIC,
    { // Attribute record
      ATTRID_BASIC_DEVICE_ENABLED,
      ZCL_DATATYPE_UINT8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclWaterMeter_DeviceEnable
    }
  },
  
#ifdef ZCL_IDENTIFY
  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GEN_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclWaterMeter_IdentifyTime
    }
  },
#endif
};

uint8 CONST zclWaterMeter_NumAttributes = ( sizeof(zclWaterMeter_Attrs) / sizeof(zclWaterMeter_Attrs[0]) );

/*********************************************************************
 * SIMPLE DESCRIPTOR
 */
// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
const cId_t zclWaterMeter_InClusterList[] =
{
  ZCL_CLUSTER_ID_GEN_BASIC,
  ZCL_CLUSTER_ID_GEN_IDENTIFY
};

#define ZCLWATERMETER_MAX_INCLUSTERS       (sizeof(zclWaterMeter_InClusterList) / sizeof(zclWaterMeter_InClusterList[0]))

const cId_t zclWaterMeter_OutClusterList[] =
{
  ZCL_CLUSTER_ID_GEN_BASIC
};

#define ZCLWATERMETER_MAX_OUTCLUSTERS     (sizeof( zclWaterMeter_OutClusterList ) / sizeof( zclWaterMeter_OutClusterList[0]))


SimpleDescriptionFormat_t zclWaterMeter_SimpleDesc =
{
  WATERMETER_ENDPOINT,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                  //  uint16 AppProfId[2];
  ZCL_HA_DEVICEID_ON_OFF_SWITCH,      //  uint16 AppDeviceId[2];
  WATERMETER_DEVICE_VERSION,            //  int   AppDevVer:4;
  WATERMETER_FLAGS,                     //  int   AppFlags:4;
  ZCLWATERMETER_MAX_INCLUSTERS,         //  byte  AppNumInClusters;
  (cId_t *)zclWaterMeter_InClusterList, //  byte *pAppInClusterList;
  ZCLWATERMETER_MAX_OUTCLUSTERS,        //  byte  AppNumInClusters;
  (cId_t *)zclWaterMeter_OutClusterList //  byte *pAppInClusterList;
};

/****************************************************************************
****************************************************************************/


