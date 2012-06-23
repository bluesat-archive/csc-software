/*
 * telemetry.c
 *
 *  Created on: 16/06/2012
 *      Author: andyc
 */

#include "service.h"
#include "telemetry.h"
#include "semphr.h"
#include "UniversalReturnCode.h"
#include "lib_string.h"

#define MAX127_COUNT 		       16 /*8 on each bus as MAX127 address is limited to 3 bits + 5 chip bits*/
#define MAX127_SENSOR_COUNT		   8
#define MAX127_BUS_LIMIT           8
#define TELEM_QUEUE_SIZE           16
#define TELEM_SEMAPHORE_BLOCK_TIME 10
/* Control Byte
 * ------------
 * BIT 7 START Star bit
 * BIT 6 SEL2 Channel address to access 000
 * BIT 5 SEL1
 * BIT 4 SEL0
 * BIT 3 RNG Full scale voltage range 1 to enable
 * BIT 2 BIP Bipolar conversion 0 for the value to be in binary
 * BIT 1 PD1 Power down bits 00 for Normal Operation
 * BIT 0 PD0
 * */
#define TELEM_I2C_CONFIG_BITS      0x88
#define TELEM_BYTE_INVALID         0xFF
#define MIN_SWEEP_TIME				100/portTICK_RATE_MS //portTickRAtems
#define DEF_SWEEP_TIME				300000/portTICK_RATE_MS //0.1sec min delay


typedef enum
{
	SETSWEEP,
	READSWEEP
}Telem_Ops;

typedef struct
{
	Telem_Ops operation; // Telem server operation
	Sweep_Type sweep;	// Determine the type of sweep if specifying the sweep
	unsigned int size;	// Declare the size of the input buffer or the sweep time
	sensor_result * buffer;	// Pointer to the buffer where the results are
} Telem_Cmd;



static TaskToken telemTask_token;

static sensor_result latest_data[MAX127_COUNT][MAX127_SENSOR_COUNT];

static void bzero(void);
static portTASK_FUNCTION(vTelemTask, pvParameters);

void vTelem_Init(unsigned portBASE_TYPE uxPriority)
{
	telemTask_token = ActivateTask(TASK_TELEM,
								"Telem",
								SEV_TASK_TYPE,
								uxPriority,
								SERV_STACK_SIZE,
								vTelemTask);

	vActivateQueue(telemTask_token, TELEM_QUEUE_SIZE);

	memset((int*)latest_data, 0, MAX127_SENSOR_COUNT * MAX127_COUNT);
}

static portTASK_FUNCTION(vTelemTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;
	Telem_Cmd *pComamndHandle;
	Sweep_Type presentSweep = DEFAULT;
	portTickType sweepDelay = DEF_SWEEP_TIME;
	for (;;)
	{
		enResult = enGetRequest(telemTask_token, &incoming_packet, sweepDelay);
		if (enResult != URC_SUCCESS)
		{
			//Perform sweep and update buffer.
			continue;
		}
		//Process command
		pComamndHandle = (Telem_Cmd *)incoming_packet.Data;
		switch (pComamndHandle->operation)
		{
			case SETSWEEP:

				presentSweep = pComamndHandle->sweep;
				sweepDelay = (pComamndHandle->size > MIN_SWEEP_TIME)?pComamndHandle->size:sweepDelay;
				break;
			case READSWEEP:
				// load sweep

				break;
			default:
				vCompleteRequest(incoming_packet.Token, URC_FAIL);
		}
		//complete request by passing the status to the sender
		vCompleteRequest(incoming_packet.Token, URC_SUCCESS);
	}
}

/*void * memcpy(void * dest, const void *src, unsigned long count)
 * takes in the output buffer and the size in number of sensor results
 * */
static unsigned int uiLoad_results (sensor_result * buffer, unsigned int size)
{
	unsigned int result = 0;
	unsigned int max_cpy;
	if (size <= 0) return result;

}

static unsigned int uiAddress_to_index (unsigned short address, I2C_BUS_CHOICE bus)
{
	unsigned int result = 0;
	unsigned short mask = 0x7; // Mask out lower 3 bits
	result = (mask && address) + MAX127_BUS_LIMIT * bus;
	return result;
}
/*
 * assumes that the address has the leading bits of the sensor already prepended
 * read a single max 127 and place the results in the buffer
 * */
static UnivRetCode enRead_sensor(sensor_lc* location)
{
	int isValid;
	unsigned int length;
	char data;
	insigned int i;
	portBASE_TYPE returnVal;
	xSemaphoreHandle telem_MUTEX;

	vSemaphoreCreateBinary( telem_MUTEX );
	for (i = 0; i < MAX127_SENSOR_COUNT; ++i)
	{
		if (!(channel_mask & (1 << i))) continue;
		// Start Conversation
		data = TELEM_I2C_CONFIG_BITS + (i << 4);
		length = 1; // write 1 byte
		returnVal = Comms_I2C_Master(location->address, I2C_WRITE, &isValid, &data, &length, telem_MUTEX, location->bus);
		xSemaphoreTake(telem_MUTEX, TELEM_SEMAPHORE_BLOCK_TIME);

		if ((!isValid) || (!returnVal)) return URC_FAIL;

		// Read Sensor Value
		length = 2; // read 2 bytes as sensor returns 12 bits

		returnVal = Comms_I2C_Master(location->address, I2C_READ, &isValid,
				&latest_data[uiAddress_to_index(location->address,location->bus )][i],
				&length, telem_MUTEX, location->bus);
		xSemaphoreTake(telem_MUTEX, TELEM_SEMAPHORE_BLOCK_TIME);

		if ((!isValid) || (!returnVal)) return URC_FAIL;
	}

	return URC_SUCCESS;
}
