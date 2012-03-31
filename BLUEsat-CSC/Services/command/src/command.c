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

static xQueueHandle xTaskQueueHandles[NUM_TASKID];
static struct taskToken TaskTokens[NUM_TASKID];

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
				(portCHAR*)&usIndex,
				TYPE_SERVICE, 
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

		if (xResult == pdTRUE)
		{
			if (incoming_packet.Dest == TASK_COMMAND)
			{
				//TODO msg for command task
			}
			else if (incoming_packet.Dest < NUM_TASKID && xTaskQueueHandles[incoming_packet.Dest] != NULL)
			// forward msg to destination task Q
			{
				xResult = xQueueSend(xTaskQueueHandles[incoming_packet.Dest], &incoming_packet, NO_BLOCK);

				if (xResult != pdTRUE)
				{
					//return request fail
					vCompleteRequest(incoming_packet.Token, URC_FAIL);
				}
			}
			else
			{
				//TODO return task missing msg
			}
		}
	}
}

UnivRetCode enProcessRequest (MessagePacket *pMessagePacket, portTickType block_time)
{
	//insert quest into command task queue
	switch (xQueueSend(xTaskQueueHandles[TASK_COMMAND], pMessagePacket, block_time))
	{
		//put request task into sleep
		case pdTRUE	:	xSemaphoreTake((pMessagePacket->Token)->TaskSemphr, portMAX_DELAY);
						//return processed request result
						return (pMessagePacket->Token)->enRetVal;
		default		:	return URC_FAIL;
	}
}

UnivRetCode enGetRequest (TaskToken taskToken,
						MessagePacket *pMessagePacket,
						portTickType block_time)
{
	//retrieve request from queue and copy into given buffer
	switch (xQueueReceive(xTaskQueueHandles[taskToken->enTaskID], pMessagePacket, block_time))
	{
		case pdTRUE	:	return URC_SUCCESS;
		default		:	return URC_FAIL;
	}
}

void vCompleteRequest(TaskToken taskToken, UnivRetCode enRetVal)
{
	//store result value inside request task token
	taskToken->enRetVal = enRetVal;
	//wake request task from sleep
	xSemaphoreGive(taskToken->TaskSemphr);
}

TaskToken ActivateTask(TaskID 		enTaskID,
						const portCHAR 	*pcTaskName,
						TASK_TYPE 	enTaskType,
						unsigned 	portBASE_TYPE uxPriority,
						unsigned 	portSHORT usStackSize,
						pdTASK_CODE pvTaskFunction)
{
	//create task in memory
	xTaskCreate(pvTaskFunction, (signed portCHAR *)pcTaskName, usStackSize, NULL, uxPriority, NULL);
	//store task profile in array
	TaskTokens[enTaskID].pcTaskName		= (portCHAR *)pcTaskName;
	TaskTokens[enTaskID].enTaskType		= enTaskType;
	TaskTokens[enTaskID].enTaskID		= enTaskID;
	//create semaphore for task
	vSemaphoreCreateBinary(TaskTokens[enTaskID].TaskSemphr);
	//exhuast task semaphore
	xSemaphoreTake(TaskTokens[enTaskID].TaskSemphr, NO_BLOCK);
	
	//return pointer to taks profile
	return &TaskTokens[enTaskID];
}

void vActivateQueue(TaskToken taskToken, unsigned portSHORT usNumElement)
{
	//create task queue memory
	xTaskQueueHandles[taskToken->enTaskID] = xQueueCreate(usNumElement, sizeof(MessagePacket));
}

portCHAR *pcGetTaskName(TaskToken taskToken)
{
	//get task name from profile
	return taskToken->pcTaskName;
}

TaskID enGetTaskID(TaskToken taskToken)
{
	//get task ID from profile
	return taskToken->enTaskID;
}
