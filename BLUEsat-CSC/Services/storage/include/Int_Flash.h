 /**
 *  \file Int_Flash.h
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

#ifndef INT_FLASH_H_
#define INT_FLASH_H_

/**
 * \brief Initialise internal flash management service
 *
 * \param[in] uxPriority Priority for internal flash management service.
 */
void vIntFlash_Init(unsigned portBASE_TYPE uxPriority);

#endif /* INT_FLASH_H_ */
