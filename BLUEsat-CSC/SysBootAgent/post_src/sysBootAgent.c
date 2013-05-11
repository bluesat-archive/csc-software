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
    rtc_time_t time;

#ifdef GPIO_H_
	Gpio_Init();
#endif//

#ifdef UART_H_
	//UART driver
	Comms_UART_Init();
#endif

#ifdef EMC_H_
	//External Memory Controller driver
	vEMC_Init();
#endif//

#ifdef I2C_H_
	Comms_I2C_Init();
#endif//*/

#ifdef SWITCHING_H_
	Switching_Init();
#endif//

#ifdef MODEM_H_
	//Comms_Modem_Timer_Init();
#endif//

#ifdef IAP_H_
	//Internal Application Programming (IAP)
	/* NO INITIALISATION REQUIRED */
#endif//

#ifdef RTC_H_
	//RTC clock
	rtc_init();
    time.rtcSec = 0;
    time.rtcMin = 0;
    time.rtcHour = 0;
    time.rtcMday = 1;
    time.rtcMon = 1;
    time.rtcYear = 1337;
    rtc_set_current_time(time);
    rtc_start();
#endif

#ifdef WATCHDOG_H_
    watchdog_init();
#endif

	return 0;
}

//Priority used for all service tasks
#define SERV_TASK_PRIORITY	30

unsigned int initServices(void)
{

#ifdef MEMORY_H_
	//memory task
	//vMemory_Init(SERV_TASK_PRIORITY);
#endif//

#ifdef COMMAND_H_
	//Command task
	vCommand_Init(SERV_TASK_PRIORITY + 1);
#endif

#ifdef STORAGE_H_
	//memory task
	//vStorage_Init(SERV_TASK_PRIORITY);
#endif//*/

#ifdef DEBUG_H_
	//debug task
	vDebug_Init(SERV_TASK_PRIORITY);
#endif//*/

#ifdef BEACON_H_
	//vBeacon_Init(SERV_TASK_PRIORITY + 1);
#endif//

#ifdef TELEMETRY_H_
	vTelemInit(SERV_TASK_PRIORITY);
#endif

#ifdef COMMS_DTMF_H_
	//Comms_DTMF_Init();
#endif

#ifdef COMMS_H_
	//vComms_Init(SERV_TASK_PRIORITY);
#endif
	return 0;
}

//Priority used for all application tasks
#define APP_TASK_PRIORITY		25

unsigned int initApplications(void)
{

#ifdef DEMO_APPLICATION_1_H_
	//Demonstration application 1
	//vDemoApp1_Init(APP_TASK_PRIORITY);
#endif

#ifdef DEMO_APPLICATION_2_H_
	//Demonstration application 2
	//vDemoApp2_Init(APP_TASK_PRIORITY);
#endif

#ifdef MEMORY_DEMO_H_
	//Demonstration additional volatile memory and stack memory usage
	//vMemoryDemo_Init(APP_TASK_PRIORITY);
#endif

#ifdef STORAGE_DEMO_H_
	//Demonstration CSC storage
	//vStorageDemo_Init(APP_TASK_PRIORITY);
#endif

#ifdef GPIODEMO_H_
	//vGpioDemo_Init(APP_TASK_PRIORITY);
#endif

#ifdef SWITCHINGDEMO_H_
//	vSwitchingDemo_Init(APP_TASK_PRIORITY);
#endif

#ifdef MODEMDEMO_H_
	//vModemDemo_Init(APP_TASK_PRIORITY);
#endif

#ifdef TELEMETRYDEMO_H_
	//vTelemetryDemoInit(APP_TASK_PRIORITY);
#endif

#ifdef DTMFDEMO_H_
//	vDTMFDemo_Init(APP_TASK_PRIORITY);
#endif

#ifdef COMMSDEMO_H_
	//vCommsDemo_Init(APP_TASK_PRIORITY);
#endif
	return 0;
}
