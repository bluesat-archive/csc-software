 /**
 *  \file commsCtrlDemo.c
 *
 *  \brief An application testing comms control task
 *
 *  \author $Author: Sam Jiang $
 *  \version 1.0
 *
 *  $Date: 2012-07-21 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */
#include "application.h"
#include "commsCtrlDemo.h"
#include "debug.h"

static TaskToken DEMO_TaskToken;
static portTASK_FUNCTION(vDemoTask, pvParameters);
int iSendData(TaskToken token, char *data, int size);

void vCommsDemo_Init(unsigned portBASE_TYPE uxPriority)
{
	DEMO_TaskToken = ActivateTask(TASK_COMMS_DEMO,
								"Comms_demo",
								APP_TASK_TYPE,
								uxPriority,
								APP_STACK_SIZE,
								vDemoTask);
}

static portTASK_FUNCTION(vDemoTask, pvParameters)
{
	(void) pvParameters;
	char data[128] = "Test data 1";
	iSendData(DEMO_TaskToken,data,11);
	while(1);
}
