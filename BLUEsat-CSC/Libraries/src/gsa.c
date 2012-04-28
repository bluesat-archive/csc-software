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

typedef union Seg_Headers
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
} *Header;

#define HB_HEADER_SIZE 		sizeof(union Seg_Headers)
#define HB_SML_HEADER_SIZE 	HB_HEADER_SIZE - 2
#define DB_HEADER_SIZE 		HB_HEADER_SIZE - 4

typedef struct
{
	unsigned portLONG	DataSize;
} Data_Info;

typedef enum
{
	STATE_USED_DELETED	= 0,
	STATE_USED_DATA		= 1,
	STATE_USED_HEAD		= 2,
	STATE_FREE			= 3
} MEM_SEG_STATE;

//set given address its state in state table
void vAssignState(GSACore *pGSACore,
				unsigned portLONG ulAddr,
				MEM_SEG_STATE enState);

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
		if (xVerifyBlock(ulStartAddr, pGSACore->MemSegSize, CHECKSUM_HEADER))
		{
			enTmpState = STATE_USED_DATA;

			if (((Header)ulStartAddr)->H) enTmpState = STATE_USED_HEAD;

			continue;
		}

		enTmpState = STATE_FREE;

		if (pGSACore->xIsMemSegFree != NULL)
		{
			if(!pGSACore->xIsMemSegFree(ulStartAddr)) enTmpState = STATE_USED_DELETED;
		}
	}
}

void vAssignState(GSACore *pGSACore,
				unsigned portLONG ulAddr,
				MEM_SEG_STATE enState)
{
	unsigned portCHAR ucClearMask;
	unsigned portCHAR ucShiftFactor;

	//convert address to base address space
	ulAddr -= pGSACore->StartAddr;

	//calculate position shift factor
	ucShiftFactor = (ulAddr % NUM_STATES_PER_BYTE)*STATE_SIZE_BIT;

	ucClearMask = ~(STATE_FREE << ucShiftFactor);

	ucClearMask &= pGSACore->StateTable[ulAddr / NUM_STATES_PER_BYTE];

	pGSACore->StateTable[ulAddr / NUM_STATES_PER_BYTE] = ucClearMask | (enState << ucShiftFactor);
}

#define DEFAULT_VALID_CHECKSUM	0x0000ffff
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
		((Header)ulAddr)->Checksum = 0;

		ulDataSum = ulAddToSum(0, ulAddr, HB_HEADER_SIZE / sizeof(unsigned portSHORT));

		ulDataSum = ulAddToSum(ulDataSum, ulAddr + enMemSegSize - sizeof(Data_Info), sizeof(Data_Info) / sizeof(unsigned portSHORT));

		((Header)ulAddr)->Checksum = usGenerateChecksum(ulDataSum);
	}
}






