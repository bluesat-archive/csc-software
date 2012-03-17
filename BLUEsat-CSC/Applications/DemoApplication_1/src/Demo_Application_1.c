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

static xSemaphoreHandle MSG_MUTEX;
static signed portBASE_TYPE xMSGReturnStatus;

static portTASK_FUNCTION(vDemoTask, pvParameters);

static CALLBACK_FUNCTION(vMessageCallBack, xReturnStatus);

void vDemoApp1_Init(unsigned portBASE_TYPE uxPriority)
{
	DEMO_TaskToken = ActivateTask(TASK_DEMO_APP_1, (const signed char *)"DemoApp1", TYPE_APPLICATION, uxPriority, APP_STACK_SIZE, vDemoTask);

	vActivateQueue(DEMO_TaskToken, DEMO_Q_SIZE, sizeof(Demo_Message));

	vSemaphoreCreateBinary(MSG_MUTEX);

	xSemaphoreTake(MSG_MUTEX, 0);
}

static portTASK_FUNCTION(vDemoTask, pvParameters)
{
	(void) pvParameters;
	signed portBASE_TYPE xResult;
	Demo_Message incoming_message;
	Cmd_Message	outgoing_message;
	Demo_Message *pMessageHandle = (Demo_Message *)&outgoing_message;

	for ( ; ; )
	{
		xResult = xGet_Message(DEMO_TaskToken, &incoming_message, MESSAGE_WAIT_TIME);

		if (xResult == pdTRUE)
		{
			if (!vDebug_Print(DEMO_TaskToken, incoming_message.pMsg, incoming_message.usLength, vMessageCallBack)) xSemaphoreTake(MSG_MUTEX, portMAX_DELAY);
			(incoming_message.CallBackFunc)(pdPASS);
		}

		pMessageHandle->Src = TASK_DEMO_APP_1;
		pMessageHandle->Dest = TASK_DEMO_APP_2;
		pMessageHandle->CallBackFunc = vMessageCallBack;
		pMessageHandle->Length = DEMOAPP_MSG_SIZE;
		pMessageHandle->pMsg = (signed portCHAR *)"Demo Application 1 said 'hi'\n\r";
		pMessageHandle->usLength = 50;
		if (xCommand_Push(&outgoing_message, portMAX_DELAY)) xSemaphoreTake(MSG_MUTEX, portMAX_DELAY);
	}
}

static CALLBACK_FUNCTION(vMessageCallBack, xReturnStatus)
{
	xSemaphoreGive(MSG_MUTEX);
	xMSGReturnStatus = xReturnStatus;
}


