 /**
 *  \file switchingDemo.c
 *
 *  \brief An application demonstrating how an application operate
 *
 *  \author $Author: Sam Jiang $
 *  \version 1.0
 *
 *  $Date: 2012-06-23 13:35:54 +1100 (Sat, 23 Jun 2012) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#include "application.h"
#include "switchingDemo.h"
#include "debug.h"

static TaskToken DEMO_TaskToken;

static portTASK_FUNCTION(vDemoTask, pvParameters);

void vSwitchingDemo_Init(unsigned portBASE_TYPE uxPriority)
{
	DEMO_TaskToken = ActivateTask(TASK_SWITHCING_DEMO,
								"gpio_demo",
								APP_TASK_TYPE,
								uxPriority,
								APP_STACK_SIZE,
								vDemoTask);

	// initialise the port direction
	setGPIOOutput();
}

static portTASK_FUNCTION(vDemoTask, pvParameters)
{
	int i;
	int j = 0;

	(void) pvParameters;

	static unsigned int dummy = 0xffff;

	for ( ; ; )
	{
		for (i = 0; i < 0xfffff; ++i)
		{
			dummy =~ dummy;
		}

		if (j % 2 == 0)
		{
			setGPIOallHigh();
		} else
		{
			setGPIOallLow();
		}
		++j;

	}
}

