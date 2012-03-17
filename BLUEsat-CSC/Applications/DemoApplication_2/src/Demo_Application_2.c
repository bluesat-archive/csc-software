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

static xSemaphoreHandle MSG_MUTEX;
static signed portBASE_TYPE xMSGReturnStatus;

static portTASK_FUNCTION(vDemoTask, pvParameters);

static CALLBACK_FUNCTION(vMessageCallBack, xReturnStatus);

void vDemoApp2_Init(unsigned portBASE_TYPE uxPriority)
{
	DEMO_TaskToken = ActivateTask(TASK_DEMO_APP_2, (const signed char *)"DemoApp2", TYPE_APPLICATION, uxPriority, APP_STACK_SIZE, vDemoTask);

	vActivateQueue(DEMO_TaskToken, DEMO_Q_SIZE, sizeof(Demo_Message));

	vSemaphoreCreateBinary(MSG_MUTEX);

	xSemaphoreTake(MSG_MUTEX, NO_BLOCK);
}

static portTASK_FUNCTION(vDemoTask, pvParameters)
{
	(void) pvParameters;
	signed portBASE_TYPE xResult;
	Demo_Message incoming_message;

	for ( ; ; )
	{
		xResult = xGet_Message(DEMO_TaskToken, &incoming_message, portMAX_DELAY);

		if (xResult == pdTRUE)
		{
			if (!vDebug_Print(DEMO_TaskToken, incoming_message.pMsg, incoming_message.usLength, vMessageCallBack)) xSemaphoreTake(MSG_MUTEX, portMAX_DELAY);
			(incoming_message.CallBackFunc)(pdPASS);
		}
	}
}

static CALLBACK_FUNCTION(vMessageCallBack, xReturnStatus)
{
	xSemaphoreGive(MSG_MUTEX);
	xMSGReturnStatus = xReturnStatus;
}


