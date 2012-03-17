/**
 * Demo_Application_2.c - An application to demonstrating
 * 						  how an application should operate
 *
 * Create by: James Qin
 */

#include "application.h"
#include "Demo_Application_2.h"
#include "command.h"
#include "debug.h"

#define DEMO_Q_SIZE	1

typedef struct
{
	MESSAGE_HEADER;
	signed portCHAR *pMsg;
	unsigned portSHORT usLength;
} Demo_Message;

#define DEMOAPP_MSG_SIZE sizeof(Demo_Message) - sizeof(MESSAGE_HEADER)

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

	vActivateQueue(DEMO_TaskToken, DEMO_Q_SIZE, sizeof(Demo_Message));
}

static portTASK_FUNCTION(vDemoTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	Demo_Message incoming_message;

	for ( ; ; )
	{
		enResult = enGet_Message(DEMO_TaskToken, &incoming_message, portMAX_DELAY);

		if (enResult == URC_SUCCESS)
		{
			enResult = enDebug_Print(DEMO_TaskToken, incoming_message.pMsg, incoming_message.usLength);
			vCompleteRequest(incoming_message.Token, enResult);
		}
	}
}


