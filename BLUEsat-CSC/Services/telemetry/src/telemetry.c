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

#define MAX127_COUNT 		       16
#define MAX127_SENSOR_COUNT		   8
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
	portCHAR * buffer;	// Pointer to the buffer where the results are
} Telem_Cmd;



static TaskToken telemTask_token;

static unsigned short latest_data[MAX127_COUNT][MAX127_SENSOR_COUNT];

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
	portTickType sweepDelay = portMAX_DELAY;
	for (;;)
	{
		enResult = enGetRequest(telemTask_token, &incoming_packet, sweepDelay);
		if (enResult != URC_SUCCESS)
		{
			//Perform sweep nand update buffer.
			continue;
		}
		//Process command
		pComamndHandle = (Telem_Cmd *)incoming_packet.Data;


	}
}



static int iRead_sensor(sensor_lc* location)
{
	int isValid;
	int length;
	char data;
	int i = 0;
	int returnVal;
	xSemaphoreHandle telem_MUTEX;

	vSemaphoreCreateBinary( telem_MUTEX );
	while (i < MAX127_SENSOR_COUNT)
	{
		if (!(channel_mask & (1 << i)))
		{
			i++;
			continue;
		}

		data = TELEM_I2C_CONFIG_BITS + (i << 4);
		length = 1; // write 1 byte
		returnVal = Comms_I2C_Master(location->address, I2C_WRITE, &isValid, &data, &length, telem_MUTEX, location->bus);
		xSemaphoreTake(telem_MUTEX, TELEM_SEMAPHORE_BLOCK_TIME);

		if ((!isValid) || (!returnVal)) return URC_FAIL;

		length = 2; // read 2 bytes
		returnVal = Comms_I2C_Master(location->address, I2C_READ, &isValid,
				&latest_data[location->address + (location->bus * MAX127_SENSOR_COUNT)][i],
				&length, telem_MUTEX, location->bus);
		xSemaphoreTake(telem_MUTEX, TELEM_SEMAPHORE_BLOCK_TIME);

		if ((!isValid) || (!returnVal)) return URC_FAIL;
		i++;
	}

	return URC_SUCCESS;
}
