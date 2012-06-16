/*
 * telemetry.c
 *
 *  Created on: 16/06/2012
 *      Author: andyc
 */

#include "service.h"
#include "telemetry.h"
#include "semphr.h"

static TaskToken telemTask_token;

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

void retrieve_data(char* output)
{

}

void setSweep(sensor_lc* config, unsigned int time_interval)
{



}


static int iRead_sensor(char* buf, sensor_lc* location)
{


	return 0;
}

static int iStart_conversation_raw(sensor_lc* location)
{
	int isValid;
	int length = 1;
	char data;
	xSemaphoreHandle telem_MUTEX;

	data = TELEM_I2C_CONFIG_BITS + (location->channel_mask << 4);
	Comms_I2C_Master(location->address, I2C_WRITE, &isValid, &data, &length, telem_MUTEX, location->bus);
	xSemaphoreTake(telem_MUTEX, TELEM_SEMAPHORE_BLOCK_TIME);

	return isValid;
}

static int iRead_value_raw(char* buf, sensor_lc* location)
{
	int length;

	return 0;
}

static void config_sweep(sensor_lc* location, unsigned int time_interval)
{



}

static int read_sweep(void)
{



	return 0;
}
