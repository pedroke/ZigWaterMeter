#include "ZComDef.h"
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_watermeter.h"
#include "onboard.h"
#include "hal_adc.h"

/*********************************************************************
 * GLOBAL VARIABLES
 */
byte zclWaterMeter_TaskID;

uint16 zclWaterMeterSeqNum = 0;
uint8 zclWaterMeter_Status = 0;
uint32 zclWaterMeter_Counter = 0;
uint16 zclWaterMeter_Threshold = 440;

static uint16 countAlive = 0;
static uint32 lastCounter = 0;
static uint8 isOnReflective = 0;

static const uint32 WM_MAX_COUNTER = 0xFFFFFFFF;
/*********************************************************************
 * LOCAL VARIABLES
 */
afAddrType_t zclWaterMeter_DstAddr;

// Endpoint to allow SYS_APP_MSGs
static endPointDesc_t waterMeter_TestEp =
{
  WATERMETER_ENDPOINT,                  // endpoint
  &zclWaterMeter_TaskID,
  (SimpleDescriptionFormat_t *)NULL,  // No Simple description for this test endpoint
  (afNetworkLatencyReq_t)0            // No Network Latency req
};

devStates_t zclWaterMeter_NwkState = DEV_INIT;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
// Functions to process ZCL Foundation incoming Command/Response messages
static void zclWaterMeter_ProcessIncomingMsg( zclIncomingMsg_t *msg );
static uint8 zclWaterMeter_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg );
static void zclWaterMeter_ProcessIdentifyTimeChange(void);
static void zclWaterMeter_BasicResetCB(void);
static void zclWaterMeter_IdentifyCB( zclIdentify_t *pCmd );

// Water meter
static void wm_initPorts(void);
static void wm_readSensor(void);
/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclWaterMeter_CmdCallbacks =
{
  zclWaterMeter_BasicResetCB,               // Basic Cluster Reset command
  zclWaterMeter_IdentifyCB,                 // Identify command
  NULL,                                   // Identify Trigger Effect command
  NULL,                                   // Identify Query Response command
  NULL,                                   // On/Off cluster commands
  NULL,                                   // On/Off cluster enhanced command Off with Effect
  NULL,                                   // On/Off cluster enhanced command On with Recall Global Scene
  NULL,                                   // On/Off cluster enhanced command On with Timed Off
  NULL,                                   // RSSI Location command
  NULL                                    // RSSI Location Response command
};


/*********************************************************************
 * @fn          zclWaterMeter_Init
 *
 * @brief       Initialization function for the zclGeneral layer.
 *
 * @param       none
 *
 * @return      none
 */
void zclWaterMeter_Init( byte task_id )
{
  zclWaterMeter_TaskID = task_id;
  
  wm_initPorts();
  // Set destination address to indirect
  zclWaterMeter_DstAddr.addrMode = (afAddrMode_t)AddrNotPresent;
  zclWaterMeter_DstAddr.endPoint = 0;
  zclWaterMeter_DstAddr.addr.shortAddr = 0;

  // This app is part of the Home Automation Profile
  zclHA_Init( &zclWaterMeter_SimpleDesc );

  // Register the ZCL General Cluster Library callback functions
  zclGeneral_RegisterCmdCallbacks( WATERMETER_ENDPOINT, &zclWaterMeter_CmdCallbacks );

  // Register the application's attribute list
  zcl_registerAttrList( WATERMETER_ENDPOINT, zclWaterMeter_NumAttributes, zclWaterMeter_Attrs );

  // Register the Application to receive the unprocessed Foundation command/response messages
  zcl_registerForMsg( zclWaterMeter_TaskID );
  
  // Register for a test endpoint
  afRegister( &waterMeter_TestEp );

  osal_start_timerEx( zclWaterMeter_TaskID, WATERMETER_REPORTING_EVT, 30000 );
  osal_start_timerEx( zclWaterMeter_TaskID, WATERMETER_READ_SENSOR_EVT, 500 );
}

void wm_sendReport(void)
{
		// send report
		zclReportCmd_t *pReportCmd;
  
		uint8 NUM_ATTRS = 3;
		pReportCmd = osal_mem_alloc( sizeof(zclReportCmd_t) + ( NUM_ATTRS * sizeof(zclReport_t) ) );
		if ( pReportCmd != NULL )
		{
			pReportCmd->numAttr = NUM_ATTRS;
    
			pReportCmd->attrList[0].attrID = ATTRID_STATUS;
			pReportCmd->attrList[0].dataType = ZCL_DATATYPE_BOOLEAN;
			pReportCmd->attrList[0].attrData = (void *)(&zclWaterMeter_Status);
    
			pReportCmd->attrList[1].attrID = ATTRID_COUNTER;
			pReportCmd->attrList[1].dataType = ZCL_DATATYPE_UINT32;
			pReportCmd->attrList[1].attrData = (void *)(&zclWaterMeter_Counter);

			pReportCmd->attrList[2].attrID = ATTRID_THRESHOLD;
			pReportCmd->attrList[2].dataType = ZCL_DATATYPE_UINT16;
			pReportCmd->attrList[2].attrData = (void *)(&zclWaterMeter_Threshold);
                        
			zclWaterMeter_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
			zclWaterMeter_DstAddr.addr.shortAddr = 0;
			zclWaterMeter_DstAddr.endPoint=1;
    
			zcl_SendReportCmd( WATERMETER_ENDPOINT, &zclWaterMeter_DstAddr, ZCL_CLUSTER_ID_GEN_BASIC, pReportCmd, ZCL_FRAME_CLIENT_SERVER_DIR, false, zclWaterMeterSeqNum++ );
                        countAlive = 0;
		}
  
		osal_mem_free( pReportCmd );
}

void wm_initPorts(void) {
  // IR LED transmiter = P1.1
  P1SEL = 0x00; // whole P1 port set as general I/O
  P1DIR = 0x02; // whole P1 port set as input, P1.1 output - LED
  P1INP = 0x02; // pull up the whole port, P1.1 - tristate
}

static void wm_readSensor(void) {
  P1_1 = 1; // turn on IR
  // Read from P0_2
  HalAdcSetReference(HAL_ADC_REF_AVDD);
  uint16 analogValue = HalAdcRead(HAL_ADC_CHN_AIN2, HAL_ADC_RESOLUTION_10);
  P1_1 = 0; // turn off IR

  uint8 signal = analogValue > zclWaterMeter_Threshold;
  
  if(signal) {
    if(!isOnReflective) {
          isOnReflective = 1;
          if(!zclWaterMeter_Status) {
            zclWaterMeter_Status = 1;
            wm_sendReport();
          }
	  if(zclWaterMeter_Counter == WM_MAX_COUNTER) {
		  wm_sendReport();
		  zclWaterMeter_Counter = 0;
		  lastCounter = 0;
	  } else {
		zclWaterMeter_Counter++;
	  }
    }
  } else if(isOnReflective) {
    isOnReflective = 0;
    if(!zclWaterMeter_Status) {
      zclWaterMeter_Status = 1;
      wm_sendReport();
    }
  }
}

/*********************************************************************
 * @fn          zclWaterMeter_event_loop
 *
 * @brief       Event Loop Processor for zclGeneral.
 *
 * @param       none
 *
 * @return      none
 */
uint16 zclWaterMeter_event_loop( uint8 task_id, uint16 events )
{
  afIncomingMSGPacket_t *MSGpkt;
  (void)task_id;  // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    while ( (MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( zclWaterMeter_TaskID )) )
    {
      switch ( MSGpkt->hdr.event )
      {
        case ZCL_INCOMING_MSG:
          // Incoming ZCL Foundation command/response messages
          zclWaterMeter_ProcessIncomingMsg( (zclIncomingMsg_t *)MSGpkt );
          break;
                    
        case ZDO_STATE_CHANGE:
          zclWaterMeter_NwkState = (devStates_t)(MSGpkt->hdr.status);

          // now on the network
          if ( (zclWaterMeter_NwkState == DEV_ZB_COORD) ||
               (zclWaterMeter_NwkState == DEV_ROUTER)   ||
               (zclWaterMeter_NwkState == DEV_END_DEVICE) )
          {
             // on network
          }
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & WATERMETER_REPORTING_EVT ) {
    countAlive++;
    if(zclWaterMeter_Counter > lastCounter) {
      lastCounter = zclWaterMeter_Counter;
    } else if(zclWaterMeter_Status) {
      zclWaterMeter_Status = 0;
      wm_sendReport();
    } else if(countAlive == 360) {
      wm_sendReport();
    }

    osal_start_timerEx( zclWaterMeter_TaskID, WATERMETER_REPORTING_EVT, 30000 );
    
    return ( events ^ WATERMETER_REPORTING_EVT );
  }

  if ( events & WATERMETER_READ_SENSOR_EVT ) {
    wm_readSensor();
    osal_start_timerEx( zclWaterMeter_TaskID, WATERMETER_READ_SENSOR_EVT, 500 );
    
    return ( events ^ WATERMETER_READ_SENSOR_EVT );
  }
  
  if ( events & WATERMETER_IDENTIFY_TIMEOUT_EVT )
  {
    if ( zclWaterMeter_IdentifyTime > 0 ) {
		zclWaterMeter_IdentifyTime--;
	}
	
    zclWaterMeter_ProcessIdentifyTimeChange();
    
    return ( events ^ WATERMETER_IDENTIFY_TIMEOUT_EVT );
  }  
  
  // Discard unknown events
  return 0;
}

static void zclWaterMeter_BasicResetCB( void )
{
  NLME_LeaveReq_t leaveReq;
  // Set every field to 0
  osal_memset( &leaveReq, 0, sizeof( NLME_LeaveReq_t ) );

  // This will enable the device to rejoin the network after reset.
  leaveReq.rejoin = TRUE;

  // Set the NV startup option to force a "new" join.
  zgWriteStartupOptions( ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE );

  // Leave the network, and reset afterwards
  if ( NLME_LeaveReq( &leaveReq ) != ZSuccess )
  {
    // Couldn't send out leave; prepare to reset anyway
    ZDApp_LeaveReset( FALSE );
  }
}

/******************************************************************************
 *
 *  Functions for processing ZCL Foundation incoming Command/Response messages
 *
 *****************************************************************************/

/*********************************************************************
 * @fn      zclWaterMeter_ProcessIncomingMsg
 *
 * @brief   Process ZCL Foundation incoming message
 *
 * @param   pInMsg - pointer to the received message
 *
 * @return  none
 */
static void zclWaterMeter_ProcessIncomingMsg( zclIncomingMsg_t *pInMsg )
{
  switch ( pInMsg->zclHdr.commandID )
  {
    case ZCL_CMD_DEFAULT_RSP:
      zclWaterMeter_ProcessInDefaultRspCmd( pInMsg );
      break;
    default:
      break;
  }

  if ( pInMsg->attrCmd )
    osal_mem_free( pInMsg->attrCmd );
}

static void zclWaterMeter_ProcessIdentifyTimeChange( void )
{
  if ( zclWaterMeter_IdentifyTime > 0 )
  {
    osal_start_timerEx( zclWaterMeter_TaskID, WATERMETER_IDENTIFY_TIMEOUT_EVT, 1000 );
  }
  else
  {
    osal_stop_timerEx( zclWaterMeter_TaskID, WATERMETER_IDENTIFY_TIMEOUT_EVT );
  }
}

static void zclWaterMeter_IdentifyCB( zclIdentify_t *pCmd )
{
  zclWaterMeter_IdentifyTime = pCmd->identifyTime;
  zclWaterMeter_ProcessIdentifyTimeChange();
}



/*********************************************************************
 * @fn      zclWaterMeter_ProcessInDefaultRspCmd
 *
 * @brief   Process the "Profile" Default Response Command
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  none
 */
static uint8 zclWaterMeter_ProcessInDefaultRspCmd( zclIncomingMsg_t *pInMsg )
{
  // zclDefaultRspCmd_t *defaultRspCmd = (zclDefaultRspCmd_t *)pInMsg->attrCmd;
  // Device is notified of the Default Response command.
  (void)pInMsg;
  return TRUE;
}

/****************************************************************************
****************************************************************************/