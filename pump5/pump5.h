//cSpell:disable
/* pump5.h */

#include <driver/adc.h>
#include <driver/mcpwm.h>

#ifndef MY_PUMP5_H
#define MY_PUMP5_H

/**
 * data needed for handling time intervals when the clock can change, and
 * values can wrap from maximum back to zero.
 *
 * @member epoch linux epoch time reference
 * @member millis milliseconds
 *   initiallly, the raw millis() time reference
 *   evolve to offset delta to compare epoch values before and after clock adjustments
 */
struct smart_time_t {
  unsigned long epoch;
  unsigned long millis;
};

/** data to uniquely identify a motor when setting pwm duty cycle */
struct mcpwm_id_t {
  mcpwm_unit_t unit;
  mcpwm_timer_t timer;
  mcpwm_operator_t opr; // ? MCPWM_OPR_A¦B
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

enum watering_state_t {
  MOISTURE_GOOD = 0,
  TOO_DRY,
  DELIVERING_WATER,
  SOAKING_IN
};

/** state «machine» context data for a single sensor+pump pair */
struct pair_data_t {
  watering_state_t state;
  adc1_channel_t sensor;
  mcpwm_id_t pump;
  String name; // «unique» identifier to use for reporting
  watering_triggers_t wateringTriggers;
  float pumpingSpeed;
  struct smart_time_t goneDryTime;
  struct smart_time_t targetTime;
  unsigned long wateringTime; // millis
};

struct pump_config_t {
  mcpwm_config_t      init;
  mcpwm_io_signals_t  io;
  /*unsigned*/ int    gpio;
};

// Setting values that can be used to control when watering is to be started
// and how much water to provide
# define MOISTURE_READING     (1 << 0)
// # define AMBIENT_TEMPERATURE  (1 << 1)
// # define AMBIENT_HUMIDITY     (1 << 2)
// # define TIME_OF_DAY          (1 << 3)
// # define AMBIENT_LIGHT        (1 << 4)

#endif
