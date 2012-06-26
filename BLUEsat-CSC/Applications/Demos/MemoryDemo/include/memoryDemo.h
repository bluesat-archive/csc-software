 /**
 *  \file memoryDemo.h
 *
 *  \brief An application demonstrating how to use pvJMalloc for additional memory
 *  		and periodic trigger stack usage for all tasks
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#ifndef MEMORY_DEMO_H_
#define MEMORY_DEMO_H_

/**
 * \brief Initialise Memory Demo application
 *
 * \param[in] uxPriority Priority for Memory Demo application.
 */
void vMemoryDemo_Init(unsigned portBASE_TYPE uxPriority);

#endif /* MEMORY_DEMO_H_ */
