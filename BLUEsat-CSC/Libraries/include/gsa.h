 /**
 *  \file gsa.h
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

#ifndef GSA_H_
#define GSA_H_

#include "FreeRTOS.h"

typedef enum
{						//Maximum memory supported 2^15 * Segment Size
	BYTE_64		= 64,	//2,097,152 bytes
	BYTE_128	= 128,	//4,194,304 bytes
	BYTE_256	= 256,	//8,388,608 bytes
	BYTE_512	= 512,	//16,777,216 bytes
	BYTE_1024	= 1024,	//33,554,432 bytes
	BYTE_2048	= 2048,	//67,108,864 bytes
	BYTE_4096	= 4096,	//134,217,728 bytes
	BYTE_8192	= 8192	//268,435,456 bytes
} MEM_SEG_SIZE;

typedef unsigned portSHORT 	(*WriteToMemSegPtr)	(unsigned portLONG ulMemSegAddr, portCHAR *pData, unsigned portSHORT usSize);
typedef unsigned portSHORT 	(*ReadFromMemSegPtr)(unsigned portLONG ulMemSegAddr, portCHAR *pBuffer, unsigned portSHORT usSize);
typedef unsigned portLONG	(*GetNextMemSegPtr)	(void);
typedef void 				(*DeleteMemSegPtr)	(unsigned portLONG ulMemSegAddr);
typedef portBASE_TYPE		(*xIsMemSegFreePtr)	(unsigned portLONG ulMemSegAddr);

#ifndef NO_DEBUG
	typedef void			(*DebugTracePtr)	(portCHAR *pcFormat,
												unsigned portLONG Insert1,
												unsigned portLONG Insert2,
												unsigned portLONG Insert3);
#else

	#define DebugTrace void *
	#define	DebugTracePtr(a, b, c, d)	DebugTracePtr = NULL

#endif /* NO_DEBUG */

typedef struct
{
	/****** Compulsory fields ******/
	/* memory specification */
	unsigned portLONG		StartAddr;
	MEM_SEG_SIZE			MemSegSize;
	/* management resource */
	unsigned portSHORT		StateTableSize;
	unsigned portCHAR *		StateTable;
	unsigned portSHORT		DataTableSize;
	void *					DataTable;
	/* function pointers */
	GetNextMemSegPtr		GetNextMemSeg;
	DebugTracePtr			DebugTrace;

	/******* Optional fields *******/
	/* function pointers */
	WriteToMemSegPtr		WriteToMemSeg;
	ReadFromMemSegPtr		ReadFromMemSeg;
	DeleteMemSegPtr			DeleteMemSeg;
	xIsMemSegFreePtr		xIsMemSegFree;

	/******* DO NOT MODIFY GSA USE ONLY ******/
	unsigned portSHORT		DataTableIndex;
} GSACore;

#define NUM_BITS_PER_BYTE		8
#define STATE_SIZE_BIT			2
#define NUM_STATES_PER_BYTE		(NUM_BITS_PER_BYTE / STATE_SIZE_BIT)
//calculate state table size in bytes
#define STATE_TABLE_SIZE(StartAddr, EndAddr, MemSegSize) ((((EndAddr - StartAddr) / MemSegSize) / NUM_STATES_PER_BYTE) + sizeof(unsigned portLONG)) //extra for padding

#define DATA_TABLE_ENTRY_SIZE	8
//calculate data table size in bytes
#define DATA_TABLE_SIZE(NumEntry) (NumEntry * DATA_TABLE_ENTRY_SIZE)

//initialise GSACore
void vInitialiseCore(GSACore *pGSACore);

//map out memory segments and assign state
void vSurveyMemory(GSACore *pGSACore,
					unsigned portLONG ulStartAddr,
					unsigned portLONG ulEndAddr);

//find next free state memory segment in given range
unsigned portLONG ulFindNextFreeState(GSACore *pGSACore,
									unsigned portLONG ulStartAddr,
									unsigned portLONG ulEndAddr);

/************************************************* Operations ********************************************************/

portBASE_TYPE xGSAWrite(GSACore *pGSACore,
						unsigned portCHAR ucAID,
						unsigned portCHAR ucDID,
						unsigned portLONG ulSize,
						portCHAR *pcData);

#endif	/* GSA_H_ */
