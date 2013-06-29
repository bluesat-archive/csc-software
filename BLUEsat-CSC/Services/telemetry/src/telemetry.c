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
#include "power_monitor.h"
#include "rtc.h"

//#define TELEM_DEBUG 1

/* Telemetry hardware information definition. */
#define TRANSLATOR_COUNT                4
#define MAX_TRANSLATOR_SENSOR_COUNT     10
#define TELEM_QUEUE_SIZE                16

/* Telemetry I2C control definition. */
#define TELEM_SEMAPHORE_BLOCK_TIME      (portTICK_RATE_MS * 5)

/* Telemetry sweep control definition. */
#define DEF_SWEEP_TIME                  2000 / portTICK_RATE_MS /* 20 seconds. */

/* Telemetry interface sensor count definition. */
static unsigned int telemInterfaceSensorCount[] = {9, 8, 10, 9};

TaskToken telemTaskToken;
static xSemaphoreHandle telemMutex;

static int magicNum = 0;

static void
telemetry_read_single(unsigned int index, char *buffer, unsigned int size)
{
    /* Perform memory access for a single entry. */
    unsigned int nbytes = ((size < sizeof(struct telem_storage_entry_t)) ?
            size : sizeof(struct telem_storage_entry_t));
    struct telem_storage_entry_t entry;
#ifdef TELEM_DEBUG
    struct telem_storage_entry_t *ptr;
#endif

    telemetry_storage_read_index(index, &entry);
    memcpy(buffer, &entry, nbytes);
    /********** testing. *************/
    vDebugPrint(telemTaskToken, "TELEM | Now check the content is actually stored - single...\n\r",
            NO_INSERT, NO_INSERT, NO_INSERT);

#ifdef TELEM_DEBUG
    /* The following two lines are for debugging. Can be commented out. */
    ptr = (struct telem_storage_entry_t *)buffer;
    telemetry_print_entry_content(ptr);
#endif
}

static void
telemetry_read_latest(char *buffer, unsigned int size)
{
    /* Perform memory access for the latest entry. */
    unsigned int nbytes = ((size < sizeof(struct telem_storage_entry_t)) ?
            size : sizeof(struct telem_storage_entry_t));
    struct telem_storage_entry_t entry;
#ifdef TELEM_DEBUG
    struct telem_storage_entry_t *ptr;
#endif

    telemetry_storage_read_cur(&entry);
    memcpy(buffer, &entry, nbytes);

    /********** testing. *************/
    vDebugPrint(telemTaskToken, "TELEM | Now check the content is actually stored...\n\r",
            NO_INSERT, NO_INSERT, NO_INSERT);
#ifdef TELEM_DEBUG
    ptr = (struct telem_storage_entry_t *)buffer;
    telemetry_print_entry_content(ptr);
#endif
}

static void
telemetry_sensor_store(int interface, struct telem_storage_entry_t *entry)
{
    unsigned int i;
    char *curSensor;
    unsigned short curResult;
    unsigned int baseIndex = 0;
    int j;

    /* Calculate base index. */
    for (i = 0; i < (unsigned int)interface; i++) {
        baseIndex += telemInterfaceSensorCount[interface];
    }

    for (i = 0; i < telemInterfaceSensorCount[interface]; i++) {
        curSensor = (char *)telemetry_sensor_map[interface][i];
        for (j = 0; j < 8; j++) {
            vDebugPrint(telemTaskToken, "%d ", telemetry_sensor_map[interface][i][j], NO_INSERT, NO_INSERT);
        }
        vDebugPrint(NULL, "\r\n",0, NO_INSERT, NO_INSERT);
        curResult = (unsigned short)telemetry_core_read(BUS2, interface, curSensor);
        telemetry_core_print_temperature(curResult, telemTaskToken);
        //curResult = (unsigned char)(magicNum * 1337 + i + (100 * interface))%1000;
        /* Store current result. */
        vDebugPrint(telemTaskToken, "TELEM | cur result is %d...at %d\n\r", curResult, baseIndex+i, 0);
        entry->values[baseIndex+ i] = curResult;
    }
    magicNum++;
}

static void
power_monitor_print(unsigned short *voltages, unsigned short *currents)
{
    int i;
    for (i = 0; i < 16; i++) {
        vDebugPrint(telemTaskToken, "POWER | Dev %d voltage %d\n\r", i, voltages[i], NO_INSERT);
        vDebugPrint(telemTaskToken, "POWER | Dev %d current %d\n\r", i, currents[i], NO_INSERT);
    }
}

static void
telemetry_sensor_poll(void)
{
    rtc_time_t time;
    int i;
    struct telem_storage_entry_t entry;
    memset((char *)&entry, 0, sizeof(struct telem_storage_entry_t));
    for (i = 0; i < TRANSLATOR_COUNT; i++) {
        telemetry_core_conversion(BUS2, i);
    }
    vDebugPrint(telemTaskToken, "TELEM | End of conversion stage...\n\r", 0,
                NO_INSERT, NO_INSERT);

    /* Read all data and store into the storage. */
    telemetry_sensor_store(0, &entry);
    telemetry_sensor_store(1, &entry);
    telemetry_sensor_store(2, &entry); /* Tray 2. */
    telemetry_sensor_store(3, &entry);

    /* Read power monitor data. */
    //power_monitor_sweep(BUS1, &(entry.voltages[0]), &(entry.currents[0]));

    /* Calculate timestamp. */
    /* Timestamp format. */
    /*  31 - 22 | 21 - 17 | 16 - 12 | 11 - 6 | 5 - 0  */
    /*  unused  |   day   |   hour  | minute | second */
    rtc_get_current_time(&time);
    entry.timestamp = (time.rtcMday << 17) | (time.rtcHour << 12) |
            (time.rtcMin << 6) | time.rtcSec;
    vDebugPrint(telemTaskToken, "timestamp is %d\r\n", entry.timestamp, 0, 0);
    vDebugPrint(telemTaskToken, "TELEM | cur addr is %d...\n\r", (int)cur, 0, 0);

    //power_monitor_print(&(entry.voltages[0]), &(entry.currents[0]));
    /* Store the data in memory. */
    telemetry_storage_write(&entry);
}

void
telemetry_print_entry_content(struct telem_storage_entry_t *entry)
{
    int i;
    for (i = 0; i < TELEM_SENSOR_COUNT; i++) {
        vDebugPrint(telemTaskToken, "TELEM | Sensor %d: value is %d\r\n", i, entry->values[i], 0);
    }
    vDebugPrint(telemTaskToken, "TELEM | Timestamp is %d\r\n", entry->timestamp, 0, 0);
}

/* Telemetry service main function. */
static portTASK_FUNCTION(vTelemTask, pvParameters)
{
    (void) pvParameters;
    UnivRetCode enResult;
    MessagePacket incomingPacket;
    telem_command_t *pComamndHandle;
    //rtc_time_t time;

    /* Test feed dog. */
    /*while (1) {
        rtc_get_current_time(&time);
        vDebugPrint(telemTaskToken, "Time is %d: %d: %d\r\n", time.rtcHour, time.rtcMin, time.rtcSec);
        vSleep(1000);
        watchdog_feed();
    }*/

    /* Initialise telemetry semaphore. */
    vSemaphoreCreateBinary(telemMutex);

    /* Set up temperature sensor semaphore. */
    telem_core_semph_create();
    /* Set up power monitor semaphore. */
    power_mon_core_semph_create();
    telemetry_storage_reset();
    telemetry_storage_init();

    for (;;)
    {
        enResult = enGetRequest(telemTaskToken, &incomingPacket, DEF_SWEEP_TIME);

        /* Feed the dog. */
        //watchdog_feed();

        /* Poll once when it gets unblocked regardless. */
        telemetry_sensor_poll();

        if (enResult != URC_SUCCESS) continue;

        vDebugPrint(telemTaskToken, "Message | process message...\n\r", 0,
                    NO_INSERT, NO_INSERT);
        /* Process command. */
        pComamndHandle = (telem_command_t *)incomingPacket.Data;
        switch (pComamndHandle->operation)
        {
            case TELEM_READ_SINGLE:
                vDebugPrint(telemTaskToken, "Message | read single sensor...\n\r", NO_INSERT,
                        NO_INSERT, NO_INSERT);
                telemetry_read_single(pComamndHandle->index, pComamndHandle->buffer,
                        pComamndHandle->size);
                break;
            case TELEM_READ_LATEST:
                vDebugPrint(telemTaskToken, "Message | read latest sensor...\n\r", NO_INSERT,
                        NO_INSERT, NO_INSERT);
                telemetry_read_latest(pComamndHandle->buffer, pComamndHandle->size);
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
                                1024,
                                vTelemTask);

    vActivateQueue(telemTaskToken, TELEM_QUEUE_SIZE);

    return URC_SUCCESS;
}

