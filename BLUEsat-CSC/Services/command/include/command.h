/*
 * command.h - Central command task for message forwarding,
 * 			   task creation and message queue creation.
 *
 *  Created by: James Qin
 */

#ifndef COMMAND_H_
#define COMMAND_H_

typedef enum
{
	TASK_COMMAND,
	NUM_TASKID
} TaskID;

typedef enum
{
	APPLICATION,
	SERVICE
} TASK_TYPE;

#ifdef SERVICE_H_
	struct taskToken
	{
		const signed portCHAR 	*pcTaskName;
		TASK_TYPE				enTaskType;
		TaskID					enTaskID;
	};
#endif

typedef struct taskToken *TaskToken;

#define CMD_MSG_SIZE_WORD 	2

typedef struct
{
	unsigned portCHAR 	Src;
	unsigned portCHAR 	Dest;
	unsigned portSHORT 	Length;
	unsigned portLONG 	Msg[CMD_MSG_SIZE_WORD];	//can casted into a struct
} Cmd_Message;

void vCommand_Init( unsigned portBASE_TYPE uxPriority );

signed portBASE_TYPE xCommand_Push (Cmd_Message *pMessage, portTickType block_time);

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
