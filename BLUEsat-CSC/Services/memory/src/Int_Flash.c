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
#define	MEMORY_SEGMENT_SIZE			BYTE_512

//task token for accessing services
static TaskToken Flash_TaskToken;

static GSACore IntFlashCore;

//TODO optimisation, try consolidate memory creation functions
//GSACore state table memory maker task function
//this function is used when malloc fail to create memory
static portTASK_FUNCTION(vCreateStateTableMemory, pvParameters);

//GSACore data table memory maker task function
//this function is used when malloc fail to create memory
static portTASK_FUNCTION(vCreateDataTableMemory, pvParameters);

//GSACore buffer memory maker task function
//this function is used when malloc fail to create memory
static portTASK_FUNCTION(vCreateBuffereMemory, pvParameters);

//prototype for task function
static portTASK_FUNCTION(vFlashTask, pvParameters);

//locate free memory segment
unsigned portLONG GetNextMemSegFn(void);

portBASE_TYPE WriteToMemSegFn(unsigned portLONG ulMemSegAddr);

//check memory segment is free
static portBASE_TYPE xIsMemSegFreeFn(unsigned portLONG ulMemSegAddr);

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

	IntFlashCore.MemSegBuffer = NULL;	//TODO use malloc, currently simulate malloc return NULL

	IntFlashCore.StateTableSize = STATE_TABLE_SIZE(FlashSecAdds[START_SECTOR],
													FlashSecAdds[END_SECTOR],
													MEMORY_SEGMENT_SIZE);
	IntFlashCore.StateTable = NULL;		//TODO use malloc, currently simulate malloc return NULL

	IntFlashCore.DataTableSize = DATA_TABLE_SIZE(NORMAL_DATA_TABLE_SIZE);
	IntFlashCore.DataTable = NULL;		//TODO use malloc, currently simulate malloc return NULL

	if (IntFlashCore.DataTable == NULL)
	{
		usExtraMemory += (DATA_TABLE_SIZE(DEFAULT_DATA_TABLE_SIZE) / sizeof(unsigned portLONG))
								+ (DATA_TABLE_SIZE(DEFAULT_DATA_TABLE_SIZE) % sizeof(unsigned portLONG) > 0);
		pvTaskFn = vCreateDataTableMemory;
	}

	if (IntFlashCore.StateTable == NULL)
	{
		usExtraMemory += (IntFlashCore.StateTableSize / sizeof(unsigned portLONG))
								+ (IntFlashCore.StateTableSize % sizeof(unsigned portLONG) > 0);
		pvTaskFn = vCreateStateTableMemory;
	}

	if (IntFlashCore.MemSegBuffer == NULL)
	{
		usExtraMemory += (MEMORY_SEGMENT_SIZE / sizeof(unsigned portLONG))
								+ (MEMORY_SEGMENT_SIZE % sizeof(unsigned portLONG) > 0);
		pvTaskFn = vCreateBuffereMemory;
	}

	IntFlashCore.GetNextMemSeg = GetNextMemSegFn;
	IntFlashCore.xIsMemSegFree = xIsMemSegFreeFn;
	IntFlashCore.WriteToMemSeg = WriteToMemSegFn;
	
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

static portTASK_FUNCTION(vCreateStateTableMemory, pvParameters)
{
	(void) pvParameters;
	unsigned portCHAR ucMemory[STATE_TABLE_SIZE(FlashSecAdds[START_SECTOR], FlashSecAdds[END_SECTOR], MEMORY_SEGMENT_SIZE)];

	IntFlashCore.StateTable = ucMemory;

	if (IntFlashCore.DataTable == NULL) vCreateDataTableMemory(NULL);

	vFlashTask(NULL);
}

static portTASK_FUNCTION(vCreateDataTableMemory, pvParameters)
{
	(void) pvParameters;
	unsigned portCHAR ucMemory[DATA_TABLE_SIZE(DEFAULT_DATA_TABLE_SIZE)];

	IntFlashCore.DataTableSize = DATA_TABLE_SIZE(DEFAULT_DATA_TABLE_SIZE);
	
	IntFlashCore.DataTable = (void *)ucMemory;

	vFlashTask(NULL);
}

static portTASK_FUNCTION(vCreateBuffereMemory, pvParameters)
{
	(void) pvParameters;
	unsigned portLONG ulMemory[MEMORY_SEGMENT_SIZE / sizeof(unsigned portLONG)];

	IntFlashCore.MemSegBuffer = ulMemory;

	if (IntFlashCore.StateTable == NULL) vCreateStateTableMemory(NULL);

	if (IntFlashCore.DataTable == NULL) vCreateDataTableMemory(NULL);

	vFlashTask(NULL);
}

static portTASK_FUNCTION(vFlashTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;
	MemoryContent *pContentHandle;

    vDebugPrint(Flash_TaskToken, "Initialisation!\n\r", NO_INSERT, NO_INSERT, NO_INSERT);
	vInitialiseCore(&IntFlashCore);

    vDebugPrint(Flash_TaskToken, "Survey memory!\n\r", NO_INSERT, NO_INSERT, NO_INSERT);
	vSurveyMemory(&IntFlashCore, FlashSecAdds[START_SECTOR], FlashSecAdds[END_SECTOR]);

	vBuildDataTable(&IntFlashCore, FlashSecAdds[START_SECTOR], FlashSecAdds[END_SECTOR], pdTRUE, TASK_MEM_INT_FLASH);

	vDebugPrint(Flash_TaskToken, "Free blocks: %d\n\rData blocks: %d\n\rDead blocks: %d\n\r",
				usCountState(&IntFlashCore, FlashSecAdds[START_SECTOR], FlashSecAdds[END_SECTOR], STATE_FREE),
				usCountState(&IntFlashCore, FlashSecAdds[START_SECTOR], FlashSecAdds[END_SECTOR], STATE_USED_DATA)
					+ usCountState(&IntFlashCore, FlashSecAdds[START_SECTOR], FlashSecAdds[END_SECTOR], STATE_USED_HEAD),
				usCountState(&IntFlashCore, FlashSecAdds[START_SECTOR], FlashSecAdds[END_SECTOR], STATE_DELETED));

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

/*********************************** function pointers *********************************/
unsigned portLONG GetNextMemSegFn(void)
{
	//TODO make this function evenly wear out memory
	return ulFindNextFreeState(&IntFlashCore, FlashSecAdds[START_SECTOR], FlashSecAdds[END_SECTOR]);
}

portBASE_TYPE WriteToMemSegFn(unsigned portLONG ulMemSegAddr)
{
	if (Ram_To_Flash((void *)ulMemSegAddr, (void *)IntFlashCore.MemSegBuffer, IntFlashCore.MemSegSize) != CMD_SUCCESS) return pdFALSE;

	return pdTRUE;
}

static portBASE_TYPE xIsMemSegFreeFn(unsigned portLONG ulMemSegAddr)
{
	unsigned portLONG *pulTmpPtr;

	//fresh erase all bits = 1
	for (pulTmpPtr = (unsigned portLONG *)ulMemSegAddr;
		pulTmpPtr < (unsigned portLONG *)(ulMemSegAddr + IntFlashCore.MemSegSize);
		++pulTmpPtr)
	{
		if (*pulTmpPtr != 0xffffffff) return pdFALSE;
	}

	return pdTRUE;
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

