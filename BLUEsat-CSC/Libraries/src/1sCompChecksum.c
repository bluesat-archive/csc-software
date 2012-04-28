 /**
 *  \file 1sCompChecksum.c
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

#include "1sCompChecksum.h"

unsigned portLONG ulAddToSum(unsigned portLONG ulDataSum,
							unsigned portLONG ulAddr,
							unsigned portSHORT usNumDataShorts)
{
	unsigned portSHORT usIndex;
	unsigned portSHORT *pusAddr = (unsigned portSHORT *)ulAddr;

	for (usIndex = 0; usIndex < usNumDataShorts; ++usIndex)
	{
		ulDataSum += pusAddr[usIndex];
	}

	return ulDataSum;
}


unsigned portSHORT usGenerateChecksum(unsigned portLONG ulDataSum)
{
	ulDataSum = (ulDataSum & 0xffff) + (ulDataSum >> 16);

	return ((unsigned portSHORT) ~ulDataSum);
}


portBASE_TYPE xVerifyChecksum(unsigned portLONG ulDataSum)
{
	ulDataSum = (ulDataSum & 0xffff) + (ulDataSum >> 16);

	return (((unsigned portSHORT)~ulDataSum) == 0);
}
