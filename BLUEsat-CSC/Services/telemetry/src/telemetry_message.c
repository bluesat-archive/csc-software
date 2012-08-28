/*
 * telemetry_message.c
 * Telemetry message service
 * Created on: 04/08/2012
 * Author: andyc
 */

#include "service.h"
#include "telemetry_message.h"

#define TELEM_MESSAGE_QUEUE_SIZE 16

TaskToken telemMessageTaskToken;


/*--------------------------Telemetry message internal implementation--------------------------*/
/*
 * Telemetry message task main function.
 */
static portTASK_FUNCTION(vTelemMessageTask, pvParameters)
{
    (void) pvParameters;

    for (;;)
    {

    }
}

/*--------------------------Telemetry message public interfaces--------------------------*/

UnivRetCode vTelemMessageInit(unsigned portBASE_TYPE uxPriority)
{
    telemMessageTaskToken = ActivateTask(TASK_TELEM,
                                "Telem",
                                SEV_TASK_TYPE,
                                uxPriority,
                                SERV_STACK_SIZE,
                                vTelemMessageTask);

    vActivateQueue(telemMessageTaskToken, TELEM_MESSAGE_QUEUE_SIZE);

    return URC_SUCCESS;
}
