/**
 * irrigation state machine transition code
 *
 * This contains most of the processing for handling sensor based plant
 * irrigation. It should probably be turned into a class that does the complete
 * job. Currently, the top level state processing `switch` statement is not
 * included.
 *
 * «class_instance».next()
 * «class_instance».OnTransitionOut(callback)
 */

/**
 * utility methods for working with individual irrigation zone state machines
 */

#include "irrigation_state.h"

bool wateringInProgress = false;

/**
 * reserve globally shared power resource
 */
bool takePowerToken()
{
  // better to use mutex or singleton class instance
  // for now, use a simple global flag THAT NO ONE ELSE SHOULD TOUCH
  bool gotToken = false;
  // portENTER_CRITICAL();
  if (!wateringInProgress) {
    wateringInProgress = true;
    gotToken = true;
  }
  // portEXIT_CRITICAL();
  return gotToken;
}

/**
 * done with the globally shared power resource
 */
void releasePowerToken()
{
  wateringInProgress = false;
} // end releasePowerToken()

/**
 * process when the state is MOISTURE_GOOD
 *
 * The latest information is that the soil moisture is within acceptable limits
 * Collect new sensor readings, and transistion to new state if needed
 *
 * @param[in,out] context irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenMoistureGood(irrigation_context_t * context, smart_time_t timeTick)
{
  unsigned long watering_time = waterNeeded(&context->zone);
  if (watering_time > 0) {
    context->state = TOO_DRY;
    context->gone_dry_time = timeTick;
    context->target_time = smartOffsetMillis(timeTick, watering_time);
  }
} // end whenMoistureGood()

/**
 * process when the state is TOO_DRY
 *
 * This is a transient state. It immediately transitions to either
 * START_WATERING, or WATERING_BLOCK, depending whether resources for watering
 * are available or not
 *
 * @param[in,out] context irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenTooDry(irrigation_context_t * context, smart_time_t timeTick)
{
  // if (haveResources(context))
  if (takePowerToken()) {
    context->state = START_WATERING;
    return;
  }
  context->state = WATERING_BLOCKED;
} // end whenTooDry()

/**
 * process when the state is WATERING_BLOCKED
 *
 * Resources needed to deliver water for the context are not currently available.
 * Currently that is just power for the pump, but could be water reservoir
 * empty, or context maintainance in progress.
 *
 * @param[in,out] context irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenWateringBlocked(irrigation_context_t * context, smart_time_t timeTick)
{
  // if (haveResources(context))
  if (takePowerToken()) {
    // Could recheck moisture level/recalculate pump time if been waiting awhile
    context->state = START_WATERING;
  }

  if (smartTimeCompare(timeTick, context->target_time) >= 0) {
    // PROBLEM: somebody has not released the power reservation
    // Or watering has been configure to run for a long time
    context->state = RESOURCE_LOCK_TIMEOUT;
  }
} // end whenWateringBlocked()

/**
 * process when the state is RESOURCE_LOCK_TIMEOUT
 *
 * This is a transient state. It immediately transitions (back) to
 * WATERING_BLOCKED. It exists to separate the external reporting (here) from
 * the general state transitioning code handled by the methods in
 * irrigation_state.cpp
 *
 * @param[in,out] context irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenResourceTimeout(irrigation_context_t * context, smart_time_t timeTick)
{
  // set when to report again if not cleared yet
  context->target_time = smartOffsetMillis(timeTick, POWER_WAIT_TIMEOUT);
  context->state = WATERING_BLOCKED;
  // Nothing to do: the state state itself triggers external processing
} // end whenResourceTimeout()

/**
 * process when the state is START_WATERING
 *
 * This is a transient state. It immediately transitions to DELIVERING_WATER
 * It exists to handle the common code that would otherwise be needed for
 * TOO_DRY and WATERING_BLOCKED
 *
 * @param[in,out] context irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenStartWatering(irrigation_context_t * context, smart_time_t timeTick)
{
  // Safe to start pumping water
  // recheck amount of water needed, in case been blocked for awhile
  unsigned long watering_time = waterNeeded(&context->zone);
  if (watering_time > 0) {
    context->state = DELIVERING_WATER;
    startPump(context->zone.pump);
    context->target_time = smartOffsetMillis(timeTick, watering_time);
    return;
  }
  // By the time got access to the pump, no water actually needed
  releasePowerToken();
  context->state = MOISTURE_GOOD;
}

/**
 * process when the state is DELIVERING_WATER
 *
 * The pump is running. Wait until enough water has been delivered.
 *
 * @param[in,out] context irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenDeliveringWater(irrigation_context_t * context, smart_time_t timeTick)
{
  if (smartTimeCompare(timeTick, context->target_time) >= 0) {
    // Water has been delivered long enough for now
    stopPump(context->zone.pump);
    releasePowerToken(); // not using power any longer
    // Configure interval where soil moisture is not checked
    context->target_time = smartOffsetMillis(timeTick, context->zone.rules.soakingInterval);
    context->state = SOAKING_IN;
  }
} // end whenDeliveringWater()

/**
 * process when the state is SOAKING_IN
 *
 * Water was just delivered. Wait for it to soak in, so the that moisture
 * readings will be (reasonably) accurate
 *
 * @param[in,out] context irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenSoakingIn(irrigation_context_t * context, smart_time_t timeTick)
{
  if (smartTimeCompare(timeTick, context->target_time) >= 0) {
    // Might not actually be `good`, but start checking again
    context->state = MOISTURE_GOOD;
  }
} // end whenSoakingIn()
