/* Automatic Plant Watering

  Capacitive moisture sensor combined with windshield washer pump to supply
  more water when the detected moisture falls below a configured limit.

  This version is using one state machine per sensor+pump pair in a single
  thread. None of the statemachine code is `allowed` to block. Each state
  machine can transition between states independent of the others. There is a
  single resource lock that is common between all of the pumps. Only one pump
  is allowed to operate at a time, to limit the needed capacity for the shared
  power supply.

  Really useful site: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/adc.html
 */
#include "pump5.h"

const float DEFAULT_MOISTURE_TRIGGER = 30.0; // 30%
const unsigned long DEFAULT_WATERING_INTERVAL = 1000; // 1 second «TESTING»
const unsigned long DEFAULT_SOAKING_INTERVAL = 5000; // 5 second «TESTING»

const unsigned long POWER_WAIT_TIMEOUT = 10000; // 10 seconds; better get power by then

// Capacitive moisture sensor configuration data «this needs to be per sensor»
const adc1_channel_t sensorPin = ADC1_CHANNEL_6; // gpio 34 for reading moisture sensor
const adc_atten_t ADC_ATTEN_DB = ADC_ATTEN_DB_11; // 3.9V maximum for sensor readings
const int airValue = 2000;    //you need to replace this value with calibration Value_1
const int waterValue = 1210;  //you need to replace this value with calibration Value_2

#define CONTROL_PAIRS 2
struct pair_data_t stateData[CONTROL_PAIRS];
struct pump_config_t configData[CONTROL_PAIRS];
struct pump_config_t basePumpConfiguration;
struct watering_triggers_t baseWateringTriggers;

bool wateringInProgress = false;

void setup() {
  Serial.begin(115200);        // open serial port, set the baud rate
  SoilSensorInit();
  //onceOnlySensorInit();
  // reservoirLevelSensorInit();
  // Potentially unique per pair configuration settings
  // store/read from NVS

  // Some data is needed for esp module configuration, but the state machines
  // do not use or care about the values. Other data is needed both for
  // configuration, and state management.
  setDefaultPumpConfiguration(&basePumpConfiguration);
  setDefaultWateringTriggers(&baseWateringTriggers);
  for (unsigned int i = 0; i < CONTROL_PAIRS; i++) {
      configData[i] = basePumpConfiguration;
      stateData[i].wateringTriggers = baseWateringTriggers;
  }

  // for each sensor+pump pair, fill in the unique configuration details
  // configurePair01()
  // read from nvs
  stateData[0].name = "first pair";
  stateData[0].sensor = ADC1_CHANNEL_6;
  stateData[0].wateringTriggers.soakingInterval = 6000; // override default
  stateData[0].pump.unit = MCPWM_UNIT_0;
  stateData[0].pump.timer = MCPWM_TIMER_0;
  stateData[0].pump.opr = MCPWM_OPR_A;
  configData[0].gpio = 16;
  // The common initialization for all sensor+pump pairs
  for (unsigned int i = 0; i < CONTROL_PAIRS; i++) {
      stateData[i].state = MOISTURE_GOOD;
      stateData[i].targetTime = smartDeltaTime(0);
      configData[i] = basePumpConfiguration;
      pumpInit(stateData[i].pump, configData[i]);
      //sensorInit(i);
  }
}

void loop() {
  smart_time_t smartTime = getSmartTime();
  for (unsigned int i = 0; i < CONTROL_PAIRS; i++) {
    processPair(&stateData[i], smartTime);
  }
}

/**
 * manage state machine transitions for a single sensor-pump pair
 *
 * keywords: next, transition, update, advance
 *
 * @param[in] pair sensor+pump state machine context
 * @param[in] referenceTimeMarker base time reference for (possible) transition
 *   needs to be smarter time reference to handle both internal «micros()»
 *   wrapping, and adjustments based on ntp update
 */
void processPair(pair_data_t * pair, smart_time_t referenceTimeMarker)
{
  switch (pair -> state) {
    case MOISTURE_GOOD:
      whenMoistureGood(pair, referenceTimeMarker);
      break;
    case TOO_DRY:
      whenTooDry(pair, referenceTimeMarker);
      break;
    case DELIVERING_WATER:
      whenDeliveringWater(pair, referenceTimeMarker);
      break;
    case SOAKING_IN:
      whenSoakingIn(pair, referenceTimeMarker);
      break;
  }
} // end processPair()

void whenMoistureGood(pair_data_t * pair, smart_time_t referenceTimeMarker)
{
  pair -> wateringTime = waterNeeded(pair);
  if (pair -> wateringTime > 0) {
    pair -> state = TOO_DRY;
    pair -> goneDryTime = referenceTimeMarker;
    pair -> targetTime = smartOffsetMillis(referenceTimeMarker, POWER_WAIT_TIMEOUT);
  }
} // end whenMoistureGood()

void whenTooDry(pair_data_t * pair, smart_time_t referenceTimeMarker)
{
  if (takePower()) {
    // Safe to start pumping water
    pair -> state = DELIVERING_WATER;
    setPumpSpeed(pair -> pump, pair -> pumpingSpeed);
    pair -> targetTime = smartOffsetMillis(referenceTimeMarker, pair -> wateringTime);
    return;
  }

  if (smartTimeCompare(referenceTimeMarker, pair -> targetTime) >= 0) {
    // PROBLEM: somebody has not released the power reservation
    // Or watering has been configure to run for a long time
    powerLockedTooLong(pair -> name, pair -> goneDryTime, referenceTimeMarker);
    pair -> targetTime = smartOffsetMillis(referenceTimeMarker, POWER_WAIT_TIMEOUT);
  }
} // end whenTooDry()

void whenDeliveringWater(pair_data_t * pair, smart_time_t referenceTimeMarker)
{
  if (smartTimeCompare(referenceTimeMarker, pair -> targetTime) >= 0) {
    // Water has been delivered long enough for now
    setPumpSpeed(pair -> pump, 0.0);
    releasePower();
    // Configure interval where soil moisture is not checked
    pair -> targetTime = smartOffsetMillis(referenceTimeMarker, pair -> wateringTriggers.soakingInterval);
    pair -> state = SOAKING_IN;
  }
} // end whenDeliveringWater()

void whenSoakingIn(pair_data_t * pair, smart_time_t referenceTimeMarker)
{
  if (smartTimeCompare(referenceTimeMarker, pair -> targetTime) >= 0) {
    // Might not actually be `good`, but start checking again
    pair -> state = MOISTURE_GOOD;
  }
} // end whenSoakingIn()

/**
 * configure a single pump so that it can be used by the state machine processing
 *
 * @param pumpCtl data needed to access a pump once it has been configured
 * @param pumpCfg data only needed to configure a pump
 */
void pumpInit(mcpwm_id_t pumpCtl, pump_config_t pumpCfg)
{
  mcpwm_gpio_init(pumpCtl.unit, pumpCfg.io, pumpCfg.gpio);
  mcpwm_init(pumpCtl.unit, pumpCtl.timer, &pumpCfg.init);
  mcpwm_set_frequency(pumpCtl.unit, pumpCtl.timer, pumpCfg.init.frequency);

  mcpwm_deadtime_disable(pumpCtl.unit, pumpCtl.timer);
  mcpwm_sync_disable(pumpCtl.unit, pumpCtl.timer);
  mcpwm_start(pumpCtl.unit, pumpCtl.timer);
}

void  SoilSensorInit()
{
  adc_power_acquire(); // only once, regardless of number of sensors
  adc_gpio_init(ADC_UNIT_1, (adc_channel_t)sensorPin);
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(sensorPin, ADC_ATTEN_DB);
}

/**
 * manage reporting of long waits to access power for a pump
 *
 * @param[in] pairName identification or the waiting pump
 * @param[in] waitingSince initial time point attempted to start watering
 */
void powerLockedTooLong(String pairName, const smart_time_t waitingSince, smart_time_t currentTime)
{
  // possibilities: track and queue repeating reports
  // possible multiple waiting
  // possible serious problem: pump not shutting off, and creating a flood
  Serial.printf("%s waiting for pump power for %lu seconds",
    pairName.c_str(), smartDeltaMillis(currentTime, waitingSince)); // DEBUG TRACE
}

/**
 * set the speed for a single pump
 *
 * @param pump structure with data to specify a unique pump
 * @param speed 0.0 to 100.0% pwm
 */
void setPumpSpeed(mcpwm_id_t pump, const float speed)
{
  mcpwm_set_duty(pump . unit, pump . timer, pump . opr, speed);
}

/**
 * global flag (mutex) to allow access to limited power supply shared by all pumps
 */
bool takePower()
{
  // better to use mutex or singleton class instance
  // for now, use a simple global flag THAT NO ONE ELSE SHOULD TOUCH
  bool powerTaken = false;
  // portENTER_CRITICAL();
  if (!wateringInProgress) {
    wateringInProgress = true;
    powerTaken = true;
  }
  // portEXIT_CRITICAL();
  return powerTaken;
}

/**
 * Allow others to use power
 */
void releasePower()
{
  wateringInProgress = false;
}

/**
 * determine the amount of water that is currently needed
 *
 * @param[in] triggerData configuration of trigger criteria
 * @param[in] checkTime base time reference associated with test
 * @return length of time to pump water
 */
unsigned long waterNeeded(const pair_data_t * pair)
{
  watering_triggers_t config = pair -> wateringTriggers;
  // only config.mode == MOISTURE_READING implemented
  if (getMoisturePercent(pair -> sensor) < config.moisturePercentage) {
    return config.wateringInterval;
  }
  return 0; // No water needed at this time
}

/**
 * measure soil moisture
 *
 * @param[in] sourceChannel adc channel to acquire measurement from
 */
int getMoisturePercent(const adc1_channel_t sourceChannel)
{
  int soilMoistureValue;
  int soilMoisturePercent;
  soilMoistureValue = 0;
  for (int i = 0; i<16; i++) {
    int adc_value = adc1_get_raw(sourceChannel);
    soilMoistureValue += adc_value;
    delay(15);
    Serial.print(sourceChannel);
    Serial.print(" Measurement = ");
    Serial.println(adc_value);
  }
  soilMoistureValue /= 16;
  Serial.print("soilMoistureValue = ");
  Serial.println(soilMoistureValue);
  soilMoisturePercent = map(soilMoistureValue, airValue, waterValue, 0, 100);
  if(soilMoisturePercent < 0) { soilMoisturePercent = 0; }
  if(soilMoisturePercent > 100) { soilMoisturePercent = 100; }
  Serial.print("soilMoisturePercent = ");
  Serial.print(soilMoisturePercent);
  Serial.println(" %");
  return soilMoisturePercent;
}

/**
 * setup a default pump configuration, to reduce duplicate setup code
 *
 * @param[out] config default pump configuration data
 */
void setDefaultPumpConfiguration(pump_config_t * config)
{
  config -> init.frequency = 100;
  config -> init.cmpr_a = 99;
  config -> init.cmpr_b = 100;
  config -> init.duty_mode = MCPWM_DUTY_MODE_0;
  config -> init.counter_mode = MCPWM_UP_COUNTER;
  config -> io = MCPWM0A;
  config -> gpio = 0; // no default, must be explicitly set each time
} // end setDefaultPumpConfiguration()

/**
 * setup a default watering triggers, to reduce duplicate setup code
 *
 * @param[out] triggers default watering triggers data
 */
void setDefaultWateringTriggers(watering_triggers_t * triggers)
{
  triggers -> mode = MOISTURE_READING;
  triggers -> moisturePercentage = DEFAULT_MOISTURE_TRIGGER;
  triggers -> wateringInterval = DEFAULT_WATERING_INTERVAL;
  triggers -> soakingInterval = DEFAULT_SOAKING_INTERVAL;
} // end setDefaultWateringTriggers()


// Turn following functions into a SmartTime library, with 'real' functionality
// overload addition, subtraction, comparison operators

unsigned long smartDeltaMillis(smart_time_t start, smart_time_t end)
{
  return end.millis - start.millis;
} // end smartDeltaMillis

smart_time_t smartDeltaTime(unsigned long millis) {
  struct smart_time_t sTime;
  sTime.epoch = 0;
  sTime.millis = millis;
  return sTime;
} // end smartDeltaTime()

smart_time_t getSmartTime()
{
  smart_time_t sTime;
  sTime.epoch = 0;
  sTime.millis = millis();
  return sTime;
} // end getSmartTime()

smart_time_t smartOffsetMillis(smart_time_t base, unsigned long offset)
{
  smart_time_t sTime;
  sTime.epoch = base.epoch;
  sTime.millis = base.millis + offset;
  return sTime;
} // end smartOffsetMillis()

int smartTimeCompare(smart_time_t base, smart_time_t other) {
  // return base.millis <=> other.millis;
  if (base.millis < other.millis) {
    return -1;
  }
  if (base.millis > other.millis) {
    return 1;
  }
  return 0;
}
