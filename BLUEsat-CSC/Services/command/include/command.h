/*
 * command.h - Central command task for message forwarding,
 * 			   task creation and message queue creation.
 *
 *  Created by: James Qin
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include "UniversalReturnCode.h"

typedef enum
{
	TASK_COMMAND,
	TASK_DEBUG,
	/* DEMO APP TASK */
	TASK_DEMO_APP_1,
	TASK_DEMO_APP_2,
	/*****************/
	NUM_TASKID
} TaskID;

typedef enum
{
	TYPE_APPLICATION,
	TYPE_SERVICE
} TASK_TYPE;

#define TASK_NAME_MAX_CHAR	10

#ifdef SERVICE_H_
	struct taskToken
	{
		const signed portCHAR 	*pcTaskName;
		TASK_TYPE				enTaskType;
		TaskID					enTaskID;
		xSemaphoreHandle		TaskSemphr;
		UnivRetCode				enRetVal;
	};
#endif

typedef struct taskToken *TaskToken;

#define MESSAGE_HEADER struct {unsigned portCHAR Src; unsigned portCHAR Dest; TaskToken Token; unsigned portSHORT Length;}

typedef struct
{
	MESSAGE_HEADER;
	unsigned portLONG 	Msg;	//can be used as a pointer
} Cmd_Message;

void vCommand_Init(unsigned portBASE_TYPE uxPriority);

UnivRetCode enCommand_Push (Cmd_Message *pMessage, portTickType block_time);

UnivRetCode enGet_Message (TaskToken taskToken,
						void *pMessageBuffer,
						portTickType block_time);

void vCompleteRequest(TaskToken taskToken, UnivRetCode enRetVal);

TaskToken ActivateTask(TaskID enTaskID,
						const signed portCHAR* const pcTaskName,
						TASK_TYPE enTaskType,
						unsigned portBASE_TYPE uxPriority,
						unsigned portSHORT usStackSize,
						pdTASK_CODE pvTaskFunction);

void vActivateQueue(TaskToken taskToken,
					unsigned portSHORT usNumElement,
					unsigned portSHORT usElementSize);

#endif /* COMMAND_H_ */
