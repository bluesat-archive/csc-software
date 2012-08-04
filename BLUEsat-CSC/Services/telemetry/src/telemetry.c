/*
 * telemetry.c
 * Telemetry service
 * Created on: 16/06/2012
 * Author: andyc, colint
 */

#include "service.h"
#include "telemetry.h"
#include "i2c.h"
#include "semphr.h"
#include "UniversalReturnCode.h"
#include "lib_string.h"
#include "debug.h"

/* Telemetry hardware information definition. */
#define MAX127_COUNT 		            16
#define MAX127_SENSOR_COUNT		        8
#define MAX127_BUS_LIMIT                8
#define TELEM_QUEUE_SIZE                16
#define MAX127_TOTAL_RESULT_ELEMENTS    MAX127_SENSOR_COUNT * MAX127_COUNT

/* Telemetry I2C control definition. */
#define TELEM_I2C_CONFIG_BITS      		0x88
#define TELEM_BYTE_INVALID         		0xFF
#define MAX127_SLAVE_BASE_ADDRESS       40
#define TELEM_SEMAPHORE_BLOCK_TIME      (portTICK_RATE_MS * 5)

/* Telemetry period control definition. */
#define HIGH_PERIOD 					1
#define MEDIUM_PERIOD 					2
#define LOW_PERIOD 						3
#define PERIOD_MOD						(HIGH_PERIOD * LOW_PERIOD * MEDIUM_PERIOD)

/* Telemetry sweep control definition. */
#define DEF_SWEEP_TIME                  2000/portTICK_RATE_MS // 20 seconds

typedef enum
{
	SETSWEEP,
	READSWEEP
} Telem_Ops;

typedef struct
{
    /* Telem server operation. */
	Telem_Ops operation;
	/* Declare the size of the input buffer or the sweep buffer. */
	unsigned int size;
	union
	{
	    /* For storing the sweep settings. */
		Entity_sweep_params *paramBuffer;
		/* Pointer to the buffer where the results are. */
		sensor_result *buffer;
	};
} Telem_Cmd;

TaskToken telemTask_token;

static sensor_result latest_data[MAX127_COUNT][MAX127_SENSOR_COUNT];
static xSemaphoreHandle telem_MUTEX;
const sensor_result *latest_data_buffer = (sensor_result*)latest_data;

static portTASK_FUNCTION(vTelemTask, pvParameters);

static inline Request_Rate telem_trigger_count_check(unsigned int count)
{
	Request_Rate returnRate;
	returnRate.high = 1;
	returnRate.medium = !(count % medium);
	returnRate.low = !(count % low);
	return returnRate;
}

static inline UnivRetCode telem_setup_sensor_location(sensor_lc *location, unsigned int sensor_index)
{
	if (location == NULL)
	{
		return URC_FAIL;
	}

	location->address = (sensor_index / MAX127_SENSOR_COUNT) % MAX127_BUS_LIMIT;
	location->channel_mask = 1 << (sensor_index % MAX127_SENSOR_COUNT);
	location->bus = sensor_index / (MAX127_SENSOR_COUNT * MAX127_BUS_LIMIT);

	return URC_SUCCESS;
}

static inline unsigned int uiAddress_to_index (unsigned short address, unsigned char bus)
{
	unsigned int result = 0;
	unsigned short mask = 0x7; // Mask out lower 3 bits
	result = (address & mask) + MAX127_BUS_LIMIT * bus;
	return result;
}

/*void * memcpy(void * dest, const void *src, unsigned long count)
 * takes in the output buffer and the size in number of sensor results
 * simple buffer copy function to populate the given data with as much data as possible
 * size of the buffer is in element entries
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
	int isValid = 1;
	unsigned int length;
	char data = 0;
	unsigned int i;
	char readBuffer[2];
	portBASE_TYPE returnVal;

	for (i = 0; i < MAX127_SENSOR_COUNT; ++i)
	{
		if (!(location->channel_mask & (1 << i))) continue;
		// Start Conversation
        vDebugPrint(telemTask_token, "i is %d, address is %d, location->bus is %d\n\r",
                i, location->address, location->bus);
		data = TELEM_I2C_CONFIG_BITS + (i * 16);
		length = 1; // write 1 byte
		returnVal = Comms_I2C_Master(MAX127_SLAVE_BASE_ADDRESS + location->address, I2C_WRITE,
		        (char*)&isValid, (char*)&data,
				(short*)&length, telem_MUTEX, location->bus);
		xSemaphoreTake(telem_MUTEX, TELEM_SEMAPHORE_BLOCK_TIME);

        // Read Sensor Value
        length = 2; // read 2 bytes as sensor returns 12 bits
        readBuffer[0] = 0xFF;
        readBuffer[1] = 0xFF;
        returnVal = Comms_I2C_Master(MAX127_SLAVE_BASE_ADDRESS + location->address, I2C_READ,
                (char*)&isValid,
                (char*)readBuffer,
                (short*)&length, telem_MUTEX, location->bus);
        xSemaphoreTake(telem_MUTEX, TELEM_SEMAPHORE_BLOCK_TIME);

        /* Process to voltage. */
        unsigned short result = ((readBuffer[0] * 16 + readBuffer[1]/16) * 2.4414);
        latest_data[uiAddress_to_index(location->address,location->bus)][i] = result;
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

    Init_Sweep_Entities(currentRate);
	while (Get_Next_Entity(&currentEntityGroup, &currentResolution) == URC_SUCCESS)
	{
		vDebugPrint(telemTask_token, "Group base %d total_sensors %d\n\r",
		        currentEntityGroup.sensor_base,
		        currentEntityGroup.total_sensors, NO_INSERT);
		// Found out the resolution level
		for (i = currentEntityGroup.sensor_base; i < currentEntityGroup.sensor_base +
			currentEntityGroup.total_sensors; i += currentResolution)
		{
			result = telem_setup_sensor_location(&currentLocation, i);
			if (result != URC_SUCCESS)
			{
				sweepResult = URC_FAIL;
				continue;
			}
			result = enRead_sensor(&currentLocation);
		}
	}
	telem_debug_print();
	return sweepResult;
}

static UnivRetCode perform_set_sweep(Telem_Cmd *telemCmd)
{
	int i;
	int result;

	if (telemCmd->size < MAX_NUM_GROUPS)
	{
		return URC_FAIL;
	}

	for (i = 0; i < MAX_NUM_GROUPS; ++i)
	{
		result = Alter_Sweep_Entity(i, telemCmd->paramBuffer[i].resolution,
				telemCmd->paramBuffer[i].rate);
		if (result != URC_SUCCESS)
		{
			return URC_FAIL;
		}
	}

	return URC_SUCCESS;
}

void telem_debug_print(void)
{
	int i;
	for (i = 0; i < MAX127_SENSOR_COUNT; ++i)
	{
        vDebugPrint(telemTask_token, "SENSOR %d: result is %d\n\r", i,
                latest_data[i / MAX127_BUS_LIMIT][i % MAX127_SENSOR_COUNT],
                NO_INSERT);
	}
}

/*
 * Telemetry service initialisation.
 */
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

/*
 * Telemetry service main function.
 */
static portTASK_FUNCTION(vTelemTask, pvParameters)
{
	(void) pvParameters;
	UnivRetCode enResult;
	MessagePacket incoming_packet;
	Telem_Cmd *pComamndHandle;
	UnivRetCode result;
	unsigned int triggerCount = 0;
	Request_Rate currentRate = telem_trigger_count_check(triggerCount);

	/* Initialise telemetry semaphore. */
	vSemaphoreCreateBinary( telem_MUTEX );

	/* Initialise sweep. */
	Init_Sweep();

	for (;;)
	{
		enResult = enGetRequest(telemTask_token, &incoming_packet, DEF_SWEEP_TIME);
		vDebugPrint(telemTask_token, "Request return value is %d\n\r", (unsigned portLONG) enResult,
					NO_INSERT, NO_INSERT);
		if (enResult != URC_SUCCESS)
		{
			vDebugPrint(telemTask_token, "No message | perform normal sweep...\n\r", 0,
						NO_INSERT, NO_INSERT);
			//Perform sweep and update buffer.
			result = perform_current_sweep(currentRate);
			continue;
		}

        vDebugPrint(telemTask_token, "New message | process message...\n\r", 0,
                    NO_INSERT, NO_INSERT);
		//Process command
		pComamndHandle = (Telem_Cmd *)incoming_packet.Data;
		switch (pComamndHandle->operation)
		{
			case SETSWEEP:
	            vDebugPrint(telemTask_token, "Message | Set sweep...\n\r", 0,
	                        NO_INSERT, NO_INSERT);
				result = perform_set_sweep(pComamndHandle);
				if (result != URC_SUCCESS) break;
				//Perform sweep and update buffer.
				result = perform_current_sweep(currentRate);
				break;
			case READSWEEP:
                vDebugPrint(telemTask_token, "Message | Read sweep...\n\r", 0,
                            NO_INSERT, NO_INSERT);
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

