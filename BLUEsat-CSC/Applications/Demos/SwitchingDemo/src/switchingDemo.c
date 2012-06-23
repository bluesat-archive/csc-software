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

		if ((pcInputBuf[0] == '0'))
		{
			switching_OPMODE(DEVICE_MODE);
		} else if ((pcInputBuf[0] == '1'))
		{
			switch (pcInputBuf[1])
			{
				case '0':
					switching_TX_Device(BEACON);
					break;
				case '1':
					switching_TX_Device(AFSK_1);
					break;
				case '2':
					switching_TX_Device(AFSK_2);
					break;
				case '3':
					switching_TX_Device(GMSK_1);
					break;
				case '4':
					switching_TX_Device(GMSK_2);
					break;
				default:
					switching_OPMODE(REPEATER_MODE);
			}
		} else if ((pcInputBuf[0] == '2'))
		{
			switching_TX(TX_1);
		} else if ((pcInputBuf[0] == '3'))
		{
			switching_TX(TX_2);
		} else if ((pcInputBuf[0] == '4'))
		{
			switching_RX(RX_1);
		} else if ((pcInputBuf[0] == '5'))
		{
			switching_RX(RX_2);
		} else if ((pcInputBuf[0] == '6'))
		{
			switching_RX_Device(AFSK_1);
		} else if ((pcInputBuf[0] == '7'))
		{
			switching_RX_Device(AFSK_2);
		} else if ((pcInputBuf[0] == '8'))
		{
			switching_RX_Device(GMSK_1);
		} else if ((pcInputBuf[0] == '9'))
		{
			switching_RX_Device(GMSK_2);
		}
	}
}

#define MENU_L00 " ----------------------------------- Menu -----------------------------------\n\r
#define MENU_L01 0\t- DEVICE MODE\n\r
#define MENU_L02 1\t- REPEATER MODE\n\r
#define MENU_L03 2\t- Read Data @@(DID), $$$$(Size), %%%%(offset)\n\r
#define MENU_L04 3\t- Check Size @@(DID)\n\r
#define MENU_L05 4\t- Delete slot @@(DID)\n\r
#define MENU_L06 5\t- Format\n\r
#define MENU_L07 6\t- Print FMT Table\n\r
#define MENU_L08 7\t- Display this menu\n\r
#define MENU_L09 8\t- Display this menu\n\r
#define MENU_L10 9\t- Display this menu\n\r
#define MENU_L11 10\t- Display this menu\n\r
#define MENU_L12 11\t- Display this menu\n\r
#define MENU_L13 12\t- Display this menu\n\r
#define MENU_L14 13\t- Display this menu\n\r
#define MENU_L15 14\t- Display this menu\n\r
#define MENU_L16 ----------------------------------------------------------------------------\n\r"
#define MENU MENU_L00 MENU_L01 MENU_L02 MENU_L03 MENU_L04 MENU_L05 MENU_L06 MENU_L07 MENU_L08 MENU_L09 MENU_L10 MENU_L11 MENU_L12 MENU_L13 MENU_L14 MENU_L15 MENU_L16

void vPrintMenu(void)
{
	vDebugPrint(DEMO_TaskToken,
				"\n\r%1000s\n\r",
				(unsigned portLONG)MENU,
				NO_INSERT,
				NO_INSERT);
}
