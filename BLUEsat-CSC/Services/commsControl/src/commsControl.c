 /**
 *  \file commsControl.h
 *
 *  \brief control task for message sending
 *  This task sends message to switching circuit control
 *  to switch to the modem, then send message to modem
 *
 *  \author $Author: Sam Jiang $
 *  \version 1.0
 *
 *  $Date: 2/6/2012
 */

#include "service.h"
#include "commsControl.h"
#include "switching.h"
#include "modem.h"
#include "debug.h"

#define COMMS_Q_SIZE	16

typedef struct
{
	int size;
	char *data;
} Message;

//task token for accessing services
static TaskToken Comms_TaskToken;

//prototype for task function
static portTASK_FUNCTION(vCommsTask, pvParameters);

void vComms_Init(unsigned portBASE_TYPE uxPriority)
{
	/* initialise the beacon task by inserting into scheduler*/
	Comms_TaskToken = ActivateTask(TASK_COMMS,
								"Comms",
								SEV_TASK_TYPE,
								uxPriority,
								SERV_STACK_SIZE,
								vCommsTask);

	vActivateQueue(Comms_TaskToken, COMMS_Q_SIZE);

}

static portTASK_FUNCTION(vCommsTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;
	int counter_should_not_be_final = 0;

	for ( ; ; )
	{
		// grab message from queue
		enResult = enGetRequest(Comms_TaskToken, &incoming_packet, portMAX_DELAY);
		if (enResult != URC_SUCCESS) continue;


		// grab semaphore
		switching_takeSemaphore();

		// grab sysinfo
		// determine what to do based on current status

		// call switching circuit
		switching_OPMODE(DEVICE_MODE);

		if (counter_should_not_be_final % 4 == 0)
		{
			switching_TX_Device(AFSK_1);
			switching_TX(TX_1);

			Comms_Modem_Write_Str(((Message*)incoming_packet.Data)->data, 1, MODEM_1 );
			modem_takeSemaphore(MODEM_1);
		} else if (counter_should_not_be_final % 4 == 1)
		{
			switching_TX_Device(AFSK_1);
			switching_TX(TX_2);
			Comms_Modem_Write_Str(((Message*)incoming_packet.Data)->data, 1, MODEM_1 );
			modem_takeSemaphore(MODEM_1);
		} else if (counter_should_not_be_final % 4 == 2)
		{
			switching_TX_Device(AFSK_2);
			switching_TX(TX_1);
			Comms_Modem_Write_Str(((Message*)incoming_packet.Data)->data, 1, MODEM_1 );
			modem_takeSemaphore(MODEM_2);
		} else {
			switching_TX_Device(AFSK_2);
			switching_TX(TX_2);
			Comms_Modem_Write_Str(((Message*)incoming_packet.Data)->data, 1, MODEM_1 );
			modem_takeSemaphore(MODEM_2);
		}

		// release semaphore
		switching_giveSemaphore();
		vDebugPrint(Comms_TaskToken,"The sentence is %11s\n\r",(long)((Message*)incoming_packet.Data)->data,NO_INSERT,NO_INSERT);

		vCompleteRequest(incoming_packet.Token, URC_SUCCESS);
		counter_should_not_be_final++;
	}
}

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

	return enProcessRequest(&mp,  0);;
}
