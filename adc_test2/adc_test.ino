/*
Basic testing of analog input
*/
#include <driver/adc.h>


// Capacitive moisture sensor configuration data
const adc1_channel_t sensorPin = ADC1_CHANNEL_6; // gpio 34 for reading moisture sensor
const adc_atten_t ADC_ATTEN_DB = ADC_ATTEN_DB_11; // 3.9V maximum for sensor readings
const unsigned long RATE_LIMIT = 1000; // microseconds

void setup() {
  Serial.begin(115200);     // open serial port, set the baud rate

  adc_power_acquire();
  adc_gpio_init(ADC_UNIT_1, (adc_channel_t)sensorPin);
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(sensorPin, ADC_ATTEN_DB);
}

void loop() {
  int adc_value = adc1_get_raw(sensorPin);
  Serial.print("Measurement = ");
  Serial.println(adc_value);
  delay(250);
}
