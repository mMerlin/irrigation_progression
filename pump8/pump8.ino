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
#include "pump8.h"

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
struct irrigation_context_t allZones[DEFINED_ZONES];

void setup() {
  Serial.begin(SERIAL_BAUD);      // open serial port, set the baud rate
  while (!Serial.available()) {}  // Wait until Serial is really ready
  Serial.println("Start pump8 test sketch");
  // hardware_initialize();

  preFillZones(allZones, DEFINED_ZONES);
  // Initialize active irrigation zones
  configureZone(&allZones[0], sunflowers);
  wateringInProgress = false;
} // end setup()

void loop() {
  smart_time_t smartTime = getSmartTime();
  // checkWaterLevel(smartTime);
  for (size_t i = 0; i < DEFINED_ZONES; i++) {
    checkIrrigationZone(&allZones[i], smartTime);
  }
  // Serial.println(":"); // TRACE
  delay(READING_INTERVAL);
} // end loop()

void checkIrrigationZone(irrigation_context_t * iZone, smart_time_t timeTick)
{
  // state transitions from processing are used to trigger events outside of
  // the state machine. The state machine code handles the actual transitions
  // and irrigation, but notifications are external. Possibly implemented as
  // callbacks later.
  switch (iZone -> state) {
    case ZONE_DISABLED:
      break;
    case MOISTURE_GOOD:
      whenMoistureGood(iZone, timeTick);
      if (iZone->state != MOISTURE_GOOD) {
        logStateInformation("%s has gone dry", iZone, timeTick);
      }
      break;
    case TOO_DRY:
      whenTooDry(iZone, timeTick);
      if (iZone->state != START_WATERING) {
        logStateInformation("irrigation resources for %s needed", iZone, timeTick);
      }
      break;
    case WATERING_BLOCKED:
      whenWateringBlocked(iZone, timeTick);
      break;
    case RESOURCE_LOCK_TIMEOUT:
      whenResourceTimeout(iZone, timeTick);
      logResourceTimeout(iZone, timeTick);
      break;
    case START_WATERING:
      whenStartWatering(iZone, timeTick);
      if (iZone->state == MOISTURE_GOOD) {
        logStateInformation("watering canceled for %s", iZone, timeTick);
      }
      break;
    case DELIVERING_WATER:
      whenDeliveringWater(iZone, timeTick);
      if (iZone->state != DELIVERING_WATER) {
        logStateInformation("watering event finished for %s", iZone, timeTick);
      }
      break;
    case SOAKING_IN:
      whenSoakingIn(iZone, timeTick);
      if (iZone->state != SOAKING_IN) {
        logStateInformation("post watering soak period finished for %s",
          iZone, timeTick);
      }
      break;
    default:
      Serial.printf("checkIrrigationZone: unhandled state %d for \"%s\"\n",
        iZone->state, iZone->zone.name.c_str()); // DEBUG ERROR
      // shut everything down to a safe state, and scream for help
  }
} // end checkIrrigationZone()

void logStateInformation(const String msgFormat,
  const irrigation_context_t * const iZone, const smart_time_t tick)
{
  // logging the latest raw and calibrated sensor reading is for DEBUG, and
  // will not really be correct with the current implementation once multiple
  // zone are active concurrently.
  String logFormat = "LOG: " + msgFormat + " as of time tick «%lu,%lu»¦%u|%f\n";
  Serial.printf(logFormat.c_str(), iZone->zone.name.c_str(),
    tick.epoch, tick.millis, raw_adc_reading, calibrated_measurement);
}

void logResourceTimeout(const irrigation_context_t * const iZone,
  const smart_time_t tick)
{
  // manage reporting of long waits to access power for a pump
  // (or other watering resources)
  // possibilities: track and queue repeating reports
  // possible multiple contexts (zone) waiting simultaneously
  // possible serious problem: pump not shutting off, and creating a flood
  logStateInformation("resource wait timeout for %s", iZone, tick);
  Serial.printf("LOG: -- has now waited for resources %lu milliseconds\n",
    smartDeltaMillis(iZone->gone_dry_time, tick));
}

/**
 * populate an array of irrigation zones with empty data
 *
 * @param[out] context array of irrigation contexts
 * @param[in] count the number of contexts in the array
 */
void preFillZones(irrigation_context_t * context, const size_t count)
{
  for (size_t i = 0; i < count; i++)
  {
    context[i].state = ZONE_DISABLED;
    context[i].zone = UNUSED_ZONE;
    context[i].gone_dry_time = NULL_TIME;
    context[i].target_time = NULL_TIME;
  }
} // end preFillZones()

/**
 * add data to an irrigation context, and make it active
 *
 * @param[out] context the irrigation context to use
 * @param[in] zone populated watering zone configuration data
 */
void configureZone(irrigation_context_t * context, const watering_zone_t zone)
{
  context->zone = zone;
  context->state = MOISTURE_GOOD;
} // end configureZone()
