/* Automatic Plant Watering

  Capacitive moisture sensor combined with windshield washer pump to supply
  more water when the detected moisture falls below a configured limit.

  Really useful site: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/adc.html
 */
#include <driver/adc.h>
#include <driver/mcpwm.h>

enum watering_state_t {
  MOISTURE_GOOD = 0,
  TOO_DRY,
  DELIVERING_WATER,
  SOAKING_IN
};

// Setting values that can be used to control when watering is to be started
// and how much water to provide
# define MOISTURE_READING     (1 << 0)
// # define AMBIENT_TEMPERATURE  (1 << 1)
// # define AMBIENT_HUMIDITY     (1 << 2)
// # define TIME_OF_DAY          (1 << 3)
// # define AMBIENT_LIGHT        (1 << 4)

/** data to uniquely identify a motor when setting pwm duty cycle */
struct my_mcpwm_id_t {
  mcpwm_unit_t unit;
  mcpwm_timer_t timer;
  mcpwm_operator_t opr;
  // unsigned int opr; // ? MCPWM_OPR_A¦B
};

/** data used to control when (and how much) to pump water
 *
 * For some of the envisioned (far future) scenarios, this will need to be a class
*/
struct watering_triggers_t {
  unsigned int mode; // or of bit flags
  float moisturePercentage; // `too dry` level
  // float ambientTemperature;
  // float ambientHumidity;
  // time_t earliestStart; // multiple intervals needs a class; struct not enough
  // time_t latestStart;
  // time_t previousWatering;
  // // multiple versions in the future; weather forecast
  unsigned long wateringInterval; // watering time without sensor feedback
  unsigned long soakingInterval; // also while ignoring sensor readings
};

/** state «machine» context data for a single sensor+pump pair */
struct pair_data_t {
  watering_state_t state;
  adc1_channel_t sensor;
  my_mcpwm_id_t pump;
  String name; // «unique» identifier to use for reporting
  watering_triggers_t wateringTriggers;
  float pumpingSpeed;
  unsigned long goneDryTime;
  unsigned long targetMillis;
  unsigned long wateringTime; // millis
};

struct pump_config_t {
  mcpwm_config_t      init;
  mcpwm_io_signals_t  io;
  /*unsigned*/ int    gpio;
};

const unsigned long POWER_WAIT_TIMEOUT = 10000; // 10 seconds; better get power by then
// Capacitive moisture sensor configuration data «this needs to be per sensor»
const adc1_channel_t sensorPin = ADC1_CHANNEL_6; // gpio 34 for reading moisture sensor
const adc_atten_t ADC_ATTEN_DB = ADC_ATTEN_DB_11; // 3.9V maximum for sensor readings
const int airValue = 2000;    //you need to replace this value with calibration Value_1
const int waterValue = 1210;  //you need to replace this value with calibration Value_2

bool wateringInProgress = false;
#define CONTROL_PAIRS 2
struct pair_data_t stateData[CONTROL_PAIRS];
struct pump_config_t configData[CONTROL_PAIRS];
struct pump_config_t basePumpConfiguration;

void setup()
{
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
  for (unsigned int i = 0; i < CONTROL_PAIRS; i++) {
      configData[i] = basePumpConfiguration;
  }

  stateData[0].name = "first pair";
  stateData[0].sensor = ADC1_CHANNEL_6;
  stateData[0].wateringTriggers.mode = MOISTURE_READING;
  stateData[0].wateringTriggers.moisturePercentage = 30;
  stateData[0].wateringTriggers.wateringInterval = 1000;
  stateData[0].wateringTriggers.soakingInterval = 5000;
  stateData[0].pump.unit = MCPWM_UNIT_0;
  stateData[0].pump.timer = MCPWM_TIMER_0;
  stateData[0].pump.opr = MCPWM_OPR_A;
  configData[0].gpio = 16;
  // The common initialization for all sensor+pump pairs
  for (unsigned int i = 0; i < CONTROL_PAIRS; i++) {
      stateData[i].state = MOISTURE_GOOD;
      stateData[i].targetMillis = 0.0;
      configData[i] = basePumpConfiguration;
      pumpInit(stateData[i].pump, configData[i]);
      //sensorInit(i);
  }
}

void loop()
{
  // smart_time_t smartTime = getSmartTime();
  unsigned long stateMillis = millis();
  for (unsigned int i = 0; i < CONTROL_PAIRS; i++) {
    processPair(&stateData[i], stateMillis);
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
void processPair(pair_data_t * pair, unsigned long referenceTimeMarker)
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

void whenMoistureGood(pair_data_t * pair, unsigned long referenceTimeMarker)
{
  pair -> wateringTime = waterNeeded(pair);
  if (pair -> wateringTime > 0) {
    pair -> state = TOO_DRY;
    pair -> goneDryTime = referenceTimeMarker;
    pair -> targetMillis = referenceTimeMarker + POWER_WAIT_TIMEOUT;
  }
} // end whenMoistureGood()

void whenTooDry(pair_data_t * pair, unsigned long referenceTimeMarker)
{
  if (takePower()) {
    // Safe to start pumping water
    pair -> state = DELIVERING_WATER;
    setPumpSpeed(&pair -> pump, pair -> pumpingSpeed);
    pair -> targetMillis = referenceTimeMarker + pair -> wateringTime;
    return;
  }

  if (referenceTimeMarker >= pair -> targetMillis) {
    // PROBLEM: somebody has not released the power reservation
    // Or watering has been configure to run for a long time
    powerLockedTooLong(pair -> name.c_str(), pair -> goneDryTime, referenceTimeMarker);
    pair -> targetMillis = referenceTimeMarker + POWER_WAIT_TIMEOUT;
  }
} // end whenTooDry()

void whenDeliveringWater(pair_data_t * pair, unsigned long referenceTimeMarker)
{
  if (referenceTimeMarker >= pair -> targetMillis) {
    // Water has been delivered long enough for now
    setPumpSpeed(&pair -> pump, 0.0);
    releasePower();
    // Configure the interval where soil moisture is not checked
    pair -> targetMillis = referenceTimeMarker + pair -> wateringTriggers.soakingInterval;
    pair -> state = SOAKING_IN;
  }
} // end whenDeliveringWater()

void whenSoakingIn(pair_data_t * pair, unsigned long referenceTimeMarker)
{
  if (referenceTimeMarker >= pair -> targetMillis) {
    // Might not actually be `good`, but start checking again
    pair -> state = MOISTURE_GOOD;
  }
} // end whenSoakingIn()

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
 * manage reporting of long waits to access power for a pump
 *
 * @param[in] pairName identification or the waiting pump
 * @param[in] waitingSince initial time point attempted to start watering
 */
void powerLockedTooLong(const char * pairName, const unsigned long waitingSince, unsigned long currentTime)
{
  // possibilities: track and queue repeating reports
  // possible multiple waiting
  // possible serious problem: pump not shutting off, and creating a flood
  Serial.printf("%s waiting for pump power for %lu seconds",
    pairName, currentTime - waitingSince); // DEBUG TRACE
    // (unsigned long)
}

void SoilSensorInit()
{
  adc_power_acquire(); // only once, regardless of number of sensors
  adc_gpio_init(ADC_UNIT_1, (adc_channel_t)sensorPin);
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(sensorPin, ADC_ATTEN_DB);
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
 * set the speed for a single pump
 *
 * @param pump structure with data to specify a unique pump
 * @param speed 0.0 to 100.0% pwm
 */
void setPumpSpeed(const my_mcpwm_id_t * pump, const float speed)
{
  mcpwm_set_duty(pump->unit, pump->timer, pump->opr, speed);
}

/**
 * configure a single pump so that it can be used by the state machine processing
 *
 * @param pumpCtl data needed to access a pump once it has been configured
 * @param pumpCfg data only needed to configure a pump
 */
void pumpInit(my_mcpwm_id_t pumpCtl, pump_config_t pumpCfg)
{
  mcpwm_gpio_init(pumpCtl.unit, pumpCfg.io, pumpCfg.gpio);
  mcpwm_init(pumpCtl.unit, pumpCtl.timer, &pumpCfg.init);
  mcpwm_set_frequency(pumpCtl.unit, pumpCtl.timer, pumpCfg.init.frequency);

  mcpwm_deadtime_disable(pumpCtl.unit, pumpCtl.timer);
  mcpwm_sync_disable(pumpCtl.unit, pumpCtl.timer);
  mcpwm_start(pumpCtl.unit, pumpCtl.timer);
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
