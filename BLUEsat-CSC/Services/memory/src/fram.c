 /**
 *  \file fram.c
 *
 *  \brief FRAM management
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
#include "fram.h"
#include "debug.h"
#include "gsa.h"
#include "StorageOpControl.h"

#define FRAM_Q_SIZE	1

//task token for accessing services
static TaskToken FRAM_TaskToken;

static GSACore FRAMCore;

//prototype for task function
static portTASK_FUNCTION(vFRAMTask, pvParameters);

void vFRAM_Init(unsigned portBASE_TYPE uxPriority)
{
	FRAM_TaskToken = ActivateTask(TASK_MEM_FRAM,
								"FRAM",
								TYPE_SERVICE,
								uxPriority,
								SERV_STACK_SIZE,
								vFRAMTask);

	vActivateQueue(FRAM_TaskToken, FRAM_Q_SIZE);
}

static portTASK_FUNCTION(vFRAMTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;
	MemoryContent *pContentHandle;

	vDebugPrint(FRAM_TaskToken, "Ready!\n\r", NO_INSERT, NO_INSERT, NO_INSERT);

	for ( ; ; )
	{
		enResult = enGetRequest(FRAM_TaskToken, &incoming_packet, portMAX_DELAY);

		if (enResult != URC_SUCCESS) continue;

		pContentHandle = (MemoryContent *)incoming_packet.Data;

		vCompleteRequest(incoming_packet.Token, enProcessStorageReq(&FRAMCore, pContentHandle));
	}
}
