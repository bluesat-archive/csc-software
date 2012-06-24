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
	/* Task ID start */
	TASK_COMMAND,
	TASK_DEBUG,
	TASK_MEMORY,
	TASK_BEACON,
	TASK_TELEM,
	TASK_COMMS,
	TASK_MEM_INT_FLASH,
	TASK_MEM_FRAM,
	TASK_DEMO_APP_1,
	TASK_DEMO_APP_2,
	TASK_GPIO_DEMO,
	TASK_MEMORY_DEMO,
	TASK_SWITCHING_DEMO,
	TASK_MODEM_DEMO,
	/** Task ID end **/
	NUM_TASKID,		/* <--- task ID list size */
	/* Virtual task IDs */
	NO_TASK
} TaskID;

//task type identifier
typedef enum
{
	TYPE_APPLICATION,
	TYPE_SERVICE
} TASK_TYPE;

#define TASK_NAME_MAX_CHAR	16

#ifdef SERVICE_H_
	//definition of TaskToken
	//only exposed to services
	struct taskToken
	{
		portCHAR  				*pcTaskName;
		TASK_TYPE				enTaskType;
		TaskID					enTaskID;
		UnivRetCode				enRetVal;
	};
#endif

typedef struct taskToken *TaskToken;

//general message packet format for IPC
typedef struct
{
	TaskToken 			Token;
	TaskID 				Src;
	TaskID 				Dest;
	unsigned portLONG 	Data;	//can be used as a pointer
} MessagePacket;

#define NO_BLOCK			0

/**
 * \brief Initialise command service
 *
 * \param[in] uxPriority Priority for command service.
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

TaskToken ActivateTask(TaskID 		enTaskID,
						portCHAR 	*pcTaskName,
						TASK_TYPE 	enTaskType,
						unsigned 	portBASE_TYPE uxPriority,
						unsigned 	portSHORT usStackSize,
						pdTASK_CODE pvTaskFunction);

/**
 * \brief Create queue for taskToken owner
 *
 * \param[in] taskToken Task token of a task.
 *
 * \param[in] usNumElement Number of element in queue
 *
 * \returns Size of queue been created
 */
unsigned portSHORT vActivateQueue(TaskToken taskToken, unsigned portSHORT usNumElement);

/**
 * \brief Return Task token owner's task name
 *
 * \param[in] taskToken Task token of a task.
 *			
 * \returns Task name
 */
portCHAR *pcGetTaskName(TaskToken taskToken);

/**
 * \brief Return Task token owner's taskID
 *
 * \param[in] taskToken Task token of a task.
 *			
 * \returns Task name
 */
TaskID enGetTaskID(TaskToken taskToken);

/**
 * \brief Sleep for specified time
 *
 * \param[in] usTimeMS Time sleep in milliseconds
 */
void vSleep(unsigned portSHORT usTimeMS);

#if !defined(NO_DEBUG) && (INCLUDE_uxTaskGetStackHighWaterMark == 1)
	/**
	 * \brief Print unused stack size for all tasks
	 */
	void vShowAllTaskUnusedStack(void);
#endif

#endif /* COMMAND_H_ */
