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
	};
#endif

typedef struct taskToken *TaskToken;

#define CALLBACK_FUNCTION(vFunction, xReturnStatus) void vFunction(signed portBASE_TYPE xReturnStatus)

typedef void (*CALLBACK)(signed portBASE_TYPE xReturnStatus);

#define MESSAGE_HEADER struct {unsigned portCHAR Src; unsigned portCHAR Dest; CALLBACK CallBackFunc; unsigned portSHORT Length;}

#define CMD_MSG_SIZE_WORD 	2

typedef struct
{
	MESSAGE_HEADER;
	unsigned portLONG 	Msg[CMD_MSG_SIZE_WORD];	//can casted into a struct
} Cmd_Message;

void vCommand_Init(unsigned portBASE_TYPE uxPriority);

signed portBASE_TYPE xCommand_Push (Cmd_Message *pMessage, portTickType block_time);

signed portBASE_TYPE xGet_Message (TaskToken taskToken,
									void *pMessageBuffer,
									portTickType block_time);

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
