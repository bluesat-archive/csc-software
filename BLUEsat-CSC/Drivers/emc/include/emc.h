/**
 *  \file emc.h
 *
 *  \brief External Memory Controller for access
 *  		static memory onboard.
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2012-03-18 17:16:42 +1000 (Sun, 18 Mar 2012) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#ifndef EMC_H_
#define EMC_H_

#include "UC_Selection.h"

//static banks initialise control
#define STATIC_BANK_0_ENABLED		1
#define STATIC_BANK_1_ENABLED		1
#define STATIC_BANK_2_ENABLED		0
#define STATIC_BANK_3_ENABLED		0

//static banks start address
#define STATIC_BANK_0_START_ADDR	0x80000000
#define STATIC_BANK_1_START_ADDR	0x81000000
#define STATIC_BANK_2_START_ADDR	0x82000000
#define STATIC_BANK_3_START_ADDR	0x83000000

//static banks size
#define STATIC_BANK_0_SIZE			0x00100000	//0x00200000 <- should be this but address line is wrong, thus size halved
#define STATIC_BANK_1_SIZE			0x00040000	//0x00080000 <- should be this but address line is wrong, thus size halved
#define STATIC_BANK_2_SIZE			0x00000000
#define STATIC_BANK_3_SIZE			0x00000000

/**
 * \brief Initialise External Memory Controller.
 */
void EMC_Init(void);

#endif /* EMC_H_ */
