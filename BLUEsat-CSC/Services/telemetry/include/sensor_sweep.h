/*
 * sensor_sweep.h
 *
 *  Created on: 24/06/2012
 *      Author: colint
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
} Level;

typedef struct
{
   unsigned short sensor_base;
   unsigned short total_sensors;
} Entity_group;

typedef struct
{
   Level resolution;
   Level rate;
} Entity_sweep_params;


typedef struct
{
   short high:1;
   short medium:1;
   short low:1;
} Request_Rate;

typedef enum
{
   critical_systems = 0,
   battery,
   power,
   radios,
   payload,
   solar,
   max_entities
} Entity;

/*Sensor Groupings */
static const Entity_group Entity_Mappings []={{0,21},
                                       {21,21},
                                       {42,21},
                                       {63,21},
                                       {84,21},
                                       {105,21}};

static const Entity_sweep_params Entity_Default = {low,high};
static const Request_Rate Request_Rate_Default = {1,0,0};

UnivRetCode Init_Sweep(void);
UnivRetCode Alter_Sweep_Entity(Entity entity, Level resolution, Level rate);
void Req_Sweep_Entities (Request_Rate Rates);
UnivRetCode Get_Next_Entity (Entity_group* Next_Entity, Level* Resolution);


#endif
