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

#define INTFLASH_START_SECTOR				26
#define INTFLASH_START_SECTOR_ADDR			SECTOR26ADDR
#define INTFLASH_END_SECTOR					27
#define INTFLASH_END_SECTOR_ADDR			SECTOR27ADDR
#define	INTFLASH_BLOCK_SIZE					BYTE_512

//task token for accessing services
static TaskToken Flash_TaskToken;

static GSACore IntFlashCore;

//prototype for task function
static portTASK_FUNCTION(vIntFlashTask, pvParameters);

static unsigned portLONG WriteBufferFn(void);

static portBASE_TYPE xIsBlockFreeFn(unsigned portLONG ulBlockAddr);

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

static unsigned portLONG IntFlashBlockBuffer[INTFLASH_BLOCK_SIZE / sizeof(portLONG)];
static unsigned portCHAR IntFlashStateTable[STATE_TABLE_SIZE(INTFLASH_START_SECTOR_ADDR,
													INTFLASH_END_SECTOR_ADDR,
													INTFLASH_BLOCK_SIZE)];

static portTASK_FUNCTION(vIntFlashTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;
	StorageContent *pContentHandle;

	//setup GSACore
	IntFlashCore.StartAddr 		= FlashSecAdds[INTFLASH_START_SECTOR];
	IntFlashCore.EndAddr 		= FlashSecAdds[INTFLASH_END_SECTOR];
	IntFlashCore.BlockSize		= INTFLASH_BLOCK_SIZE;
	IntFlashCore.BlockBuffer 	= IntFlashBlockBuffer;
	IntFlashCore.StateTable 	= IntFlashStateTable;
	IntFlashCore.StateTableSize = STATE_TABLE_SIZE(INTFLASH_START_SECTOR_ADDR,
													INTFLASH_END_SECTOR_ADDR,
													INTFLASH_BLOCK_SIZE);
	//setup optimisation
	IntFlashCore.Optimisation = NULL;
	//setup function pointers
	IntFlashCore.xIsBlockFree 	= xIsBlockFreeFn;
	IntFlashCore.WriteBuffer 	= WriteBufferFn;
#ifndef NO_DEBUG
	IntFlashCore.DebugTrace 	= DebugTraceFn;
#endif /* NO_DEBUG */

	//initialise GSACore
	vInitialiseCore(&IntFlashCore);
    vDebugPrint(Flash_TaskToken, "Ready!\n\r", NO_INSERT, NO_INSERT, NO_INSERT);

    vDebugPrint(Flash_TaskToken,
				"Free: %d, Valid: %d, Dead %d!\n\r",
				usBlockStateCount(&IntFlashCore,
									INTFLASH_START_SECTOR_ADDR,
									INTFLASH_END_SECTOR_ADDR,
									GSA_EXT_STATE_FREE),
				usBlockStateCount(&IntFlashCore,
									INTFLASH_START_SECTOR_ADDR,
									INTFLASH_END_SECTOR_ADDR,
									GSA_EXT_STATE_VALID),
				usBlockStateCount(&IntFlashCore,
									INTFLASH_START_SECTOR_ADDR,
									INTFLASH_END_SECTOR_ADDR,
									GSA_EXT_STATE_DEAD));

	for ( ; ; )
	{
		enResult = enGetRequest(Flash_TaskToken, &incoming_packet, portMAX_DELAY);

		if (enResult != URC_SUCCESS) continue;

		pContentHandle = (StorageContent *)incoming_packet.Data;

		vCompleteRequest(incoming_packet.Token, enProcessStorageReq(&IntFlashCore,
																	incoming_packet.Src,
																	pContentHandle));

	    vDebugPrint(Flash_TaskToken,
					"Free: %d, Valid: %d, Dead %d!\n\r",
					usBlockStateCount(&IntFlashCore,
										INTFLASH_START_SECTOR_ADDR,
										INTFLASH_END_SECTOR_ADDR,
										GSA_EXT_STATE_FREE),
					usBlockStateCount(&IntFlashCore,
										INTFLASH_START_SECTOR_ADDR,
										INTFLASH_END_SECTOR_ADDR,
										GSA_EXT_STATE_VALID),
					usBlockStateCount(&IntFlashCore,
										INTFLASH_START_SECTOR_ADDR,
										INTFLASH_END_SECTOR_ADDR,
										GSA_EXT_STATE_DEAD));
	}
}

/*********************************** function pointers *********************************/
static unsigned portLONG WriteBufferFn(void)
{
	unsigned portLONG ulFreeBlockAddr;

	//TODO implement smart wear leveling free block selection scheme
	//TODO implement data write treatment on write failure
	ulFreeBlockAddr = ulGetNextFreeBlock(&IntFlashCore, INTFLASH_START_SECTOR_ADDR, INTFLASH_END_SECTOR_ADDR);

	if (ulFreeBlockAddr == (unsigned portLONG)NULL) return (unsigned portLONG)NULL;

	if (Ram_To_Flash((void *)ulFreeBlockAddr, (void *)IntFlashCore.BlockBuffer, IntFlashCore.BlockSize) != CMD_SUCCESS) return 0;

	return ulFreeBlockAddr;
}

static portBASE_TYPE xIsBlockFreeFn(unsigned portLONG ulBlockAddr)
{
	unsigned portLONG *pulTmpPtr;

	//fresh erase all bits == 1
	for (pulTmpPtr = (unsigned portLONG *)ulBlockAddr;
		pulTmpPtr < (unsigned portLONG *)(ulBlockAddr + IntFlashCore.BlockSize);
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

