/**
 * command.c - Central command task for message forwarding,
 * 			   task creation and message queue creation.
 *
 *  Created by: James Qin
 */


#include "service.h"
#include "task.h"
#include "queue.h"
#include "command.h"

#define CMD_Q_SIZE			1
#define CMD_PUSH_BLK_TIME	0

static xQueueHandle xTaskQueueHandles[NUM_TASKID];
static struct taskToken TaskTokens[NUM_TASKID];

static portTASK_FUNCTION(vCommandTask, pvParameters);

void vCommand_Init(unsigned portBASE_TYPE uxPriority)
{
	unsigned portSHORT usIndex;

	for (usIndex = 0; usIndex < NUM_TASKID; usIndex++)
	{
		xTaskQueueHandles[usIndex]		= NULL;
		TaskTokens[usIndex].pcTaskName	= NULL;
		TaskTokens[usIndex].enRetVal	= 0;
	}

	ActivateTask(TASK_COMMAND, 
				(const signed char *)"Command", 
				TYPE_SERVICE, 
				uxPriority, 
				SERV_STACK_SIZE, 
				vCommandTask);

	vActivateQueue(&TaskTokens[TASK_COMMAND], CMD_Q_SIZE, sizeof(Cmd_Message));
}

static portTASK_FUNCTION(vCommandTask, pvParameters)
{
	(void) pvParameters;
	signed portBASE_TYPE xResult;
	Cmd_Message incoming_message;

	for ( ; ; )
	{
		xResult = xQueueReceive(xTaskQueueHandles[TASK_COMMAND], &incoming_message, portMAX_DELAY);

		if (xResult == pdTRUE)
		{
			if (incoming_message.Dest == TASK_COMMAND)
			{
				//TODO msg for command task
			}
			else if (incoming_message.Dest < NUM_TASKID && xTaskQueueHandles[incoming_message.Dest] != NULL)
			// forward msg to destination task Q
			{
				xResult = xQueueSend(xTaskQueueHandles[incoming_message.Dest], &incoming_message, NO_BLOCK);

				if (xResult != pdTRUE)
				{
					vCompleteRequest(incoming_message.Token, URC_FAIL);
				}
			}
			else
			{
				//TODO return task missing msg
			}
		}
	}
}

UnivRetCode enCommand_Push (Cmd_Message *pMessage, portTickType block_time)
{
	switch (xQueueSend(xTaskQueueHandles[TASK_COMMAND], pMessage, block_time))
	{
		case pdTRUE	:	xSemaphoreTake((pMessage->Token)->TaskSemphr, portMAX_DELAY);
						return (pMessage->Token)->enRetVal;
		default		:	return URC_FAIL;
	}
}

UnivRetCode enGet_Message (TaskToken taskToken,
									void *pMessageBuffer,
									portTickType block_time)
{
	switch (xQueueReceive(xTaskQueueHandles[taskToken->enTaskID], pMessageBuffer, block_time))
	{
		case pdTRUE	:	return URC_SUCCESS;
		default		:	return URC_FAIL;
	}
}

void vCompleteRequest(TaskToken taskToken, UnivRetCode enRetVal)
{
	taskToken->enRetVal = enRetVal;
	xSemaphoreGive(taskToken->TaskSemphr);
}

TaskToken ActivateTask(TaskID enTaskID,
						const signed portCHAR* const pcTaskName,
						TASK_TYPE enTaskType,
						unsigned portBASE_TYPE uxPriority,
						unsigned portSHORT usStackSize,
						pdTASK_CODE pvTaskFunction)
{
	xTaskCreate(pvTaskFunction, pcTaskName, usStackSize, NULL, uxPriority, NULL);

	TaskTokens[enTaskID].pcTaskName		= pcTaskName;
	TaskTokens[enTaskID].enTaskType		= enTaskType;
	TaskTokens[enTaskID].enTaskID		= enTaskID;
	vSemaphoreCreateBinary(TaskTokens[enTaskID].TaskSemphr);
	xSemaphoreTake(TaskTokens[enTaskID].TaskSemphr, NO_BLOCK);

	return &TaskTokens[enTaskID];
}

void vActivateQueue(TaskToken taskToken,
					unsigned portSHORT usNumElement,
					unsigned portSHORT usElementSize)
{
	xTaskQueueHandles[taskToken->enTaskID] = xQueueCreate(usNumElement, usElementSize);
}
