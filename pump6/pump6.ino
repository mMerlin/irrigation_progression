/* Automatic Plant Watering

  Capacitive moisture sensor combined with windshield washer pump to supply
  more water when the detected moisture falls below a configured limit.

  Using 'polyfill' library to access standard `analogWrite()` function
 */
#include <analogWrite.h>

/// Sensor values are currently 16 bits, but in case that changes, use a custom type
typedef uint16_t sensor_reading_t;
typedef uint32_t pwm_setting_t;
typedef uint8_t gpio_pin_t;

/**
 * Storage for all of the unique information needed to manage a single `zone`
 *
 * A zone uses a single pump and a single moisture sensor.
 *
 * @member sensor analog input pin number to read moisture sensor
 * @member airValue moisture sensor calibration value for dry (in air)
 * @member waterValue moisture sensor calibration value for wet (in water)
 * @member tooDry moisture percentage trigger value when need to supply water
 * @member pump PWM output pin to control pump motor
 * @member interval milliseconds to run pump when watering started
 * @member soak milliseconds to wait after adding water before reading sensor
 * @member speed pwm setting while running pump
 */
struct watering_zone_t {
  String name;
  gpio_pin_t sensor;
  sensor_reading_t airValue;
  sensor_reading_t waterValue;
  float tooDry;
  gpio_pin_t pump;
  unsigned long interval;
  unsigned long soak;
  pwm_setting_t speed;
};

const unsigned long SERIAL_BAUD = 115200;
// must be the same as used to generate the sensor calibration values
const size_t ADC_BITS = 12; // 10 is standard Arduino; ESP32 can do 12
const double PWM_BASE_FREQUENCY = 5000;
const uint8_t PWM_BASE_RESOLUTION = 8; // bits; duty 0 … 255
//const uint8_t PWM_BASE_RESOLUTION = 10; // bits; duty 0 … 1023
//const uint8_t PWM_BASE_RESOLUTION = 12; // bits; duty 0 … 4095
const unsigned long READING_INTERVAL = 2000;
const uint32_t PWM_MAX_VALUE = ((1 << PWM_BASE_RESOLUTION) - 1);

struct watering_zone_t sunflowers = {
  "sunflowers",
  A2, // gpio 34 sensor
  2000, // airValue
  1210, // waterValue
  30.0, // tooDry
  32, // pump gpio pin
  1000, // interval
  5000, // soak «temporary, DEBUG, development»
  PWM_MAX_VALUE >> 3 // speed
};

struct watering_zone_t allZones[] = {
  sunflowers
};

void setup() {
  Serial.begin(SERIAL_BAUD);        // open serial port, set the baud rate
  while (!Serial.available()) {}    // Wait until Serial is really ready
  Serial.println("Start pump6 test sketch");
  Serial.printf("PWM max %d %#x: speed %d\n",
    PWM_MAX_VALUE, PWM_MAX_VALUE, sunflowers.speed); // DEBUG

  // global init for all sensors
  analogReadResolution(ADC_BITS);
  // if used, ADC2 pins can use different resolutions. The resolution to use can
  // be set for every single read, if calling the lower level adc2_get_raw()
  // function.

  // global init defaults for all pumps: could be changed later per pump
  analogWriteFrequency(PWM_BASE_FREQUENCY);
  analogWriteResolution(PWM_BASE_RESOLUTION);
}

void loop() {
  // checkWaterLevel();
  if (getSoilMoisture(allZones[0]) < allZones[0].tooDry) {
    pumpWater(allZones[0]);
  }
  delay(READING_INTERVAL);
}

/**
 * get moisture percentage for a zone
 *
 * @param zone configuration data for a watering zone
 * @return soil moisture percentage
 */
float getSoilMoisture(const watering_zone_t zone)
{
  sensor_reading_t rawADC = analogRead(zone.sensor);
  float moisturePercent = map(rawADC, zone.airValue, zone.waterValue, 0, 100);
  if(moisturePercent < 0) { moisturePercent = 0; }
  if(moisturePercent > 100) { moisturePercent = 100; }
  Serial.printf("%s «gpio %d» reads as %d for %f%% soil moisture\n",
    zone.name.c_str(), zone.sensor, rawADC, moisturePercent);
  return moisturePercent;
}

/**
 * deliver water for a watering zone
 *
 * @param zone configuration data for a watering zone
 */
void pumpWater(const watering_zone_t zone)
{
//  Serial.printf("Water %s\n", zone.name.c_str());
  Serial.printf("Turning %s pump on\n", zone.name.c_str());
  analogWrite(zone.pump, zone.speed);
//  analogWrite(zone.pump, zone.speed, PWM_MAX_VALUE);
  delay(zone.interval); // pump water
  Serial.printf("Turning %s pump off\n", zone.name.c_str());
  analogWrite(zone.pump, 0); // done watering interval
  delay(zone.soak); // wait for water to soak in before testing again
}
