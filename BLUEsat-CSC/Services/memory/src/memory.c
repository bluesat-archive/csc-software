 /**
 *  \file memory.c
 *
 *  \brief Provide storage to CSC
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
#include "memory.h"
#include "uart.h"
#include "emc.h"

#define MEMORY_Q_SIZE	2

//debug task message format
typedef struct
{
	signed portCHAR *pcString;
	unsigned portSHORT usLength;
} MemoryContent;

#define MEMORY_CONTENT_SIZE	sizeof(MemoryContent)

//task token for accessing services
static TaskToken Memory_TaskToken;

//prototype for task function
static portTASK_FUNCTION(vMemoryTask, pvParameters);

void vMemory_Init(unsigned portBASE_TYPE uxPriority)
{
	Memory_TaskToken = ActivateTask(TASK_MEMORY, 
								(const signed portCHAR *)"Memory", 
								TYPE_SERVICE, 
								uxPriority, 
								SERV_STACK_SIZE, 
								vMemoryTask);
								
	vActivateQueue(Memory_TaskToken, MEMORY_Q_SIZE);
}

static portTASK_FUNCTION(vMemoryTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;
	//MemoryContent *pContentHandle;

	/* start temporary test block */
	unsigned portLONG ulAddress;
	unsigned portCHAR ucValue;
	unsigned portSHORT usValue;
	
	//SRAM test
	
	//FRAM test
	for (ulAddress = STATIC_BANK_1_START_ADDR, ucValue = 0;
		ulAddress < STATIC_BANK_1_START_ADDR + STATIC_BANK_1_SIZE;
		ulAddress += sizeof(portCHAR), ++ucValue)
	{
		*(unsigned portCHAR)ulAddress = ucValue;
	}
	
	for (ulAddress = STATIC_BANK_1_START_ADDR, ucValue = 0;
		ulAddress < STATIC_BANK_1_START_ADDR + STATIC_BANK_1_SIZE;
		ulAddress += sizeof(portCHAR), ++ucValue)
	{
		if (*(unsigned portCHAR)ulAddress != ucValue)
		{
			enDebug_Print(Memory_TaskToken, "bad\n\r", 10);
			break;
		}
	}
	
	//FLASH test
	
	/* end temporary test block */
	
	for ( ; ; )
	{
		enResult = enGetRequest(Memory_TaskToken, &incoming_packet, portMAX_DELAY);

		if (enResult == URC_SUCCESS)
		{
			//TODO process request
		}
	}
}

