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

#include "application.h"
#include "memoryDemo.h"
#include "memory.h"
#include "debug.h"
#include "lib_string.h"

#define MAX_FIB_SIZE		48
#define SLEEP_TIME			10000

//task token for accessing services and other applications
static TaskToken MemDEMO_TaskToken;

static portTASK_FUNCTION(vMemDemoTask, pvParameters);

void vMemoryDemo_Init(unsigned portBASE_TYPE uxPriority)
{
	MemDEMO_TaskToken = ActivateTask(TASK_MEMORY_DEMO,
									"MemoryDemo",
									APP_TASK_TYPE,
									uxPriority,
									APP_STACK_SIZE,
									vMemDemoTask);
}

static portTASK_FUNCTION(vMemDemoTask, pvParameters)
{
	(void) pvParameters;
	unsigned portLONG *	pulFibonacciSeq;
	unsigned portCHAR	ucFibIndex;
	unsigned portLONG	ulFibN_Minus1, ulFibN_Minus2;

	pulFibonacciSeq = (unsigned portLONG *)pvJMalloc(MAX_FIB_SIZE * sizeof(portLONG));

	if (pulFibonacciSeq != NULL)
	{
		for(pulFibonacciSeq[0] = ulFibN_Minus2 = 0, pulFibonacciSeq[1] = ulFibN_Minus1 = 1, ucFibIndex = 2;
			ucFibIndex < MAX_FIB_SIZE;
			++ucFibIndex)
		{
			pulFibonacciSeq[ucFibIndex] = ulFibN_Minus1 + ulFibN_Minus2;
			ulFibN_Minus2 = ulFibN_Minus1;
			ulFibN_Minus1 = pulFibonacciSeq[ucFibIndex];
		}
		for(ucFibIndex = 0; ucFibIndex < MAX_FIB_SIZE; ++ucFibIndex)
		{
			vDebugPrint(MemDEMO_TaskToken,
						"%d\n\r",
						pulFibonacciSeq[ucFibIndex],
						NO_INSERT,
						NO_INSERT);
		}
	}

	for ( ; ; )
	{
		vShowAllTaskUnusedStack();

		vSleep(SLEEP_TIME);
	}
}

