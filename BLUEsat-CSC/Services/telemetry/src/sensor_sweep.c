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

static TelemEntityConfig sweep[maxEntities];
static TelemRequestRate current_sweep_rate;
static Entity current_entity = maxEntities;

UnivRetCode enTelemInitSweep(void)
{
	int i;
	// Retrieve sweep from memory

	// Check if sweep found

	// If sweep not found create default sweep

	// Create default sweep
	for (i = 0; i < maxEntities; ++i)
	{
		sweep[i] = entityDefault;
	}

	current_sweep_rate = requestRateDefault;
	return URC_SUCCESS;
}

UnivRetCode enTelemAlterSweepEntity(Entity entity, TelemLevel resolution, TelemLevel rate)
{
	if (entity == maxEntities)
	{
		return URC_FAIL;
	}

	sweep[entity].resolution = resolution;
	sweep[entity].rate = rate;

	return URC_SUCCESS;
}

void vTelemAlterSweepEntity(TelemRequestRate rates)
{
	current_sweep_rate = rates;
	current_entity = 0;
}

UnivRetCode enTelemGetNextEntity(TelemEntityGroup* nextEntity, TelemLevel* resolution)
{
	Entity i;
	if (current_entity == maxEntities)
	{
		return URC_FAIL;
	}
	/* Find the next entity by looping through. */
	for (i = current_entity; i < maxEntities; ++i)
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

		*nextEntity = entityMappings[current_entity];
		*resolution = sweep[i].resolution;
		++current_entity;
		return URC_SUCCESS;
	}

	return URC_FAIL;
}

