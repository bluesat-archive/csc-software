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

int transmitTele = 1;
int transmitBeacon = 1;


extern char DTMF_BUFF[DTMF_SIZE];
extern int DTMF_BUFF_SP; //Start position
extern int DTMF_BUFF_EP; //End position

void vComms_Init(unsigned portBASE_TYPE uxPriority)
{

	Comms_TaskToken = ActivateTask(TASK_COMMS,
								   "Comms",
								   SEV_TASK_TYPE,
								   uxPriority,
								   1024,
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
    char input [128];
    char actual [128];

    unsigned int actual_size = 128;
    int i, m;
    switching_RX(0);
	switching_OPMODE(DEVICE_MODE);

	switching_TX_Device(AFSK_1);
	setModemTransmit(1);
	switching_TX(0);
	for ( ; ; )
	{
		m = 0;
		// grab message from telem log
		//telemetry_storage_read_index(0,&temp);
		if (transmitTele){
			m = telemetry_storage_read_cur(&temp);
			vDebugPrint(Comms_TaskToken,"m = %d and time = %d\r\n",m,temp.timestamp,NO_INSERT);
			input[0] = ((temp.timestamp & (63 << 6))>>6)/10 + '0';
			input[1] = ((temp.timestamp & (63 << 6))>>6)%10 + '0';
			input[2] = ':';
			input[3] = (temp.timestamp & 63)/10 + '0';
			input[4] = (temp.timestamp & 63)%10 + '0';
			input[5] = '\r';
			input[6] = 'T';
			input[7] = 'X';
			input[8] = ':';
			input[9] = '\r';
			for (i = 0; i < 6; i++ ){
				if (i > 9){
					input[10+i*7] = i - 10 +'A';
				} else {
					input[10+i*7] = i+'0';
				}
				input[10+i*7+1] = ':';
				input[10+i*7+2] = temp.values[i]/20+'0';
				input[10+i*7+3] = (temp.values[i]/2)%10+'0';
				input[10+i*7+4] = '.';
				if (temp.values[i]%2 == 0){
					input[10+i*7+5] = '0';
				} else {
					input[10+i*7+5] = '5';
				}
				input[10+i*7+6] = '\r';
			}

			present.srcSize = 52;
			present.src = input;
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

			vSetToken(Comms_TaskToken);
			actual_size = 128;
			memset (actual, 0, 128);
			ax25Entry (&present, actual, &actual_size );

			//vDebugPrint(Comms_TaskToken, "%d, %33x\n\r",actual_size ,actual, NO_INSERT);
			// TX and modem will be chosen by GS

			Comms_Modem_Write_Str(actual, actual_size);
			modem_takeSemaphore();
			Comms_Modem_Write_Str(actual, actual_size);
			modem_takeSemaphore();

			input[6] = 'B';
			input[7] = 'a';
			input[8] = 't';
			input[9] = 't';
			input[10] = 'e';
			input[11] = 'r';
			input[12] = 'y';
			input[13] = ':';
			input[14] = '\r';

			for (i = 0; i < 9; i++ ){
				if (i > 9){
					input[15+i*7] = i - 10 +'A';
				} else {
					input[15+i*7] = i+'0';
				}
				input[15+i*7+1] = ':';
				input[15+i*7+2] = temp.values[i+6]/20+'0';
				input[15+i*7+3] = (temp.values[i+6]/2)%10+'0';
				input[15+i*7+4] = '.';
				if (temp.values[i]%2 == 0){
					input[15+i*7+5] = '0';
				} else {
					input[15+i*7+5] = '5';
				}
				input[15+i*7+6] = '\r';
			}

			present.srcSize = 78;
			present.src = input;
			actual_size = 128;
			memset (actual, 0, 128);
			ax25Entry (&present, actual, &actual_size );

			Comms_Modem_Write_Str(actual, actual_size);
			modem_takeSemaphore();

			input[6] = 'C';
			input[7] = 'S';
			input[8] = 'C';
			input[9] = ':';
			input[10] = '\r';

			for (i = 0; i < 10; i++ ){
				if (i > 9){
					input[11+i*7] = i - 10 +'A';
				} else {
					input[11+i*7] = i+'0';
				}
				input[11+i*7+1] = ':';
				input[11+i*7+2] = temp.values[i+15]/20+'0';
				input[11+i*7+3] = (temp.values[i+15]/2)%10+'0';
				input[11+i*7+4] = '.';
				if (temp.values[i]%2 == 0){
					input[11+i*7+5] = '0';
				} else {
					input[11+i*7+5] = '5';
				}
				input[11+i*7+6] = '\r';
			}

			present.srcSize = 81;
			present.src = input;
			actual_size = 128;
			memset (actual, 0, 128);
			ax25Entry (&present, actual, &actual_size );

			Comms_Modem_Write_Str(actual, actual_size);
			modem_takeSemaphore();

			input[6] = 'R';
			input[7] = 'X';
			input[8] = ':';
			input[9] = '\r';

			for (i = 0; i < 9; i++ ){
				if (i > 9){
					input[10+i*7] = i - 10 +'A';
				} else {
					input[10+i*7] = i+'0';
				}
				input[10+i*7+1] = ':';
				input[10+i*7+2] = temp.values[i+25]/20+'0';
				input[10+i*7+3] = (temp.values[i+25]/2)%10+'0';
				input[10+i*7+4] = '.';
				if (temp.values[i]%2 == 0){
					input[10+i*7+5] = '0';
				} else {
					input[10+i*7+5] = '5';
				}
				input[10+i*7+6] = '\r';
			}

			present.srcSize = 73;
			present.src = input;
			actual_size = 128;
			memset (actual, 0, 128);
			ax25Entry (&present, actual, &actual_size );

			Comms_Modem_Write_Str(actual, actual_size);
			modem_takeSemaphore();
		}

		vDebugPrint(Comms_TaskToken,"SP %d EP %d\r\n",DTMF_BUFF_SP,DTMF_BUFF_EP,NO_INSERT);

		// compose string
		// hh:mm:ss\tID:-cc.c\t....
		//for (i = 0, m = 100000; i < 6; i++, m/=10){
			//input[i] = ((temp.timestamp/m)%10) - '0';
		//}
		if (DTMF_BUFF_SP == DTMF_BUFF_EP){
			memcpy (input,"No new DTMF\r",12);
			present.srcSize = 12;
		} else {
			i = 0;
			while (DTMF_BUFF_SP != DTMF_BUFF_EP){
				if (DTMF_BUFF[DTMF_BUFF_SP] < 10){
					input[i] = DTMF_BUFF[DTMF_BUFF_SP]+'0';
				} else if (DTMF_BUFF[DTMF_BUFF_SP] == 10){
					input[i] = '0';
				} else if (DTMF_BUFF[DTMF_BUFF_SP] == 11){
					input[i] = '*';
				} else {
					input[i] = '#';
				}
				i++;
				DTMF_BUFF_SP = (DTMF_BUFF_SP + 1)% DTMF_SIZE;
			}
			input[i] = '\r';
			present.srcSize = i+1;
		}

		// call ax25 to encode it
		present.src = input;
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

		vSetToken(Comms_TaskToken);
		actual_size = 64;
		memset (actual, 0, 64);
		ax25Entry (&present, actual, &actual_size );

		vDebugPrint(Comms_TaskToken, "%d, %33x\n\r",actual_size ,actual, NO_INSERT);
		// TX and modem will be chosen by GS

		Comms_Modem_Write_Str(actual, actual_size);
		modem_takeSemaphore();
		Comms_Modem_Write_Str(actual, actual_size);
		modem_takeSemaphore();

		vDebugPrint(Comms_TaskToken,"T\r\n",NO_INSERT,NO_INSERT,NO_INSERT);

		//now beacon
		if (transmitBeacon){
			switching_OPMODE(DEVICE_MODE);
			switching_TX_Device(BEACON);

			vDebugPrint(Comms_TaskToken, "Finished setup switching TX device, transmit for %d ms\n\r", TRANSMISSION_TIME ,0, 0);
		}
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
