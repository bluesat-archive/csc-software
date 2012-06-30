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
{						//Maximum memory supported (2^15 - 1) * Segment Size
	BYTE_64		= 64,	//2,097,152 bytes	- 64
	BYTE_128	= 128,	//4,194,304 bytes	- 128
	BYTE_256	= 256,	//8,388,608 bytes	- 256
	BYTE_512	= 512,	//16,777,216 bytes	- 512
	BYTE_1024	= 1024,	//33,554,432 bytes	- 1024
	BYTE_2048	= 2048,	//67,108,864 bytes	- 2048
	BYTE_4096	= 4096,	//134,217,728 bytes - 4096
	BYTE_8192	= 8192	//268,435,456 bytes	- 8192
} GSA_BLOCK_SIZE;

#define MIN_BLOCK_SIZE		BYTE_64

typedef unsigned portLONG	(*WriteBufferPtr)	(void);
//typedef unsigned portSHORT 	(*ReadBlockToBufPtr)(unsigned portLONG ulBlockAddr);
typedef portBASE_TYPE		(*xIsBlockFreePtr)	(unsigned portLONG ulBlockAddr);

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
	unsigned portLONG AID:		6;
	unsigned portLONG DID:		8;
	unsigned portLONG LastHBI:	15;
	unsigned portLONG Upadding:	3;	//usable padding
	unsigned portLONG Size;
} Data_Table_Entry;

typedef struct
{
	Data_Table_Entry 	*DataTable;
	unsigned portSHORT	DataTableIndex;
} GSA_Optimisation;

typedef struct
{
	/****** Compulsory fields ******/
	/* memory specification */
	unsigned portLONG		StartAddr;
	unsigned portLONG		EndAddr;
	GSA_BLOCK_SIZE			BlockSize;
	/* management resource */
	unsigned portCHAR *		StateTable;
	unsigned portSHORT		StateTableSize;
	/* buffer */
	unsigned portLONG *		BlockBuffer;
	/* function pointers */
	WriteBufferPtr			WriteBuffer;
	xIsBlockFreePtr			xIsBlockFree;
	DebugTracePtr			DebugTrace;
	/********** Optional ***********/
	//TODO implement and enable indirectly read memory
	/* function pointer */
	//ReadBlockToBufPtr		ReadBlockToBuf;
	/*  optimisation */
	GSA_Optimisation *		Optimisation;
} GSACore;

#define STATE_SIZE_BIT			2
#define NUM_STATE_PER_BYTE		4
#define STATE_TABLE_SIZE(StartAddr, EndAddr, BlockSize)(((EndAddr-StartAddr)/BlockSize)/NUM_STATE_PER_BYTE)+1	//+padding

//initialise GSACore
void vInitialiseCore(GSACore const *pGSACore);

typedef enum
{
	GSA_EXT_STATE_FREE,
	GSA_EXT_STATE_DEAD,
	GSA_EXT_STATE_VALID
} GSA_EXT_STATE;

unsigned portSHORT usBlockStateCount(GSACore const *	pGSACore,
									unsigned portLONG 	ulStartAddr,
									unsigned portLONG 	ulEndAddr,
									GSA_EXT_STATE		enState);

unsigned portLONG ulGetNextFreeBlock(GSACore const *	pGSACore,
									unsigned portLONG 	ulStartAddr,
									unsigned portLONG 	ulEndAddr);

/************************************************* Operations ********************************************************/

portBASE_TYPE xGSAWrite(GSACore const *pGSACore,
						unsigned portCHAR ucAID,
						unsigned portCHAR ucDID,
						unsigned portLONG ulSize,
						portCHAR *pcData);

unsigned portLONG ulGSARead(GSACore const *pGSACore,
							unsigned portCHAR ucAID,
							unsigned portCHAR ucDID,
							unsigned portLONG ulOffset,
							unsigned portLONG ulSize,
							portCHAR *pucBuffer);
						
unsigned portLONG ulGSASize(GSACore const *pGSACore,
							unsigned portCHAR ucAID,
							unsigned portCHAR ucDID);

#endif	/* GSA_H_ */

/* reference only */

///******************************** Block Info ********************************/
//typedef union
//{
//	struct
//	{
//		/* word 0 */
//		unsigned portLONG Checksum:    	16;	//header checksum
//		unsigned portLONG H:			 1;	//head block bit
//		unsigned portLONG NDBI:	  		15;	//Next Data Block Index
//	};
//
//	struct
//	{
//		/* word 0 */
//		unsigned portLONG Checksum:    	16;	//header checksum
//		unsigned portLONG H:			 1; //head block bit
//		unsigned portLONG PrevHBI:		15;	//previous Head Block Index
//		/* word 1 */
//		unsigned portLONG AID:			 6; //Application ID
//		unsigned portLONG DID:			 8; //Data ID
//		unsigned portLONG Terminal:		 1; //terminal block flag
//		unsigned portLONG FDBI_U:		 1;	//First Data Block Index used
//		//extended short
//		unsigned portLONG FDBI:			15;	//First Data Block Index
//		unsigned portLONG UUpadding:  	 1;	//unusable padding
//	};
//} Header;

//#define HB_HEADER_SIZE 		sizeof(Header)
//#define HB_SML_HEADER_SIZE 	(HB_HEADER_SIZE - 2)
//#define DB_HEADER_SIZE 		(HB_HEADER_SIZE - 4)
//
//typedef struct
//{
//	unsigned portLONG	DataSize;
//} TreeInfo;

