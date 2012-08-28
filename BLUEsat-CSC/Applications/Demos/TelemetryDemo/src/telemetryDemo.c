#include "application.h"
#include "telemetry.h"
#include "telemetryDemo.h"
#include "debug.h"
#include "UniversalReturnCode.h"
#include "lib_string.h"

#define INPUT 0
#define OUTPUT 1
#define MESSAGE_WAIT_TIME 500
#define MESSAGE_QUEUE_SIZE 1
#define TELEM_INPUT_BUFFER_SIZE 10
#define TELEM_DEMO_MAX127_COUNT 16
#define TELEM_DEMO_MAX127_SENSOR_COUNT 8
#define TELEM_CLEAR_SCREEN "\e[2J\e[1;1H"
#define SET_SWEEP_MESSAGE_ARG_NUM       3

static TaskToken telemDemoTaskToken;
static unsigned short currentData[TELEM_DEMO_MAX127_COUNT][TELEM_DEMO_MAX127_SENSOR_COUNT];
static TelemEntityConfig currentSetting[MAX_NUM_GROUPS];
static TelemCommand command;

static portTASK_FUNCTION(vTelemDemoTask, pvParameters);

static inline UnivRetCode
enTelemMessageSend(TaskToken taskToken)
{
    return enTelemServiceMessageSend(taskToken, (unsigned portLONG)&command);
}

static inline void
vTelemPrintMenu(void)
{
	vDebugPrint(telemDemoTaskToken, "Telemetry Demo Started!\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
	vDebugPrint(telemDemoTaskToken, "--------------------------------------\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
	vDebugPrint(telemDemoTaskToken, "(2): Set Sweep (resolution[arg1], rate[arg2])\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
	vDebugPrint(telemDemoTaskToken, "(3): Read Sweep (no arg)\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
	vDebugPrint(telemDemoTaskToken, "(4): Print latest data (no arg)\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
}

static inline void
vTelemConvertCommand(char *inputBuf, unsigned int *outputBuf, int size)
{
	int i;
	for (i = 0; i < size; ++i) outputBuf[i] = inputBuf[i] - '0';
}

static void
vTelemReadSweep(void *buffer, TaskToken token)
{
    command.operation = READSWEEP;
    command.size = TELEM_DEMO_MAX127_COUNT * TELEM_DEMO_MAX127_SENSOR_COUNT;
    /* Pass in the data for processing. */
    command.buffer = (unsigned short*)buffer;
    enTelemMessageSend(token);
}

/* This sets all the entities groups to the same setting. */
static void
vTelemSampleSetSweep(int resolution, int rate, TaskToken token)
{
    int i;
    command.operation = SETSWEEP;
    command.size = SET_SWEEP_MESSAGE_ARG_NUM;
    for (i = 0; i < MAX_NUM_GROUPS; ++i)
    {
        currentSetting[i].resolution = resolution;
        currentSetting[i].rate = rate;
    }

    command.paramBuffer = currentSetting;
    enTelemMessageSend(token);
}

void
vTelemetryDemoInit(unsigned portBASE_TYPE uxPriority)
{
	telemDemoTaskToken = ActivateTask(TASK_TELEM_DEMO,
								"telemetry_demo",
								APP_TASK_TYPE,
								uxPriority,
								APP_STACK_SIZE,
								vTelemDemoTask);
}

static
portTASK_FUNCTION(vTelemDemoTask, pvParameters)
{
	(void) pvParameters;
	char inputBuffer[TELEM_INPUT_BUFFER_SIZE];
	unsigned int usReadLen;
	unsigned int commandBuffer[TELEM_INPUT_BUFFER_SIZE - 1];

	vDebugPrint(telemDemoTaskToken, "%s", (unsigned portLONG)TELEM_CLEAR_SCREEN,
			NO_INSERT, NO_INSERT);

	while (1)
	{
	    vTelemPrintMenu();
		/* Bzero the input buffer. */
		memset((char*)inputBuffer, 0, TELEM_INPUT_BUFFER_SIZE);

		usReadLen = usDebugRead(inputBuffer, TELEM_INPUT_BUFFER_SIZE);
		if (usReadLen > TELEM_INPUT_BUFFER_SIZE) continue;
        vDebugPrint(telemDemoTaskToken, "argc is %d\n\r", (unsigned portLONG)usReadLen, NO_INSERT,
                NO_INSERT);
		/* Convert input. */
		vTelemConvertCommand(inputBuffer, commandBuffer, TELEM_INPUT_BUFFER_SIZE - 1);

		/*
		 * The input should all be numbers.
		 * 1st Number: Call number
		 * 2nd Number: arg0
		 * 3rd Number: arg1
		 */
		switch (commandBuffer[0])
		{
			case TELEM_DEMO_SET_SWEEP:
				vDebugPrint(telemDemoTaskToken, "In Set Sweep | command[1] %d | command[2] %d\n\r",
						(unsigned portLONG)commandBuffer[1], (unsigned portLONG)commandBuffer[2],
						NO_INSERT);

				vTelemSampleSetSweep(commandBuffer[1], commandBuffer[2], telemDemoTaskToken);
				break;
			case TELEM_DEMO_READ_SWEEP:
				vDebugPrint(telemDemoTaskToken, "In Read Sweep\n\r", NO_INSERT,
							NO_INSERT, NO_INSERT);
				vTelemReadSweep((void*)currentData, telemDemoTaskToken);
				break;
			case TELEM_DEMO_PRINT_LATEST_DATA:
				vDebugPrint(telemDemoTaskToken, "In Print Data\n\r", NO_INSERT,
							NO_INSERT, NO_INSERT);
				vTelemDebugPrint();
				break;
			default:
				/* Should never get here. */
				break;
		}
	}
}
