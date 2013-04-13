#ifndef TELEMETRYDEMO_H_
#define TELEMETRYDEMO_H_

#define TELEM_DEMO_SENSOR_COUNT 40

typedef enum
{
	TELEM_DEMO_READ_SINGLE = 1,
	TELEM_DEMO_READ_LATEST
} TelemDemoCalls;

struct telem_demo_storage_entry_t
{
    unsigned short values[TELEM_DEMO_SENSOR_COUNT];
    unsigned int timestamp;
};

void vTelemetryDemoInit(unsigned portBASE_TYPE uxPriority);

#endif /* TELEMETRYDEMO_H_ */
