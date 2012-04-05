 /**
 *  \file memory.h
 *
 *  \brief Provide storage to CSC
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#include "command.h"
#include "UniversalReturnCode.h"

/**
 * \brief Initialise memory service
 *
 * \param[in] uxPriority Priority for memory service.
 */
void vMemory_Init(unsigned portBASE_TYPE uxPriority);

/**
 * \brief Delete data given AID & DID
 *
 * \param[in] taskToken Task token from request task
 * \param[in] ucDID		Data ID
 *
 * \returns URC_SUCCESS or URC_FAIL of the operation
 */
UnivRetCode enDataDelete(TaskToken taskToken,
						unsigned portCHAR ucDID);

/**
 * \brief Return the stored data size for given AID & DID
 *
 * \param[in] taskToken Task token from request task
 * \param[in] ucDID		Data ID
 *
 * \returns URC_SUCCESS or URC_FAIL of the operation
 */
UnivRetCode enDataSize(TaskToken taskToken,
						unsigned portCHAR ucDID);

/**
 * \brief Read stored data for given AID & DID
 *
 * \param[in] taskToken Task token from request task
 * \param[in] ucDID			Data ID
 * \param[in] ulOffset		Offset point for data read in bytes
 * \param[in] ulSize		Number of bytes to be read
 * \param[out] pucBuffer	Pointer to data return buffer
 *
 * \returns URC_SUCCESS or URC_FAIL of the operation
 */
UnivRetCode enDataRead(TaskToken taskToken,
						unsigned portCHAR ucDID,
						unsigned portLONG ulOffset,
						unsigned portLONG ulSize,
						portCHAR *pucBuffer);

/**
 * \brief Store data for given AID & DID
 *
 * \param[in] taskToken Task token from request task
 * \param[in] ucDID		Data ID
 * \param[in] ulSize	Bytes of data to be stored
 * \param[in] pcData	Pointer to data
 *
 * \returns URC_SUCCESS or URC_FAIL of the operation
 */
UnivRetCode enDataStore(TaskToken taskToken,
						unsigned portCHAR ucDID,
						unsigned portLONG ulSize,
						portCHAR *pcData);

/**
 * \brief Append data for given AID & DID
 *
 * \param[in] taskToken Task token from request task
 * \param[in] ucDID		Data ID
 * \param[in] ulSize	Bytes of data to be stored
 * \param[in] pcData	Pointer to data
 *
 * \returns URC_SUCCESS or URC_FAIL of the operation
 */
UnivRetCode enDataAppend(TaskToken taskToken,
						unsigned portCHAR ucDID,
						unsigned portLONG ulSize,
						portCHAR *pcData);

#endif /* MEMORY_H_ */
