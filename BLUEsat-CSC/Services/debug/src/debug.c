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
#include "lib_string.h"

#define MSN(x)							   (x >> 0x4)	//Most Significant Nibble
#define LSN(x)							   (x &  0xf)	//Least Significant Nibble

#define DEBUG_Q_SIZE	5

//debug task message format
typedef struct
{
	portCHAR *pcDebugString;
	unsigned portSHORT usLength;
} DebugContent;

#define DEBUG_CONTENT_SIZE	sizeof(DebugContent)

//task token for accessing services
static TaskToken Debug_TaskToken;

//prototype for task function
static portTASK_FUNCTION(vDebugTask, pvParameters);

/**
 * \brief Writes string to UART 1 char at a time
 *
 * \param[in] pcDebugString Pointer to debug string.
 * \param[in] usLength Length of debug string.
 */
void vPrintString(portCHAR *pcDebugString, unsigned portSHORT usLength);

void vDebug_Init(unsigned portBASE_TYPE uxPriority)
{
	Debug_TaskToken = ActivateTask(TASK_DEBUG, 
								"Debug",
								TYPE_SERVICE, 
								uxPriority, 
								SERV_STACK_SIZE, 
								vDebugTask);
								
	vActivateQueue(Debug_TaskToken, DEBUG_Q_SIZE);
}

static portTASK_FUNCTION(vDebugTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;
	DebugContent *pContentHandle;

	for ( ; ; )
	{
		enResult = enGetRequest(Debug_TaskToken, &incoming_packet, portMAX_DELAY);

		if (enResult == URC_SUCCESS)
		{
			//print string to UART
			vPrintString((portCHAR *)(incoming_packet.Token)->pcTaskName, TASK_NAME_MAX_CHAR);
			vPrintString(">", 1);
			pContentHandle = (DebugContent *)incoming_packet.Data;
			vPrintString(pContentHandle->pcDebugString, pContentHandle->usLength);
			//complete request by passing the status to the sender
			vCompleteRequest(incoming_packet.Token, URC_SUCCESS);
		}
	}
}

UnivRetCode enDebug_Print(TaskToken taskToken,
						portCHAR *pcDebugString,
						unsigned portSHORT usLength)
{
	MessagePacket outgoing_packet;
	DebugContent debugContent;

	//identify requester
	switch (taskToken->enTaskType)
	{
		//services' debug message always get printed first
		case TYPE_SERVICE		:	//print message to UART
									vPrintString((portCHAR *)taskToken->pcTaskName, TASK_NAME_MAX_CHAR);
									vPrintString(">", 1);
									vPrintString(pcDebugString, usLength);
									return URC_SUCCESS;

		//applications' debug message always gets queued
		case TYPE_APPLICATION	:	//create request packet
									outgoing_packet.Src				= taskToken->enTaskID;
									outgoing_packet.Dest			= TASK_DEBUG;
									outgoing_packet.Token			= taskToken;
									outgoing_packet.Length			= DEBUG_CONTENT_SIZE;
									outgoing_packet.Data			= (unsigned portLONG)&debugContent;
									//create tag along data
									debugContent.pcDebugString		= pcDebugString;
									debugContent.usLength			= usLength;
									return enProcessRequest(&outgoing_packet, portMAX_DELAY);

		default					:	return URC_FAIL;
	}
}
/*
void vPrintHex(portCHAR *pcDebugString, unsigned portSHORT usLength)
{
	vAcquireUARTChannel(WRITE0, portMAX_DELAY);
	{
		for (; usLength-- > 0;)
		{
			Comms_UART_Write_Char(cValToHex(MSN(pcDebugString[usLength])), portMAX_DELAY);
			Comms_UART_Write_Char(cValToHex(LSN(pcDebugString[usLength])), portMAX_DELAY);
		}
	}
	vReleaseUARTChannel(WRITE0);
}
*/
void vPrintString(portCHAR *pcDebugString, unsigned portSHORT usLength)
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
