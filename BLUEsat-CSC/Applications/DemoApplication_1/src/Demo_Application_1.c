 /**
 *  \file Demo_Application_1.c
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
#include "Demo_Application_1.h"
#include "debug.h"
#include "memory.h"

#define DEMO_Q_SIZE	1

typedef struct
{
	portCHAR *pMsg;			//pointer to message
	unsigned portSHORT usLength;	//message length
} DemoContent;

#define DEMO_CONTENT_SIZE	sizeof(DemoContent)

//task token for accessing services and other applications
static TaskToken DEMO_TaskToken;

static portTASK_FUNCTION(vDemoTask, pvParameters);

void vDemoApp1_Init(unsigned portBASE_TYPE uxPriority)
{
	DEMO_TaskToken = ActivateTask(TASK_DEMO_APP_1, 
								"DemoApp1",
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
		enResult = enGetRequest(DEMO_TaskToken, &incoming_packet, portMAX_DELAY);

		if (enResult == URC_SUCCESS)
		{
			//access tagged data
			pContentHandle = (DemoContent *)incoming_packet.Data;
			//print message
			enDebug_Print(DEMO_TaskToken, pContentHandle->pMsg, pContentHandle->usLength);
			//complete request by passing the status to the sender
			vCompleteRequest(incoming_packet.Token, URC_SUCCESS);
		}
	}
}

UnivRetCode enMessage_To_Q(TaskToken taskToken,
							portCHAR *pcDebugString,
							unsigned portSHORT usLength)
{
	MessagePacket outgoing_packet;
	DemoContent domeContent;
	
	//create packet with printing request information
	outgoing_packet.Src			= enGetTaskID(taskToken);
	outgoing_packet.Dest		= TASK_DEMO_APP_1;
	outgoing_packet.Token		= taskToken;
	outgoing_packet.Length		= DEMO_CONTENT_SIZE;
	outgoing_packet.Data		= (unsigned portLONG)&domeContent;	
	//store message in a struct and tag along with the request packet
	domeContent.pMsg			= pcDebugString;
	domeContent.usLength		= usLength;
	
	return enProcessRequest(&outgoing_packet, portMAX_DELAY);
}


