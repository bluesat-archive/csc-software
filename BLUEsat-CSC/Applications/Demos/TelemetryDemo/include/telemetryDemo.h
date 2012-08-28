#ifndef TELEMETRYDEMO_H_
#define TELEMETRYDEMO_H_

typedef enum
{
	TELEM_DEMO_SET_SWEEP = 1,
	TELEM_DEMO_READ_SWEEP,
	TELEM_DEMO_PRINT_LATEST_DATA
} TelemDemoCalls;

void vTelemetryDemoInit(unsigned portBASE_TYPE uxPriority);

#endif /* TELEMETRYDEMO_H_ */
