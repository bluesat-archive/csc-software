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
#include "Comms_DTMF.h"



#define AX25_PID_NO_LAYER3_PROTOCOL_UI_MODE 0xF0

//global variable for modem usage
int modem;

//task token for accessing services
static TaskToken Comms_TaskToken;

//prototype for task function
static portTASK_FUNCTION(vCommsTask, pvParameters);

extern char DTMF_BUFF[DTMF_SIZE];
extern int DTMF_BUFF_SP; //Start position
extern int DTMF_BUFF_EP; //End position

void vComms_Init(unsigned portBASE_TYPE uxPriority)
{

	Comms_TaskToken = ActivateTask(TASK_COMMS,
								   "Comms",
								   SEV_TASK_TYPE,
								   uxPriority,
								   SERV_STACK_SIZE,
								   vCommsTask);

	led_init();

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
    char actual [256];
    memset (actual, 0, 256);
    unsigned int actual_size = 0;
    int i, m;
	for ( ; ; )
	{
		// grab message from telem log
		//telemetry_storage_read_cur(&temp);

		vDebugPrint(Comms_TaskToken,"SP %d EP %d\r\n",DTMF_BUFF_SP,DTMF_BUFF_EP,NO_INSERT);

		// compose string
		// hh:mm:ss\tID:-cc.c\t....
		//for (i = 0, m = 100000; i < 6; i++, m/=10){
			//input[i] = ((temp.timestamp/m)%10) - '0';
		//}
		if (DTMF_BUFF_SP == DTMF_BUFF_EP){

			input[0] = 'N';
			input[1] = 'o';
			input[2] = ' ';
			input[3] = 'n';
			input[4] = 'e';
			input[5] = 'w';
			input[6] = ' ';
			input[7] = 'D';
			input[8] = 'T';
			input[9] = 'M';
			input[10] = 'F';
			input[11] = '\r';
			input[12] = '\0';

			//memcpy(input,"No new DTMF\r",1);
			present.srcSize = 12;
		} else {
			i = 0;
			while (DTMF_BUFF_SP != DTMF_BUFF_EP){
				input[i] = DTMF_BUFF[DTMF_BUFF_SP];
				i++;
				DTMF_BUFF_SP = (DTMF_BUFF_SP + 1)% DTMF_SIZE;
			}
			input[i] = '\r';
			present.srcSize = i+1;
		}

		// call ax25 to encode it
		present.src = input;
		present.route.dest.callSign[0] = 'B';
		present.route.dest.callSign[1] = 'L';
		present.route.dest.callSign[2] = 'U';
		present.route.dest.callSign[3] = 'S';
		present.route.dest.callSign[4] = 'A';
		present.route.dest.callSign[5] = 'T';
		present.route.dest.callSign[6] = '\0';

		//memcpy (present.route.src.callSign, "BLUEGS",CALLSIGN_SIZE);
		present.route.src.callSign[0] = 'B';
		present.route.src.callSign[1] = 'L';
		present.route.src.callSign[2] = 'U';
		present.route.src.callSign[3] = 'E';
		present.route.src.callSign[4] = 'G';
		present.route.src.callSign[5] = 'S';
		present.route.src.callSign[6] = '\0';

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

		//vSetToken(Comms_TaskToken);

		ax25Entry (&present, actual, &actual_size );
		int i = 0;
		while(1){
			i++;
			led(i/10000);
		}
		//vDebugPrint(Comms_TaskToken, "%d, %23x\n\r",actual_size ,actual, NO_INSERT);
		// TX and modem will be chosen by GS

		switching_OPMODE(DEVICE_MODE);

		switching_TX_Device(AFSK_1);

		Comms_Modem_Write_Str(actual, actual_size);

		//modem_takeSemaphore();

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
