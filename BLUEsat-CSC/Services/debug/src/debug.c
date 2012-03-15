/**
 * debug.c - Using UART to output debug message to terminal
 *
 * Create by: James Qin
 */

#include "service.h"
#include "debug.h"
#include "uart.h"

#define DEBUG_Q_SIZE	5

typedef struct
{
	MESSAGE_HEADER;
	signed portCHAR *pDebugString;
	unsigned portSHORT usLength;
} Debug_Message;

#define DEBUG_MSG_SIZE sizeof(Debug_Message) - sizeof(MESSAGE_HEADER)

static TaskToken Debug_TaskToken;
static portTASK_FUNCTION(vDebugTask, pvParameters);
void vPrintString(signed portCHAR *pDebugString, unsigned portSHORT usLength);

void vDebug_Init(unsigned portBASE_TYPE uxPriority)
{
	Debug_TaskToken = ActivateTask(TASK_DEBUG, (const signed portCHAR *)"Debug", TT_SERVICE, uxPriority, SERV_STACK_SIZE, vDebugTask);
	vActivateQueue(Debug_TaskToken, DEBUG_Q_SIZE, sizeof(Debug_Message));
}

static portTASK_FUNCTION(vDebugTask, pvParameters)
{
	(void) pvParameters;
	signed portBASE_TYPE xResult;
	Debug_Message incoming_message;

	for ( ; ; )
	{
		xResult = xGet_Message(Debug_TaskToken, &incoming_message, portMAX_DELAY);

		if (xResult == pdPASS)
		{
			vPrintString(incoming_message.pDebugString, incoming_message.usLength);
			(incoming_message.CallBackFunc)(pdPASS);
		}
	}
}

void vDebug_Print(TaskToken taskToken,
				signed portCHAR *pDebugString,
				unsigned portSHORT usLength,
				CALLBACK CallBackFunc)
{
	Cmd_Message	outgoing_message;
	Debug_Message *pMessageHandle = (Debug_Message *)&outgoing_message;

	switch (taskToken->enTaskType)
	{
		case TT_SERVICE		:	vPrintString(pDebugString, usLength);
								break;

		case TT_APPLICATION	:	if (CallBackFunc != NULL)
								{
									pMessageHandle->Src = taskToken->enTaskID;
									pMessageHandle->Dest = TASK_DEBUG;
									pMessageHandle->CallBackFunc = CallBackFunc;
									pMessageHandle->Length = DEBUG_MSG_SIZE;
									pMessageHandle->pDebugString = pDebugString;
									pMessageHandle->usLength = usLength;
									xCommand_Push(&outgoing_message, portMAX_DELAY);
								}
								break;

		default				:	// nothing to do
								break;
	}
}

void vPrintString(signed portCHAR *pDebugString, unsigned portSHORT usLength)
{
	unsigned portSHORT usIndex;

	vAcquireUARTChannel(WRITE0, portMAX_DELAY);
	{
		for (usIndex = 0; usIndex < usLength && pDebugString[usIndex] != '\0'; usIndex++)
		{
			Comms_UART_Write_Char(pDebugString[usIndex], portMAX_DELAY);
		}
	}
	vReleaseUARTChannel(WRITE0);
}
