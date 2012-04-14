 /**
 *  \file fram.h
 *
 *  \brief Internal flash memory management
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */
#ifndef FRAM_H_
#define FRAM_H_

/**
 * \brief Initialise FRAM service
 *
 * \param[in] uxPriority Priority for FRAM service.
 */
void vFRAM_Init(unsigned portBASE_TYPE uxPriority);

#endif /* FRAM_H_ */
