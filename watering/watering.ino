/* Automatic Plant Watering

  Capacitive moisture sensor combined with windshield washer pump to supply
  more water when the detected moisture falls below a configured limit.

  This code uses «almost» standard Arduino code to access and manipulate
  the hardware I/O pins. There are esp specific libraries that give enhanced
  features, but they do not really seem to be needed for reading the sensors
  and controlling the pump motor. Instead, a 'polyfill' library is used, that
  uses the esp32 ledc library internally, but provides a function that acts
  like the generic Arduino `analogWrite()` function.
 */
#include <analogWrite.h>

/**
 * Collect several pieces of information that are used to control a single pump
 *
 * @member sensor GPIO pin number to read analog moisture sensor values
 * @member tooDry moisture percentage below which watering is started
 * @member inAir sensor calibration air value
 * @member inWater sensor calibration water value
 * @member pump GPIO pin number to write pwm setting for pump motor
 * @member speed pwm setting to run the pump motor
 * @member interval milliseconds to run pump for single watering event
 * @member soak milliseconds to let water soak end before checking moisture again
 */
// Pair sensor and motor control pins. One sensor is used to control a single motor
struct zone_t {
  size_t sensor;
  size_t tooDry;
  size_t inAir;
  size_t inWater;
  size_t pump;
  size_t speed;
  unsigned long interval;
  unsigned long soak;
};
const size_t ANALOG_BITS = 12; // 10 is standard Arduino; ESP32 can do 12

// later «some of» this should be saved to, and read from NVS «flash»
// watering `zone`
const zone_t firstpot = {
  A2, // sensor «GPIO 34»
  30, // tooDry
  2000, // inAir «at used read resolution»
  1210, // inWater «at used read resolution»
  16, // pump
  110, // speed
  2500, // interval
  5000 // soak «DEBUG and development»
};

const unsigned long SERIAL_BAUD = 115200;

void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial.available()) {} // Wait until Serial is really ready

  // Tweak (global not per pin) configuration to get extended range
  analogReadResolution(ANALOG_BITS);
  initializezone(firstpot);
}

void loop() {
  if (getmoisturepercent(firstpot) < firstpot.tooDry) {
    waterPlants(firstpot);
  }
}

void initializezone(const zone_t zone)
{
  pinMode(zone.pump, OUTPUT);
  // If desired, the analogWrite settings can be tweaked here
  analogWriteFrequency(zone.pump, 5000);
  analogWriteResolution(zone.pump, 12);
}

size_t getmoisturepercent(const zone_t zone) {
  int soilMoisturePercent;
  size_t soilMoistureValue = analogRead(zone.sensor);
  // Serial.print("soilMoistureValue = ");
  // Serial.println(soilMoistureValue);
  soilMoisturePercent = map(soilMoistureValue, zone.inAir, zone.inWater, 0, 100);
  if(soilMoisturePercent < 0) { soilMoisturePercent = 0; }
  if(soilMoisturePercent > 100) { soilMoisturePercent = 100; }
  // Serial.print("soilMoisturePercent = ");
  // Serial.print(soilMoisturePercent);
  // Serial.println(" %");
  Serial.printf("soil Moisture value = %d; %d%%\n",
    soilMoistureValue, soilMoisturePercent);
  return soilMoisturePercent;
}

void waterPlants(const zone_t zone)
{
  pumpOn(zone);
  delay (zone.interval);                   // water the plant(s)
  pumpOff(zone);
  delay (zone.soak);                       // let the water soak in
}

void pumpOn(const zone_t zone)
{
  reportPumpStateChange(zone, "on");
  analogWrite(zone.pump, zone.speed);
}

void pumpOff(const zone_t zone)
{
  reportPumpStateChange(zone, "off");
  analogWrite(zone.pump, 0);
}

void reportPumpStateChange(const zone_t zone, const char * state)
{
  Serial.printf("Turning pump at pin %d %s\n", zone.pump, state);
}
