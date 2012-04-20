 /**
 *  \file StorageOpControl.c
 *
 *  \brief Translate storage operations to GSA operations
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#include "StorageOpControl.h"

UnivRetCode enProcessStorageReq(GSACore *pGSACore,
								unsigned portCHAR ucAID,
								MemoryContent *pMemoryContent)
{
	//translate operation
	switch (pMemoryContent->Operation)
	{
		case MEM_STORE	:	pGSACore->DebugTracePtr("AID: %d requested Store\n\r", ucAID, 0, 0);
							break;

		case MEM_APPEND	:	pGSACore->DebugTracePtr("AID: %d requested Append\n\r", ucAID, 0, 0);
							break;

		case MEM_DELETE	:	pGSACore->DebugTracePtr("AID: %d requested Delete\n\r", ucAID, 0, 0);
							break;

		case MEM_SIZE	:	pGSACore->DebugTracePtr("AID: %d requested Size\n\r", ucAID, 0, 0);
							break;

		case MEM_READ	:	pGSACore->DebugTracePtr("AID: %d requested Read\n\r", ucAID, 0, 0);
							break;

		default			:	pGSACore->DebugTracePtr("AID: %d requested unknown operation %d!\n\r",
													ucAID,
													pMemoryContent->Operation, 0);
							return URC_FAIL;
	}

	return URC_SUCCESS;
}
