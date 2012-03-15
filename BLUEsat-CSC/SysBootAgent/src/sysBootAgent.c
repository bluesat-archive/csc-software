/*
 *
 *  sysBootAgent.c - System component initialisation agent
 *
 *  Created by: James Qin
 */

#include "FreeRTOS.h"
#include "sysBootAgent.h"
#include "ActiveModuleIncList.h"

unsigned int initDrivers(void)
{
	#ifdef UART_H_
		Comms_UART_Init();
	#endif

	return 0;
}

#define COMMAND_TASK_PRIORITY	30
#define DEBUG_TASK_PRIORITY		25

unsigned int initServices(void)
{
	#ifdef COMMAND_H_
		vCommand_Init(COMMAND_TASK_PRIORITY);
	#endif

	#ifdef DEBUG_H_
		vDebug_Init(DEBUG_TASK_PRIORITY);
	#endif

	return 0;
}

unsigned int initApplications(void)
{
	return 0;
}
