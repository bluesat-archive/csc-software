 /**
 *  \file command.h
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

#ifndef COMMAND_H_
#define COMMAND_H_

#include "UniversalReturnCode.h"

//list of tasks
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

//task type identifier
typedef enum
{
	TYPE_APPLICATION,
	TYPE_SERVICE
} TASK_TYPE;

#define TASK_NAME_MAX_CHAR	10

#ifdef SERVICE_H_
	//definition of TaskToken
	//only exposed to services
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

//general message packet format for IPC
typedef struct
{
	MESSAGE_HEADER;
	unsigned portLONG 	Data;	//can be used as a pointer
} MessagePacket;

/**
 * \brief Initialise command service
 *
 * \param[in] uxPriority Priority for debug service.
 */
void vCommand_Init(unsigned portBASE_TYPE uxPriority);

/**
 * \brief Attempt to process request & put request task to sleep
 *
 * \param[in] pMessagePacket Pointer to message packet to be processed.
 *
 * \param[in] block_time Time to wait when inserting into command service queue (ms).
 *			
 * \returns enum Containing the processed result
 */
UnivRetCode enProcessRequest (MessagePacket *pMessagePacket, portTickType block_time);

/**
 * \brief Attempt to retrieve requestion from queue
 *
 * \param[in] taskToken Task token of request task.
 *
 * \param[out] pMessagePacket Pointer to message packet buffer.
 *
 * \param[in] block_time Time to wait when retrieving item from queue (ms).
 *			
 * \returns SUCCESS or FAIL
 */
UnivRetCode enGetRequest (TaskToken taskToken,
						MessagePacket *pMessagePacket,
						portTickType block_time);

/**
 * \brief Store result and wake request task
 *
 * \param[in] taskToken Task token of request task.
 *
 * \param[in] enRetVal Result value.
 */						
void vCompleteRequest(TaskToken taskToken, UnivRetCode enRetVal);

/**
 * \brief Create task and create task profile
 *
 * \param[in] enTaskID Task identifier.
 *
 * \param[in] pcTaskName Task name.
 *
 * \param[in] enTaskType Task type Application or Service.
 *
 * \param[in] uxPriority Priority for debug service.
 *
 * \param[in] usStackSize Task stack size.
 *
 * \param[in] pvTaskFunction Task function.
 *			
 * \returns Task token for accessing services and applications
 */

TaskToken ActivateTask(TaskID enTaskID,
						const signed portCHAR* const pcTaskName,
						TASK_TYPE enTaskType,
						unsigned portBASE_TYPE uxPriority,
						unsigned portSHORT usStackSize,
						pdTASK_CODE pvTaskFunction);

/**
 * \brief Create queue for taskToken owner
 *
 * \param[in] taskToken Task token of a task.
 *
 * \param[in] usNumElement Number of element in queue
 */
void vActivateQueue(TaskToken taskToken, unsigned portSHORT usNumElement);

/**
 * \brief Return Task token owner's task name
 *
 * \param[in] taskToken Task token of a task.
 *			
 * \returns Task name
 */
const signed portCHAR *pcGetTaskName(TaskToken taskToken);

/**
 * \brief Return Task token owner's taskID
 *
 * \param[in] taskToken Task token of a task.
 *			
 * \returns Task name
 */
TaskID enGetTaskName(TaskToken taskToken);

#endif /* COMMAND_H_ */
