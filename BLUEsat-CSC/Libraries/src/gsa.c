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
} Data_Info;


/*************************************** Start Internal Function Prototypes  ****************************************/

static unsigned portSHORT usAddrToIndex(GSACore *pGSACore, unsigned portLONG ulAddr);

//convert block index to address
static unsigned portLONG ulIndexToAddr(GSACore *pGSACore, unsigned portSHORT usBlockIndex);


/******************************************** Data Table Optimisation ***********************************************/
typedef struct
{
	unsigned portLONG AID:		6;
	unsigned portLONG DID:		8;
	unsigned portLONG LastHBI:	15;
	unsigned portLONG Upadding:	3;	//usable padding
	unsigned portLONG Size;
} Data_Table_Entry;

//find corresponding data table entry
static Data_Table_Entry *pFindDataTableEntry(GSACore *pGSACore,
									unsigned portCHAR ucAID,
									unsigned portCHAR ucDID);

//add new data table entry
static portBASE_TYPE xAddDataTableEntry(GSACore *pGSACore,
								unsigned portCHAR ucAID,
								unsigned portCHAR ucDID,
								unsigned portSHORT usLastHBI,
								unsigned portLONG ulSize);

//remove data table entry
static void vRemoveDataTableEntry(GSACore *pGSACore,
							Data_Table_Entry *pDataTableEntry);

/***************************************** Start Function Implementations *****************************************/

//initialise GSACore
void vInitialiseCore(GSACore *pGSACore)
{
	(void)usAddrToIndex;
	(void)ulIndexToAddr;
	(void)pFindDataTableEntry;
	(void)xAddDataTableEntry;
	(void)vRemoveDataTableEntry;
	//initialise state table
	memset(pGSACore->StateTable, 0, pGSACore->StateTableSize);
}

//map out memory segments and assign state
portBASE_TYPE xSurveyMemory(GSACore *pGSACore)
{
	(void) pGSACore;
	pGSACore->DebugTrace("Hello world\n\r", 0,0,0);
	return pdTRUE;
}

/************************************************* Operations ********************************************************/

portBASE_TYPE xGSAWrite(GSACore *pGSACore,
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

unsigned portLONG ulGSARead(GSACore *pGSACore,
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

unsigned portLONG ulGSASize(GSACore *pGSACore,
							unsigned portCHAR ucAID,
							unsigned portCHAR ucDID)
{
	(void) pGSACore;
	(void) ucAID;
	(void) ucDID;

	return 0;
}

/************************************************* Internal Functions *************************************************/

static unsigned portSHORT usAddrToIndex(GSACore *pGSACore, unsigned portLONG ulAddr)
{
	return ((ulAddr - pGSACore->StartAddr) / pGSACore->MemSegSize);
}

static unsigned portLONG ulIndexToAddr(GSACore *pGSACore, unsigned portSHORT usBlockIndex)
{
	return (usBlockIndex * pGSACore->MemSegSize + pGSACore->StartAddr);
}

/******************************************** Data Table Optimisation ***********************************************/

static Data_Table_Entry *pFindDataTableEntry(GSACore *pGSACore,
									unsigned portCHAR ucAID,
									unsigned portCHAR ucDID)
{
	(void) pGSACore;
	(void) ucAID;
	(void) ucDID;

	return NULL;
}

static portBASE_TYPE xAddDataTableEntry(GSACore *pGSACore,
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

static void vRemoveDataTableEntry(GSACore *pGSACore,
							Data_Table_Entry *pDataTableEntry)
{
	(void) pGSACore;
	(void) pDataTableEntry;
	;
}
