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
	//UART driver
	Comms_UART_Init();
#endif

#ifdef EMC_H_
	//External Memory Controller driver
	EMC_Init();
#endif

#ifdef INTERNAL_FLASH_H_
	//Internal Flash Memory driver
	/* NO INITIALISATION REQUIRED */
#endif

	return 0;
}

//Priority used for all service tasks
#define SERV_TASK_PRIORITY	30

unsigned int initServices(void)
{
#ifdef COMMAND_H_
	//Command task
	vCommand_Init(SERV_TASK_PRIORITY);
#endif

#ifdef DEBUG_H_
	//debug task
	vDebug_Init(SERV_TASK_PRIORITY);
#endif

	return 0;
}

//Priority used for all application tasks
#define APP_TASK_PRIORITY		25

unsigned int initApplications(void)
{
#ifdef DEMO_APPLICATION_1_H_
	//Demonstration application 1
	vDemoApp1_Init(APP_TASK_PRIORITY);
#endif

#ifdef DEMO_APPLICATION_2_H_
	//Demonstration application 2
	vDemoApp2_Init(APP_TASK_PRIORITY);
#endif

	return 0;
}
