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

/**
 * \brief Writes a single character onto the port
 *
 * \param[in] uxPriority Priority for debug service.
 */
void vDebug_Init(unsigned portBASE_TYPE uxPriority);

/**
 * \brief Writes a single character onto the port
 *
 * \param[in] uxPriority Priority for debug service.
 */
UnivRetCode enDebug_Print(TaskToken taskToken,
						signed portCHAR *pcDebugString,
						unsigned portSHORT usLength);

#endif /* DEBUG_H_ */
