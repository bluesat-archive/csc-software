/*
 * sensor_sweep.c
 *
 *  Created on: 24/06/2012
 *      Author: andyc, colint
 *
 */

#include "debug.h"
#include "sensor_sweep.h"
#include "UniversalReturnCode.h"
#include "telemetry.h"

static Entity_sweep_params sweep[max_entities];
static Request_Rate current_sweep_rate;
static Entity current_entity = max_entities;

UnivRetCode Init_Sweep(void)
{
	int i;
	// Retrieve sweep from memory

	// Check if sweep found

	// If sweep not found create default sweep

	// Create default sweep
	for (i = 0; i < max_entities; ++i)
	{
		sweep[i] = Entity_Default;
	}

	current_sweep_rate = Request_Rate_Default;
	return URC_SUCCESS;
}

UnivRetCode Alter_Sweep_Entity(Entity entity, Level resolution, Level rate)
{
	if (entity == max_entities)
	{
		return URC_FAIL;
	}

	sweep[entity].resolution = resolution;
	sweep[entity].rate = rate;

	return URC_SUCCESS;
}

void Init_Sweep_Entities(Request_Rate Rates)
{
	current_sweep_rate = Rates;
	current_entity = 0;
}

UnivRetCode Get_Next_Entity(Entity_group* Next_Entity, Level* Resolution)
{
	Entity i;
	if (current_entity == max_entities)
	{
		return URC_FAIL;
	}
	/* Find the next entity by looping through. */
	for (i = current_entity; i < max_entities; ++i)
	{
		switch (sweep[i].rate)
		{
			case high:
				if (current_sweep_rate.high) break;
			case medium:
				if (current_sweep_rate.medium) break;
			case low:
				if (current_sweep_rate.low) break;
			default:
				continue;
		}

		*Next_Entity = Entity_Mappings[current_entity];
		*Resolution = sweep[i].resolution;
		++current_entity;
		return URC_SUCCESS;
	}

	return URC_FAIL;
}

