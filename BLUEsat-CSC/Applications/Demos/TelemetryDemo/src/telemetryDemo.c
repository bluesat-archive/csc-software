#include "application.h"
#include "telemetryDemo.h"
#include "debug.h"

#define INPUT 0
#define OUTPUT 1
#define MESSAGE_WAIT_TIME 500
#define MESSAGE_QUEUE_SIZE 1
#define TELEM_INPUT_BUFFER_SIZE 100

static TaskToken Telem_TaskToken;

static inline void
vTelemPrintMenu(void)
{
	vDebugPrint(Telem_TaskToken, "Telemetry Demo Started!\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
}


static portTASK_FUNCTION(vTelemDemoTask, pvParameters);

void vTelemetryDemo_Init(unsigned portBASE_TYPE uxPriority)
{
	DEMO_TaskToken = ActivateTask(TASK_TELEM_DEMO,
								"telemetry_demo",
								APP_TASK_TYPE,
								uxPriority,
								APP_STACK_SIZE,
								vTelemDemoTask);

	vActivateQueue(Telem_TaskToken, MESSAGE_QUEUE_SIZE);
}

static portTASK_FUNCTION(vTelemDemoTask, pvParameters)
{
	(void) pvParameters;
	char *inputBuffer[TELEM_INPUT_BUFFER_SIZE];

	while (1)
	{
		vTelemPrintMenu();


	}


}
