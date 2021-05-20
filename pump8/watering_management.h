#ifndef watering_management_h
#define watering_management_h

#include <Arduino.h>
// Using 'polyfill' library to allow standard `analogWrite()` function use.
// With that, no ESP32 specific api libraries or methods are needed. for PWM
// motor control or analog sensor reading.
#include <analogWrite.h>

/**
 * data structures and methods to access analog sensors and PWM motor controls
 */

/// Sensor values are currently 16 bits, but in case that changes, use a custom type
typedef uint16_t sensor_reading_t;
typedef uint32_t pwm_setting_t;
typedef uint8_t gpio_pin_t;

/**
 * capacitive moisture sensor calibration data
 *
 * unique for each sensor
 */
struct moisture_calibration_t {
  // raw in air «dry = 0%» reading
  sensor_reading_t airValue;
  // raw in water «wet = 100%» reading
  sensor_reading_t waterValue;
};

/// information for acquiring moisture values
struct moisture_sensor_t {
  /// source gpio pin number for analog moisture readings
  gpio_pin_t gpio_pin;
  /// data needed for accurate translation of raw reading to moisture percentage
  moisture_calibration_t moisture_calibration;
};

/// information for controlling a water pump
struct pump_motor_t {
  /// target gpio pin number for motor controller PWM signals
  gpio_pin_t gpio_pin;
  /// pwm setting while running the pump
  pwm_setting_t speed;
  // could use additional information, to ramp up to full power over time, or
  // start at higher power, then throttle back over time
};

/** data used to control when (and how much) to pump water
 *
 * For some of the envisioned (far future) scenarios, this will need to be a class
 * a struct is not sufficent to directly handle variable numbers of «array» members
*/
struct watering_triggers_t {
  /// lowest desired soil moisture percentage
  float moisturePercentage;
  /* future ideas
    float ambientTemperature;
    float ambientHumidity;
    float ambientLight;
    time_t earliestStart; // (variable) multiple intervals needs a class
    time_t latestStart;
    time_t previousWatering;
    // // multiple versions in the future; weather forecast
  */
  /// milliseconds to continue watering once started
  unsigned long wateringInterval;
  /// milliseconds to continue ignoring sensor readings after watering finished
  unsigned long soakingInterval;
};

/**
 * Storage for all of the unique information needed to manage a single `zone`
 *
 * A zone uses a single pump and a single moisture sensor.
 */
struct watering_zone_t {
  /// human readable identification for the zone
  String name;
  /// information about the moisture sensor
  moisture_sensor_t sensor;
  /// information to determine when and how much to water
  watering_triggers_t rules;
  /// information about the water pump motor
  pump_motor_t pump;
};

// an empty configuration to clone when a zone is not being used
const struct watering_zone_t UNUSED_ZONE = {
  "unused",
  {0, {4095, 0}}, // sensor
  {0, 0, 0}, // rules
  {0, 0} // pump
};

extern sensor_reading_t raw_adc_reading; // DEBUG
extern float calibrated_measurement; // DEBUG

float getSoilMoisture(const moisture_sensor_t);
unsigned long waterNeeded(const watering_zone_t *);
void startPump(pump_motor_t);
void stopPump(pump_motor_t);

#endif
