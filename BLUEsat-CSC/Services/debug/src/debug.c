 /**
 *  \file debug.c
 *
 *  \brief Using UART to output debug message to terminal
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */
 
#include "service.h"
#include "debug.h"
#include "uart.h"

#define DEBUG_Q_SIZE	5

//debug task message format
typedef struct
{
	MESSAGE_HEADER;
	signed portCHAR *pcDebugString;
	unsigned portSHORT usLength;
} Debug_Message;

#define DEBUG_MSG_SIZE sizeof(Debug_Message) - sizeof(MESSAGE_HEADER)

//task token for accessing services
static TaskToken Debug_TaskToken;

//prototype for task function
static portTASK_FUNCTION(vDebugTask, pvParameters);

/**
 * \brief Writes a single character onto the port
 *
 * \param[in] pcDebugString Pointer to debug string.
 * \param[in] usLength Length of debug string.
 */
void vPrintString(signed portCHAR *pcDebugString, unsigned portSHORT usLength);

void vDebug_Init(unsigned portBASE_TYPE uxPriority)
{
	Debug_TaskToken = ActivateTask(TASK_DEBUG, 
								(const signed portCHAR *)"Debug", 
								TYPE_SERVICE, 
								uxPriority, 
								SERV_STACK_SIZE, 
								vDebugTask);
								
	vActivateQueue(Debug_TaskToken, DEBUG_Q_SIZE, sizeof(Debug_Message));
}

static portTASK_FUNCTION(vDebugTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	Debug_Message incoming_message;

	for ( ; ; )
	{
		enResult = enGet_Message(Debug_TaskToken, &incoming_message, portMAX_DELAY);

		if (enResult == URC_SUCCESS)
		{
			vPrintString((signed portCHAR *)(incoming_message.Token)->pcTaskName, TASK_NAME_MAX_CHAR);
			vPrintString((signed portCHAR *)">", 1);
			vPrintString(incoming_message.pcDebugString, incoming_message.usLength);
			vCompleteRequest(incoming_message.Token, URC_SUCCESS);
		}
	}
}

UnivRetCode enDebug_Print(TaskToken taskToken,
						signed portCHAR *pcDebugString,
						unsigned portSHORT usLength)
{
	Cmd_Message	outgoing_message;
	Debug_Message *pMessageHandle = (Debug_Message *)&outgoing_message;

	switch (taskToken->enTaskType)
	{
		case TYPE_SERVICE		:	vPrintString((signed portCHAR *)taskToken->pcTaskName, TASK_NAME_MAX_CHAR);
									vPrintString((signed portCHAR *)">", 1);
									vPrintString(pcDebugString, usLength);
									return URC_SUCCESS;

		case TYPE_APPLICATION	:	pMessageHandle->Src				= taskToken->enTaskID;
									pMessageHandle->Dest			= TASK_DEBUG;
									pMessageHandle->Token			= taskToken;
									pMessageHandle->Length			= DEBUG_MSG_SIZE;
									pMessageHandle->pcDebugString	= pcDebugString;
									pMessageHandle->usLength		= usLength;
									return enCommand_Push(&outgoing_message, portMAX_DELAY);

		default					:	return URC_FAIL;
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
