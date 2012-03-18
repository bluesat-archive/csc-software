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

	#ifdef EMC_H_
		EMC_Init();
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


#define DEMOAPP_1_TASK_PRIORITY		20
#define DEMOAPP_2_TASK_PRIORITY		20

unsigned int initApplications(void)
{
	#ifdef DEMO_APPLICATION_1_H_
		vDemoApp1_Init(DEMOAPP_1_TASK_PRIORITY);
	#endif

	#ifdef DEMO_APPLICATION_2_H_
		vDemoApp2_Init(DEMOAPP_2_TASK_PRIORITY);
	#endif

	return 0;
}
