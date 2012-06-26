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
#include "storage.h"
#include "debug.h"
#include "Int_Flash.h"
#include "fram.h"
#include "StorageOpControl.h"

#define STOAGE_Q_SIZE	1

//task token for accessing services
static TaskToken Storage_TaskToken;

//prototype for task function
static portTASK_FUNCTION(vStorageTask, pvParameters);

/**
 * \brief Forward request to different memory management depend on requester taskID
 *
 * \param[in] taskToken Task token from request task
 * \param[in] pStorageContent Request details
 *
 * \returns Request result or URC_FAIL of the operation
 */
static UnivRetCode enStorageForwardSwitch(TaskToken taskToken, StorageContent *pStorageContent);

void vStorage_Init(unsigned portBASE_TYPE uxPriority)
{
	Storage_TaskToken = ActivateTask(TASK_STORAGE,
								"Storage",
								SEV_TASK_TYPE,
								uxPriority,
								SERV_STACK_SIZE,
								vStorageTask);

	vActivateQueue(Storage_TaskToken, STOAGE_Q_SIZE);

#ifndef MEM_TEST
	//initialise internal flash memory management
	vIntFlash_Init(uxPriority);

	//initialise FRAM management
	//vFRAM_Init(uxPriority);
#endif
}

static portTASK_FUNCTION(vStorageTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;

	for ( ; ; )
	{
		enResult = enGetRequest(Storage_TaskToken, &incoming_packet, portMAX_DELAY);

		if (enResult != URC_SUCCESS) continue;

		//TODO process request
	}
}

static UnivRetCode enStorageForwardSwitch(TaskToken taskToken, StorageContent *pStorageContent)
{
	MessagePacket outgoing_packet;

	outgoing_packet.Src			= taskToken->enTaskID;
	outgoing_packet.Token		= taskToken;
	outgoing_packet.Data		= (unsigned portLONG)pStorageContent;

	//forward to different memory type
	switch (outgoing_packet.Src)
	{
		case	TASK_MEMORY_DEMO:	outgoing_packet.Dest = TASK_MEM_INT_FLASH;
									break;

		default					:	return URC_MEM_NOT_ON_STORAGE_LIST;
	}

	vDebugPrint(Storage_TaskToken,
				"Forwarded %50s request!\n\r",
				(unsigned portLONG)taskToken->pcTaskName,
				NO_INSERT,
				NO_INSERT);

	return enProcessRequest(&outgoing_packet, portMAX_DELAY);
}

UnivRetCode enDataDelete(TaskToken taskToken,
						unsigned portCHAR ucDID)
{
	StorageContent storageContent;

	if (ucDID >= (1 << DID_BIT_SIZE)) return URC_MEM_INVALID_DID;

	storageContent.Operation		= MEM_DELETE;
	storageContent.DID			= ucDID;
	storageContent.pulRetValue	= NULL;
	storageContent.Ptr			= NULL;

	return enStorageForwardSwitch(taskToken, &storageContent);
}

UnivRetCode enDataSize(TaskToken taskToken,
						unsigned portCHAR ucDID,
						unsigned portLONG *pulDataSize)
{
	StorageContent storageContent;

	if (ucDID >= (1 << DID_BIT_SIZE)) return URC_MEM_INVALID_DID;

	storageContent.Operation 	= MEM_SIZE;
	storageContent.DID			= ucDID;
	storageContent.pulRetValue	= pulDataSize;
	storageContent.Ptr			= NULL;

	return enStorageForwardSwitch(taskToken, &storageContent);
}

UnivRetCode enDataRead(TaskToken taskToken,
						unsigned portCHAR ucDID,
						unsigned portLONG ulOffset,
						unsigned portLONG ulSize,
						portCHAR *pucBuffer,
						unsigned portLONG *pulReadRetSize)
{
	StorageContent storageContent;

	if (ucDID >= (1 << DID_BIT_SIZE)) return URC_MEM_INVALID_DID;

	storageContent.Operation 	= MEM_READ;
	storageContent.DID			= ucDID;
	storageContent.Offset		= ulOffset;
	storageContent.Ptr			= pucBuffer;
	storageContent.Size			= ulSize;
	storageContent.pulRetValue	= pulReadRetSize;

	return enStorageForwardSwitch(taskToken, &storageContent);
}

UnivRetCode enDataStore(TaskToken taskToken,
						unsigned portCHAR ucDID,
						unsigned portLONG ulSize,
						portCHAR *pcData)
{
	StorageContent storageContent;

	if (ucDID >= (1 << DID_BIT_SIZE)) return URC_MEM_INVALID_DID;

	storageContent.Operation 	= MEM_STORE;
	storageContent.DID			= ucDID;
	storageContent.Ptr			= pcData;
	storageContent.Size			= ulSize;
	storageContent.pulRetValue	= NULL;

	return enStorageForwardSwitch(taskToken, &storageContent);
}

UnivRetCode enDataAppend(TaskToken taskToken,
						unsigned portCHAR ucDID,
						unsigned portLONG ulSize,
						portCHAR *pcData)
{
	StorageContent storageContent;

	if (ucDID >= (1 << DID_BIT_SIZE)) return URC_MEM_INVALID_DID;

	storageContent.Operation 	= MEM_APPEND;
	storageContent.DID			= ucDID;
	storageContent.Ptr			= pcData;
	storageContent.Size			= ulSize;
	storageContent.pulRetValue	= NULL;

	return enStorageForwardSwitch(taskToken, &storageContent);
}
