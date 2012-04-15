 /**
 *  \file Demo_Application_2.c
 *
 *  \brief An application demonstrating how an application operate
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
#include "Demo_Application_2.h"
#include "Demo_Application_1.h"
#include "debug.h"

#define MESSAGE_WAIT_TIME 500

//task token for accessing services and other applications
static TaskToken DEMO_TaskToken;

static portTASK_FUNCTION(vDemoTask, pvParameters);

void vDemoApp2_Init(unsigned portBASE_TYPE uxPriority)
{
	DEMO_TaskToken = ActivateTask(TASK_DEMO_APP_2, 
								"DemoApp2",
								APP_TASK_TYPE,
								uxPriority, 
								APP_STACK_SIZE, 
								vDemoTask);
}

static portTASK_FUNCTION(vDemoTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;

	for ( ; ; )
	{
		enResult = enMessage_To_Q(DEMO_TaskToken, "Hello from DemoApp2!\n\r");
	}
}


