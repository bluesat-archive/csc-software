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
{
	BYTE_32		= 32,
	BYTE_64		= 64,
	BYTE_128	= 128,
	BYTE_256	= 256,
	BYTE_512	= 512,
	BYTE_1024	= 1024,
	BYTE_2048	= 2048,
	BYTE_4096	= 4096,
	BYTE_8192	= 8192
} MEM_SEG_SIZE;

typedef enum
{
	STATE_FREE,
	STATE_USED,
	STATE_TO_BE_FREED
} BLOCK_STATE;

typedef unsigned portSHORT 	(*WriteToMemSeg)	(void * pMemSeg, portCHAR *pData, unsigned portSHORT usSize);
typedef unsigned portSHORT 	(*ReadFromMemSeg)	(void * pMemSeg, portCHAR *pBuffer, unsigned portSHORT usSize);
typedef void *				(*GetNextMemSeg)	(MEM_SEG_SIZE enMemSegSize);
typedef void 				(*UpdateMemSegState)(void * pMemSeg, MEM_SEG_SIZE enMemSegSize);

#ifndef NO_DEBUG
	typedef void			(*DebugTrace)		(portCHAR *pcFormat,
												unsigned portLONG Insert1,
												unsigned portLONG Insert2,
												unsigned portLONG Insert3);
#else

	#define DebugTrace void *
	#define	DebugTracePtr(a, b, c, d)	DebugTracePtr = NULL

#endif /* NO_DEBUG */

typedef struct
{
	MEM_SEG_SIZE		enMemSegSize;
	/* function pointers */
	WriteToMemSeg		WriteToMemSegPtr;
	ReadFromMemSeg		ReadFromMemSegPtr;
	GetNextMemSeg		GetNextMemSegPtr;
	UpdateMemSegState	UpdateMemSegStatePtr;
	DebugTrace			DebugTracePtr;
} GSACore;

#endif	/* GSA_H_ */
