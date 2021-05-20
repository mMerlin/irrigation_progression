/**
 * methods to access analog sensors and PWM motor controls using self contained
 * data structures
 */
#include "watering_management.h"

sensor_reading_t raw_adc_reading; // DEBUG
float calibrated_measurement; // DEBUG

/**
 * get moisture percentage for a zone
 *
 * @param zone configuration data for a watering zone
 * @return soil moisture percentage
 */
float getSoilMoisture(const moisture_sensor_t sensor)
{
  sensor_reading_t rawADC = analogRead(sensor.gpio_pin);
  raw_adc_reading = rawADC; // DEBUG
  float moisturePercent = map(rawADC, sensor.moisture_calibration.airValue,
    sensor.moisture_calibration.waterValue, 0, 100);
  calibrated_measurement = moisturePercent; // DEBUG
  moisturePercent = constrain(moisturePercent, 0, 100);
  // Serial.printf("|%f|", moisturePercent); // DEBUG TRACE
  // Serial.printf("gpio %d reads as %d for %f%% soil moisture\n",
  //   sensor.gpio_pin, rawADC, moisturePercent); // DEBUG // LOG
  return moisturePercent;
} // end getSoilMoisture()

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
