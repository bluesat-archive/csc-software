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
} Data_Info;

typedef struct
{
	unsigned portLONG AID:		6;
	unsigned portLONG DID:		8;
	unsigned portLONG LastHBI:	15;
	unsigned portLONG Upadding:	3;	//usable padding
	unsigned portLONG Size;
} Data_Table_Entry;

#define MAX_BLOCKS	(1 << 16)

/******************************************** Start Function Prototypes  *********************************************/

//set given address its state in state table
void vAssignState(GSACore *pGSACore,
				unsigned portLONG ulAddr,
				MEM_SEG_STATE enState);

//get memory segment state
MEM_SEG_STATE enGetState(GSACore *pGSACore,
						unsigned portLONG ulAddr);

typedef enum
{
	OP_COUNT,
	OP_FIND_NEXT
} STATE_TABLE_OP;

//use state table to complete specified operation
unsigned portLONG ulUseStateTable(GSACore *pGSACore,
								unsigned portLONG ulStartAddr,
								unsigned portLONG ulEndAddr,
								MEM_SEG_STATE enState,
								STATE_TABLE_OP enOperation);

typedef enum
{
	TREE_DELETE,
	TREE_VERIFY,
	TREE_SET_ACTIVE
} ARCHITECTURE_OP;

//complete specified operation on architecture
unsigned portLONG ulUseArchitecture(GSACore *pGSACore,
									unsigned portLONG ulHBAddr,
									unsigned portSHORT usStopHBlock,
									ARCHITECTURE_OP enOperation);


typedef enum
{
	CHECKSUM_HEADER,
	CHECKSUM_DATA
} CHECKSUM_TYPE;

//verify checksum for given data is valid
portBASE_TYPE xVerifyBlock(unsigned portLONG ulAddr,
							MEM_SEG_SIZE enMemSegSize,
							CHECKSUM_TYPE enType);

//assign checksum to memory segment
void vAssignChecksum(unsigned portLONG ulAddr,
					MEM_SEG_SIZE enMemSegSize,
					CHECKSUM_TYPE enType);

//find corresponding data table entry
Data_Table_Entry *pFindDataTableEntry(GSACore *pGSACore,
									unsigned portCHAR ucAID,
									unsigned portCHAR ucDID);

//add new data table entry
portBASE_TYPE xAddDataTableEntry(GSACore *pGSACore,
								unsigned portCHAR ucAID,
								unsigned portCHAR ucDID,
								unsigned portSHORT usLastHBI,
								unsigned portLONG ulSize);

//remove data table entry
void vRemoveDataTableEntry(GSACore *pGSACore,
							Data_Table_Entry *pDataTableEntry);

//convert address to block index
unsigned portSHORT usAddrToIndex(GSACore *pGSACore, unsigned portLONG ulAddr);

//convert block index to address
unsigned portLONG ulIndexToAddr(GSACore *pGSACore, unsigned portSHORT usBlockIndex);

//synchronise state table with data table
void vFinaliseGSACore(GSACore *pGSACore);

//check whether given address is in service range
portBASE_TYPE xCheckAddrInRange(GSACore *pGSACore, unsigned portLONG ulAddr);

/***************************************** Start Function Implementations *****************************************/

//initialise GSACore
void vInitialiseCore(GSACore *pGSACore)
{
	//initialise data table
	pGSACore->DataTableIndex = 0;

	//initialise state table
	memset(pGSACore->StateTable, 0, pGSACore->StateTableSize);
}

//map out memory segments and assign state
portBASE_TYPE xSurveyMemory(GSACore *pGSACore,
					unsigned portLONG ulStartAddr,
					unsigned portLONG ulEndAddr)
{
	MEM_SEG_STATE enTmpState;

	if (ulStartAddr < pGSACore->StartAddr || ulEndAddr < ulStartAddr) return pdFAIL;

	if (((ulEndAddr - ulStartAddr) / pGSACore->MemSegSize) / NUM_STATES_PER_BYTE > pGSACore->StateTableSize) return pdFAIL;

	for (;ulStartAddr < ulEndAddr;
		vAssignState(pGSACore, ulStartAddr, enTmpState), ulStartAddr += pGSACore->MemSegSize)
	{
		enTmpState = STATE_DELETED;

		if ((pGSACore->xIsMemSegFree != NULL) ? pGSACore->xIsMemSegFree(ulStartAddr) : pdTRUE)
		{
			enTmpState = STATE_FREE;
			continue;
		}

		if (((Header *)ulStartAddr)->H && xVerifyBlock(ulStartAddr, pGSACore->MemSegSize, CHECKSUM_HEADER)) enTmpState = STATE_DATA;
	}

	return pdTRUE;
}

portBASE_TYPE xBuildDataTable(GSACore *pGSACore,
					unsigned portLONG ulStartAddr,
					unsigned portLONG ulEndAddr,
					unsigned portCHAR ucIsolateBuild,
					unsigned portCHAR ucAID)
{
	unsigned portLONG ulHeadBlockAddr;
	Data_Table_Entry *pDataTableEntry;
	unsigned portSHORT usBlockIndex;

	if (ulStartAddr < pGSACore->StartAddr || ulEndAddr < ulStartAddr) return pdFAIL;

	if (((ulEndAddr - ulStartAddr) / pGSACore->MemSegSize) / NUM_STATES_PER_BYTE > pGSACore->StateTableSize) return pdFAIL;

	for(ulHeadBlockAddr = ulStartAddr;
		(ulHeadBlockAddr = ulUseStateTable(pGSACore, ulHeadBlockAddr, ulEndAddr, STATE_DATA, OP_FIND_NEXT))
						!= (unsigned portLONG)NULL;
		ulHeadBlockAddr += pGSACore->MemSegSize)
	{
		if (ucIsolateBuild == pdTRUE && ((Header *)ulHeadBlockAddr)->AID != ucAID) continue;

		usBlockIndex = usAddrToIndex(pGSACore, ulHeadBlockAddr);

		pDataTableEntry = pFindDataTableEntry(pGSACore, ((Header *)ulHeadBlockAddr)->AID, ((Header *)ulHeadBlockAddr)->DID);


pGSACore->DebugTrace("Test: %d %d %h\n\r", ((Header *)ulHeadBlockAddr)->AID, ((Header *)ulHeadBlockAddr)->DID, ulHeadBlockAddr);
		if (ulUseArchitecture(pGSACore,
							ulHeadBlockAddr,
							(pDataTableEntry == NULL) ? usBlockIndex : pDataTableEntry->LastHBI,
							TREE_VERIFY) == pdFAIL)
		{
			vAssignState(pGSACore, ulHeadBlockAddr, (pGSACore->xIsMemSegFree == NULL) ? STATE_FREE : STATE_DELETED);

			continue;
		}
pGSACore->DebugTrace("Build: %d %d %h\n\r", ((Header *)ulHeadBlockAddr)->AID, ((Header *)ulHeadBlockAddr)->DID, ulHeadBlockAddr);
		if (pDataTableEntry == NULL)
		{
			xAddDataTableEntry(pGSACore,
								((Header *)ulHeadBlockAddr)->AID,
								((Header *)ulHeadBlockAddr)->DID,
								usBlockIndex,
								0);
		}
		else
		{
			pDataTableEntry->LastHBI = usBlockIndex;
		}
	}

	vFinaliseGSACore(pGSACore);

	return pdTRUE;
}

unsigned portLONG ulFindNextFreeState(GSACore *pGSACore,
									unsigned portLONG ulStartAddr,
									unsigned portLONG ulEndAddr)
{
	if (ulStartAddr < pGSACore->StartAddr || ulEndAddr < ulStartAddr)
				return (unsigned portLONG) NULL;

	if (((ulEndAddr - ulStartAddr) / pGSACore->MemSegSize) / NUM_STATES_PER_BYTE > pGSACore->StateTableSize)
				return (unsigned portLONG) NULL;

	return ulUseStateTable(pGSACore, ulStartAddr, ulEndAddr, STATE_FREE, OP_FIND_NEXT);
}

unsigned portSHORT usCountState(GSACore *pGSACore,
								unsigned portLONG ulStartAddr,
								unsigned portLONG ulEndAddr,
								MEM_SEG_STATE enState)
{
	if (ulStartAddr < pGSACore->StartAddr || ulEndAddr < ulStartAddr) return 0;

	if (((ulEndAddr - ulStartAddr) / pGSACore->MemSegSize) / NUM_STATES_PER_BYTE > pGSACore->StateTableSize) return 0;

	return ulUseStateTable(pGSACore, ulStartAddr, ulEndAddr, enState, OP_COUNT);
}

/************************************************* Operations ********************************************************/

portBASE_TYPE xGSAWrite(GSACore *pGSACore,
						unsigned portCHAR ucAID,
						unsigned portCHAR ucDID,
						unsigned portLONG ulSize,
						portCHAR *pcData)
{
	Header 		tmpHeader;
	Data_Info	tmpDataInfo;
	unsigned portLONG ulAddr = pGSACore->GetNextMemSeg();

pGSACore->DebugTrace("NextAddr: %h\n\r", ulAddr, 0, 0);

	if (ulAddr == (unsigned portLONG)NULL) return pdFALSE;

	tmpHeader.Terminal 		= pdTRUE;
	tmpHeader.H 			= pdTRUE;
	tmpHeader.AID 			= ucAID;
	tmpHeader.DID 			= ucDID;
	tmpHeader.FDBI_U 		= pdFALSE;
	tmpDataInfo.DataSize 	= ulSize;

	if (pGSACore->WriteToMemSeg != NULL)
	{
		memcpy((portCHAR *)pGSACore->MemSegBuffer, (portCHAR *)&tmpHeader, HB_SML_HEADER_SIZE);
		//currently assuming ulsize is << MemSegSize
		memcpy((portCHAR *)(unsigned portLONG)pGSACore->MemSegBuffer + HB_SML_HEADER_SIZE, pcData, ulSize);

		memcpy((portCHAR *)(unsigned portLONG)pGSACore->MemSegBuffer + pGSACore->MemSegSize - sizeof(Data_Info), (portCHAR *)&tmpDataInfo, sizeof(Data_Info));

		vAssignChecksum((unsigned portLONG)pGSACore->MemSegBuffer, pGSACore->MemSegSize, CHECKSUM_HEADER);

		return pGSACore->WriteToMemSeg(ulAddr);
	}

	return pdTRUE;
}

unsigned portLONG ulGSASize(GSACore *pGSACore,
							unsigned portCHAR ucAID,
							unsigned portCHAR ucDID)
{
	Data_Table_Entry *pDataTableEntry;

	pDataTableEntry = pFindDataTableEntry(pGSACore, ucAID, ucDID);

	if (pDataTableEntry != NULL) return pDataTableEntry->Size; else return 0;
}

/************************************************* Internal Functions *************************************************/

#define STATE_MASK_BIT	3	//11

void vAssignState(GSACore *pGSACore,
				unsigned portLONG ulAddr,
				MEM_SEG_STATE enState)
{
	unsigned portCHAR ucClearMask;
	unsigned portCHAR ucShiftFactor;
	unsigned portSHORT usStateTableIndex;

	usStateTableIndex 	= ((ulAddr - pGSACore->StartAddr) / pGSACore->MemSegSize);

	//calculate position shift factor
	ucShiftFactor 		= (usStateTableIndex % NUM_STATES_PER_BYTE)*STATE_SIZE_BIT;

	//state table index
	usStateTableIndex 	/= NUM_STATES_PER_BYTE;

	//create clear mask
	ucClearMask = ~(STATE_MASK_BIT << ucShiftFactor);

	//clear state from state table
	ucClearMask &= pGSACore->StateTable[usStateTableIndex];

	//assign state to state table
	pGSACore->StateTable[usStateTableIndex] = ucClearMask | (enState << ucShiftFactor);
}

MEM_SEG_STATE enGetState(GSACore *pGSACore,
						unsigned portLONG ulAddr)
{
	unsigned portCHAR ucShiftFactor;
	unsigned portSHORT usStateTableIndex;

	usStateTableIndex 	= ((ulAddr - pGSACore->StartAddr) / pGSACore->MemSegSize);

	//calculate position shift factor
	ucShiftFactor 		= (usStateTableIndex % NUM_STATES_PER_BYTE)*STATE_SIZE_BIT;

	//state table index
	usStateTableIndex 	/= NUM_STATES_PER_BYTE;

	return ((pGSACore->StateTable[usStateTableIndex] >> ucShiftFactor) & 0xf);
}

unsigned portLONG ulUseStateTable(GSACore *pGSACore,
								unsigned portLONG ulStartAddr,
								unsigned portLONG ulEndAddr,
								MEM_SEG_STATE enState,
								STATE_TABLE_OP enOperation)
{
	unsigned portSHORT usStateTableIndex;
	unsigned portCHAR  ucShiftFactor;
	unsigned portSHORT usNumSegInRange;
	unsigned portSHORT usIndex;
	unsigned portSHORT usCount;

	usStateTableIndex 	= ((ulStartAddr - pGSACore->StartAddr) / pGSACore->MemSegSize);
	ucShiftFactor 		= (usStateTableIndex % NUM_STATES_PER_BYTE)*STATE_SIZE_BIT;
	usStateTableIndex 	/= NUM_STATES_PER_BYTE;
	usNumSegInRange 	= (ulEndAddr - ulStartAddr) / pGSACore->MemSegSize;

	for (usIndex = 0, usCount = 0; usIndex < usNumSegInRange; ++usIndex, ucShiftFactor += STATE_SIZE_BIT)
	{
		if (ucShiftFactor == NUM_STATES_PER_BYTE*STATE_SIZE_BIT)
		{
			ucShiftFactor = 0;
			++usStateTableIndex;
		}

		if ((pGSACore->StateTable[usStateTableIndex] & (STATE_MASK_BIT << ucShiftFactor)) == (enState << ucShiftFactor))
		{
			if (enOperation == OP_FIND_NEXT) return (usIndex*pGSACore->MemSegSize + ulStartAddr);

			++usCount;
		}
	}

	if(enOperation == OP_COUNT)
	{
		return usCount;
	}
	else
	{
		return (unsigned portLONG)NULL;
	}
}

unsigned portLONG ulUseArchitecture(GSACore *pGSACore,
									unsigned portLONG ulHBAddr,
									unsigned portSHORT usStopHBlock,
									ARCHITECTURE_OP enOperation)
{
//	typedef enum
//	{
//		TREE_DELETE,
//		TREE_VERIFY,
//		TREE_SET_ACTIVE
//	} ARCHITECTURE_OP;
	unsigned portLONG ulBlockCount = 0;
	unsigned portSHORT usHBlockIndex;
	unsigned portSHORT usDBlockIndex;
	unsigned portLONG ulDBAddr;
	unsigned portLONG ulSize = 0;
	unsigned portCHAR ucAID, ucDID;

	usHBlockIndex = usAddrToIndex(pGSACore, ulHBAddr);
	ucAID = ((Header *)ulHBAddr)->AID;
	ucDID = ((Header *)ulHBAddr)->DID;

	do
	{
		if (((Header *)ulHBAddr)->FDBI_U)
		{
			for(ulDBAddr = ulIndexToAddr(pGSACore, ((Header *)ulHBAddr)->FDBI);
				++ulBlockCount < MAX_BLOCKS;
				ulDBAddr = ulIndexToAddr(pGSACore, usDBlockIndex))
			{
				if (!xCheckAddrInRange(pGSACore, ulDBAddr)) return pdFALSE;

				if (enOperation == TREE_VERIFY)
				{
					if (enGetState(pGSACore, ulDBAddr) == STATE_FREE) return pdFALSE;
					if (!xVerifyBlock(ulDBAddr, pGSACore->MemSegSize, CHECKSUM_HEADER)) return pdFALSE;
				}
				else if (enOperation == TREE_SET_ACTIVE)
				{
					vAssignState(pGSACore, ulDBAddr, STATE_DATA);
				}

				usDBlockIndex = ((Header *)ulDBAddr)->NDBI;
				if (usDBlockIndex == usHBlockIndex) break;
				break;
			}
		}

		if (++ulBlockCount >= MAX_BLOCKS) return pdFALSE;

		if (enOperation == TREE_SET_ACTIVE)
		{
			ulSize += ((Data_Info *)(ulHBAddr + pGSACore->MemSegSize - sizeof(Data_Info)))->DataSize;
		}

		if (((Header *)ulHBAddr)->Terminal) break;

		usHBlockIndex = ((Header *)ulHBAddr)->PrevHBI;

		ulHBAddr = ulIndexToAddr(pGSACore, usHBlockIndex);

		if (enOperation == TREE_VERIFY)
		{
			if (!xCheckAddrInRange(pGSACore, ulHBAddr)) return pdFALSE;

			if (enGetState(pGSACore, ulHBAddr) != STATE_DATA
					|| (ucAID != ((Header *)ulHBAddr)->AID
							|| ucDID != ((Header *)ulHBAddr)->DID)) return pdFALSE;
		}
	}
	while (usHBlockIndex != usStopHBlock);

	if (enOperation == TREE_SET_ACTIVE)
	{
		return ulSize;
	}
	else
	{
		return pdTRUE;
	}
}

portBASE_TYPE xVerifyBlock(unsigned portLONG ulAddr,
							MEM_SEG_SIZE enMemSegSize,
							CHECKSUM_TYPE enType)
{
	unsigned portLONG ulDataSum = DEFAULT_VALID_CHECKSUM;

	if (enType == CHECKSUM_HEADER)
	{
		ulDataSum = ulAddToSum(0, ulAddr, HB_HEADER_SIZE / sizeof(unsigned portSHORT));

		ulDataSum = ulAddToSum(ulDataSum, ulAddr + enMemSegSize - sizeof(Data_Info), sizeof(Data_Info) / sizeof(unsigned portSHORT));
	}

	return xVerifyChecksum(ulDataSum);
}

void vAssignChecksum(unsigned portLONG ulAddr,
					MEM_SEG_SIZE enMemSegSize,
					CHECKSUM_TYPE enType)
{
	unsigned portLONG ulDataSum;

	if (enType == CHECKSUM_HEADER)
	{
		((Header *)ulAddr)->Checksum = 0;

		ulDataSum = ulAddToSum(0, ulAddr, HB_HEADER_SIZE / sizeof(unsigned portSHORT));

		ulDataSum = ulAddToSum(ulDataSum, ulAddr + enMemSegSize - sizeof(Data_Info), sizeof(Data_Info) / sizeof(unsigned portSHORT));

		((Header *)ulAddr)->Checksum = usGenerateChecksum(ulDataSum);
	}
}

Data_Table_Entry *pFindDataTableEntry(GSACore *pGSACore,
									unsigned portCHAR ucAID,
									unsigned portCHAR ucDID)
{
	Data_Table_Entry *pDataTable = (Data_Table_Entry *)pGSACore->DataTable;
	unsigned portSHORT usIndex;

	for (usIndex = 0; usIndex < pGSACore->DataTableIndex; usIndex++)
	{
		if (pDataTable[usIndex].AID == ucAID
				&& pDataTable[usIndex].DID == ucDID) return &pDataTable[usIndex];
	}

	return NULL;
}

portBASE_TYPE xAddDataTableEntry(GSACore *pGSACore,
								unsigned portCHAR ucAID,
								unsigned portCHAR ucDID,
								unsigned portSHORT usLastHBI,
								unsigned portLONG ulSize)
{
	Data_Table_Entry *pDataTable;

	if (pGSACore->DataTableIndex >= pGSACore->DataTableSize) return pdFAIL;

	pDataTable = (Data_Table_Entry *)pGSACore->DataTable;

	pDataTable[pGSACore->DataTableIndex].AID = ucAID;
	pDataTable[pGSACore->DataTableIndex].DID = ucDID;
	pDataTable[pGSACore->DataTableIndex].LastHBI = usLastHBI;
	pDataTable[pGSACore->DataTableIndex].Size = ulSize;

	++(pGSACore->DataTableIndex);

	return pdPASS;
}

void vRemoveDataTableEntry(GSACore *pGSACore,
							Data_Table_Entry *pDataTableEntry)
{
	Data_Table_Entry *pDataTable;

	if (pDataTableEntry == NULL || pGSACore->DataTableIndex == 0) return;

	pDataTable = (Data_Table_Entry *)pGSACore->DataTable;

	--(pGSACore->DataTableIndex);

	pDataTableEntry->AID 		= pDataTable[pGSACore->DataTableIndex].AID;
	pDataTableEntry->DID 		= pDataTable[pGSACore->DataTableIndex].DID;
	pDataTableEntry->LastHBI 	= pDataTable[pGSACore->DataTableIndex].LastHBI;
	pDataTableEntry->Size 		= pDataTable[pGSACore->DataTableIndex].Size;
}

unsigned portSHORT usAddrToIndex(GSACore *pGSACore, unsigned portLONG ulAddr)
{
	//convert to base address space
	ulAddr -= pGSACore->StartAddr;

	return (ulAddr / pGSACore->MemSegSize);
}

unsigned portLONG ulIndexToAddr(GSACore *pGSACore, unsigned portSHORT usBlockIndex)
{
	return (usBlockIndex * pGSACore->MemSegSize + pGSACore->StartAddr);
}

void vFinaliseGSACore(GSACore *pGSACore)
{
	Data_Table_Entry *pDataTable = (Data_Table_Entry *)pGSACore->DataTable;
	unsigned portSHORT usIndex;

	for (usIndex = 0; usIndex < pGSACore->DataTableIndex; usIndex++)
	{
		pDataTable[usIndex].Size = ulUseArchitecture(pGSACore,
													ulIndexToAddr(pGSACore, pDataTable[usIndex].LastHBI),
													pDataTable[usIndex].LastHBI,
													TREE_SET_ACTIVE);
	}
}

portBASE_TYPE xCheckAddrInRange(GSACore *pGSACore, unsigned portLONG ulAddr)
{
	if (ulAddr < pGSACore->StartAddr) return pdFAIL;
	if (ulAddr >= (pGSACore->StateTableSize * NUM_STATES_PER_BYTE * pGSACore->MemSegSize + pGSACore->StartAddr)) return pdFAIL;

	return pdTRUE;
}

