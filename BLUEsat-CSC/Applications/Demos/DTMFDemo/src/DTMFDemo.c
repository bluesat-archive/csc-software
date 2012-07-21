 /**
 *  \file DTMFDemo.c
 *
 *  \brief An application demonstrating DTMF
 *
 *  \author $Author: Sam Jiang $
 *  \version 1.0
 *
 *  $Date: 2012-07-21 16:35:54 +1100 (Sun, 02 June 2012) $
 *  \warning No Warnings for now
 *  \bug No Bugs for now
 *  \note No Notes for now
 */

#include "application.h"
#include "DTMFDemo.h"
#include "debug.h"
#include "DTMF_Common.h"

static TaskToken DEMO_TaskToken;
static portTASK_FUNCTION(vDemoTask, pvParameters);
void Comms_DTMF_Read(DtmfTone *elem, int *new, portTickType xBlockTime);
int iSendData(TaskToken token, char *data, int size);

void vDTMFDemo_Init(unsigned portBASE_TYPE uxPriority)
{
	DEMO_TaskToken = ActivateTask(TASK_DTMF_DEMO,
								"DTMF_demo",
								APP_TASK_TYPE,
								uxPriority,
								APP_STACK_SIZE,
								vDemoTask);
}

static portTASK_FUNCTION(vDemoTask, pvParameters)
{
	(void) pvParameters;
	DtmfTone DTMF_elem;
	int newData = 0;
	char data;

	while(1){
		Comms_DTMF_Read(&DTMF_elem,&newData,0);
		if (newData == 1){
			vDebugPrint(DEMO_TaskToken,
									"tone:%d,decoder:%d\n\r",
									(int) DTMF_elem.tone,
									(int) DTMF_elem.decoder,

									NO_INSERT);
			data = DTMF_elem.tone;
			iSendData(DEMO_TaskToken,&data,1);
			newData = 0;
		}
	}
}
