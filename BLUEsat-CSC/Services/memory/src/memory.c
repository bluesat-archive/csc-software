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

#define MEMORY_Q_SIZE	1

#ifdef MEM_TEST
	static void vMemoryTest(unsigned portLONG ulStartAddr, unsigned portLONG ulSize);
	static UnivRetCode enMemoryTest_8(unsigned portLONG ulStartAddr, unsigned portLONG ulSize);
	static UnivRetCode enMemoryTest_16(unsigned portLONG ulStartAddr, unsigned portLONG ulSize);
	static UnivRetCode enMemoryTest_32(unsigned portLONG ulStartAddr, unsigned portLONG ulSize);
#endif

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

void vMemory_Init(unsigned portBASE_TYPE uxPriority)
{
	Memory_TaskToken = ActivateTask(TASK_MEMORY, 
								"Memory",
								SEV_TASK_TYPE,
								uxPriority, 
								SERV_STACK_SIZE, 
								vMemoryTask);
								
	vActivateQueue(Memory_TaskToken, MEMORY_Q_SIZE);

#ifndef MEM_TEST
	//initialise internal flash memory management
	vIntFlash_Init(uxPriority);

	//initialise FRAM management
	vFRAM_Init(uxPriority);
#endif
}

static portTASK_FUNCTION(vMemoryTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;
	
#ifdef MEM_TEST
	//FRAM test
	vDebugPrint(Memory_TaskToken, "FRAM Test:\n\r", 0,0,0);
	vMemoryTest(STATIC_BANK_0_START_ADDR, STATIC_BANK_0_SIZE);
	//SRAM test
	vDebugPrint(Memory_TaskToken, "SRAM Test:\n\r", 0,0,0);
	vMemoryTest(STATIC_BANK_1_START_ADDR, STATIC_BANK_1_SIZE);
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

#ifdef MEM_TEST
	static void vMemoryTest(unsigned portLONG ulStartAddr, unsigned portLONG ulSize)
	{
		//8 bits test
		vDebugPrint(Memory_TaskToken,
					"8 bits test...%d\n\r",
					enMemoryTest_8(ulStartAddr, ulSize) == URC_SUCCESS,
					0,
					0);

		//16 bits test
		vDebugPrint(Memory_TaskToken,
					"16 bits test...%d\n\r",
					enMemoryTest_16(ulStartAddr, ulSize) == URC_SUCCESS,
					0,
					0);

		//32 bits test
		vDebugPrint(Memory_TaskToken,
					"32 bits test...%d\n\r",
					enMemoryTest_32(ulStartAddr, ulSize) == URC_SUCCESS,
					0,
					0);
	}
	static UnivRetCode enMemoryTest_8(unsigned portLONG ulStartAddr, unsigned portLONG ulSize)
	{
		unsigned portCHAR ucValue;
		unsigned portLONG ulAddress;

		for (ulAddress = ulStartAddr, ucValue = 3;
			ulAddress < ulStartAddr + ulSize;
			ulAddress += sizeof(unsigned portCHAR), ++ucValue)
		{
			*(unsigned portCHAR *)ulAddress = ucValue;
		}

		for (ulAddress = ulStartAddr, ucValue = 3;
			ulAddress < ulStartAddr + ulSize;
			ulAddress += sizeof(unsigned portCHAR), ++ucValue)
		{
			if (*(unsigned portCHAR *)ulAddress != ucValue) return URC_FAIL;
		}

		return URC_SUCCESS;
	}
	static UnivRetCode enMemoryTest_16(unsigned portLONG ulStartAddr, unsigned portLONG ulSize)
	{
		unsigned portLONG ulAddress;
		unsigned portSHORT usValue;

		for (ulAddress = ulStartAddr, usValue = 3;
			ulAddress < ulStartAddr + ulSize;
			ulAddress += sizeof(unsigned portSHORT), ++usValue)
		{
			*(unsigned portSHORT *)ulAddress = usValue;
		}

		for (ulAddress = ulStartAddr, usValue = 3;
			ulAddress < ulStartAddr + ulSize;
			ulAddress += sizeof(unsigned portSHORT), ++usValue)
		{
			if (*(unsigned portSHORT *)ulAddress != usValue) return URC_FAIL;
		}

		return URC_SUCCESS;
	}
	static UnivRetCode enMemoryTest_32(unsigned portLONG ulStartAddr, unsigned portLONG ulSize)
	{
		unsigned portLONG ulAddress;

		for (ulAddress = ulStartAddr;
			ulAddress < ulStartAddr + ulSize;
			ulAddress += sizeof(unsigned portLONG))
		{
			*(unsigned portLONG *)ulAddress = ulAddress + 1;
		}

		for (ulAddress = ulStartAddr;
			ulAddress < ulStartAddr + ulSize;
			ulAddress += sizeof(unsigned portLONG))
		{
			if (*(unsigned portLONG *)ulAddress != ulAddress + 1) return URC_FAIL;
		}

		return URC_SUCCESS;
	}
#endif
