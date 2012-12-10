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

#define DEBUG_Q_SIZE		5
#define MAX_INSERTIONS		3
#define MAX_ERROR_MSG_LEN	100

//debug task message format
typedef struct
{
	portCHAR *			pcFormat;
	unsigned portLONG *	pulInsertions;
} DebugContent;

#define DEBUG_CONTENT_SIZE	sizeof(DebugContent)

//all static declaration goes in here
#ifndef NO_DEBUG
	static struct taskToken preScheduler_TaskToken = {	.pcTaskName = "BootSys",
														.enTaskType = TYPE_SERVICE,
														.enTaskID 	= NO_TASK,
														.enRetVal	= 0};
	//task token for accessing services
	static TaskToken PreScheduler_TaskToken = &preScheduler_TaskToken;
	//task token for accessing services
	static TaskToken Debug_TaskToken = NULL;

	//prototype for task function
	static portTASK_FUNCTION(vDebugTask, pvParameters);
#endif /* NO_DEBUG */

/**
 * \brief Compose and print message string
 *
 * \param[in] taskToken Task token from request task
 * \param[in] pcFormat Print format.
 * \param[in] pcInsertions Pointer to insertion data.
 *
 * \returns enum Universal return code
 */
UnivRetCode enJPrint(TaskToken 			taskToken,
					portCHAR const *	pcFormat,
					unsigned portLONG *	pulInsertions);

/**
 * \brief Print insertion data
 *
 * \param[in] pcFormat Format string
 * \param[in] pulFormatIndex Insertion format location.
 * \param[in] ulInsertion Insertion data.
 *
 * \returns enum Universal return code
 */
UnivRetCode enPrintInsertion(portCHAR const *pcFormat,
							unsigned portLONG *pulFormatIndex,
							unsigned portLONG ulInsertion);

/**
 * \brief Print data in decimal
 *
 * \param[in] ulValue Value to be printed
 */
void vPrintDecimal(unsigned portLONG ulValue);

/**
 * \brief Print data in hex decimal
 *
 * \param[in] pcPtr Pointer to location to be printed
 * \param[in] usLength Number of bytes to be printed.
 */
void vPrintHex(portCHAR const *pcPtr, unsigned portSHORT usLength);

/**
 * \brief Print data as a string
 *
 * \param[in] pcPtr Pointer to location to be printed
 * \param[in] usLength Number of bytes to be printed.
 */
void vPrintString(portCHAR const *pcPtr, unsigned portSHORT usLength);

#ifndef NO_DEBUG

	void vDebug_Init(unsigned portBASE_TYPE uxPriority)
	{
		Debug_TaskToken = ActivateTask(TASK_DEBUG,
									"Debug",
									SEV_TASK_TYPE,
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

			if (enResult != URC_SUCCESS) continue;

			pContentHandle = (DebugContent *)incoming_packet.Data;

			//complete request by passing the status to the sender
			vCompleteRequest(incoming_packet.Token, enJPrint((incoming_packet.Token),
															pContentHandle->pcFormat,
															pContentHandle->pulInsertions));

		}
	}

	unsigned portSHORT	usDebugRead(portCHAR *			pcBuffer,
									unsigned portSHORT 	usMaxSize)
	{
		unsigned portSHORT usIndex;

		vAcquireUARTChannel(READ0, portMAX_DELAY);
		{
			for (usIndex = 0; usIndex < usMaxSize; ++usIndex)
			{
				Comms_UART_Read_Char(&pcBuffer[usIndex], portMAX_DELAY);

				if (pcBuffer[usIndex] == '\r' || pcBuffer[usIndex] == '\n') break;
			}
			pcBuffer[usIndex] = '\0';
		}
		vReleaseUARTChannel(READ0);

		return usIndex;
	}

	void vDebugPrint(TaskToken 			taskToken,
					portCHAR *			pcFormat,
					unsigned portLONG 	pcInsertion_1,
					unsigned portLONG 	pcInsertion_2,
					unsigned portLONG 	pcInsertion_3)
	{
		MessagePacket outgoing_packet;
		DebugContent debugContent;
		unsigned portLONG pulInsertions[MAX_INSERTIONS] =	{pcInsertion_1,
															pcInsertion_2,
															pcInsertion_3};
		//use pre-scheduler token for pre-scheduler print
		if (Debug_TaskToken == NULL)
		{
			taskToken = PreScheduler_TaskToken;
		}
		else if (taskToken == NULL)
		{
			return;
		}

		//identify requester
		if (taskToken->enTaskType == TYPE_SERVICE)
		{
			//services' debug message always get printed first
			enJPrint(taskToken, pcFormat, pulInsertions);
		}
		else if (taskToken->enTaskType == TYPE_APPLICATION)
		{
			//applications' debug message always gets queued
			//create request packet
			outgoing_packet.Src				= taskToken->enTaskID;
			outgoing_packet.Dest			= TASK_DEBUG;
			outgoing_packet.Token			= taskToken;
			outgoing_packet.Data			= (unsigned portLONG)&debugContent;
			//create tag along data
			debugContent.pcFormat			= pcFormat;
			debugContent.pulInsertions		= pulInsertions;
			enProcessRequest(&outgoing_packet, portMAX_DELAY);
		}
	}

#endif /* NO_DEBUG */

UnivRetCode enJPrint(TaskToken 			taskToken,
					portCHAR const *	pcFormat,
					unsigned portLONG *	pulInsertions)
{
	unsigned portCHAR 	ucInsertIndex;
	unsigned portLONG 	ulFormatIndex;
	UnivRetCode			enResult = URC_SUCCESS;

	vAcquireUARTChannel(WRITE0, portMAX_DELAY);
	{
		//print request task name
		vPrintString(taskToken->pcTaskName, TASK_NAME_MAX_CHAR);
		Comms_UART_Write_Char('>', portMAX_DELAY);

		//read through format
		for (ulFormatIndex = 0, ucInsertIndex = 0; pcFormat[ulFormatIndex] != '\0'; ++ulFormatIndex)
		{
			//insertion detected
			if (pcFormat[ulFormatIndex] == '%')
			{
				//set index to start of insertion format
				++ulFormatIndex;

				//%% == %
				if (pcFormat[ulFormatIndex] == '%')
				{
					Comms_UART_Write_Char('%', portMAX_DELAY);
					continue;
				}

				//check MAX_INSERTIONS reached
				if (ucInsertIndex == MAX_INSERTIONS)
				{
					vPrintString(" - Max insertions reached!\n\r", MAX_ERROR_MSG_LEN);
					enResult = URC_DEB_MAXED_INSERTION;
					break;
				}

				 //defer to insertion printing subroutine
				enResult = enPrintInsertion(pcFormat, &ulFormatIndex, pulInsertions[ucInsertIndex]);

				//insertion failed stopping printing and release failure value
				if (enResult != URC_SUCCESS) break;

				//move onto next insertion value
				++ucInsertIndex;
			}
			else
			{
				//write character to UART
				Comms_UART_Write_Char(pcFormat[ulFormatIndex], portMAX_DELAY);
			}
		}
	}
	vReleaseUARTChannel(WRITE0);

	return enResult;
}

UnivRetCode enPrintInsertion(portCHAR const *	pcFormat,
							unsigned portLONG *	pulFormatIndex,
							unsigned portLONG 	ulInsertion)
{
	const portCHAR MAX_LENGTH_SIZE = 4;
	unsigned portCHAR	ucLengthIndex;
	unsigned portSHORT	usLength;

	//read length value
	for (usLength = 0, ucLengthIndex = 0;
		pcFormat[*pulFormatIndex] >= '0' && pcFormat[*pulFormatIndex] <= '9';
		++(*pulFormatIndex), ++ucLengthIndex)
	{
		if (ucLengthIndex < MAX_LENGTH_SIZE)
		{
			usLength *= 10;	//shift left by 1 in base decimal
			usLength += pcFormat[*pulFormatIndex] - '0';
		}
		else
		{
			vPrintString(" - Insertion length value too long!\n\r", MAX_ERROR_MSG_LEN);

			return URC_DEB_INSERT_LEN_VAL_LONG;
		}
	}

	//check pointer value is not NULL
	if (ulInsertion == (unsigned portLONG)NULL
			&& (pcFormat[*pulFormatIndex] != 'p' && pcFormat[*pulFormatIndex] != 'd'))
	{
		vPrintString(" - Missing insertion!\n\r", MAX_ERROR_MSG_LEN);
		return URC_DEB_MISSING_INSERTION;
	}

	//identify output type
	switch (pcFormat[*pulFormatIndex])
	{
		case 's':	vPrintString((portCHAR const *)ulInsertion, usLength);
					break;
		case 'x':	if (usLength > 0) vPrintHex((portCHAR const *)ulInsertion, usLength);
					break;
		case 'p':	vPrintHex((portCHAR const *)&ulInsertion, sizeof(unsigned portLONG));
					break;
		case 'd':	vPrintDecimal(ulInsertion);
					break;
		default:
					vPrintString(" - Bad format!\n\r", MAX_ERROR_MSG_LEN);
					return URC_DEB_BAD_FORMAT;
	}

	return URC_SUCCESS;
}

#define LONG_TO_DECIMAL_BYTE_SIZE	10
void vPrintDecimal(unsigned portLONG ulValue)
{
	portCHAR pcBuffer[LONG_TO_DECIMAL_BYTE_SIZE + 1];	//include '\0'
	unsigned portSHORT usIndex = LONG_TO_DECIMAL_BYTE_SIZE;

	memset(pcBuffer, '\0', LONG_TO_DECIMAL_BYTE_SIZE + 1);

	do
	{
		pcBuffer[--usIndex] = ulValue % 10 + '0';
		ulValue /= 10;
	}
	while (ulValue > 0 && usIndex > 0);

	vPrintString(&pcBuffer[usIndex], LONG_TO_DECIMAL_BYTE_SIZE);
}

void vPrintHex(portCHAR const *pcPtr, unsigned portSHORT usLength)
{
	vPrintString("0x", sizeof("0x"));

	for (; usLength-- > 0;)
	{
		Comms_UART_Write_Char(cValToHex(MSN(pcPtr[usLength])), portMAX_DELAY);
		Comms_UART_Write_Char(cValToHex(LSN(pcPtr[usLength])), portMAX_DELAY);
	}
}

void vPrintString(portCHAR const *pcPtr, unsigned portSHORT usLength)
{
	unsigned portSHORT usIndex;

	for (usIndex = 0; usIndex < usLength && pcPtr[usIndex] != '\0'; usIndex++)
	{
		Comms_UART_Write_Char(pcPtr[usIndex], portMAX_DELAY);
	}
}

