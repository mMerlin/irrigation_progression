#ifndef irrigation_state_h
#define irrigation_state_h

#include <Arduino.h>
#include "smart_time.h"
#include "watering_management.h"

/**
 * watering_management state machine transition code
 *
 * This contains most of the processing for handling sensor based plant
 * irrigation. It should probably be turned into a class that does the complete
 * job. Currently, the top level state processing `switch` statement is not
 * included, allowing it to easily access resources external to the main
 * irrigation state code.
 */

/**
 * data structures and utility methods for working with irrigation state
 * machines
 */

/// watering state machine states
enum irrigation_state_t {
  /// ignore sensors and never water
  ZONE_DISABLED = 0,
  /// sensors show acceptable moisture level
  MOISTURE_GOOD,
  /// too dry, reserve resources needed to irrigate
  RESERVE_RESOURCES,
  /// irrigation resources locked too long
  RESOURCE_LOCK_TIMEOUT,
  /// watering is in progress
  DELIVERING_WATER,
  /// watering finished, but ignoring sensors
  SOAKING_IN
};

struct irrigation_context_t {
  irrigation_state_t state;
  smart_time_t gone_dry_time;
  smart_time_t target_time;
  watering_zone_t zone;
};

extern bool wateringInProgress;
extern const unsigned long RESOURCE_WAIT_TIMEOUT;

bool takePowerToken(void);
void releasePowerToken(void);
void whenMoistureGood(irrigation_context_t *, smart_time_t);
void whenReserveResources(irrigation_context_t *, smart_time_t);
void whenResourceTimeout(irrigation_context_t *, smart_time_t);
void whenDeliveringWater(irrigation_context_t *, smart_time_t);
void whenSoakingIn(irrigation_context_t *, smart_time_t);

#endif
