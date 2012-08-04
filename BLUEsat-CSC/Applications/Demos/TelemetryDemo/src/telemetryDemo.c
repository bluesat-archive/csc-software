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

static TaskToken Telem_TaskToken;
static sensor_result current_data[TELEM_DEMO_MAX127_COUNT][TELEM_DEMO_MAX127_SENSOR_COUNT];
static Entity_sweep_params current_setting[MAX_NUM_GROUPS];
static Telem_Cmd cmd;

static portTASK_FUNCTION(vTelemDemoTask, pvParameters);

static inline void
vTelemPrintMenu(void)
{
	vDebugPrint(Telem_TaskToken, "Telemetry Demo Started!\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
	vDebugPrint(Telem_TaskToken, "--------------------------------------\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
	vDebugPrint(Telem_TaskToken, "(2): Set Sweep (resolution[arg1], rate[arg2])\n\r", NO_INSERT,
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

static UnivRetCode
enTelemServiceMessageSend(TaskToken taskToken)
{
    MessagePacket outgoing_packet;

    //create packet with printing request information
    outgoing_packet.Src         = enGetTaskID(taskToken);
    outgoing_packet.Dest        = TASK_TELEM;
    outgoing_packet.Token       = taskToken;
    outgoing_packet.Data        = (unsigned portLONG)&cmd;
    //store message in a struct and tag along with the request packet

    return enProcessRequest(&outgoing_packet, portMAX_DELAY);
}

static void
vTelemReadSweep(void *buffer, TaskToken token)
{
    cmd.operation = READSWEEP;
    cmd.size = TELEM_DEMO_MAX127_COUNT * TELEM_DEMO_MAX127_SENSOR_COUNT;
    /* Pass in the data for processing. */
    cmd.buffer = (sensor_result*)buffer;
    enTelemServiceMessageSend(token);
}

/* This sets all the entities groups to the same setting. */
static void
vTelemSampleSetSweep(int resolution, int rate, TaskToken token)
{
    int i;
    cmd.operation = SETSWEEP;
    cmd.size = SET_SWEEP_MESSAGE_ARG_NUM;
    for (i = 0; i < MAX_NUM_GROUPS; ++i)
    {
        current_setting[i].resolution = resolution;
        current_setting[i].rate = rate;
    }

    cmd.paramBuffer = current_setting;
    enTelemServiceMessageSend(token);
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

	vDebugPrint(Telem_TaskToken, "%s", (unsigned portLONG)TELEM_CLEAR_SCREEN,
			NO_INSERT, NO_INSERT);

	while (1)
	{
	    //vTelemPrintMenu();
		/* Bzero the input buffer. */
		memset((char*)inputBuffer, 0, TELEM_INPUT_BUFFER_SIZE);

		usReadLen = usDebugRead(inputBuffer, TELEM_INPUT_BUFFER_SIZE);
		if (usReadLen > TELEM_INPUT_BUFFER_SIZE) continue;

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
				vDebugPrint(Telem_TaskToken, "In Set Sweep | command[1] %d | command[2] %d\n\r",
						(unsigned portLONG)commandBuffer[1], (unsigned portLONG)commandBuffer[2],
						NO_INSERT);

				vTelemSampleSetSweep(commandBuffer[1], commandBuffer[2], Telem_TaskToken);
				break;
			case TELEM_DEMO_READ_SWEEP:
				vDebugPrint(Telem_TaskToken, "In Read Sweep\n\r", NO_INSERT,
							NO_INSERT, NO_INSERT);
				vTelemReadSweep((void*)current_data, Telem_TaskToken);
				break;
			case TELEM_DEMO_PRINT_LATEST_DATA:
				vDebugPrint(Telem_TaskToken, "In Print Data\n\r", NO_INSERT,
							NO_INSERT, NO_INSERT);
				telem_debug_print();
				break;
			default:
				/* Should never get here. */
				break;
		}
	}
}
