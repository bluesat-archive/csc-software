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
#include "switching.h"
#include "debug.h"

#define TRANSMISSION_TIME	1000
#define BEACON_SLEEP_TIME	5000	// sleep for 1 minutes (for testing 5 seconds)

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

	unsigned portCHAR ucToggle;

	for (ucToggle = 0; ; ++ucToggle)
	{
		// grab switching circuit semaphore
		switching_takeSemaphore();

		vDebugPrint(Beacon_TaskToken, "Acquired switching circuit\n\r", 0 ,0, 0);

		// call switching circuit
		switching_OPMODE(DEVICE_MODE);
		switching_TX_Device(BEACON);

		switching_TX((ucToggle % 2 == 0) ? TX_1 : TX_2);

		vDebugPrint(Beacon_TaskToken, "Finished setup switching TX device, transmit for %d ms\n\r", TRANSMISSION_TIME ,0, 0);

		vSleep( TRANSMISSION_TIME );

		switching_TX_Device(AFSK_1);
		// release switching circuit semaphore
		switching_giveSemaphore();

		vDebugPrint(Beacon_TaskToken, "Released switching circuit, silence for %d ms\n\r", BEACON_SLEEP_TIME ,0, 0);

		vSleep( BEACON_SLEEP_TIME );
	}
}
