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

#define MAX_FIB_SIZE			48
#define MAX_ARITH_PROG_SIZE		251
#define SLEEP_TIME				10000

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
	unsigned portLONG	*pulFibonacciSeq, *pulArithProg;
	unsigned portCHAR	ucIndex;
	unsigned portLONG	ulFibN_Minus1, ulFibN_Minus2;

	pulFibonacciSeq = (unsigned portLONG *)pvJMalloc(MAX_FIB_SIZE * sizeof(portLONG));
	pulArithProg	= (unsigned portLONG *)pvJMalloc(sizeof(portLONG));

	if (pulFibonacciSeq != NULL)
	{
		for(pulFibonacciSeq[0] = ulFibN_Minus2 = 0, pulFibonacciSeq[1] = ulFibN_Minus1 = 1, ucIndex = 2;
			ucIndex < MAX_FIB_SIZE;
			++ucIndex)
		{
			pulFibonacciSeq[ucIndex] = ulFibN_Minus1 + ulFibN_Minus2;
			ulFibN_Minus2 = ulFibN_Minus1;
			ulFibN_Minus1 = pulFibonacciSeq[ucIndex];
		}
		for(ucIndex = 0; ucIndex < MAX_FIB_SIZE; ++ucIndex)
		{
			vDebugPrint(MemDEMO_TaskToken,
						"Fib[%d] = %d\n\r",
						(unsigned portLONG)ucIndex,
						pulFibonacciSeq[ucIndex],
						NO_INSERT);
		}
	}
	vDebugPrint(MemDEMO_TaskToken,
				"Fibonacci Array * = %p\n\r",
				(unsigned portLONG)pulFibonacciSeq,
				NO_INSERT,
				NO_INSERT);

	if (pulArithProg != NULL)
	{
		for(ucIndex = 0, *pulArithProg = 0; ucIndex < MAX_ARITH_PROG_SIZE; ++ucIndex)
		{
			*pulArithProg += ucIndex;
		}
		vDebugPrint(MemDEMO_TaskToken,
					"AP(250) = %d\n\r",
					*pulArithProg,
					NO_INSERT,
					NO_INSERT);
	}
	vDebugPrint(MemDEMO_TaskToken,
				"Arithmetic Progression Array * = %p\n\r",
				(unsigned portLONG)pulArithProg,
				NO_INSERT,
				NO_INSERT);

	for ( ; ; )
	{
		vShowAllTaskUnusedStack();

		vSleep(SLEEP_TIME);
	}
}

