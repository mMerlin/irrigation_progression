/* ESP32 Automatic Plant Watering

  Capacitive moisture sensors combined with windshield washer pumps to supply
  more water when the detected moisture falls below configured limits.

  This version is using one state machine per sensor+pump pair in a single
  thread. None of the statemachine code is `allowed` to block. Each state
  machine can transition between states independent of the others. There is a
  single resource lock that is common between all of the pumps, but that has
  its own state was well, and does not block. Only one pump is allowed to
  operate at a time, to limit the needed capacity for the shared power supply.
 */
#include "pump7.h"

const unsigned long SERIAL_BAUD = 115200;
const uint32_t PWM_MAX_VALUE = 255;
const unsigned long READING_INTERVAL = 450;
const unsigned long POWER_WAIT_TIMEOUT = 10000; // 10 seconds; better get power by then

// To (later) be loaded from flash memory (NVS)
const struct watering_zone_t sunflowers = {
  "zone 1",
  {A2, {2000, 1210}}, // sensor on gpio 34 plus calibration data
  // {MOISTURE_PERCENTAGE, 30.0, 1000, 5000}, // rules
  {30.0, 1000, 5000}, // rules
  {32, PWM_MAX_VALUE >> 3} // pump control on gpio 32
};

const size_t DEFINED_ZONES = 15;
// const size_t DEFINED_ZONES = sizeof(allZones) / sizeof(allZones[0]);
struct irrigation_t allZones[DEFINED_ZONES];
bool wateringInProgress = false;

void setup() {
  Serial.begin(SERIAL_BAUD);      // open serial port, set the baud rate
  while (!Serial.available()) {}  // Wait until Serial is really ready
  Serial.println("Start pump7 test sketch");
  // hardware_initialize();

  preFillZones(allZones, DEFINED_ZONES);
  // Initialize active irrigation zones
  configureZone(&allZones[0], sunflowers);
} // end setup()

void loop() {
  smart_time_t smartTime = getSmartTime();
  // checkWaterLevel(smartTime);
  for (size_t i = 0; i < DEFINED_ZONES; i++) {
    checkIrrigationZone(&allZones[i], smartTime);
  }
  Serial.println(":"); // TRACE
  delay(READING_INTERVAL);
} // end loop()

void checkIrrigationZone(irrigation_t * iZone, smart_time_t timeTick)
{
  switch (iZone -> state) {
    case ZONE_DISABLED:
      Serial.print("."); // TRACE
      break;
    case MOISTURE_GOOD:
      Serial.print("g"); // TRACE
      whenMoistureGood(iZone, timeTick);
      break;
    case TOO_DRY:
      Serial.print("D"); // TRACE
      whenTooDry(iZone, timeTick);
      break;
    case WATERING_BLOCKED:
      Serial.print("B"); // TRACE
      whenWateringBlocked(iZone, timeTick);
      break;
    case START_WATERING:
      Serial.print("w"); // TRACE
      whenStartWatering(iZone, timeTick);
      break;
    case DELIVERING_WATER:
      Serial.print("W"); // TRACE
      whenDeliveringWater(iZone, timeTick);
      break;
    case SOAKING_IN:
      Serial.print("?"); // TRACE
      whenSoakingIn(iZone, timeTick);
      break;
    default:
      Serial.printf("checkIrrigationZone: unhandled state %d for \"%s\"\n",
        iZone->state, iZone->zone.name.c_str()); // DEBUG ERROR
      // shut everything down to a safe state, and scream for help
  }
} // end checkIrrigationZone()

/**
 * process when the state is MOISTURE_GOOD
 *
 * The latest information is that the soil moisture is within acceptable limits
 * Collect new sensor readings, and transistion to new state if needed
 *
 * @param[in,out] zone irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenMoistureGood(irrigation_t * zone, smart_time_t timeTick)
{
  unsigned long watering_time = waterNeeded(&zone->zone);
  if (watering_time > 0) {
    zone->state = TOO_DRY;
    zone->gone_dry_time = timeTick;
    zone->target_time = smartOffsetMillis(timeTick, watering_time);
  }
} // end whenMoistureGood()

/**
 * process when the state is TOO_DRY
 *
 * This is a transient state. It immediately transitions to either
 * START_WATERING, or WATERING_BLOCK, depending whether resources for watering
 * are available or not
 *
 * @param[in,out] zone irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenTooDry(irrigation_t * zone, smart_time_t timeTick)
{
  // if (haveResources(zone))
  if (takePowerToken()) {
    zone->state = START_WATERING;
    return;
  }
  zone->state = WATERING_BLOCKED;
} // end whenTooDry()

/**
 * process when the state is WATERING_BLOCKED
 *
 * Resources needed to deliver water for the zone are not currently available.
 * Currently that is just power for the pump, but could be water reservoir
 * empty, or zone maintainance in progress.
 *
 * @param[in,out] zone irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenWateringBlocked(irrigation_t * zone, smart_time_t timeTick)
{
  // if (haveResources(zone))
  if (takePowerToken()) {
    // Could recheck moisture level/recalculate pump time if been waiting awhile
    zone->state = START_WATERING;
    return;
  }

  if (smartTimeCompare(timeTick, zone->target_time) >= 0) {
    // PROBLEM: somebody has not released the power reservation
    // Or watering has been configure to run for a long time
    powerLockedTooLong(zone->zone.name, zone->gone_dry_time, timeTick);
    // set when to report problem again, if not fixed yet
    zone->target_time = smartOffsetMillis(timeTick, POWER_WAIT_TIMEOUT);
  }
} // end whenWateringBlocked()

/**
 * process when the state is START_WATERING
 *
 * This is a transient state. It immediately transitions to DELIVERING_WATER
 * It exists to handle the common code that would otherwise be needed for
 * TOO_DRY and WATERING_BLOCKED
 *
 * @param[in,out] zone irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenStartWatering(irrigation_t * zone, smart_time_t timeTick)
{
  // Safe to start pumping water
  // recheck amount of water needed, in case been blocked for awhile
  unsigned long watering_time = waterNeeded(&zone->zone);
  if (watering_time > 0) {
    zone->state = DELIVERING_WATER;
    startPump(zone->zone.pump);
    zone->target_time = smartOffsetMillis(timeTick, watering_time);
    return;
  }
  // By the time got access to the pump, no water actually needed
  releasePowerToken();
  zone->state = MOISTURE_GOOD;
}

/**
 * process when the state is DELIVERING_WATER
 *
 * The pump is running. Wait until enough water has been delivered.
 *
 * @param[in,out] zone irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenDeliveringWater(irrigation_t * zone, smart_time_t timeTick)
{
  if (smartTimeCompare(timeTick, zone->target_time) >= 0) {
    // Water has been delivered long enough for now
    stopPump(zone->zone.pump);
    releasePowerToken(); // not using power any longer
    // Configure interval where soil moisture is not checked
    zone->target_time = smartOffsetMillis(timeTick, zone->zone.rules.soakingInterval);
    zone->state = SOAKING_IN;
  }
} // end whenDeliveringWater()

/**
 * process when the state is SOAKING_IN
 *
 * Water was just delivered. Wait for it to soak in, so the that moisture
 * readings will be (reasonably) accurate
 *
 * @param[in,out] zone irrigation state machine context
 * @param[in] timeTick reference time point for state processing
 */
void whenSoakingIn(irrigation_t * zone, smart_time_t timeTick)
{
  if (smartTimeCompare(timeTick, zone->target_time) >= 0) {
    // Might not actually be `good`, but start checking again
    zone->state = MOISTURE_GOOD;
  }
} // end whenSoakingIn()

/**
 * determine the amount of water that is currently needed
 *
 * @param[in] triggerData configuration of trigger criteria
 * @return length of time to pump water
 */
unsigned long waterNeeded(const watering_zone_t * zone)
{
  if (getSoilMoisture(zone->sensor) < zone->rules.moisturePercentage) {
    return zone->rules.wateringInterval;
  }
  return 0; // No water needed at this time
}

/**
 * start a pump at its configured speed
 *
 * @param pump pump configuration data
 */
void startPump(pump_motor_t pump)
{
  analogWrite(pump.gpio_pin, pump.speed);
} // end startPump()

/**
 * stop a pump
 *
 * @param pump pump configuration data
 */
void stopPump(pump_motor_t pump)
{
  analogWrite(pump.gpio_pin, 0);
} // end stopPump()

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
 * manage reporting of long waits to access power for a pump
 *
 * @param[in] zone identification or the waiting zone
 * @param[in] waitingSince initial time point attempted to start watering
 */
void powerLockedTooLong(const String zoneName, const smart_time_t waitingSince,
    const smart_time_t timeTick)
{
  // possibilities: track and queue repeating reports
  // possible multiple waiting
  // possible serious problem: pump not shutting off, and creating a flood
  Serial.printf("%s zone waiting for pump power for %lu seconds",
    zoneName.c_str(), smartDeltaMillis(timeTick, waitingSince)); // DEBUG TRACE
}

/**
 * get moisture percentage for a zone
 *
 * @param zone configuration data for a watering zone
 * @return soil moisture percentage
 */
float getSoilMoisture(const moisture_sensor_t sensor)
{
  sensor_reading_t rawADC = analogRead(sensor.gpio_pin);
  float moisturePercent = map(rawADC, sensor.moisture_calibration.airValue,
    sensor.moisture_calibration.waterValue, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);
  Serial.printf("|%f|", moisturePercent); // DEBUG TRACE
  // Serial.printf("gpio %d reads as %d for %f%% soil moisture\n",
  //   sensor.gpio_pin, rawADC, moisturePercent); // DEBUG // LOG
  return moisturePercent;
} // end getSoilMoisture()

smart_time_t getSmartTime()
{
  smart_time_t sTime;
  sTime.epoch = 0;
  sTime.millis = millis();
  return sTime;
} // end getSmartTime()

/**
 * add milliseconds to a smart time
 *
 * @param[in] base existing smart time
 * @param[in] millis offset (delta) time
 * @return smart time plus millis
 */
smart_time_t smartOffsetMillis(const smart_time_t base, const unsigned long millis)
{
  smart_time_t sTime;
  sTime.epoch = base.epoch;
  sTime.millis = base.millis + millis;
  return sTime;
} // end smartOffsetMillis()

/**
 * compare 2 smart time values
 *
 * @param[in] base base smart time reference
 * @param[in] other another smart time reference
 * @return -1,0,1 when base is <, =, > other time
 */
int smartTimeCompare(const smart_time_t base, const smart_time_t other)
{
  // return base.millis <=> other.millis;
  if (base.millis < other.millis) {
    return -1;
  }
  if (base.millis > other.millis) {
    return 1;
  }
  return 0;
} // end smartTimeCompare()

/**
 * get milliseconds difference between smart time values
 *
 * NOTE: returns unsigned value, so if end < start, a very large value will
 *   be returned
 *
 * @param start starting time reference
 * @param end ending time reference
 * @return milliseconds difference
 */
unsigned long smartDeltaMillis(smart_time_t start, smart_time_t end)
{
  return end.millis - start.millis;
} // end smartDeltaMillis

/**
 * populate an array of irrigation zones with empty data
 *
 * @param[out] zones array of irrigation zones
 * @param[in] zone_count the number of zones in the array
 */
void preFillZones(irrigation_t * zones, size_t zone_count)
{
  for (size_t i = 0; i < zone_count; i++)
  {
    zones[i].state = ZONE_DISABLED;
    zones[i].zone = UNUSED_ZONE;
    zones[i].gone_dry_time = NULL_TIME;
    zones[i].target_time = NULL_TIME;
  }
} // end preFillZones()

/**
 * add data to an irrigation zone, and make it active
 *
 * @param[out] zone the zone to use
 * @param[in] zone_data populated watering zone configuration data
 */
void configureZone(irrigation_t * zone, const watering_zone_t zone_data)
{
  zone->state = MOISTURE_GOOD;
  zone->zone = zone_data;
} // end configureZone()
