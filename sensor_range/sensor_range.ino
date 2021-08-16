/*
determine analog range of capacitive soil moisture sensor measurments after processing
through 5 volt op amp follower plus voltage divider. Sensor and Op Amp use 5 volts. Op
Amp being used is not rail to rail, and clips at about 3.9 volts. That is still to high
for the ESP32 analog input, so a voltage divider is used to attenuate that. Currently,
4.7K with 22K which brings the maximum value to a bit under 3.0 volts.
*/
#include <driver/adc.h>

// Capacitive moisture sensor configuration data
const adc1_channel_t sensorPin = ADC1_CHANNEL_6; // gpio 34 for reading moisture sensor
const adc_atten_t ADC_ATTEN_DB = ADC_ATTEN_DB_11; // 3.9V maximum for sensor readings
//const adc_atten_t ADC_ATTEN_DB = ADC_ATTEN_DB_6; // ?V maximum for sensor readings
const unsigned long RATE_LIMIT = 1000; // microseconds
const float factor = 3.14;

void setup() {
  Serial.begin(115200);     // open serial port, set the baud rate

  adc_power_acquire();
  adc_gpio_init(ADC_UNIT_1, (adc_channel_t)sensorPin);
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(sensorPin, ADC_ATTEN_DB);
  delay(500); // Wait for serial to be really ready
  Serial.println("\n\nStart testing");
}

void loop() {
  const unsigned long count = 100000;
  unsigned int mn, mx;
  unsigned long accum;
  mn = 32760;
  mx = 0;
  accum = 0;
  for (unsigned long i = 0; i < count; i++) {
    unsigned int adc_value = adc1_get_raw(sensorPin);
    accum += adc_value;
    if (adc_value < mn) {
      mn = adc_value;
    }
    if (adc_value > mx) {
      mx = adc_value;
    }
    delayMicroseconds(5);
//    delay(15);
/*
    Serial.print("Measurement = ");
    Serial.println(adc_value);

    Serial.print(", min = ");
    Serial.print(mn);
    Serial.print(", max = ");
    Serial.print(mx);
    delay(150);
/* */
  }
/* */
  Serial.print("min = ");
  Serial.print(mn);
  Serial.print(", max = ");
  Serial.print(mx);
  Serial.print(", mid = ");
  Serial.print((mn + mx)/2);
  Serial.print(", avg = ");
  Serial.print(accum/float(count));
  Serial.print(", range = ");
  Serial.print(mx - mn);
  Serial.print("; ");
  Serial.print(factor * ((mn + mx)/2) / 4096);
  Serial.print(" +- ");
//  Serial.print(3.3 * ((mx - mn)/2) / 4096);
  Serial.print(factor * ((mx - mn)/2) / 4096);
  Serial.println(" Volts");
/* */
}
