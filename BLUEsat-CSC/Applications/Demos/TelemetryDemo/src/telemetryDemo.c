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
#define TELEM_INPUT_BUFFER_SIZE 4
#define TELEM_DEMO_MAX127_COUNT 16
#define TELEM_DEMO_MAX127_SENSOR_COUNT 8

static TaskToken Telem_TaskToken;
static sensor_result current_data[TELEM_DEMO_MAX127_COUNT][TELEM_DEMO_MAX127_SENSOR_COUNT];

static portTASK_FUNCTION(vTelemDemoTask, pvParameters);

static inline void
vTelemPrintMenu(void)
{
	vDebugPrint(Telem_TaskToken, "Telemetry Demo Started!\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
	vDebugPrint(Telem_TaskToken, "--------------------------------------\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
	vDebugPrint(Telem_TaskToken, "(1): Init Sweep (no arg)\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
	vDebugPrint(Telem_TaskToken, "(2): Set Sweep (resolution, rate)\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
	vDebugPrint(Telem_TaskToken, "(3): Read Sweep (no arg)\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
	vDebugPrint(Telem_TaskToken, "(4): Print latest data (no arg)\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
}

static inline void
vTelemConvertCommand(char *inputBuf, unsigned int *outputBuf, int size)
{
	int i;
	for (i = 0; i < size; ++i)
	{
		outputBuf[i] = inputBuf[i] - '0';
	}
}

void vTelemetryDemo_Init(unsigned portBASE_TYPE uxPriority)
{
	Telem_TaskToken = ActivateTask(TASK_TELEM_DEMO,
								"telemetry_demo",
								APP_TASK_TYPE,
								uxPriority,
								APP_STACK_SIZE,
								vTelemDemoTask);

	//vActivateQueue(Telem_TaskToken, MESSAGE_QUEUE_SIZE);
}

static portTASK_FUNCTION(vTelemDemoTask, pvParameters)
{
	(void) pvParameters;
	char inputBuffer[TELEM_INPUT_BUFFER_SIZE];
	unsigned int usReadLen;
	unsigned int commandBuffer[TELEM_INPUT_BUFFER_SIZE - 1];

	while (1)
	{
		vTelemPrintMenu();
		/* Bzero the input buffer. */
		memset((char*)inputBuffer, 0, TELEM_INPUT_BUFFER_SIZE);

		usReadLen = usDebugRead(inputBuffer, TELEM_INPUT_BUFFER_SIZE);
		//if (usReadLen > TELEM_INPUT_BUFFER_SIZE) continue;

		/* Convert input. */
		vTelemConvertCommand(inputBuffer, commandBuffer, TELEM_INPUT_BUFFER_SIZE - 1);

		/* The input should all be numbers.
		 * 1st Number: Call number
		 * 2nd Number: arg0
		 * 3rd Number: arg1
		 */
		switch (commandBuffer[0])
		{
			case TELEM_DEMO_INIT_SWEEP:
				Init_Sweep();
				break;
			case TELEM_DEMO_SET_SWEEP:
				vTelemSampleSetSweep(commandBuffer[1], commandBuffer[2], Telem_TaskToken);
				break;
			case TELEM_DEMO_READ_SWEEP:
				vTelemReadSweep((void*)current_data, Telem_TaskToken);
				break;
			case TELEM_DEMO_PRINT_LATEST_DATA:
				telem_debug_print();
				break;
			default:
				/* Should never get here. */
				vDebugPrint(Telem_TaskToken, "YOU MAD: something is very wrong.\n\r", NO_INSERT,
							NO_INSERT, NO_INSERT);
				break;
		}


	}
}
