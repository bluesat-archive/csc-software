 /**
 *  \file Demo_Application_2.c
 *
 *  \brief An application demonstrating how an application operate
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#include "application.h"
#include "memoryDemo.h"
#include "memory.h"
#include "debug.h"

#define MEM_DEMO_Q_SIZE	1

//task token for accessing services and other applications
static TaskToken MemDEMO_TaskToken;

static portTASK_FUNCTION(vMemDemoTask, pvParameters);

void vMemDemo_Init(unsigned portBASE_TYPE uxPriority)
{
	MemDEMO_TaskToken = ActivateTask(TASK_MEMORY_DEMO,
									"MemoryDemo",
									TYPE_APPLICATION,
									uxPriority,
									APP_STACK_SIZE,
									vMemDemoTask);

	vActivateQueue(MemDEMO_TaskToken, MEM_DEMO_Q_SIZE);
}

static portTASK_FUNCTION(vMemDemoTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode 	enResult;
	MessagePacket 	incoming_packet;
	portCHAR		pcBuffer[21];

	enDebugPrint(MemDEMO_TaskToken, "Hello!\n\r", 0, 0, 0);

	if (enDataDelete(MemDEMO_TaskToken, 16) == URC_CMD_NO_TASK) enDebugPrint(MemDEMO_TaskToken, "Pass 1!\n\r", 0, 0, 0);
	if (enDataSize(MemDEMO_TaskToken, 16) == URC_CMD_NO_TASK) enDebugPrint(MemDEMO_TaskToken, "Pass 2!\n\r", 0, 0, 0);
	if (enDataRead(MemDEMO_TaskToken, 16, 0, 10, pcBuffer) == URC_CMD_NO_TASK) enDebugPrint(MemDEMO_TaskToken, "Pass 3!\n\r", 0, 0, 0);
	if (enDataStore(MemDEMO_TaskToken, 16, 10, pcBuffer) == URC_CMD_NO_TASK) enDebugPrint(MemDEMO_TaskToken, "Pass 4!\n\r", 0, 0, 0);
	if (enDataAppend(MemDEMO_TaskToken, 64, 10, pcBuffer) == URC_MEM_INVALID_DID) enDebugPrint(MemDEMO_TaskToken, "Pass 5!\n\r", 0, 0, 0);

	usDebugRead(pcBuffer, 49);

	enDebugPrint(MemDEMO_TaskToken, "%20s %h %10s %10x\n\r", (unsigned portLONG)pcBuffer, 0x12345678, (unsigned portLONG)"Good");

	for ( ; ; )
	{
		enResult = enGetRequest(MemDEMO_TaskToken, &incoming_packet, portMAX_DELAY);

		if (enResult == URC_SUCCESS) continue;

		//TODO implement memory demo

	}
}


