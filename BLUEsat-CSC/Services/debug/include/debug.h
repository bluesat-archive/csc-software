 /**
 *  \file debug.h
 *
 *  \brief Using UART to output debug message to terminal
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include "command.h"
#include "UniversalReturnCode.h"

#define NO_INSERT	0

/**
 * \brief Initialise debug service
 *
 * \param[in] uxPriority Priority for debug service.
 */
void vDebug_Init(unsigned portBASE_TYPE uxPriority);

/**
 * \brief Read from UART
 *
 * \param[in] pcBuffer Pointer to buffer
 * \param[in] usMaxSize Max number of bytes to be read
 *
 * \returns Number of bytes read
 */
unsigned portSHORT	usDebugRead(portCHAR *			pcBuffer,
								unsigned portSHORT 	usMaxSize);

/**
 * \brief Write debug message string
 *
 * \param[in] taskToken Task token from request task
 * \param[in] pcFormat Print format.
 * \param[in] pcInsertion_1 Insertion data 1.
 * \param[in] pcInsertion_2 Insertion data 2.
 * \param[in] pcInsertion_3 Insertion data 3.
 *
 * \returns enum Universal return code
 */
UnivRetCode enDebugPrint(TaskToken 			taskToken,
						portCHAR *			pcFormat,
						unsigned portLONG 	pcInsertion_1,
						unsigned portLONG 	pcInsertion_2,
						unsigned portLONG 	pcInsertion_3);

#endif /* DEBUG_H_ */
