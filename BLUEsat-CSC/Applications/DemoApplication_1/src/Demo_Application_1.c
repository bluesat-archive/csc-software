/**
 * Demo_Application_1.c - An application to demonstrating
 * 						  how an application should operate
 *
 * Create by: James Qin
 */

#include "application.h"
#include "Demo_Application_1.h"
#include "command.h"
#include "debug.h"

#define DEMO_Q_SIZE	1
#define MESSAGE_WAIT_TIME 500

typedef struct
{
	MESSAGE_HEADER;
	signed portCHAR *pMsg;
	unsigned portSHORT usLength;
} Demo_Message;

#define DEMOAPP_MSG_SIZE sizeof(Demo_Message) - sizeof(MESSAGE_HEADER)

static TaskToken DEMO_TaskToken;

static portTASK_FUNCTION(vDemoTask, pvParameters);

void vDemoApp1_Init(unsigned portBASE_TYPE uxPriority)
{
	DEMO_TaskToken = ActivateTask(TASK_DEMO_APP_1, 
								(const signed char *)"DemoApp1", 
								TYPE_APPLICATION, 
								uxPriority, 
								APP_STACK_SIZE, 
								vDemoTask);

	vActivateQueue(DEMO_TaskToken, DEMO_Q_SIZE, sizeof(Demo_Message));
}

static portTASK_FUNCTION(vDemoTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	Demo_Message incoming_message;
	Cmd_Message	outgoing_message;
	Demo_Message *pMessageHandle = (Demo_Message *)&outgoing_message;

	for ( ; ; )
	{
		enResult = enGet_Message(DEMO_TaskToken, &incoming_message, MESSAGE_WAIT_TIME);

		if (enResult == URC_SUCCESS)
		{
			enDebug_Print(DEMO_TaskToken, incoming_message.pMsg, incoming_message.usLength);
		}

		pMessageHandle->Src			= TASK_DEMO_APP_1;
		pMessageHandle->Dest		= TASK_DEMO_APP_2;
		pMessageHandle->Token		= DEMO_TaskToken;
		pMessageHandle->Length		= DEMOAPP_MSG_SIZE;
		pMessageHandle->pMsg		= (signed portCHAR *)"Demo Application 1 said 'hi'\n\r";
		pMessageHandle->usLength	= 50;
		enCommand_Push(&outgoing_message, portMAX_DELAY);
	}
}


