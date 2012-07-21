 /**
 *  \file modemDemo.c
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
#include "modemDemo.h"
#include "debug.h"
#include "lib_string.h"


static TaskToken DEMO_TaskToken;
void Comms_Modem_Write_Str( const portCHAR * const pcString, unsigned portSHORT usStringLength, portSHORT sel );
void setModemTransmit(portSHORT sel);
static portTASK_FUNCTION(vDemoTask, pvParameters);

void vModemDemo_Init(unsigned portBASE_TYPE uxPriority)
{
	DEMO_TaskToken = ActivateTask(TASK_MODEM_DEMO,
								"modem_demo",
								APP_TASK_TYPE,
								uxPriority,
								APP_STACK_SIZE,
								vDemoTask);
}

static portTASK_FUNCTION(vDemoTask, pvParameters)
{
	(void) pvParameters;
	portCHAR			pcInputBuf[10];
	unsigned portSHORT	usReadLen;

	setModemTransmit(1);
	for ( ; ; )
	{
		vDebugPrint(DEMO_TaskToken,
					"\n\rEnter message:\n\r",
					0,
					NO_INSERT,
					NO_INSERT);
		usReadLen = usDebugRead(pcInputBuf, 9);
		Comms_Modem_Write_Str(pcInputBuf,9, 1);
	}
}
