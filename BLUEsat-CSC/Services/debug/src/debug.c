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
	signed portCHAR *pcTaskName;
	signed portCHAR *pcDebugString;
	unsigned portSHORT usLength;
} Debug_Message;

#define DEBUG_MSG_SIZE sizeof(Debug_Message) - sizeof(MESSAGE_HEADER)

static TaskToken Debug_TaskToken;

static portTASK_FUNCTION(vDebugTask, pvParameters);

void vPrintString(signed portCHAR *pcDebugString, unsigned portSHORT usLength);

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

		if (xResult == pdTRUE)
		{
			vPrintString(incoming_message.pcTaskName, TASK_NAME_MAX_CHAR);
			vPrintString((signed portCHAR *)">", 1);
			vPrintString(incoming_message.pcDebugString, incoming_message.usLength);
			(incoming_message.CallBackFunc)(pdPASS);
		}
	}
}

signed portBASE_TYPE vDebug_Print(TaskToken taskToken,
								signed portCHAR *pcDebugString,
								unsigned portSHORT usLength,
								CALLBACK CallBackFunc)
{
	Cmd_Message	outgoing_message;
	Debug_Message *pMessageHandle = (Debug_Message *)&outgoing_message;

	switch (taskToken->enTaskType)
	{
		case TT_SERVICE		:	vPrintString((signed portCHAR *)taskToken->pcTaskName, TASK_NAME_MAX_CHAR);
								vPrintString((signed portCHAR *)">", 1);
								vPrintString(pcDebugString, usLength);
								return DEBUG_PRINT_COMPLETE;

		case TT_APPLICATION	:	if (CallBackFunc != NULL)
								{
									pMessageHandle->Src = taskToken->enTaskID;
									pMessageHandle->Dest = TASK_DEBUG;
									pMessageHandle->CallBackFunc = CallBackFunc;
									pMessageHandle->Length = DEBUG_MSG_SIZE;
									pMessageHandle->pcTaskName = (signed portCHAR *)taskToken->pcTaskName;
									pMessageHandle->pcDebugString = pcDebugString;
									pMessageHandle->usLength = usLength;
									xCommand_Push(&outgoing_message, portMAX_DELAY);
								}
								return DEBUG_PRINT_DEFERRED;

		default				:	return DEBUG_PRINT_COMPLETE;
	}
}

void vPrintString(signed portCHAR *pcDebugString, unsigned portSHORT usLength)
{
	unsigned portSHORT usIndex;

	vAcquireUARTChannel(WRITE0, portMAX_DELAY);
	{
		for (usIndex = 0; usIndex < usLength && pcDebugString[usIndex] != '\0'; usIndex++)
		{
			Comms_UART_Write_Char(pcDebugString[usIndex], portMAX_DELAY);
		}
	}
	vReleaseUARTChannel(WRITE0);
}
