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

/**
 * \brief Summation of data for 1s complement calculation
 *
 * \param[in] ulDataSum Previous data sum
 *
 * \param[in] ulAddr Address to new data
 *
 * \param[in] usNumShorts Number of data shorts to add
 *
 * \returns New data sum
 */
unsigned portLONG ulAddToSum(unsigned portLONG ulDataSum,
							unsigned portLONG ulAddr,
							unsigned portSHORT usNumShorts);

/**
 * \brief Generate checksum from collected data sum
 *
 * \param[in] ulDataSum Data sum checksum to be generated on
 *
 * \returns 1s complement checksum
 */
unsigned portSHORT usGenerateChecksum(unsigned portLONG ulDataSum);

/**
 * \brief Verify checksum from collected data sum INCLUDE checksum
 *
 * \param[in] ulDataSum Data sum INCLUDE checksum
 *
 * \returns PASS or FAIL
 */
portBASE_TYPE xVerifyChecksum(unsigned portLONG ulDataSum);

#endif /* ONES_COMP_CHECKSUM_H_ */
