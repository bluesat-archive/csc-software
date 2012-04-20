 /**
 *  \file Int_Flash.c
 *
 *  \brief Internal flash memory management
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
#include "Int_Flash.h"
#include "debug.h"
#include "gsa.h"
#include "StorageOpControl.h"

#define FLASH_Q_SIZE	1

//task token for accessing services
static TaskToken Flash_TaskToken;

static GSACore IntFlashCore;

//prototype for task function
static portTASK_FUNCTION(vFlashTask, pvParameters);

#ifndef NO_DEBUG
static void DebugTraceFn (portCHAR *pcFormat,
						unsigned portLONG Insert1,
						unsigned portLONG Insert2,
						unsigned portLONG Insert3);
#endif /* NO_DEBUG */

void vIntFlash_Init(unsigned portBASE_TYPE uxPriority)
{
	Flash_TaskToken = ActivateTask(TASK_MEM_INT_FLASH,
								"IntFlash",
								SEV_TASK_TYPE,
								uxPriority,
								SERV_STACK_SIZE,
								vFlashTask);

	vActivateQueue(Flash_TaskToken, FLASH_Q_SIZE);

#ifndef NO_DEBUG
	IntFlashCore.DebugTracePtr = DebugTraceFn;
#endif /* NO_DEBUG */
}

static portTASK_FUNCTION(vFlashTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;
	MemoryContent *pContentHandle;

	vDebugPrint(Flash_TaskToken, "Ready!\n\r", NO_INSERT, NO_INSERT, NO_INSERT);

	for ( ; ; )
	{
		enResult = enGetRequest(Flash_TaskToken, &incoming_packet, portMAX_DELAY);

		if (enResult != URC_SUCCESS) continue;

		pContentHandle = (MemoryContent *)incoming_packet.Data;

		vCompleteRequest(incoming_packet.Token, enProcessStorageReq(&IntFlashCore,
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
	vDebugPrint(Flash_TaskToken, pcFormat, Insert1, Insert2, Insert3);
}
#endif /* NO_DEBUG */

