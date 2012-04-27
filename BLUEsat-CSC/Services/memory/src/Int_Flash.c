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
#include "iap.h"
#include "debug.h"
#include "gsa.h"
#include "StorageOpControl.h"

#define FLASH_Q_SIZE	1

#define DEFAULT_DATA_TABLE_SIZE		50
#define NORMAL_DATA_TABLE_SIZE		100
#define START_SECTOR				22
#define END_SECTOR					MAX_NUM_SECTS
#define	MEMORY_SEGMENT_SIZE			512

//task token for accessing services
static TaskToken Flash_TaskToken;

static GSACore IntFlashCore;

//GSACore state table memory maker task function
//this function is used when malloc fail to create memory
static portTASK_FUNCTION(vStateTableMemory, pvParameters);

//GSACore data table memory maker task function
//this function is used when malloc fail to create memory
static portTASK_FUNCTION(vDataTableMemory, pvParameters);

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
	void *pvTaskFn;
	unsigned portSHORT usExtraMemory = 0;

	//initialise GSACore
	IntFlashCore.StartAddr = FlashSecAdds[START_SECTOR];
	IntFlashCore.MemSegSize = MEMORY_SEGMENT_SIZE;

	IntFlashCore.StateTableSize = STATE_TABLE_SIZE(FlashSecAdds[START_SECTOR],
													FlashSecAdds[END_SECTOR],
													MEMORY_SEGMENT_SIZE);
	IntFlashCore.StateTable = NULL;		//TODO use malloc, currently simulate malloc return NULL

	IntFlashCore.DataTableSize = DATA_TABLE_SIZE(NORMAL_DATA_TABLE_SIZE);
	IntFlashCore.DataTable = NULL;		//TODO use malloc, currently simulate malloc return NULL

	if (IntFlashCore.DataTable == NULL)
	{
		usExtraMemory += (IntFlashCore.StateTableSize / sizeof(unsigned portLONG))
								+ (IntFlashCore.StateTableSize % sizeof(unsigned portLONG) > 0);
		pvTaskFn = vDataTableMemory;
	}

	if (IntFlashCore.StateTable == NULL)
	{
		usExtraMemory += DATA_TABLE_SIZE(DEFAULT_DATA_TABLE_SIZE);
		pvTaskFn = vStateTableMemory;
	}

#ifndef NO_DEBUG
	IntFlashCore.DebugTrace = DebugTraceFn;
#endif /* NO_DEBUG */

	Flash_TaskToken = ActivateTask(TASK_MEM_INT_FLASH,
								"IntFlash",
								SEV_TASK_TYPE,
								uxPriority,
								SERV_STACK_SIZE + usExtraMemory,
								pvTaskFn);

	vActivateQueue(Flash_TaskToken, FLASH_Q_SIZE);
}

static portTASK_FUNCTION(vStateTableMemory, pvParameters)
{
	(void) pvParameters;
	unsigned portCHAR ucMemory[STATE_TABLE_SIZE(FlashSecAdds[START_SECTOR], FlashSecAdds[END_SECTOR], MEMORY_SEGMENT_SIZE)];

	IntFlashCore.StateTable = ucMemory;

	if (IntFlashCore.DataTable == NULL) vDataTableMemory(NULL); else vFlashTask(NULL);
}

static portTASK_FUNCTION(vDataTableMemory, pvParameters)
{
	(void) pvParameters;
	unsigned portCHAR ucMemory[DATA_TABLE_SIZE(DEFAULT_DATA_TABLE_SIZE)];

	IntFlashCore.DataTableSize = DATA_TABLE_SIZE(DEFAULT_DATA_TABLE_SIZE);
	IntFlashCore.DataTable = (void *)ucMemory;

	vFlashTask(NULL);
}

static portTASK_FUNCTION(vFlashTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;
	MemoryContent *pContentHandle;

	vInitialiseCore(&IntFlashCore);

	vSurveyMemory(&IntFlashCore, FlashSecAdds[START_SECTOR], FlashSecAdds[END_SECTOR]);

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

