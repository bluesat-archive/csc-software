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
#include "memory.h"
#include "emc.h"

#define MEMORY_START_ADDR	STATIC_BANK_1_START_ADDR
#define MEMORY_SIZE			STATIC_BANK_1_SIZE
#define MEMORY_END_ADDR		(STATIC_BANK_1_START_ADDR + STATIC_BANK_1_SIZE)
#define WORD_SIZE			sizeof(portLONG)

#ifdef MEM_TEST
	static void vMemoryTest(unsigned portLONG ulStartAddr, unsigned portLONG ulSize);
	static UnivRetCode enMemoryTest_8(unsigned portLONG ulStartAddr, unsigned portLONG ulSize);
	static UnivRetCode enMemoryTest_16(unsigned portLONG ulStartAddr, unsigned portLONG ulSize);
	static UnivRetCode enMemoryTest_32(unsigned portLONG ulStartAddr, unsigned portLONG ulSize);
#endif

unsigned portLONG	ulFreeMemoryPointer;
UnivRetCode 		enMemoryTestResult;

void vMemory_Init(unsigned portBASE_TYPE uxPriority)
{
	(void)uxPriority;

	ulFreeMemoryPointer = MEMORY_START_ADDR;
	enMemoryTestResult 	= URC_SUCCESS;
}

void *pvJMalloc(unsigned portLONG ulSize)
{
	unsigned portLONG ulMemoryPointer;

	if (enMemoryTestResult == URC_FAIL) return NULL;

	if ((ulFreeMemoryPointer + ulSize) >= MEMORY_END_ADDR) return NULL;

	ulMemoryPointer = ulFreeMemoryPointer;

	ulFreeMemoryPointer = ((ulSize / WORD_SIZE) + ((ulSize % WORD_SIZE) > 0)) * WORD_SIZE;

	return (void *)ulMemoryPointer;
}

#ifdef MEM_TEST
	static void vMemoryTest(unsigned portLONG ulStartAddr, unsigned portLONG ulSize)
	{
		//8 bits test
		vDebugPrint(Memory_TaskToken,
					"8 bits test...%d\n\r",
					enMemoryTest_8(ulStartAddr, ulSize) == URC_SUCCESS,
					0,
					0);

		//16 bits test
		vDebugPrint(Memory_TaskToken,
					"16 bits test...%d\n\r",
					enMemoryTest_16(ulStartAddr, ulSize) == URC_SUCCESS,
					0,
					0);

		//32 bits test
		vDebugPrint(Memory_TaskToken,
					"32 bits test...%d\n\r",
					enMemoryTest_32(ulStartAddr, ulSize) == URC_SUCCESS,
					0,
					0);
	}

	static UnivRetCode enMemoryTest_8(unsigned portLONG ulStartAddr, unsigned portLONG ulSize)
	{
		unsigned portCHAR ucValue;
		unsigned portLONG ulAddress;

		for (ulAddress = ulStartAddr, ucValue = 3;
			ulAddress < ulStartAddr + ulSize;
			ulAddress += sizeof(unsigned portCHAR), ++ucValue)
		{
			*(unsigned portCHAR *)ulAddress = ucValue;
		}

		for (ulAddress = ulStartAddr, ucValue = 3;
			ulAddress < ulStartAddr + ulSize;
			ulAddress += sizeof(unsigned portCHAR), ++ucValue)
		{
			if (*(unsigned portCHAR *)ulAddress != ucValue) return URC_FAIL;
		}

		return URC_SUCCESS;
	}

	static UnivRetCode enMemoryTest_16(unsigned portLONG ulStartAddr, unsigned portLONG ulSize)
	{
		unsigned portLONG ulAddress;
		unsigned portSHORT usValue;

		for (ulAddress = ulStartAddr, usValue = 3;
			ulAddress < ulStartAddr + ulSize;
			ulAddress += sizeof(unsigned portSHORT), ++usValue)
		{
			*(unsigned portSHORT *)ulAddress = usValue;
		}

		for (ulAddress = ulStartAddr, usValue = 3;
			ulAddress < ulStartAddr + ulSize;
			ulAddress += sizeof(unsigned portSHORT), ++usValue)
		{
			if (*(unsigned portSHORT *)ulAddress != usValue) return URC_FAIL;
		}

		return URC_SUCCESS;
	}

	static UnivRetCode enMemoryTest_32(unsigned portLONG ulStartAddr, unsigned portLONG ulSize)
	{
		unsigned portLONG ulAddress;

		for (ulAddress = ulStartAddr;
			ulAddress < ulStartAddr + ulSize;
			ulAddress += sizeof(unsigned portLONG))
		{
			*(unsigned portLONG *)ulAddress = ulAddress + 1;
		}

		for (ulAddress = ulStartAddr;
			ulAddress < ulStartAddr + ulSize;
			ulAddress += sizeof(unsigned portLONG))
		{
			if (*(unsigned portLONG *)ulAddress != ulAddress + 1) return URC_FAIL;
		}

		return URC_SUCCESS;
	}
#endif
