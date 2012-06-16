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
}

static portTASK_FUNCTION(vTelemTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;

	for (;;)
	{
		enResult = enGetRequest(telemTask_token, &incoming_packet, portMAX_DELAY);
		if (enResult != URC_SUCCESS) continue;


	}
}

int retrieve_data(unsigned char* output, unsigned int size)
{

}

UnivRetCode setSweep(sensor_lc* config, unsigned int time_interval)
{



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
		if (channel_mask & (1 << i))
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


static void bzero(void)
{
	int i;
	int* ptr = (int*)latest_data;

	for (i = 0; i < (MAX_SENSOR_PER_BUS * 2); ++i)
	{
		*ptr = 0;
	}
}
