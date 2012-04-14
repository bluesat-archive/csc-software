 /**
 *  \file StorageOpControl.h
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

#ifndef STORAGE_OP_CONTROL_H_
#define STORAGE_OP_CONTROL_H_

#include "gsa.h"
#include "UniversalReturnCode.h"

typedef enum
{
	MEM_STORE,      	//store and overwrite previous data
	MEM_APPEND,     	//append to previous data
	MEM_DELETE,     	//remove data
	MEM_SIZE,       	//get size of data currently stored
	MEM_READ,    		//read stored data
	//temp op code
	MEM_FLASH_FORMAT, 	//format flash sectors
	MEM_FLASH_STATUS 	//print flash status
} MEM_OPERATIONS;

#define DID_BIT_SIZE	6

//debug task message format
typedef struct
{
	unsigned portLONG Operation	:	 3;				//operation to be performed
	unsigned portLONG DID		:	 DID_BIT_SIZE;	//Data ID
	unsigned portLONG Offset	:	27;				//start offset for reading
	unsigned portLONG Size		:	28;				//data size/buffer size
	portCHAR *Ptr;									//pointer to data/buffer
} MemoryContent;

#define MEMORY_CONTENT_SIZE	sizeof(MemoryContent)

/**
 * \brief Process storage message received by different memory tasks
 *
 * \param[in] pGSACore Core for GSA.
 *
 * \param[in] pMemoryContent Storage request message.
 *
 * \returns SUCCESS or FAIL
 */
UnivRetCode enProcessStorageReq(GSACore *pGSACore, MemoryContent *pMemoryContent);

#endif /* STORAGE_OP_CONTROL_H_ */
