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

#define START_SECTOR				26
#define START_SECTOR_ADDR			SECTOR26ADDR
#define END_SECTOR					28
#define END_SECTOR_ADDR				SECTOR28ADDR
#define	MEMORY_SEGMENT_SIZE			BYTE_512

//task token for accessing services
static TaskToken Flash_TaskToken;

static GSACore IntFlashCore;

//prototype for task function
static portTASK_FUNCTION(vIntFlashTask, pvParameters);

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
	Flash_TaskToken = ActivateTask(TASK_MEM_INT_FLASH,
									"IntFlash",
									SEV_TASK_TYPE,
									uxPriority,
									SERV_STACK_SIZE,
									vIntFlashTask);

	vActivateQueue(Flash_TaskToken, FLASH_Q_SIZE);
}

static unsigned portLONG MemSegBuffer[MEMORY_SEGMENT_SIZE / sizeof(portLONG)];
static unsigned portCHAR StateTable[STATE_TABLE_SIZE(START_SECTOR_ADDR, END_SECTOR_ADDR, MEMORY_SEGMENT_SIZE)];

static portTASK_FUNCTION(vIntFlashTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;
	StorageContent *pContentHandle;

	IntFlashCore.StartAddr = FlashSecAdds[START_SECTOR];
	IntFlashCore.EndAddr = FlashSecAdds[END_SECTOR];
	IntFlashCore.MemSegSize = MEMORY_SEGMENT_SIZE;

	IntFlashCore.StateTable = StateTable;
	IntFlashCore.StateTableSize = STATE_TABLE_SIZE(FlashSecAdds[START_SECTOR],
													FlashSecAdds[END_SECTOR],
													MEMORY_SEGMENT_SIZE);
	IntFlashCore.MemSegBuffer = MemSegBuffer;

	//initialise GSACore
	IntFlashCore.xIsMemSegFree = xIsMemSegFreeFn;
	IntFlashCore.WriteToMemSeg = WriteToMemSegFn;
#ifndef NO_DEBUG
	IntFlashCore.DebugTrace = DebugTraceFn;
#endif /* NO_DEBUG */

    vDebugPrint(Flash_TaskToken, "Initialisation!\n\r", NO_INSERT, NO_INSERT, NO_INSERT);
	vInitialiseCore(&IntFlashCore);

    vDebugPrint(Flash_TaskToken, "Survey memory!\n\r", NO_INSERT, NO_INSERT, NO_INSERT);
	xSurveyMemory(&IntFlashCore);

	for ( ; ; )
	{
		enResult = enGetRequest(Flash_TaskToken, &incoming_packet, portMAX_DELAY);

		if (enResult != URC_SUCCESS) continue;

		pContentHandle = (StorageContent *)incoming_packet.Data;

		vCompleteRequest(incoming_packet.Token, enProcessStorageReq(&IntFlashCore,
																	incoming_packet.Src,
																	pContentHandle));
	}
}

/*********************************** function pointers *********************************/
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

