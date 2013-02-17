 /**
 *  \file commsCtrlDemo.c
 *
 *  \brief An application testing comms control task
 *
 *  \author $Author: Sam Jiang $
 *  \version 1.0
 *
 *  $Date: 2012-07-21 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */
#include "application.h"
#include "commsCtrlDemo.h"
#include "debug.h"
#include "ax25.h"
#include "lib_string.h"

static TaskToken COMMS_DEMO_TaskToken;
static portTASK_FUNCTION(vCommsDemoTask, pvParameters);
void Comms_Modem_Write_Str( const portCHAR * const pcString, unsigned portSHORT usStringLength, portSHORT sel );
void setModemTransmit(portSHORT sel);

void vCommsDemo_Init(unsigned portBASE_TYPE uxPriority)
{
	COMMS_DEMO_TaskToken = ActivateTask(TASK_COMMS_DEMO,
								"Comms_demo",
								APP_TASK_TYPE,
								uxPriority,
								20*APP_STACK_SIZE,
								vCommsDemoTask);
}


#define AX25_PID_NO_LAYER3_PROTOCOL_UI_MODE 0xF0
#define BLUESAT_SAT_SSID 0x2
#define BLUESAT_GS_SSID 0x7
#define MAX_PACKETS_SIZE_IN_BYTES 10

typedef struct _AX25BufferItem{
   char array[MAX_PACKETS_SIZE_IN_BYTES];
   int arraylength;
}AX25BufferItem;



static portTASK_FUNCTION(vCommsDemoTask, pvParameters)
{
	(void) pvParameters;
	unsigned int counter = 0;
	stateBlock present;
   AX25BufferItem input;

   protoReturn result;
	char actual [300];
	unsigned int actual_size = 300;
	memset (actual, 0, 300);
	memcpy (input.array,"abcedfghij",10);
	input.arraylength = 10;
	vDebugPrint(COMMS_DEMO_TaskToken, "%10s \n\r", input.array, NO_INSERT, NO_INSERT);

	//Build state block
   present.src = input.array;
   present.srcSize = 10;
   memcpy (present.route.dest.callSign,"BLUEGS",CALLSIGN_SIZE);
   memcpy (present.route.src.callSign, "BLUSAT",CALLSIGN_SIZE);
   vDebugPrint(COMMS_DEMO_TaskToken, "BLOCK TEXT \n\r", NO_INSERT, NO_INSERT, NO_INSERT);
   present.route.dest.callSignSize = 6;
   present.route.src.callSignSize = 6;
   present.route.dest.ssid = BLUESAT_GS_SSID;
   present.route.src.ssid = BLUESAT_SAT_SSID;
   present.route.repeats  = NULL;
   present.route.totalRepeats = 0;
   present.route.type = Response;
   present.presState = stateless;
   present.pid = AX25_PID_NO_LAYER3_PROTOCOL_UI_MODE;
   present.packetCnt = 0;
   present.nxtIndex = 0;
   present.mode = unconnected;
   present.completed = false;
   result = ax25Entry (&present, actual, &actual_size );
   vDebugPrint(COMMS_DEMO_TaskToken, "Message Encoded \n\r",NO_INSERT , NO_INSERT, NO_INSERT);

   //	setModemTransmit(1);
	while(1){
	      vDebugPrint(COMMS_DEMO_TaskToken, "Sending Message %d \n\r", counter++, NO_INSERT, NO_INSERT);
	     // Comms_Modem_Write_Str(actual,actual_size, 1);

	      vSleep(2000);
	}
}
