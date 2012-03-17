 /**
 *  \file Demo_Application_2.c
 *
 *  \brief An application demonstrating how an application operate
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#include "application.h"
#include "Demo_Application_2.h"
#include "Demo_Application_1.h"
#include "debug.h"

#define DEMO_Q_SIZE	1
#define MESSAGE_WAIT_TIME 500

typedef struct
{
	signed portCHAR *pMsg;			//pointer to message
	unsigned portSHORT usLength;	//message length	
} DemoContent;

#define DEMO_CONTENT_SIZE	sizeof(DemoContent)

//task token for accessing services and other applications
static TaskToken DEMO_TaskToken;

static portTASK_FUNCTION(vDemoTask, pvParameters);

void vDemoApp2_Init(unsigned portBASE_TYPE uxPriority)
{
	DEMO_TaskToken = ActivateTask(TASK_DEMO_APP_2, 
								(const signed char *)"DemoApp2", 
								TYPE_APPLICATION, 
								uxPriority, 
								APP_STACK_SIZE, 
								vDemoTask);

	vActivateQueue(DEMO_TaskToken, DEMO_Q_SIZE);
}

static portTASK_FUNCTION(vDemoTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;
	DemoContent *pContentHandle;

	for ( ; ; )
	{
		enResult = enGetRequest(DEMO_TaskToken, &incoming_packet, MESSAGE_WAIT_TIME);

		if (enResult == URC_SUCCESS)
		{
			//access tagged data
			pContentHandle = (DemoContent *)incoming_packet.Data;
			//print message
			enDebug_Print(DEMO_TaskToken, pContentHandle->pMsg, pContentHandle->usLength);
			//complete request by passing the status to the sender
			vCompleteRequest(incoming_packet.Token, URC_SUCCESS);
		}
		
		enMessage_To_Q(DEMO_TaskToken, (signed portCHAR *)"Hello!\n\r", 50);
	}
}


