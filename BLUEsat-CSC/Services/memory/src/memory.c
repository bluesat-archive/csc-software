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
#include "debug.h"
#include "emc.h"
#include "Int_Flash.h"
#include "fram.h"
#include "StorageOpControl.h"

#define MEMORY_TEST		0
#define MEMORY_Q_SIZE	1

//task token for accessing services
static TaskToken Memory_TaskToken;

//prototype for task function
static portTASK_FUNCTION(vMemoryTask, pvParameters);

/**
 * \brief Forward request to different memory management depend on requester taskID
 *
 * \param[in] taskToken Task token from request task
 * \param[in] pMemoryContent Request details
 *
 * \returns Request result or URC_FAIL of the operation
 */
static UnivRetCode enMemoryForwardSwitch(TaskToken taskToken, MemoryContent *pMemoryContent);

#if MEMORY_TEST == 1
	void vMemTest(void);
#endif

void vMemory_Init(unsigned portBASE_TYPE uxPriority)
{
	Memory_TaskToken = ActivateTask(TASK_MEMORY, 
								"Memory",
								SEV_TASK_TYPE,
								uxPriority, 
								SERV_STACK_SIZE, 
								vMemoryTask);
								
	vActivateQueue(Memory_TaskToken, MEMORY_Q_SIZE);

	//initialise internal flash memory management
	vIntFlash_Init(uxPriority);

	//initialise FRAM management
	vFRAM_Init(uxPriority);
}

static portTASK_FUNCTION(vMemoryTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;
	
#if MEMORY_TEST == 1
	vMemTest();
#endif

	for ( ; ; )
	{
		enResult = enGetRequest(Memory_TaskToken, &incoming_packet, portMAX_DELAY);

		if (enResult != URC_SUCCESS) continue;

		//TODO process request
	}
}

static UnivRetCode enMemoryForwardSwitch(TaskToken taskToken, MemoryContent *pMemoryContent)
{
	MessagePacket outgoing_packet;

	outgoing_packet.Src			= taskToken->enTaskID;
	outgoing_packet.Token		= taskToken;
	outgoing_packet.Data		= (unsigned portLONG)pMemoryContent;

	//forward to different memory type
	switch (outgoing_packet.Src)
	{
		case	TASK_MEMORY_DEMO:	outgoing_packet.Dest = TASK_MEM_INT_FLASH;
									break;

		default					:	return URC_MEM_NOT_ON_STORAGE_LIST;
	}

	vDebugPrint(Memory_TaskToken,
				"Forwarded %50s request!\n\r",
				(unsigned portLONG)taskToken->pcTaskName,
				NO_INSERT,
				NO_INSERT);

	return enProcessRequest(&outgoing_packet, portMAX_DELAY);
}

UnivRetCode enDataDelete(TaskToken taskToken,
						unsigned portCHAR ucDID)
{
	MemoryContent memoryContent;

	if (ucDID >= (1 << DID_BIT_SIZE)) return URC_MEM_INVALID_DID;

	memoryContent.Operation		= MEM_DELETE;
	memoryContent.DID			= ucDID;
	memoryContent.pulRetValue	= NULL;
	memoryContent.Ptr			= NULL;

	return enMemoryForwardSwitch(taskToken, &memoryContent);
}

UnivRetCode enDataSize(TaskToken taskToken,
						unsigned portCHAR ucDID,
						unsigned portLONG *pulDataSize)
{
	MemoryContent memoryContent;

	if (ucDID >= (1 << DID_BIT_SIZE)) return URC_MEM_INVALID_DID;

	memoryContent.Operation 	= MEM_SIZE;
	memoryContent.DID			= ucDID;
	memoryContent.pulRetValue	= pulDataSize;
	memoryContent.Ptr			= NULL;

	return enMemoryForwardSwitch(taskToken, &memoryContent);
}

UnivRetCode enDataRead(TaskToken taskToken,
						unsigned portCHAR ucDID,
						unsigned portLONG ulOffset,
						unsigned portLONG ulSize,
						portCHAR *pucBuffer,
						unsigned portLONG *pulReadRetSize)
{
	MemoryContent memoryContent;

	if (ucDID >= (1 << DID_BIT_SIZE)) return URC_MEM_INVALID_DID;

	memoryContent.Operation 	= MEM_READ;
	memoryContent.DID			= ucDID;
	memoryContent.Offset		= ulOffset;
	memoryContent.Ptr			= pucBuffer;
	memoryContent.Size			= ulSize;
	memoryContent.pulRetValue	= pulReadRetSize;

	return enMemoryForwardSwitch(taskToken, &memoryContent);
}

UnivRetCode enDataStore(TaskToken taskToken,
						unsigned portCHAR ucDID,
						unsigned portLONG ulSize,
						portCHAR *pcData)
{
	MemoryContent memoryContent;

	if (ucDID >= (1 << DID_BIT_SIZE)) return URC_MEM_INVALID_DID;

	memoryContent.Operation 	= MEM_STORE;
	memoryContent.DID			= ucDID;
	memoryContent.Ptr			= pcData;
	memoryContent.Size			= ulSize;
	memoryContent.pulRetValue	= NULL;

	return enMemoryForwardSwitch(taskToken, &memoryContent);
}

UnivRetCode enDataAppend(TaskToken taskToken,
						unsigned portCHAR ucDID,
						unsigned portLONG ulSize,
						portCHAR *pcData)
{
	MemoryContent memoryContent;

	if (ucDID >= (1 << DID_BIT_SIZE)) return URC_MEM_INVALID_DID;

	memoryContent.Operation 	= MEM_APPEND;
	memoryContent.DID			= ucDID;
	memoryContent.Ptr			= pcData;
	memoryContent.Size			= ulSize;
	memoryContent.pulRetValue	= NULL;

	return enMemoryForwardSwitch(taskToken, &memoryContent);
}

#if MEMORY_TEST == 1
	void vMemTest(void)
	{
		/* start temporary test block */
		unsigned portLONG ulAddress;
		unsigned portCHAR ucValue;
		unsigned portSHORT usValue;

		//SRAM test
		//8 bits test
		vDebugPrint(Memory_TaskToken, "SRAM 8 bits test...\n\r", 50);
		for (ulAddress = STATIC_BANK_0_START_ADDR, ucValue = 3;
			ulAddress < STATIC_BANK_0_START_ADDR + STATIC_BANK_0_SIZE;
			ulAddress += sizeof(unsigned portCHAR), ++ucValue)
		{
			*(unsigned portCHAR *)ulAddress = ucValue;
		}

		for (ulAddress = STATIC_BANK_0_START_ADDR, ucValue = 3;
			ulAddress < STATIC_BANK_0_START_ADDR + STATIC_BANK_0_SIZE;
			ulAddress += sizeof(unsigned portCHAR), ++ucValue)
		{
			if (*(unsigned portCHAR *)ulAddress != ucValue)
			{
				vDebugPrint(Memory_TaskToken, "SRAM 8 bits test failed!\n\r", 50);
				break;
			}
		}

		//FRAM test
		//8 bits test
		vDebugPrint(Memory_TaskToken, "FRAM 8 bits test...\n\r", 50);
		for (ulAddress = STATIC_BANK_1_START_ADDR, ucValue = 3;
			ulAddress < STATIC_BANK_1_START_ADDR + STATIC_BANK_1_SIZE;
			ulAddress += sizeof(unsigned portCHAR), ++ucValue)
		{
			*(unsigned portCHAR *)ulAddress = ucValue;
		}

		for (ulAddress = STATIC_BANK_1_START_ADDR, ucValue = 3;
			ulAddress < STATIC_BANK_1_START_ADDR + STATIC_BANK_1_SIZE;
			ulAddress += sizeof(unsigned portCHAR), ++ucValue)
		{
			if (*(unsigned portCHAR *)ulAddress != ucValue)
			{
				vDebugPrint(Memory_TaskToken, "FRAM 8 bits test failed!\n\r", 50);
				break;
			}
		}

		//16 bits test
		vDebugPrint(Memory_TaskToken, "FRAM 16 bits test...\n\r", 50);
		for (ulAddress = STATIC_BANK_1_START_ADDR, usValue = 3;
			ulAddress < STATIC_BANK_1_START_ADDR + STATIC_BANK_1_SIZE;
			ulAddress += sizeof(unsigned portSHORT), ++usValue)
		{
			*(unsigned portSHORT *)ulAddress = usValue;
		}

		for (ulAddress = STATIC_BANK_1_START_ADDR, usValue = 3;
			ulAddress < STATIC_BANK_1_START_ADDR + STATIC_BANK_1_SIZE;
			ulAddress += sizeof(unsigned portSHORT), ++usValue)
		{
			if (*(unsigned portSHORT *)ulAddress != usValue)
			{
				vDebugPrint(Memory_TaskToken, "FRAM 16 bits test failed!\n\r", 50);
				break;
			}
		}

		//32 bits test
		vDebugPrint(Memory_TaskToken, "FRAM 32 bits test...\n\r", 50);
		for (ulAddress = STATIC_BANK_1_START_ADDR;
			ulAddress < STATIC_BANK_1_START_ADDR + STATIC_BANK_1_SIZE;
			ulAddress += sizeof(unsigned portLONG))
		{
			*(unsigned portLONG *)ulAddress = ulAddress + 1;
		}

		for (ulAddress = STATIC_BANK_1_START_ADDR;
			ulAddress < STATIC_BANK_1_START_ADDR + STATIC_BANK_1_SIZE;
			ulAddress += sizeof(unsigned portLONG))
		{
			if (*(unsigned portLONG *)ulAddress != ulAddress + 1)
			{
				vDebugPrint(Memory_TaskToken, "FRAM 32 bits test failed!\n\r", 50);
				break;
			}
		}
		//FLASH test

		/* end temporary test block */
	}
#endif
