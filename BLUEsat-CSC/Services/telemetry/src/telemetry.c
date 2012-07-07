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
#include "sensor_sweep.h"
#include "debug.h"

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
#define TELEM_I2C_CONFIG_BITS      		0x88
#define TELEM_BYTE_INVALID         		0xFF
#define DEF_SWEEP_TIME					20000/portTICK_RATE_MS // 20 seconds

#define HIGH_PERIOD 					1
#define MEDIUM_PERIOD 					2
#define LOW_PERIOD 						3
#define PERIOD_MOD						(HIGH_PERIOD * LOW_PERIOD * MEDIUM_PERIOD)

#define MAX127_TOTAL_RESULT_ELEMENTS  	MAX127_SENSOR_COUNT * MAX127_COUNT

typedef enum
{
	SETSWEEP,
	READSWEEP
}Telem_Ops;

typedef struct
{
	Telem_Ops operation; // Telem server operation
	unsigned int size;	// Declare the size of the input buffer or the sweep buffer
	union
	{
		Entity_sweep_params *paramBuffer; // for storing the sweep settings
		sensor_result *buffer;	// Pointer to the buffer where the results are
	};
} Telem_Cmd;

static TaskToken telemTask_token;

//typedef sensor_result all_sensors_per_max127[MAX127_SENSOR_COUNT];
static sensor_result latest_data[MAX127_COUNT][MAX127_SENSOR_COUNT];

//const all_sensors_per_max127 *latest_data_buffer = (all_sensors_per_max127 *)latest_data;
const sensor_result *latest_data_buffer = (sensor_result*)latest_data;

static portTASK_FUNCTION(vTelemTask, pvParameters);

static inline Request_Rate uiTriggerCount_Check(unsigned int count)
{
	Request_Rate returnRate;
	returnRate.high = 1;
	returnRate.medium = !(count % medium);
	returnRate.low = !(count % low);
	return returnRate;
}

static inline UnivRetCode setup_sensor_lc(sensor_lc *location, unsigned int sensor_index)
{
	if (location == NULL)
	{
		return URC_FAIL;
	}

	location->address = (sensor_index / MAX127_SENSOR_COUNT) % MAX127_BUS_LIMIT;
	location->channel_mask = sensor_index % MAX127_SENSOR_COUNT;
	location->bus = sensor_index % (MAX127_SENSOR_COUNT * MAX127_BUS_LIMIT);

	return URC_SUCCESS;
}

static inline unsigned int uiAddress_to_index (unsigned short address, I2C_BUS_CHOICE bus)
{
	unsigned int result = 0;
	unsigned short mask = 0x7; // Mask out lower 3 bits
	result = (mask && address) + MAX127_BUS_LIMIT * bus;
	return result;
}

static inline void telem_debug_print(void)
{
	int i;
	for (i = 0; i < MAX127_SENSOR_COUNT * MAX127_COUNT; ++i)
	{
		vDebugPrint(telemTask_token, "result is %d\n",
				latest_data[i / MAX127_BUS_LIMIT][i % MAX127_SENSOR_COUNT],
				NO_INSERT, NO_INSERT);
	}
}

/*void * memcpy(void * dest, const void *src, unsigned long count)
 * takes in the output buffer and the size in number of sensor results
 * simple buffer copy function to populate the given data with as much data as possible
 * size of the buffer is in element entries
 *
 * */
static unsigned int uiLoad_results(sensor_result *buffer, unsigned int size)
{
	unsigned int result = 0;
	unsigned int max_elem_cpy = (size < MAX127_TOTAL_RESULT_ELEMENTS) ? size :
			MAX127_TOTAL_RESULT_ELEMENTS;
	unsigned int max_byte_cpy = max_elem_cpy * sizeof(sensor_result);
	if (size <= 0) return result;
	memcpy ((void*)buffer, (void*)latest_data_buffer, max_byte_cpy);
	return max_elem_cpy;
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
	unsigned int i;
	portBASE_TYPE returnVal;
	xSemaphoreHandle telem_MUTEX;

	vSemaphoreCreateBinary( telem_MUTEX );
	for (i = 0; MAX127_SENSOR_COUNT; ++i)
	{
		if (!(location->channel_mask & (1 << i))) continue;
		// Start Conversation
		data = TELEM_I2C_CONFIG_BITS + (i << 4);
		length = 1; // write 1 byte
		returnVal = Comms_I2C_Master(location->address, I2C_WRITE, (char*)&isValid, (char*)&data,
				(short*)&length, telem_MUTEX, location->bus);
		xSemaphoreTake(telem_MUTEX, TELEM_SEMAPHORE_BLOCK_TIME);

		if ((!isValid) || (!returnVal)) return URC_FAIL;

		// Read Sensor Value
		length = 2; // read 2 bytes as sensor returns 12 bits

		returnVal = Comms_I2C_Master(location->address, I2C_READ, (char*)&isValid,
				(char*)&latest_data[uiAddress_to_index(location->address,location->bus)][i],
				(short*)&length, telem_MUTEX, location->bus);
		xSemaphoreTake(telem_MUTEX, TELEM_SEMAPHORE_BLOCK_TIME);

		if ((!isValid) || (!returnVal)) return URC_FAIL;
	}

	return URC_SUCCESS;
}

/* If the return is URC_FAIL, at least one sensor failed. */
static UnivRetCode perform_current_sweep(Request_Rate currentRate)
{
	Entity_group currentEntityGroup;
	Level currentResolution;
	sensor_lc currentLocation;
	unsigned short i;
	UnivRetCode result;
	UnivRetCode sweepResult = URC_SUCCESS;

	Req_Sweep_Entities(currentRate);

	while (Get_Next_Entity(&currentEntityGroup, &currentResolution) == URC_SUCCESS)
	{
		// Found out the resolution level
		for (i = currentEntityGroup.sensor_base; i < currentEntityGroup.sensor_base +
			currentEntityGroup.total_sensors; i += currentResolution)
		{
			result = setup_sensor_lc(&currentLocation, i);
			if (result != URC_SUCCESS)
			{
				sweepResult = URC_FAIL;
				continue;
			}

			result = enRead_sensor(&currentLocation);
		}
	}

	return sweepResult;
}

static UnivRetCode perform_set_sweep(Telem_Cmd *cmd)
{
	int i;
	int result;

	if (cmd->size < MAX_NUM_GROUPS)
	{
		return URC_FAIL;
	}

	for (i = 0; i < MAX_NUM_GROUPS; ++i)
	{
		result = Alter_Sweep_Entity(i, cmd->paramBuffer[i].resolution, cmd->paramBuffer[i].rate);
		if (result != URC_SUCCESS)
		{
			return URC_FAIL;
		}
	}

	return URC_SUCCESS;
}

UnivRetCode vTelem_Init(unsigned portBASE_TYPE uxPriority)
{
	telemTask_token = ActivateTask(TASK_TELEM,
								"Telem",
								SEV_TASK_TYPE,
								uxPriority,
								SERV_STACK_SIZE,
								vTelemTask);

	vActivateQueue(telemTask_token, TELEM_QUEUE_SIZE);

	memset((sensor_result*)latest_data, 0, MAX127_SENSOR_COUNT * MAX127_COUNT);

	return URC_SUCCESS;
}

static portTASK_FUNCTION(vTelemTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;
	Telem_Cmd *pComamndHandle;
	UnivRetCode result;
	unsigned int triggerCount = 0;
	Request_Rate currentRate = uiTriggerCount_Check(triggerCount);

	for (;;)
	{
		enResult = enGetRequest(telemTask_token, &incoming_packet, DEF_SWEEP_TIME);
		if (enResult != URC_SUCCESS)
		{
			//Perform sweep and update buffer.
			result = perform_current_sweep(currentRate);
			vDebugPrint(telemTask_token, "result is %d\n", result, NO_INSERT, NO_INSERT);

			telem_debug_print();
			continue;
		}
		//Process command
		pComamndHandle = (Telem_Cmd *)incoming_packet.Data;
		switch (pComamndHandle->operation)
		{
			case SETSWEEP:
				result = perform_set_sweep(pComamndHandle);
				if (result != URC_SUCCESS) break;
				//Perform sweep and update buffer.
				result = perform_current_sweep(currentRate);
				vDebugPrint(telemTask_token, "result is %d\n", result, NO_INSERT, NO_INSERT);
				break;
			case READSWEEP:
				// load sweep
				uiLoad_results(pComamndHandle->buffer, pComamndHandle->size);
				break;
			default:
				vCompleteRequest(incoming_packet.Token, URC_FAIL);
		}

		triggerCount = (triggerCount + 1) % PERIOD_MOD;
		//complete request by passing the status to the sender
		vCompleteRequest(incoming_packet.Token, URC_SUCCESS);
	}
}

