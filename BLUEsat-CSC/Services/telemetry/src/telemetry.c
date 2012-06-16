/*
 * telemetry.c
 *
 *  Created on: 16/06/2012
 *      Author: andyc
 */

#include "telemetry.h"


static TaskToken telemTask_token;

static portTASK_FUNCTION(vI2CTestTask, pvParameters);


void vTelem_Init(unsigned portBASE_TYPE uxPriority)
{



}

void retrieve_data(char* output)
{



}

void setSweep(sensor_lc* config, unsigned int time_interval)
{



}


static int iRead_sensor(char* buf, sensor_lc location)
{


	return 0;
}

static int iStart_conversation_raw(sensor_lc location)
{



	return 0;
}

static int iRead_value_raw(char* buf, sensor_lc location)
{



	return 0;
}

static void config_sweep(sensor_lc location)
{



}

static int read_sweep(void)
{



	return 0;
}
