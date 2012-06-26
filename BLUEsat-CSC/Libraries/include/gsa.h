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

#define MIN_MEM_SEG_SIZE BYTE_64

typedef portBASE_TYPE 		(*WriteToMemSegPtr)	(unsigned portLONG ulMemSegAddr);
typedef unsigned portSHORT 	(*ReadFromMemSegPtr)(unsigned portLONG ulMemSegAddr);
typedef portBASE_TYPE		(*xIsMemSegFreePtr)	(unsigned portLONG ulMemSegAddr);

#ifndef NO_DEBUG
	typedef void			(*DebugTracePtr)	(portCHAR *pcFormat,
												unsigned portLONG Insert1,
												unsigned portLONG Insert2,
												unsigned portLONG Insert3);
#else

	#define DebugTracePtr void *
	#define	DebugTrace(a, b, c, d)	DebugTrace = NULL

#endif /* NO_DEBUG */

typedef struct
{
	/****** Compulsory fields ******/
	/* memory specification */
	unsigned portLONG		StartAddr;
	unsigned portLONG		EndAddr;
	MEM_SEG_SIZE			MemSegSize;
	/* management resource */
	unsigned portCHAR *		StateTable;
	unsigned portSHORT		StateTableSize;
	unsigned portLONG *		MemSegBuffer;
	/* function pointers */
	WriteToMemSegPtr		WriteToMemSeg;
	xIsMemSegFreePtr		xIsMemSegFree;
	DebugTracePtr			DebugTrace;

	/********** Optional ***********/
	/* function pointer */
	ReadFromMemSegPtr		ReadFromMemSeg;
} GSACore;

#define NUM_STATE_PER_BYTE		4
#define STATE_TABLE_SIZE(StartAddr, EndAddr, MemSegSize)(((EndAddr-StartAddr)/MemSegSize)/NUM_STATE_PER_BYTE)+1	//+padding

//initialise GSACore
void vInitialiseCore(GSACore *pGSACore);

//map out memory segments and assign state
portBASE_TYPE xSurveyMemory(GSACore *pGSACore);


/************************************************* Operations ********************************************************/

portBASE_TYPE xGSAWrite(GSACore *pGSACore,
						unsigned portCHAR ucAID,
						unsigned portCHAR ucDID,
						unsigned portLONG ulSize,
						portCHAR *pcData);

unsigned portLONG ulGSARead(GSACore *pGSACore,
							unsigned portCHAR ucAID,
							unsigned portCHAR ucDID,
							unsigned portLONG ulOffset,
							unsigned portLONG ulSize,
							portCHAR *pucBuffer);
						
unsigned portLONG ulGSASize(GSACore *pGSACore,
							unsigned portCHAR ucAID,
							unsigned portCHAR ucDID);

#endif	/* GSA_H_ */
