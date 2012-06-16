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

#ifdef I2C_H_
	Comms_I2C_Init();
#endif

#ifdef GPIO_H_
	Gpio_Init();
#endif

#ifdef SWITCHING_H_
	Switching_Init();
#endif

#ifdef IAP_H_
	//Internal Flash Memory driver (IAP)
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
	vCommand_Init(SERV_TASK_PRIORITY + 1);
#endif

#ifdef DEBUG_H_
	//debug task
	vDebug_Init(SERV_TASK_PRIORITY);
#endif

/*#ifdef MEMORY_H_
	//memory task
	vMemory_Init(SERV_TASK_PRIORITY);
#endif*/

#ifdef BEACON_H_
	vBeacon_Init(SERV_TASK_PRIORITY + 1);
#endif

#ifdef TELEM_H_
	vTelem_Init(SERV_TASK_PRIORITY);
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

#ifdef MEMORYDEMO_H_
	//Demonstration application memory
	vMemDemo_Init(APP_TASK_PRIORITY);
#endif

#ifdef GPIODEMO_H_
	vGpioDemo_Init(APP_TASK_PRIORITY);
#endif

	return 0;
}
