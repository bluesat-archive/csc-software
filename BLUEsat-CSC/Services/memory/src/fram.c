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

#ifndef NO_DEBUG
static void DebugTraceFn (portCHAR *pcFormat,
						unsigned portLONG Insert1,
						unsigned portLONG Insert2,
						unsigned portLONG Insert3);
#endif /* NO_DEBUG */

void vFRAM_Init(unsigned portBASE_TYPE uxPriority)
{
	FRAM_TaskToken = ActivateTask(TASK_MEM_FRAM,
								"FRAM",
								SEV_TASK_TYPE,
								uxPriority,
								SERV_STACK_SIZE,
								vFRAMTask);

	vActivateQueue(FRAM_TaskToken, FRAM_Q_SIZE);

#ifndef NO_DEBUG
	FRAMCore.DebugTracePtr = DebugTraceFn;
#endif /* NO_DEBUG */
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

		vCompleteRequest(incoming_packet.Token, enProcessStorageReq(&FRAMCore,
																	incoming_packet.Src,
																	pContentHandle));
	}
}

#ifndef NO_DEBUG
static void DebugTraceFn (portCHAR *pcFormat,
						unsigned portLONG Insert1,
						unsigned portLONG Insert2,
						unsigned portLONG Insert3)
{
	vDebugPrint(FRAM_TaskToken, pcFormat, Insert1, Insert2, Insert3);
}
#endif /* NO_DEBUG */

