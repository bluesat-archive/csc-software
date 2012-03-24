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

#endif /* MEMORY_H_ */
