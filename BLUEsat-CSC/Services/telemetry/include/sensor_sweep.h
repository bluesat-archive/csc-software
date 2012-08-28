/*
 * sensor_sweep.h
 *
 */

#ifndef TELEMSWEEP_H_
#define TELEMSWEEP_H_

#include "UniversalReturnCode.h"

#define MAX_NUM_GROUPS 6

typedef enum
{
   disable = 0,
   high,
   medium,
   low
} TelemLevel;

typedef struct
{
   unsigned short sensorBase;
   unsigned short totalSensors;
} TelemEntityGroup;

typedef struct
{
   TelemLevel resolution;
   TelemLevel rate;
} TelemEntityConfig;


typedef struct
{
   short high : 1;
   short medium : 1;
   short low : 1;
} TelemRequestRate;

typedef enum
{
   criticalSystems = 0,
   battery,
   power,
   radios,
   payload,
   solar,
   maxEntities
} Entity;

/*Sensor Groupings */
static const TelemEntityGroup entityMappings []=  {{0, 21},
                                                    {21, 21},
                                                    {42, 21},
                                                    {63, 21},
                                                    {84, 21},
                                                    {105, 23}};

static const TelemEntityConfig entityDefault = {high, high};
static const TelemRequestRate requestRateDefault = {1, 0, 0};

UnivRetCode enTelemInitSweep(void);
UnivRetCode enTelemAlterSweepEntity(Entity entity, TelemLevel resolution, TelemLevel rate);
void vTelemAlterSweepEntity (TelemRequestRate rates);
UnivRetCode enTelemGetNextEntity (TelemEntityGroup *nextEntity, TelemLevel *resolution);

#endif /* TELEMSWEEP_H_ */
