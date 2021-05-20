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
    context->state = RESERVE_RESOURCES;
    context->gone_dry_time = timeTick;
    context->target_time = smartOffsetMillis(timeTick, RESOURCE_WAIT_TIMEOUT);
  }
} // end whenMoistureGood()

/**
 * process when the state is RESERVE_RESOURCES
 *
 * Wait until all needed resources have been reserved, then start delivering
 * water. Delivery cancelled if no longer needed by the time the reservations
 * have succeeded. If reservations fail for too long, temporarily switch states
 * to trigger logging and/or notifications
 *
 * @param[in,out] context irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenReserveResources(irrigation_context_t * context, smart_time_t timeTick)
{
  // if (haveAllResources(context))
  if (takePowerToken()) {
    // Safe to start pumping water
    // recheck amount needed, in case resources have been blocked for awhile
    unsigned long watering_time = waterNeeded(&context->zone);
    if (watering_time > 0) {
      context->state = DELIVERING_WATER;
      startPump(context->zone.pump);
      context->target_time = smartOffsetMillis(timeTick, watering_time);
      return;
    }
    // By the time got access to all needed resources, no water actually needed
    releasePowerToken();
    context->state = MOISTURE_GOOD;
  }

  if (smartTimeCompare(timeTick, context->target_time) >= 0) {
    // PROBLEM: somebody has not released resources
    // Or watering has been configure to run for a long time
    context->state = RESOURCE_LOCK_TIMEOUT;
  }
} // end whenReserveResources()

/**
 * process when the state is RESOURCE_LOCK_TIMEOUT
 *
 * This is a transient state. It immediately transitions (back) to
 * RESERVE_RESOURCES. It exists to provide a trigger for external
 * logging and notifications.
 *
 * @param[in,out] context irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenResourceTimeout(irrigation_context_t * context, smart_time_t timeTick)
{
  // set when to report again if still not available
  context->target_time = smartOffsetMillis(timeTick, RESOURCE_WAIT_TIMEOUT);
  context->state = RESERVE_RESOURCES;
  // Nothing to do: the state transition itself triggers external processing
} // end whenResourceTimeout()

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
 * Water was just delivered. Wait for it to soak in, so that the moisture
 * readings will be (reasonably) accurate
 *
 * @param[in,out] context irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenSoakingIn(irrigation_context_t * context, smart_time_t timeTick)
{
  if (smartTimeCompare(timeTick, context->target_time) >= 0) {
    // Might not actually be `good`, but start normal checking again
    context->state = MOISTURE_GOOD;
  }
} // end whenSoakingIn()
