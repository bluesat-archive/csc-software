 /**
 *  \file memory.h
 *
 *  \brief Provide additional RAM to CSC
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

#include "FreeRTOS.h"
#include "UniversalReturnCode.h"

/**
 * \brief Initialise memory service
 *
 * \param[in] uxPriority Priority for memory service.
 */
void vMemory_Init(unsigned portBASE_TYPE uxPriority);

/**
 * \brief Request volatile memory
 *
 * \param[in] ulSize Size in bytes.
 */
void *pvJMalloc(unsigned portLONG ulSize);

#endif /* MEMORY_H_ */
