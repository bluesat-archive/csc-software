 /**
 *  \file gsa.c
 *
 *  \brief General Storage Architecture (GSA) provide data storage organisation
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#include "gsa.h"
#include "lib_string.h"
#include "1sCompChecksum.h"

/******************************** Block Info ********************************/
typedef union
{
	struct
	{
		/* word 0 */
		unsigned portLONG Checksum:    	16;	//header checksum
		unsigned portLONG H:			 1;	//head block bit
		unsigned portLONG NDBI:	  		15;	//Next Data Block Index
	};

	struct
	{
		/* word 0 */
		unsigned portLONG Checksum:    	16;	//header checksum
		unsigned portLONG H:			 1; //head block bit
		unsigned portLONG PrevHBI:		15;	//previous Head Block Index
		/* word 1 */
		unsigned portLONG AID:			 6; //Application ID
		unsigned portLONG DID:			 8; //Data ID
		unsigned portLONG Terminal:		 1; //terminal block flag
		unsigned portLONG FDBI_U:		 1;	//First Data Block Index used
		//extended short
		unsigned portLONG FDBI:			15;	//First Data Block Index
		unsigned portLONG UUpadding:  	 1;	//unusable padding
	};
} Header;

#define HB_HEADER_SIZE 		sizeof(Header)
#define HB_SML_HEADER_SIZE 	(HB_HEADER_SIZE - 2)
#define DB_HEADER_SIZE 		(HB_HEADER_SIZE - 4)

typedef struct
{
	unsigned portLONG	DataSize;
} TreeInfo;

#define DATA_FIELD_SIZE(BlockSize, HeaderSize)(BlockSize-(sizeof(TreeInfo)+HeaderSize))
#define DATA_START_ADDR(BlockAddr, HeaderSize)(BlockAddr+HeaderSize)
#define INFO_START_ADDR(BlockAddr, BlockSize)(BlockAddr+BlockSize-sizeof(TreeInfo))

/*************************************** Start Internal Function Prototypes  ****************************************/
typedef enum
{
	GSA_INT_BLOCK_STATE_DEAD	= 0,	//DEAD must == 0, else xSurveyMemory won't work properly
	GSA_INT_BLOCK_STATE_LHB 	= 1,
	GSA_INT_BLOCK_STATE_VALID	= 1,
	GSA_INT_BLOCK_STATE_HB		= 2,
	GSA_INT_BLOCK_STATE_FREE	= 3
} GSA_INT_STATE;

//map out all block in memory and assign state
static portBASE_TYPE xSurveyMemory(GSACore const *pGSACore);
//isolate valid Last Head Blocks
static void vIsolateLastHeadBlock(GSACore const *pGSACore);
//construct memory from LHBs
static void vBuildMemory(GSACore const *pGSACore);
//finalise state table according to confiuration
static void vFinaliseStateTable(GSACore const *pGSACore);

//check no dead block exist in branch
static portBASE_TYPE xCheckDataBranch(GSACore const *pGSACore,
									unsigned portLONG ulBlockAddr);

//assign specify block state to all blocks in branch
static void vSetDataBranch(GSACore const *pGSACore,
							unsigned portLONG ulBlockAddr,
							GSA_INT_STATE enState);


typedef enum
{
	OP_BLOCK_CHECK_VERIFY,
	OP_BLOCK_CHECK_APPLY
} BlockCheckOp;

//set or verify block checksums
static portBASE_TYPE xBlockChecksum(BlockCheckOp enOperation,
									unsigned portLONG ulBlockAddr,
									GSA_BLOCK_SIZE BlockSize);

typedef enum
{
	OP_STATE_TABLE_SET,
	OP_STATE_TABLE_GET
} StateTableOp;

//set or get state table
static GSA_INT_STATE enAccessStateTable(StateTableOp enOperation,
										GSACore const *pGSACore,
										unsigned portLONG ulBlockAddr,
										GSA_INT_STATE enState);

//search for first block in range matching specfied block state
static unsigned portLONG ulFindBlockViaState(GSACore const *	pGSACore,
											unsigned portLONG 	ulStartAddr,
											unsigned portLONG 	ulEndAddr,
											GSA_INT_STATE 		enState);

//get previous head block in chain
static unsigned portLONG ulPrevHeadBlock(GSACore const *pGSACore,
										unsigned portLONG ulBlockAddr);

//get next data block in chain
static unsigned portLONG ulNextDataBlock(GSACore const *pGSACore,
										unsigned portLONG ulBlockAddr);

//convert address into block index
static unsigned portSHORT usAddrToIndex(GSACore const *pGSACore,
										unsigned portLONG ulAddr);

//convert block index to address
static unsigned portLONG ulIndexToAddr(GSACore const *pGSACore,
										unsigned portSHORT usBlockIndex);

//look up existing data entry matching given AID and DID
static unsigned portLONG ulLookUpDataEntry(GSACore const *pGSACore,
											unsigned portCHAR ucAID,
											unsigned portCHAR ucDID);

/******************************************** Data Table Optimisation ***********************************************/
//find corresponding data table entry
static Data_Table_Entry *pFindDataTableEntry(GSACore const *pGSACore,
											unsigned portCHAR ucAID,
											unsigned portCHAR ucDID);

//add new data table entry
static portBASE_TYPE xAddDataTableEntry(GSACore const *pGSACore,
										unsigned portCHAR ucAID,
										unsigned portCHAR ucDID,
										unsigned portSHORT usLastHBI,
										unsigned portLONG ulSize);

//remove data table entry
static void vRemoveDataTableEntry(GSACore const *pGSACore,
								Data_Table_Entry *pDataTableEntry);
/*************************************** End Internal Function Prototypes  ****************************************/

/**************************************** Start Function Implementations *****************************************/

//initialise GSACore
void vInitialiseCore(GSACore const *pGSACore)
{
	(void)usAddrToIndex;
	(void)ulIndexToAddr;
	(void)pFindDataTableEntry;
	(void)xAddDataTableEntry;
	(void)vRemoveDataTableEntry;

	xSurveyMemory(pGSACore);
	pGSACore->DebugTrace("Memory surveyed\n\r", 0,0,0);
	vIsolateLastHeadBlock(pGSACore);
	pGSACore->DebugTrace("LHB isolated\n\r", 0,0,0);
	vBuildMemory(pGSACore);
	pGSACore->DebugTrace("Memory built\n\r", 0,0,0);
	vFinaliseStateTable(pGSACore);
	pGSACore->DebugTrace("ST finalised\n\r", 0,0,0);
}

unsigned portSHORT usBlockStateCount(GSACore const *	pGSACore,
									unsigned portLONG 	ulStartAddr,
									unsigned portLONG 	ulEndAddr,
									GSA_EXT_STATE		enState)
{
	unsigned portSHORT usCount;

	//convert state into internal representation
	switch(enState)
	{
		case GSA_EXT_STATE_FREE	:	enState = GSA_INT_BLOCK_STATE_FREE;
									break;
		case GSA_EXT_STATE_DEAD	:	enState = GSA_INT_BLOCK_STATE_DEAD;
									break;
		case GSA_EXT_STATE_VALID:	enState = GSA_INT_BLOCK_STATE_VALID;
									break;
		default					:	break;
	}

	//go through range and look for block with specified state
	for (usCount = 0; ulStartAddr < ulEndAddr; ulStartAddr += pGSACore->BlockSize)
	{
		if (enAccessStateTable(OP_STATE_TABLE_GET, pGSACore, ulStartAddr, 0) == enState) ++usCount;
	}

	return usCount;
}

unsigned portLONG ulGetNextFreeBlock(GSACore const *	pGSACore,
									unsigned portLONG 	ulStartAddr,
									unsigned portLONG 	ulEndAddr)
{
	ulStartAddr = ulFindBlockViaState(pGSACore, ulStartAddr, ulEndAddr, GSA_INT_BLOCK_STATE_FREE);

	if (ulStartAddr != (unsigned portLONG)NULL) enAccessStateTable(OP_STATE_TABLE_SET, pGSACore, ulStartAddr, GSA_INT_BLOCK_STATE_DEAD);

	return ulStartAddr;
}

/************************************************* Operations ********************************************************/

portBASE_TYPE xGSAWrite(GSACore const *pGSACore,
						unsigned portCHAR ucAID,
						unsigned portCHAR ucDID,
						unsigned portLONG ulSize,
						portCHAR *pcData)
{
	unsigned portLONG ulBlockAddr;
	//simple write

	ulSize = (ulSize > DATA_FIELD_SIZE(pGSACore->BlockSize, HB_SML_HEADER_SIZE)) ?
						DATA_FIELD_SIZE(pGSACore->BlockSize, HB_SML_HEADER_SIZE) :
						ulSize;

	ulBlockAddr = (unsigned portLONG)pGSACore->BlockBuffer;

	((Header *)ulBlockAddr)->H 			= 1;
	((Header *)ulBlockAddr)->Terminal	= 1;
	((Header *)ulBlockAddr)->AID		= ucAID;
	((Header *)ulBlockAddr)->DID		= ucDID;
	((Header *)ulBlockAddr)->PrevHBI	= 0;
	((Header *)ulBlockAddr)->FDBI_U		= 0;

	memcpy((void *)DATA_START_ADDR(ulBlockAddr, HB_SML_HEADER_SIZE), (void *)pcData, ulSize);

	((TreeInfo *)INFO_START_ADDR(ulBlockAddr, pGSACore->BlockSize))->DataSize = ulSize;

	xBlockChecksum(OP_BLOCK_CHECK_APPLY, ulBlockAddr, pGSACore->BlockSize);

	ulBlockAddr = pGSACore->WriteBuffer();

	//pGSACore->DebugTrace("%p\n\r%512x\n\r%d\n\r", ulBlockAddr, ulBlockAddr, ulSize);

	//set branch to be valid
	if (ulBlockAddr != (unsigned portLONG)NULL)
	{
		vSetDataBranch(pGSACore, ulBlockAddr, GSA_INT_BLOCK_STATE_VALID);
	}
	else
	{
		return pdFAIL;
	}

	return pdPASS;
}

unsigned portLONG ulGSARead(GSACore const *pGSACore,
							unsigned portCHAR ucAID,
							unsigned portCHAR ucDID,
							unsigned portLONG ulOffset,
							unsigned portLONG ulSize,
							portCHAR *pucBuffer)
{
	(void)ulOffset;
	unsigned portLONG ulLastHBlockAddr;

	ulLastHBlockAddr = ulLookUpDataEntry(pGSACore, ucAID, ucDID);

	if (ulLastHBlockAddr == (unsigned portLONG)NULL) return 0;

	ulSize = ((TreeInfo *)INFO_START_ADDR(ulLastHBlockAddr, pGSACore->BlockSize))->DataSize;

	memcpy((void *)pucBuffer, (void *)DATA_START_ADDR(ulLastHBlockAddr, HB_SML_HEADER_SIZE), ulSize);

	return ulSize;
}

unsigned portLONG ulGSASize(GSACore const *pGSACore,
							unsigned portCHAR ucAID,
							unsigned portCHAR ucDID)
{
	unsigned portLONG ulLastHBlockAddr;

	ulLastHBlockAddr = ulLookUpDataEntry(pGSACore, ucAID, ucDID);

	if (ulLastHBlockAddr == (unsigned portLONG)NULL) return 0;

	return ((TreeInfo *)INFO_START_ADDR(ulLastHBlockAddr, pGSACore->BlockSize))->DataSize;
}

/************************************************* Internal Functions *************************************************/
static portBASE_TYPE xSurveyMemory(GSACore const *pGSACore)
{
	unsigned portLONG ulBlockAddr;

	//initialise state table
	memset(pGSACore->StateTable, 0, pGSACore->StateTableSize);

	//loop all blocks
	for (ulBlockAddr = pGSACore->StartAddr;
		ulBlockAddr < pGSACore->EndAddr;
		ulBlockAddr += pGSACore->BlockSize)
	{
		//check block is free if check function exists
		if (pGSACore->xIsBlockFree != NULL)
		{
			if (pGSACore->xIsBlockFree(ulBlockAddr))
			{
				//set block state as free
				enAccessStateTable(OP_STATE_TABLE_SET, pGSACore, ulBlockAddr, GSA_INT_BLOCK_STATE_FREE);
				continue;
			}
		}
		//verify block integrity
		if (xBlockChecksum(OP_BLOCK_CHECK_VERIFY, ulBlockAddr, pGSACore->BlockSize))
		{
			//set block state as Head Block
			if (((Header *)ulBlockAddr)->H)
			{
				enAccessStateTable(OP_STATE_TABLE_SET, pGSACore, ulBlockAddr, GSA_INT_BLOCK_STATE_HB);
			}
			else
			{
				enAccessStateTable(OP_STATE_TABLE_SET, pGSACore, ulBlockAddr, GSA_INT_BLOCK_STATE_VALID);
			}
		}
	}

	return pdTRUE;
}

static void vIsolateLastHeadBlock(GSACore const *pGSACore)
{
	unsigned portLONG ulLastHBlockAddr, ulHBlockAddr;

	//find all head blocks
	for (ulLastHBlockAddr = ulFindBlockViaState(pGSACore,
											pGSACore->StartAddr,
											pGSACore->EndAddr,
											GSA_INT_BLOCK_STATE_HB);
		ulLastHBlockAddr != (unsigned portLONG)NULL;
		ulLastHBlockAddr += pGSACore->BlockSize,
		ulLastHBlockAddr = ulFindBlockViaState(pGSACore,
											ulLastHBlockAddr,
											pGSACore->EndAddr,
											GSA_INT_BLOCK_STATE_HB))
	{
		//assign LHB state to HB
		enAccessStateTable(OP_STATE_TABLE_SET, pGSACore, ulLastHBlockAddr, GSA_INT_BLOCK_STATE_LHB);

		//go thorugh head block chain
		for(ulHBlockAddr = ulPrevHeadBlock(pGSACore, ulLastHBlockAddr);
			ulHBlockAddr != (unsigned portLONG)NULL;
			ulHBlockAddr = ulPrevHeadBlock(pGSACore, ulHBlockAddr))
		{
			//check validity of each data block chain
			if (!xCheckDataBranch(pGSACore, ulHBlockAddr))
			{
				enAccessStateTable(OP_STATE_TABLE_SET, pGSACore, ulLastHBlockAddr, GSA_INT_BLOCK_STATE_DEAD);
				enAccessStateTable(OP_STATE_TABLE_SET, pGSACore, ulHBlockAddr, GSA_INT_BLOCK_STATE_DEAD);
				break;
			}
			//check head block credential match current LHB
			if (((Header *)ulLastHBlockAddr)->AID != ((Header *)ulHBlockAddr)->AID ||
				((Header *)ulLastHBlockAddr)->DID != ((Header *)ulHBlockAddr)->DID)
			{
				enAccessStateTable(OP_STATE_TABLE_SET, pGSACore, ulLastHBlockAddr, GSA_INT_BLOCK_STATE_DEAD);
				break;
			}
			//check HB is previously found LHB
			if (enAccessStateTable(OP_STATE_TABLE_GET, pGSACore, ulHBlockAddr, 0) == GSA_INT_BLOCK_STATE_LHB)
			{
				enAccessStateTable(OP_STATE_TABLE_SET, pGSACore, ulLastHBlockAddr, GSA_INT_BLOCK_STATE_HB);
				break;
			}
		}
	}
}

static void vBuildMemory(GSACore const *pGSACore)
{
	unsigned portLONG ulLastHBlockAddr, ulHBlockAddr;

	//find all LHB
	for (ulLastHBlockAddr = ulFindBlockViaState(pGSACore,
											pGSACore->StartAddr,
											pGSACore->EndAddr,
											GSA_INT_BLOCK_STATE_LHB);
		ulLastHBlockAddr != (unsigned portLONG)NULL;
		ulLastHBlockAddr += pGSACore->BlockSize,
		ulLastHBlockAddr = ulFindBlockViaState(pGSACore,
											ulLastHBlockAddr,
											pGSACore->EndAddr,
											GSA_INT_BLOCK_STATE_LHB))
	{
		//go through all HB mark them valid
		for(ulHBlockAddr = ulPrevHeadBlock(pGSACore, ulLastHBlockAddr);
			ulHBlockAddr != (unsigned portLONG)NULL;
			ulHBlockAddr = ulPrevHeadBlock(pGSACore, ulHBlockAddr))
		{
			enAccessStateTable(OP_STATE_TABLE_SET, pGSACore, ulHBlockAddr, GSA_INT_BLOCK_STATE_VALID);
		}
	}

	//find remaining HB
	for (ulHBlockAddr = ulFindBlockViaState(pGSACore,
											pGSACore->StartAddr,
											pGSACore->EndAddr,
											GSA_INT_BLOCK_STATE_HB);
		ulHBlockAddr != (unsigned portLONG)NULL;
		ulHBlockAddr += pGSACore->BlockSize,
		ulHBlockAddr = ulFindBlockViaState(pGSACore,
											ulHBlockAddr,
											pGSACore->EndAddr,
											GSA_INT_BLOCK_STATE_HB))
	{
		// mark them dead
		enAccessStateTable(OP_STATE_TABLE_SET, pGSACore, ulHBlockAddr, GSA_INT_BLOCK_STATE_DEAD);
		//remove connected data branch
		vSetDataBranch(pGSACore, ulHBlockAddr, GSA_INT_BLOCK_STATE_DEAD);
	}
}

static void vFinaliseStateTable(GSACore const *pGSACore)
{
	unsigned portLONG ulBlockAddr;

	//if xIsBlockFree function is not define convert all dead block to free
	for (ulBlockAddr = ulFindBlockViaState(pGSACore,
											pGSACore->StartAddr,
											pGSACore->EndAddr,
											GSA_INT_BLOCK_STATE_DEAD);
		ulBlockAddr != (unsigned portLONG)NULL && pGSACore->xIsBlockFree == NULL;
		ulBlockAddr += pGSACore->BlockSize,
		ulBlockAddr = ulFindBlockViaState(pGSACore,
											ulBlockAddr,
											pGSACore->EndAddr,
											GSA_INT_BLOCK_STATE_DEAD))
	{
		enAccessStateTable(OP_STATE_TABLE_SET, pGSACore, ulBlockAddr, GSA_INT_BLOCK_STATE_FREE);
	}
}

static portBASE_TYPE xCheckDataBranch(GSACore const *pGSACore,
									unsigned portLONG ulBlockAddr)
{
	for (; ulBlockAddr != (unsigned portLONG)NULL;
		ulBlockAddr = ulNextDataBlock(pGSACore, ulBlockAddr))
	{
		if (enAccessStateTable(OP_STATE_TABLE_GET, pGSACore, ulBlockAddr, 0) == GSA_INT_BLOCK_STATE_DEAD) return pdFAIL;
	}

	return pdPASS;
}

static void vSetDataBranch(GSACore const *pGSACore,
							unsigned portLONG ulBlockAddr,
							GSA_INT_STATE enState)
{
	for (; ulBlockAddr != (unsigned portLONG)NULL;
		ulBlockAddr = ulNextDataBlock(pGSACore, ulBlockAddr))
	{
		enAccessStateTable(OP_STATE_TABLE_SET, pGSACore, ulBlockAddr, enState);
	}
}

static portBASE_TYPE xBlockChecksum(BlockCheckOp enOperation,
									unsigned portLONG ulBlockAddr,
									GSA_BLOCK_SIZE BlockSize)
{
	unsigned portLONG ulDataSum;

	//set checksum field to 0 if applying checksum
	if (enOperation == OP_BLOCK_CHECK_APPLY) ((Header *)ulBlockAddr)->Checksum = 0;

	//add header data value together
	ulDataSum = ulAddToSum(0, ulBlockAddr, sizeof(Header) / sizeof(short));

	//add tree info data value together
	ulDataSum = ulAddToSum(ulDataSum,
							ulBlockAddr + BlockSize - sizeof(TreeInfo),
							sizeof(TreeInfo) / sizeof(short));

	//return verification result if verifying
	if (enOperation == OP_BLOCK_CHECK_VERIFY) return xVerifyChecksum(ulDataSum);

	//assign checksum value
	((Header *)ulBlockAddr)->Checksum = usGenerateChecksum(ulDataSum);

	//return don't care value as pdPASS
	return pdPASS;
}

static GSA_INT_STATE enAccessStateTable(StateTableOp enOperation,
										GSACore const *pGSACore,
										unsigned portLONG ulBlockAddr,
										GSA_INT_STATE enState)
{
	static const unsigned portCHAR STATE_MASK = 0x3;
	unsigned portSHORT usStateTableIndex;
	unsigned portCHAR ucStateShiftFactor;

	//convert address to block index space
	usStateTableIndex = usAddrToIndex(pGSACore, ulBlockAddr);
	//calculate shift factor
	ucStateShiftFactor = (usStateTableIndex % NUM_STATE_PER_BYTE) * STATE_SIZE_BIT;
	//calculate state table index
	usStateTableIndex /= NUM_STATE_PER_BYTE;

	if (enOperation == OP_STATE_TABLE_SET)
	{
		pGSACore->StateTable[usStateTableIndex] &= ~(STATE_MASK << ucStateShiftFactor);
		pGSACore->StateTable[usStateTableIndex] |= enState << ucStateShiftFactor;
	}
	else if (enOperation == OP_STATE_TABLE_GET)
	{
		return ((pGSACore->StateTable[usStateTableIndex] >> ucStateShiftFactor) & STATE_MASK);
	}

	//return don't care or unknown as dead
	return GSA_INT_BLOCK_STATE_DEAD;
}

static unsigned portLONG ulFindBlockViaState(GSACore const *	pGSACore,
											unsigned portLONG 	ulStartAddr,
											unsigned portLONG 	ulEndAddr,
											GSA_INT_STATE 		enState)
{
	//go through range and look for block with free state
	for (; ulStartAddr < ulEndAddr; ulStartAddr += pGSACore->BlockSize)
	{
		if (enAccessStateTable(OP_STATE_TABLE_GET, pGSACore, ulStartAddr, 0) == enState) return ulStartAddr;
	}

	return (unsigned portLONG)NULL;
}

static unsigned portLONG ulPrevHeadBlock(GSACore const *pGSACore,
										unsigned portLONG ulBlockAddr)
{
	if (((Header *)ulBlockAddr)->H == 0 || ((Header *)ulBlockAddr)->Terminal) return (unsigned portLONG)NULL;

	return ulIndexToAddr(pGSACore, ((Header *)ulBlockAddr)->PrevHBI);
}

static unsigned portLONG ulNextDataBlock(GSACore const *pGSACore,
										unsigned portLONG ulBlockAddr)
{
	if (((Header *)ulBlockAddr)->H)
	{
		if (((Header *)ulBlockAddr)->FDBI_U)
			return ulIndexToAddr(pGSACore, ((Header *)ulBlockAddr)->FDBI);
	}
	else
	{
		if (ulIndexToAddr(pGSACore, ((Header *)ulBlockAddr)->NDBI) != ulBlockAddr)
			return ulIndexToAddr(pGSACore, ((Header *)ulBlockAddr)->NDBI);
	}

	return (unsigned portLONG)NULL;
}

static unsigned portSHORT usAddrToIndex(GSACore const *pGSACore,
										unsigned portLONG ulAddr)
{
	return ((ulAddr - pGSACore->StartAddr) / pGSACore->BlockSize);
}

static unsigned portLONG ulIndexToAddr(GSACore const *pGSACore,
										unsigned portSHORT usBlockIndex)
{
	return (usBlockIndex * pGSACore->BlockSize + pGSACore->StartAddr);
}

static unsigned portLONG ulLookUpDataEntry(GSACore const *pGSACore,
											unsigned portCHAR ucAID,
											unsigned portCHAR ucDID)
{
	unsigned portLONG ulLastHBlockAddr;

	//TODO implement optimisation via data table

	for (ulLastHBlockAddr = ulFindBlockViaState(pGSACore,
											pGSACore->StartAddr,
											pGSACore->EndAddr,
											GSA_INT_BLOCK_STATE_LHB);
		ulLastHBlockAddr != (unsigned portLONG)NULL;
		ulLastHBlockAddr += pGSACore->BlockSize,
		ulLastHBlockAddr = ulFindBlockViaState(pGSACore,
											ulLastHBlockAddr,
											pGSACore->EndAddr,
											GSA_INT_BLOCK_STATE_LHB))
	{
		if (((Header *)ulLastHBlockAddr)->AID == ucAID &&
			((Header *)ulLastHBlockAddr)->DID == ucDID) break;
	}

	return ulLastHBlockAddr;
}

/******************************************** Data Table Optimisation ***********************************************/

static Data_Table_Entry *pFindDataTableEntry(GSACore const *pGSACore,
											unsigned portCHAR ucAID,
											unsigned portCHAR ucDID)
{
	(void) pGSACore;
	(void) ucAID;
	(void) ucDID;

	return NULL;
}

static portBASE_TYPE xAddDataTableEntry(GSACore const *pGSACore,
										unsigned portCHAR ucAID,
										unsigned portCHAR ucDID,
										unsigned portSHORT usLastHBI,
										unsigned portLONG ulSize)
{
	(void) pGSACore;
	(void) ucAID;
	(void) ucDID;
	(void) usLastHBI;
	(void) ulSize;

	return pdPASS;
}

static void vRemoveDataTableEntry(GSACore const *pGSACore,
							Data_Table_Entry *pDataTableEntry)
{
	(void) pGSACore;
	(void) pDataTableEntry;
	;
}
