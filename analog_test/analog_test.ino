/* Analog read «adc» and write «pwm» testing for ESP32

  Using 'polyfill' library to allow standard `analogWrite()` function use
 */
#include <analogWrite.h>

const unsigned long SERIAL_BAUD = 115200;

const size_t OUTPUT_PIN = 32; // for pwm with analogWrite()
const size_t INPUT_PIN = A2; // gpio 34 for analogRead()
// const size_t ADC_BITS = 10; // standard Arduino UNO ADC range
const size_t ADC_BITS = 12; // 10 is standard Arduino; ESP32 can do 12
// const unsigned long READING_INTERVAL = 1000; // millis

const size_t MAX_PWM = 255; // at base resolution?

void setup() {
  Serial.begin(SERIAL_BAUD);
  // while (!Serial.available()) {} // Wait until Serial is really ready
  Serial.println("starting test sketch");

  // Tweak (global not per pin) configuration to get desired range
  analogReadResolution(ADC_BITS);

  pinMode(OUTPUT_PIN, OUTPUT);
  // analogWriteFrequency(OUTPUT_PIN, 5000);
  analogWriteResolution(OUTPUT_PIN, 12);
}

void loop() {
  size_t rawADC = analogRead(INPUT_PIN);
  Serial.printf("raw ADC = %d/n", rawADC);
  // delay(READING_INTERVAL);
  fadeCycle();
}

void fadeCycle()
{
  for(size_t i = 0; i < MAX_PWM; i++) {
    analogWrite(OUTPUT_PIN, i);
    delay(2); // slow stepping down `a little`
  }
  for(size_t i = MAX_PWM; i > 0; i--) {
    analogWrite(OUTPUT_PIN, i);
    delay(2); // slow stepping down `a little`
  }
}
