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
	STORAGE_STORE,      	//store and overwrite previous data
	STORAGE_APPEND,     	//append to previous data
	STORAGE_DELETE,     	//remove data
	STORAGE_SIZE,       	//get size of data currently stored
	STORAGE_READ,    		//read stored data
	//temp op code
	STORAGE_FORMAT, 		//format flash sectors
	STORAGE_STATUS, 		//print flash status
} STORAGE_OPERATIONS;

#define OP_BIT_SIZE		3
#define DID_BIT_SIZE	6
#define OFF_BIT_SIZE	27
#define SIZE_BIT_SIZE	28

//debug task message format
typedef struct
{
	unsigned portLONG Operation	:	OP_BIT_SIZE;	//operation to be performed
	unsigned portLONG DID		:	DID_BIT_SIZE;	//Data ID
	unsigned portLONG Offset	:	OFF_BIT_SIZE;	//start offset for reading
	unsigned portLONG Size		:	SIZE_BIT_SIZE;	//data size/buffer size
	portCHAR *Ptr;									//pointer to data/buffer
	unsigned portLONG *pulRetValue;					//pointer to location for storing return value
} StorageContent;

#define STORAGE_CONTENT_SIZE	sizeof(StorageContent)

/**
 * \brief Process storage message received by different memory tasks
 *
 * \param[in] pGSACore 			Core for GSA.
 * \param[in] ucAID 			Application ID
 * \param[in] pStorageContent 	Storage request message.
 *
 * \returns SUCCESS or FAIL
 */
UnivRetCode enProcessStorageReq(GSACore *pGSACore,
								unsigned portCHAR ucAID,
								StorageContent *pStorageContent);

#endif /* STORAGE_OP_CONTROL_H_ */
