 /**
 *  \file command.c
 *
 *  \brief Provide Inter Process Communication (IPC)
 *
 *  \author $Author: James Qin $
 *  \version 1.0
 *
 *  $Date: 2010-10-24 23:35:54 +1100 (Sun, 24 Oct 2010) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */


#include "service.h"
#include "task.h"
#include "queue.h"
#include "command.h"

#define CMD_Q_SIZE			1
#define CMD_PUSH_BLK_TIME	0

#define MAX_QUEUE_REQ_SIZE	10

static xQueueHandle 	xTaskQueueHandles	[NUM_TASKID];
static struct taskToken TaskTokens			[NUM_TASKID];
static xSemaphoreHandle	TaskSemphrs			[NUM_TASKID];
static xTaskHandle 		TaskHandles			[NUM_TASKID];

static portTASK_FUNCTION(vCommandTask, pvParameters);

void vCommand_Init(unsigned portBASE_TYPE uxPriority)
{
	unsigned portSHORT usIndex;

	//initialise management arrary
	for (usIndex = 0; usIndex < NUM_TASKID; usIndex++)
	{
		xTaskQueueHandles[usIndex]		= NULL;
		TaskTokens[usIndex].pcTaskName	= NULL;
		TaskTokens[usIndex].enRetVal	= 0;
	}

	ActivateTask(TASK_COMMAND, 
				"Command",
				SEV_TASK_TYPE,
				uxPriority, 
				SERV_STACK_SIZE, 
				vCommandTask);

	vActivateQueue(&TaskTokens[TASK_COMMAND], CMD_Q_SIZE);
}

static portTASK_FUNCTION(vCommandTask, pvParameters)
{
	(void) pvParameters;
	signed portBASE_TYPE xResult;
	MessagePacket incoming_packet;

	for ( ; ; )
	{
		xResult = xQueueReceive(xTaskQueueHandles[TASK_COMMAND], &incoming_packet, portMAX_DELAY);

		if (xResult != pdTRUE) continue;

		if (incoming_packet.Dest == TASK_COMMAND)
		{
			//TODO msg for command task
		}
		else
		{
			//TODO log error for exceptions
			/***** exception check *****/
			if (incoming_packet.Dest >= NUM_TASKID)
			{
				//return invalid task
				vCompleteRequest(incoming_packet.Token, URC_CMD_INVALID_TASK);
				continue;
			}
			else if (TaskTokens[incoming_packet.Dest].pcTaskName == NULL)
			{
				//return no task
				vCompleteRequest(incoming_packet.Token, URC_CMD_NO_TASK);
				continue;
			}
			else if (xTaskQueueHandles[incoming_packet.Dest] == NULL)
			{
				//return no queue
				vCompleteRequest(incoming_packet.Token, URC_CMD_NO_QUEUE);
				continue;
			}
			/***************************/

			// forward msg to destination task Q
			xResult = xQueueSend(xTaskQueueHandles[incoming_packet.Dest], &incoming_packet, NO_BLOCK);

			if (xResult == pdTRUE) continue;

			//return request fail
			vCompleteRequest(incoming_packet.Token, URC_BUSY);
		}
	}
}

//TODO (1)review whether application specified block time be allowed while waiting for committed request
UnivRetCode enProcessRequest (MessagePacket *pMessagePacket, portTickType block_time)
{
	//catch NO message packet input input
	if (pMessagePacket == NULL) return URC_FAIL;

	//insert quest into command task queue
	if (xQueueSend(xTaskQueueHandles[TASK_COMMAND], pMessagePacket, block_time) == pdTRUE)
	{
		//put request task into sleep
		xSemaphoreTake(TaskSemphrs[(pMessagePacket->Token)->enTaskID], portMAX_DELAY);  //(1)
		//return processed request result
		return (pMessagePacket->Token)->enRetVal;
	}
	else
	{
		return URC_BUSY;
	}
}

UnivRetCode enGetRequest (TaskToken taskToken,
						MessagePacket *pMessagePacket,
						portTickType block_time)
{
	//catch NO token and NO message packet input input
	if (taskToken == NULL || pMessagePacket == NULL) return URC_FAIL;
	//catch NO queue
	if (xTaskQueueHandles[taskToken->enTaskID] == NULL) return URC_CMD_NO_QUEUE;

	//retrieve request from queue and copy into given buffer
	if (xQueueReceive(xTaskQueueHandles[taskToken->enTaskID], pMessagePacket, block_time) == pdTRUE)
	{
		return URC_SUCCESS;
	}
	else
	{
		return URC_FAIL;
	}
}

void vCompleteRequest(TaskToken taskToken, UnivRetCode enRetVal)
{
	//catch NO token input
	if (taskToken == NULL) return;
	//store result value inside request task token
	taskToken->enRetVal = enRetVal;
	//wake request task from sleep
	xSemaphoreGive(TaskSemphrs[taskToken->enTaskID]);
}

TaskToken ActivateTask(TaskID 		enTaskID,
						portCHAR 	*pcTaskName,
						TASK_TYPE 	enTaskType,
						unsigned 	portBASE_TYPE uxPriority,
						unsigned 	portSHORT usStackSize,
						pdTASK_CODE pvTaskFunction)
{
	TaskToken taskToken = NULL;

	//catch NO task name input
	if (pcTaskName == NULL) return NULL;

	taskENTER_CRITICAL();
	{
		//detect task already exist
		if (TaskTokens[enTaskID].pcTaskName == NULL)
		{
			//create task
			xTaskCreate(pvTaskFunction, (signed portCHAR *)pcTaskName, usStackSize, NULL, uxPriority, &TaskHandles[enTaskID]);

			//store task profile in array
			TaskTokens[enTaskID].pcTaskName		= pcTaskName;
			TaskTokens[enTaskID].enTaskType		= enTaskType;
			TaskTokens[enTaskID].enTaskID		= enTaskID;

			//create semaphore for task
			vSemaphoreCreateBinary(TaskSemphrs[enTaskID]);

			//exhaust task semaphore
			xSemaphoreTake(TaskSemphrs[enTaskID], NO_BLOCK);

			taskToken = &TaskTokens[enTaskID];
		}
	}
	taskEXIT_CRITICAL();
	
	//return pointer to task profile
	return taskToken;
}

unsigned portSHORT vActivateQueue(TaskToken taskToken, unsigned portSHORT usNumElement)
{
	taskENTER_CRITICAL();
	{
		//detect queue already exist
		if (xTaskQueueHandles[taskToken->enTaskID] == NULL)
		{
			//trim requested queue size
			usNumElement = (usNumElement > MAX_QUEUE_REQ_SIZE) ? MAX_QUEUE_REQ_SIZE : usNumElement;

			//create task queue memory
			xTaskQueueHandles[taskToken->enTaskID] = xQueueCreate(usNumElement, sizeof(MessagePacket));
		}
		else
		{
			//TODO work out how to return existing queue size
			usNumElement = 0;
		}
	}
	taskEXIT_CRITICAL();

	return usNumElement;
}

portCHAR *pcGetTaskName(TaskToken taskToken)
{
	//catch NO token input
	if (taskToken == NULL) return NULL;

	//get task name from profile
	return taskToken->pcTaskName;
}

TaskID enGetTaskID(TaskToken taskToken)
{
	//catch NO token input
	if (taskToken == NULL) return NO_TASK;

	//get task ID from profile
	return taskToken->enTaskID;
}

void vSleep(unsigned portSHORT usTimeMS)
{
	vTaskDelay( usTimeMS / portTICK_RATE_MS );
}

#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
	#include "debug.h"
	void vShowAllTaskUnusedStack(void)
	{
		unsigned portSHORT usIndex;

		vDebugPrint(&TaskTokens[TASK_COMMAND],
					"********** Unused Stack Table ***********\n\r",
					0, 0, 0);

		for (usIndex = 0; usIndex < NUM_TASKID; usIndex++)
		{
			if (TaskTokens[usIndex].pcTaskName != NULL)
			{
				vDebugPrint(&TaskTokens[TASK_COMMAND],
							"%16s\t\t%d\n\r",
							(unsigned portLONG)TaskTokens[usIndex].pcTaskName,
							(unsigned portLONG)uxTaskGetStackHighWaterMark(TaskHandles[usIndex]),0);
			}
		}
	}
#endif
