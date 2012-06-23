 /**
 *  \file beaconControl.h
 *
 *  \brief control task for beacon
 *  This task sends message to switching circuit control
 *  to switch the modem for beacon
 *
 *  \author $Author: andyc $
 *  \version 1.0
 *
 *  $Date: 2/6/2012
 */

#include "service.h"
#include "beaconControl.h"
#include "task.h"
#include "switching.h"
#include "debug.h"

//task token for accessing services
static TaskToken Beacon_TaskToken;

//prototype for task function
static portTASK_FUNCTION(vBeaconTask, pvParameters);

void vBeacon_Init(unsigned portBASE_TYPE uxPriority)
{
	/* initialise the beacon task by inserting into scheduler*/
	Beacon_TaskToken = ActivateTask(TASK_BEACON,
								"Beacon",
								SEV_TASK_TYPE,
								uxPriority,
								SERV_STACK_SIZE,
								vBeaconTask);

}

static portTASK_FUNCTION(vBeaconTask, pvParameters)
{
	(void) pvParameters;

	int i = 0;
	vDebugPrint(Beacon_TaskToken, "before\n\r", 0 ,0, 0);
	while (1)
	{
		// grab semaphore
		switching_takeSemaphore();

		// call switching circuit
		switching_OPMODE(DEVICE_MODE);
		switching_TX_Device(BEACON);
		//// debug

		vDebugPrint(Beacon_TaskToken, "after switching TX device\n\r", 0 ,0, 0);

		// end of debug

		if (i % 2 == 0)
		{
			switching_TX(TX_1);
		} else
		{
			switching_TX(TX_2);
		}

		vTaskDelay( 1000 / portTICK_RATE_MS );

		//// debug

		vDebugPrint(Beacon_TaskToken, "after 1 seconds wake up\n\r", 0 ,0, 0);


		// end of debug
		switching_TX_Device(AFSK_1);
		// release semaphore
		switching_giveSemaphore();

		// sleep for 1 minutes (for testing 5 seconds)
		vTaskDelay( 50000 / portTICK_RATE_MS );
		//// debug

		vDebugPrint(Beacon_TaskToken, "after 50 seconds wake up\n\r", 0 ,0, 0);


		// end of debug
		i++;
	}
}
