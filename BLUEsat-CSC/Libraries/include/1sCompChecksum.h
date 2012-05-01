 /**
 *  \file 1sCompChecksum.h
 *
 *  \brief Calculate 16-bit one's complement of the one's complement sum
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#ifndef ONES_COMP_CHECKSUM_H_
#define ONES_COMP_CHECKSUM_H_

#include "FreeRTOS.h"

#define DEFAULT_VALID_CHECKSUM	0x0000ffff

//add data short to existing sum
//return new sum
unsigned portLONG ulAddToSum(unsigned portLONG ulDataSum,
							unsigned portLONG ulAddr,
							unsigned portSHORT usNumDataShorts);

//create checksum from data sum
unsigned portSHORT usGenerateChecksum(unsigned portLONG ulDataSum);

//verify checksum from data sum
portBASE_TYPE xVerifyChecksum(unsigned portLONG ulDataSum);

#endif /* ONES_COMP_CHECKSUM_H_ */
