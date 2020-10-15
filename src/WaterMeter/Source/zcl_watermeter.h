#ifndef ZCL_WATERMETER_H
#define ZCL_WATERMETER_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"

/*********************************************************************
 * CONSTANTS
 */
#define WATERMETER_ENDPOINT           8

// Events for the sample app
#define WATERMETER_READ_SENSOR_EVT              0x0001 
#define WATERMETER_REPORTING_EVT		          0x0002
#define WATERMETER_IDENTIFY_TIMEOUT_EVT		  0x0003

extern CONST uint8 zclWaterMeter_NumAttributes;

// custom attributes
#define ATTRID_STATUS 41363
#define ATTRID_COUNTER 41364
#define ATTRID_THRESHOLD 41365

/*********************************************************************
 * VARIABLES
 */
extern SimpleDescriptionFormat_t zclWaterMeter_SimpleDesc;

extern CONST zclAttrRec_t zclWaterMeter_Attrs[];

extern uint16 zclWaterMeter_IdentifyTime;

extern uint16 zclWaterMeterSeqNum;

extern uint8 zclWaterMeter_Status;

extern uint32 zclWaterMeter_Counter;

extern uint16 zclWaterMeter_Threshold;

/*********************************************************************
 * FUNCTIONS
 */

extern void zclWaterMeter_Init( byte task_id );

extern UINT16 zclWaterMeter_event_loop( byte task_id, UINT16 events );

extern void wm_sendReport(void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_WATERMETER_H */
