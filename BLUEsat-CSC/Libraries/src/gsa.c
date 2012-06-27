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
} Tree_Info;

/*************************************** Start Internal Function Prototypes  ****************************************/
typedef enum
{
	GSA_INT_BLOCK_STATE_DEAD	= 0,	//DEAD must == 0, else xSurveyMemory won't work properly
	GSA_INT_BLOCK_STATE_HB 		= 1,
	GSA_INT_BLOCK_STATE_VALID	= 1,
	GSA_INT_BLOCK_STATE_LHB		= 2,
	GSA_INT_BLOCK_STATE_FREE	= 3
} GSA_INT_STATE;

//map out all block in memory and assign state
static portBASE_TYPE xSurveyMemory(GSACore const *pGSACore);

typedef enum
{
	OP_BLOCK_CHECK_VERIFY,
	OP_BLOCK_CHECK_APPLY
} BlockCheckOp;

static portBASE_TYPE xBlockChecksum(BlockCheckOp enOperation,
									unsigned portLONG ulBlockAddr,
									GSA_BLOCK_SIZE BlockSize);

typedef enum
{
	OP_STATE_TABLE_SET,
	OP_STATE_TABLE_GET
} StateTableOp;

static GSA_INT_STATE enAccessStateTable(StateTableOp enOperation,
										GSACore const *pGSACore,
										unsigned portLONG ulBlockAddr,
										GSA_INT_STATE enState);

static unsigned portSHORT usAddrToIndex(GSACore const *pGSACore,
										unsigned portLONG ulAddr);

//convert block index to address
static unsigned portLONG ulIndexToAddr(GSACore const *pGSACore,
										unsigned portSHORT usBlockIndex);


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
	//go through range and look for block with free state
	for (; ulStartAddr < ulEndAddr; ulStartAddr += pGSACore->BlockSize)
	{
		if (enAccessStateTable(OP_STATE_TABLE_GET, pGSACore, ulStartAddr, 0) == GSA_INT_BLOCK_STATE_FREE) return ulStartAddr;
	}

	return (unsigned portLONG)NULL;
}

/************************************************* Operations ********************************************************/

portBASE_TYPE xGSAWrite(GSACore const *pGSACore,
						unsigned portCHAR ucAID,
						unsigned portCHAR ucDID,
						unsigned portLONG ulSize,
						portCHAR *pcData)
{
	(void) pGSACore;
	(void) ucAID;
	(void) ucDID;
	(void) ulSize;
	(void) pcData;

	return pdTRUE;
}

unsigned portLONG ulGSARead(GSACore const *pGSACore,
							unsigned portCHAR ucAID,
							unsigned portCHAR ucDID,
							unsigned portLONG ulOffset,
							unsigned portLONG ulSize,
							portCHAR *pucBuffer)
{
	(void) pGSACore;
	(void) ucAID;
	(void) ucDID;
	(void) ulSize;
	(void) ulOffset;
	(void) pucBuffer;

	return 0;
}

unsigned portLONG ulGSASize(GSACore const *pGSACore,
							unsigned portCHAR ucAID,
							unsigned portCHAR ucDID)
{
	(void) pGSACore;
	(void) ucAID;
	(void) ucDID;

	return 0;
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
		if (xBlockChecksum(OP_BLOCK_CHECK_VERIFY, ulBlockAddr, pGSACore->BlockSize) && ((Header *)ulBlockAddr)->H)
		{
			//set block state as Head Block
			enAccessStateTable(OP_STATE_TABLE_SET, pGSACore, ulBlockAddr, GSA_INT_BLOCK_STATE_HB);
		}
	}

	pGSACore->DebugTrace("Memory surveyed\n\r", 0,0,0);

	return pdTRUE;
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
	ulDataSum = ulAddToSum(ulDataSum, ulBlockAddr + BlockSize - sizeof(Tree_Info), sizeof(Tree_Info) / sizeof(short));

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

static unsigned portSHORT usAddrToIndex(GSACore const *pGSACore, unsigned portLONG ulAddr)
{
	return ((ulAddr - pGSACore->StartAddr) / pGSACore->BlockSize);
}

static unsigned portLONG ulIndexToAddr(GSACore const *pGSACore, unsigned portSHORT usBlockIndex)
{
	return (usBlockIndex * pGSACore->BlockSize + pGSACore->StartAddr);
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
