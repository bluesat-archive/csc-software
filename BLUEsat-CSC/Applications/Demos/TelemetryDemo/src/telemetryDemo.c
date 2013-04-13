#include "application.h"
#include "telemetry.h"
#include "telemetryDemo.h"
#include "debug.h"
#include "UniversalReturnCode.h"
#include "lib_string.h"

#define INPUT 0
#define OUTPUT 1
#define TELEM_INPUT_BUFFER_SIZE 10
#define TELEM_CLEAR_SCREEN "\e[2J\e[1;1H"

static TaskToken telemDemoTaskToken;
static telem_command_t command;

static portTASK_FUNCTION(vTelemDemoTask, pvParameters);

static inline UnivRetCode
enTelemMessageSend(TaskToken taskToken)
{
    return enTelemServiceMessageSend(taskToken, (unsigned portLONG)&command);
}

static void
vTelemPrintMenu(void)
{
	vDebugPrint(telemDemoTaskToken, "Telemetry Demo Started!\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
	vDebugPrint(telemDemoTaskToken, "--------------------------------------\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
	vDebugPrint(telemDemoTaskToken, "(1): Read single sensor (index[arg0])\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
	vDebugPrint(telemDemoTaskToken, "(2): Read latest sensor (no arg)\n\r", NO_INSERT,
				NO_INSERT, NO_INSERT);
}

static void
vTelemConvertCommand(char *inputBuf, unsigned int *outputBuf, int size)
{
	int i;
	for (i = 0; i < size; ++i) outputBuf[i] = inputBuf[i] - '0';
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

static void
vTelemReadSingle(int index, struct telem_demo_storage_entry_t *entry,
        TaskToken token)
{
    command.operation = TELEM_READ_SINGLE;
    command.index = index;
    command.buffer = (char*)entry;
    command.size = sizeof(struct telem_demo_storage_entry_t);
    enTelemMessageSend(token);
}

static void
vTelemReadLatest(struct telem_demo_storage_entry_t *entry,
        TaskToken token)
{
    command.operation = TELEM_READ_LATEST;
    command.buffer = (char*)entry;
    command.size = sizeof(struct telem_demo_storage_entry_t);
    enTelemMessageSend(token);
}

static
portTASK_FUNCTION(vTelemDemoTask, pvParameters)
{
	(void) pvParameters;
	char inputBuffer[TELEM_INPUT_BUFFER_SIZE];
	unsigned int usReadLen;
	unsigned int commandBuffer[TELEM_INPUT_BUFFER_SIZE - 1];
	struct telem_demo_storage_entry_t entry;

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
		 */
		switch (commandBuffer[0])
		{
			case TELEM_DEMO_READ_SINGLE:
				vDebugPrint(telemDemoTaskToken, "Read single -(index) %d\n\r",
						(unsigned portLONG)commandBuffer[1], NO_INSERT, NO_INSERT);

				vTelemReadSingle(commandBuffer[1], &entry, telemDemoTaskToken);
				break;
			case TELEM_DEMO_READ_LATEST:
				vDebugPrint(telemDemoTaskToken, "Read latest\n\r", NO_INSERT,
							NO_INSERT, NO_INSERT);
				vTelemReadLatest(&entry, telemDemoTaskToken);
				break;
			default:
				/* Should never get here. */
				break;
		}
	}
}
