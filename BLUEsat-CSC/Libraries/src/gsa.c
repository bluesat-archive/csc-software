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
#define HB_SML_HEADER_SIZE 	HB_HEADER_SIZE - 2
#define DB_HEADER_SIZE 		HB_HEADER_SIZE - 4

typedef struct
{
	unsigned portLONG	DataSize;
} Data_Info;

//set given address its state in state table
void vAssignState(GSACore *pGSACore,
				unsigned portLONG ulAddr,
				MEM_SEG_STATE enState);

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
	CHECKSUM_HEADER,
	CHECKSUM_DATA
} CHECKSUM_TYPE;

//verify checksum for given data is valid
portBASE_TYPE xVerifyBlock(unsigned portLONG	ulAddr,
							MEM_SEG_SIZE enMemSegSize,
							CHECKSUM_TYPE enType);

//assign checksum for given data
void vAssignChecksum(unsigned portLONG	ulAddr,
					MEM_SEG_SIZE enMemSegSize,
					CHECKSUM_TYPE enType);

//initialise GSACore
void vInitialiseCore(GSACore *pGSACore)
{
	//initialise data table
	pGSACore->DataTableIndex = 0;

	//initialise state table
	memset(pGSACore->StateTable, 0, pGSACore->StateTableSize);
}

//map out memory segments and assign state
void vSurveyMemory(GSACore *pGSACore,
					unsigned portLONG ulStartAddr,
					unsigned portLONG ulEndAddr)
{
	MEM_SEG_STATE enTmpState;

	for (;ulStartAddr < ulEndAddr;
		vAssignState(pGSACore, ulStartAddr, enTmpState), ulStartAddr += pGSACore->MemSegSize)
	{
		enTmpState = STATE_USED_DELETED;

		if (pGSACore->xIsMemSegFree != NULL && pGSACore->xIsMemSegFree(ulStartAddr))
		{
			enTmpState = STATE_FREE;
			continue;
		}

		if (xVerifyBlock(ulStartAddr, pGSACore->MemSegSize, CHECKSUM_HEADER))
		{
			enTmpState = STATE_USED_DATA;

			if (((Header *)ulStartAddr)->H) enTmpState = STATE_USED_HEAD;
		}
	}
}

unsigned portLONG ulFindNextFreeState(GSACore *pGSACore,
									unsigned portLONG ulStartAddr,
									unsigned portLONG ulEndAddr)
{
	return ulUseStateTable(pGSACore, ulStartAddr, ulEndAddr, STATE_FREE, OP_FIND_NEXT);
}

unsigned portSHORT usCountState(GSACore *pGSACore,
								unsigned portLONG ulStartAddr,
								unsigned portLONG ulEndAddr,
								MEM_SEG_STATE enState)
{
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
	tmpDataInfo.DataSize 	= ulSize;

	if (pGSACore->WriteToMemSeg != NULL)
	{
		strncpy((portCHAR *)pGSACore->MemSegBuffer, (portCHAR *)&tmpHeader, HB_SML_HEADER_SIZE);

		//currently assuming ulsize is << MemSegSize
		strncpy((portCHAR *)(unsigned portLONG)pGSACore->MemSegBuffer + HB_SML_HEADER_SIZE, pcData, ulSize);

		strncpy((portCHAR *)(unsigned portLONG)pGSACore->MemSegBuffer + pGSACore->MemSegSize - sizeof(Data_Info), (portCHAR *)&tmpDataInfo, sizeof(Data_Info));

		vAssignChecksum((unsigned portLONG)pGSACore->MemSegBuffer, pGSACore->MemSegSize, CHECKSUM_HEADER);

		return pGSACore->WriteToMemSeg(ulAddr);
	}

	return pdTRUE;
}

/************************************************* Internal Functions *************************************************/

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
	ucClearMask = ~(STATE_FREE << ucShiftFactor);

	//clear state from state table
	ucClearMask &= pGSACore->StateTable[usStateTableIndex];

	//assign state to state table
	pGSACore->StateTable[usStateTableIndex] = ucClearMask | (enState << ucShiftFactor);
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

		if ((pGSACore->StateTable[usStateTableIndex] & (STATE_FREE << ucShiftFactor)) == (enState << ucShiftFactor))
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


