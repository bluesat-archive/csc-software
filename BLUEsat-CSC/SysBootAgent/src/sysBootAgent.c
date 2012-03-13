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

unsigned int initServices(void)
{
	return 0;
}

unsigned int initApplications(void)
{
	return 0;
}
