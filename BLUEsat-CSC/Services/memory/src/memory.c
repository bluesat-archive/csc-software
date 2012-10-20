 /**
 *  \file memory.c
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
 
#include "service.h"
#include "task.h"
#include "memory.h"
#include "emc.h"
#include "debug.h"

#define MEMORY_START_ADDR	STATIC_BANK_0_START_ADDR
#define MEMORY_SIZE			0x00200000
#define MEMORY_END_ADDR		(MEMORY_START_ADDR + MEMORY_SIZE)
#define WORD_SIZE			sizeof(portLONG)

unsigned portLONG	ulFreeMemoryPointer;
portBASE_TYPE 		xMemoryUsable		= pdFALSE;

void vMemory_Init(unsigned portBASE_TYPE uxPriority)
{
	(void)uxPriority;
	portBASE_TYPE 	xTestResult		= pdTRUE;

	ulFreeMemoryPointer = MEMORY_START_ADDR;

	//xTestResult = enMemoryTest(MEMORY_START_ADDR, MEMORY_SIZE, TEST_8_BITS);
	//if (xTestResult) xTestResult = enMemoryTest(MEMORY_START_ADDR, MEMORY_SIZE, TEST_16_BITS);
	if (xTestResult) xTestResult = enMemoryTest(MEMORY_START_ADDR, MEMORY_SIZE, TEST_32_BITS);

	xMemoryUsable = xTestResult;

	if (xMemoryUsable)
	{
		vDebugPrint(NULL, "External RAM Test ... Pass!\n\r", 0, 0, 0);
	}
	else
	{
		vDebugPrint(NULL, "External RAM Test ... Fail!\n\r", 0, 0, 0);
	}

}

void *pvJMalloc(unsigned portLONG ulSize)
{
	unsigned portLONG ulMemoryPointer = (unsigned portLONG)NULL;

	taskENTER_CRITICAL();
	{
		if ((xMemoryUsable != pdFALSE) && ((ulFreeMemoryPointer + ulSize) < MEMORY_END_ADDR))
		{
			ulMemoryPointer = ulFreeMemoryPointer;

			ulFreeMemoryPointer += ((ulSize / WORD_SIZE) + ((ulSize % WORD_SIZE) > 0)) * WORD_SIZE;
		}
	}
	taskEXIT_CRITICAL();

	return (void *)ulMemoryPointer;
}

#define INIT_TEST_VALUE		7
UnivRetCode enMemoryTest(unsigned portLONG 	ulStartAddr,
						unsigned portLONG 	ulSize,
						TestType			enTestType)
{
	unsigned portLONG ulAddr;
	unsigned portLONG ulTestValue;
	unsigned portCHAR ucTestSize = enTestType;

	//restrict input size to match test parameter
	ulSize = (ulSize / ucTestSize) * ucTestSize;

	for (ulAddr = ulStartAddr, ulTestValue = INIT_TEST_VALUE;
		ulAddr < ulStartAddr + ulSize;
		ulAddr += ucTestSize, ++ulTestValue)
	{
		switch(enTestType)
		{
			case(TEST_8_BITS)	:	*((unsigned char *)ulAddr) = (unsigned char)ulTestValue;
									break;
			case(TEST_16_BITS)	:	*((unsigned short *)ulAddr) = (unsigned short)ulTestValue;
									break;
			case(TEST_32_BITS)	:	*((unsigned long *)ulAddr) = (unsigned long)ulTestValue;
									break;
			default				:	break;
		}
	}

	for (ulAddr = ulStartAddr, ulTestValue = INIT_TEST_VALUE;
		ulAddr < ulStartAddr + ulSize;
		ulAddr += ucTestSize, ++ulTestValue)
	{
		switch(enTestType)
		{
			case(TEST_8_BITS)	:	if (*((unsigned char *)ulAddr) != (unsigned char)ulTestValue) return URC_FAIL;
									break;
			case(TEST_16_BITS)	:	if (*((unsigned short *)ulAddr) != (unsigned short)ulTestValue) return URC_FAIL;
									break;
			case(TEST_32_BITS)	:	if (*((unsigned long *)ulAddr) != (unsigned long)ulTestValue) return URC_FAIL;
									break;
			default				:	break;
		}
	}

	return URC_SUCCESS;
}
