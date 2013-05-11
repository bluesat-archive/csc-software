 /**
 *  \file commsControl.h
 *
 *  \brief control task for message sending
 *  This task is now hacked to send telem log and beacon
 *
 *  \author $Author: Sam Jiang $
 *  \version 2.0
 *
 *  $Date: 4/5/2013
 */

#include "service.h"
#include "commsControl.h"
#include "switching.h"
#include "modem.h"
#include "debug.h"
#include "telemetry_storage.h"
#include "ax25.h"
#include "lib_string.h"

#define AX25_PID_NO_LAYER3_PROTOCOL_UI_MODE 0xF0

//global variable for modem usage
int modem;

//task token for accessing services
static TaskToken Comms_TaskToken;

//prototype for task function
static portTASK_FUNCTION(vCommsTask, pvParameters);

void vComms_Init(unsigned portBASE_TYPE uxPriority)
{

	Comms_TaskToken = ActivateTask(TASK_COMMS,
								   "Comms",
								   SEV_TASK_TYPE,
								   uxPriority,
								   SERV_STACK_SIZE,
								   vCommsTask);

}

/*
 * Init switching circuit
 * init modem
 * encode and send packet
 * wait 10 secs
 * encode and send packet
 *
 *
 * */

static portTASK_FUNCTION(vCommsTask, pvParameters)
{
	(void) pvParameters;
	struct telem_storage_entry_t temp;
	stateBlock present;
    char input [64];
    char actual [64];
    unsigned int actual_size = 64;
    int i, m;
	for ( ; ; )
	{
		// grab message from telem log
		telemetry_storage_read_cur(&temp);

		vDebugPrint(Comms_TaskToken,"got log\r\n",NO_INSERT,NO_INSERT,NO_INSERT);
		// compose string
		// hh:mm:ss\tID:-cc.c\t....
		for (i = 0, m = 100000; i < 6; i++, m/=10){
			input[i] = ((temp.timestamp/m)%10) - '0';
		}
		input[6] = '\r';
		// call ax25 to encode it
		present.src = input;
		present.srcSize = 7;
		memcpy (present.route.dest.callSign,"BLUSAT",CALLSIGN_SIZE);
		memcpy (present.route.src.callSign, "BLUEGS",CALLSIGN_SIZE);

		present.route.dest.callSignSize = 6;
		present.route.src.callSignSize = 6;
		present.route.dest.ssid = 1;
		present.route.src.ssid = 1;
		present.route.repeats  = NULL;
		present.route.totalRepeats = 0;
		present.route.type = Response;
		present.presState = stateless;
		present.pid = AX25_PID_NO_LAYER3_PROTOCOL_UI_MODE;
		present.packetCnt = 0;
		present.nxtIndex = 0;
		present.mode = unconnected;
		present.completed = false;

		ax25Entry (&present, actual, &actual_size );

		vDebugPrint(Comms_TaskToken, "%d, %23x\n\r",actual_size ,actual, NO_INSERT);

		// TX and modem will be chosen by GS
		switching_OPMODE(DEVICE_MODE);
		switching_TX_Device(AFSK_1);
		Comms_Modem_Write_Str(actual, actual_size);
		modem_takeSemaphore();

		vDebugPrint(Comms_TaskToken,"T\r\n",NO_INSERT,NO_INSERT,NO_INSERT);

		//now beacon
		switching_OPMODE(DEVICE_MODE);
		switching_TX_Device(BEACON);

		vDebugPrint(Comms_TaskToken, "Finished setup switching TX device, transmit for %d ms\n\r", TRANSMISSION_TIME ,0, 0);

		vSleep( TRANSMISSION_TIME );
		//turn off beacon
		switching_TX_Device(AFSK_1);
	}
}
/*
int iSendData(TaskToken token, char *data, int size)
{
	Message m;
	m.data = data;
	m.size = size;
	MessagePacket mp;
	mp.Src = token->enTaskID;
	mp.Dest = TASK_COMMS;
	mp.Token = Comms_TaskToken;
	mp.Data = (unsigned long)&m;

	return enProcessRequest(&mp,  0);
}*/
