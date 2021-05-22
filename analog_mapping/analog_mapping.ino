/* check which GPIO pin matches each A«n» pin for standard Arduino code
 */
const unsigned long SERIAL_BAUD = 115200;

void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial.available()) {} // Wait until Serial is really ready
  Serial.println("Arduino digital to Analog pin numbers\n");

  Serial.printf("A0…13: %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
    A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13);
  // A0…13: 26 25 34 39 36 4 14 32 15 33 27 12 13 35
}

void loop() {
  delay(500000);
}
