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
#include "telemetry_core.h"
#include "telemetry_storage.h"
#include "telemetry_sensor_map.h"

/* Telemetry hardware information definition. */
#define TRANSLATOR_COUNT                4
#define MAX_TRANSLATOR_SENSOR_COUNT     10
#define TELEM_QUEUE_SIZE                16

/* Telemetry I2C control definition. */
#define TELEM_SEMAPHORE_BLOCK_TIME      (portTICK_RATE_MS * 5)

/* Telemetry sweep control definition. */
#define DEF_SWEEP_TIME                  2000 / portTICK_RATE_MS /* 20 seconds. */

/* Telemetry interface sensor count definition. */
static unsigned int telemInterfaceSensorCount[] = {10, 10, 10, 10};

TaskToken telemTaskToken;
static xSemaphoreHandle telemMutex;

static void
telemetry_read_all(unsigned int interface, char *buffer, unsigned int size)
{
    /* Perform memory access for all the previous data. */
}

static void
telemetry_read_single(unsigned int interface, unsigned int index, char *buffer, unsigned int size)
{
    /* Perform memory access for a single entry. */
}

static void
telemetry_read_latest(unsigned int interface, char *buffer, unsigned int size)
{
    /* Perform memory access for the latest entry. */
}

static void
telemetry_sensor_store(int interface)
{
    int i;
    char *curSensor;
    unsigned short curResult;

    for (i = 0; i < telemInterfaceSensorCount[interface]; i++) {
        curSensor = (char *)telemetry_sensor_map[interface][i];
        curResult = (unsigned short)telemetry_core_read(BUS1, interface, curSensor);
    }
}

static void
telemetry_sensor_poll(void)
{
    int i;
    for (i = 0; i < TRANSLATOR_COUNT; i++) {
        telemetry_core_conversion(BUS1, i);
    }

    /* Read all data and store into the storage. */
    telemetry_sensor_store(0);
}


/* Telemetry service main function. */
static portTASK_FUNCTION(vTelemTask, pvParameters)
{
    (void) pvParameters;
    UnivRetCode enResult;
    MessagePacket incomingPacket;
    telem_command_t *pComamndHandle;
    UnivRetCode result;

    /* Initialise telemetry semaphore. */
    vSemaphoreCreateBinary(telemMutex);

    telem_core_semph_create();

    for (;;)
    {
        enResult = enGetRequest(telemTaskToken, &incomingPacket, DEF_SWEEP_TIME);

        /* Poll once when it gets unblocked regardless. */
        telemetry_sensor_poll();

        if (enResult != URC_SUCCESS) continue;

        vDebugPrint(telemTaskToken, "Message | process message...\n\r", 0,
                    NO_INSERT, NO_INSERT);
        /* Process command. */
        pComamndHandle = (telem_command_t *)incomingPacket.Data;
        switch (pComamndHandle->operation)
        {
            case TELEM_READ_ALL:
                vDebugPrint(telemTaskToken, "Message | read all sensors...\n\r", NO_INSERT,
                        NO_INSERT, NO_INSERT);
                telemetry_read_all(pComamndHandle->interface, pComamndHandle->buffer,
                        pComamndHandle->size);
                break;
            case TELEM_READ_SINGLE:
                vDebugPrint(telemTaskToken, "Message | read single sensor...\n\r", NO_INSERT,
                        NO_INSERT, NO_INSERT);
                telemetry_read_single(pComamndHandle->interface, pComamndHandle->index,
                        pComamndHandle->buffer, pComamndHandle->size);
                break;
            case TELEM_READ_LATEST:
                vDebugPrint(telemTaskToken, "Message | read latest sensor...\n\r", NO_INSERT,
                        NO_INSERT, NO_INSERT);
                telemetry_read_latest(pComamndHandle->interface, pComamndHandle->buffer,
                        pComamndHandle->size);
                break;
            default:
                vCompleteRequest(incomingPacket.Token, URC_FAIL);
                break;
        }

        /* Complete request by passing the status to the sender. */
        vCompleteRequest(incomingPacket.Token, URC_SUCCESS);
    }
}

/*-----------------------------Telemetry public interfaces-------------------------------*/
UnivRetCode enTelemServiceMessageSend(TaskToken taskToken, unsigned portLONG data)
{
    MessagePacket outgoingPacket;

    /* Save information into the outgoing packet. */
    outgoingPacket.Src         = enGetTaskID(taskToken);
    outgoingPacket.Dest        = TASK_TELEM;
    outgoingPacket.Token       = taskToken;
    outgoingPacket.Data        = data;

    return enProcessRequest(&outgoingPacket, portMAX_DELAY);
}


UnivRetCode
vTelemInit(unsigned portBASE_TYPE uxPriority)
{
    telemTaskToken = ActivateTask(TASK_TELEM,
                                "Telem",
                                SEV_TASK_TYPE,
                                uxPriority,
                                SERV_STACK_SIZE,
                                vTelemTask);

    vActivateQueue(telemTaskToken, TELEM_QUEUE_SIZE);

    return URC_SUCCESS;
}

