 /**
 *  \file switchingDemo.c
 *
 *  \brief An application demonstrating how an application operate
 *
 *  \author $Author: Sam Jiang $
 *  \version 1.0
 *
 *  $Date: 2012-06-23 13:35:54 +1100 (Sat, 23 Jun 2012) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#include "application.h"
#include "switchingDemo.h"
#include "debug.h"
#include "lib_string.h"

#define SWITCHING_NO_BLOCK 0

#define REPEATER_MODE 1
#define DEVICE_MODE 0

#define TX_1 0
#define TX_2 1

#define RX_1 0
#define RX_2 1

#define BEACON 0
#define AFSK_1 1
#define AFSK_2 2
#define GMSK_1 3
#define GMSK_2 4

void switching_TX(unsigned char TX);

void switching_RX(unsigned char RX);

void switching_OPMODE(unsigned char mode);

void switching_TX_Device(unsigned char device);

void switching_RX_Device(unsigned char device);
void vPrintMenu(void);

static TaskToken DEMO_TaskToken;

static portTASK_FUNCTION(vDemoTask, pvParameters);

void vSwitchingDemo_Init(unsigned portBASE_TYPE uxPriority)
{
	DEMO_TaskToken = ActivateTask(TASK_SWITCHING_DEMO,
								"gpio_demo",
								APP_TASK_TYPE,
								uxPriority,
								APP_STACK_SIZE,
								vDemoTask);
}

static portTASK_FUNCTION(vDemoTask, pvParameters)
{
	(void) pvParameters;
	portCHAR			pcInputBuf[3];
	unsigned portSHORT	usReadLen;

	vPrintMenu();

	for ( ; ; )
	{
		usReadLen = usDebugRead(pcInputBuf, 2);

		if ((pcInputBuf[0] == 'S' && pcInputBuf[1] == 'D') && usReadLen > 4)
		{

		}
	}
}

#define MENU_L00 " ----------------------------------- Menu -----------------------------------\n\r
#define MENU_L01 SD@@(Data)\t- Store Data @@(DID)\n\r
#define MENU_L02 AD@@(Data)\t- Append Data @@(DID)\n\r
#define MENU_L03 RD@@$$$$%%%%\t- Read Data @@(DID), $$$$(Size), %%%%(offset)\n\r
#define MENU_L04 CS@@\t\t- Check Size @@(DID)\n\r
#define MENU_L05 DD@@\t\t- Delete slot @@(DID)\n\r
#define MENU_L06 FF\t\t- Format\n\r
#define MENU_L07 PT\t\t- Print FMT Table\n\r
#define MENU_L08 MU\t\t- Display this menu\n\r
#define MENU_L09 Note: All value in decimal\n\r
#define MENU_L10 ----------------------------------------------------------------------------\n\r"
#define MENU MENU_L00 MENU_L01 MENU_L02 MENU_L03 MENU_L04 MENU_L05 MENU_L06 MENU_L07 MENU_L08 MENU_L09 MENU_L10

void vPrintMenu(void)
{
	vDebugPrint(DEMO_TaskToken,
				"\n\r%1000s\n\r",
				(unsigned portLONG)MENU,
				NO_INSERT,
				NO_INSERT);
}
