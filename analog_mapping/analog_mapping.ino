/* Find out which GPIO pin matches each A«n» pin for standard Arduino code
 *
 * Each of the esp boards are potentially different. The pins brought out from
 * the esp32 module to the board pins can vary. The relation between a GPIO pin
 * number and an ADC«1¦2» CH«n» is fixed based on the module. The relation
 * between an ADC«1¦2» CH«n» and the Arduino A«n» pin is not. That is setup in
 * the board code, which can be different for each board. This sketch provides
 * a simple way to determine the pin mapping when the information can not be
 * found in the documentation.
 */
const unsigned long SERIAL_BAUD = 115200;

void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial.available()) {} // Wait until Serial is really ready
  Serial.println("Arduino digital to Analog pin number mapping\n");

  // Pick a board to test with, compile (verify) using the full set of analog
  // pin numbers, then comment out the ones that fail because they are not
  // defined in this scope. Coment out both the format string line, and the
  // variable reference itself. Upload, and collect the results from the
  // serial monitor.
  Serial.printf(
    "%d=A0\n"
    "%d=A1\n"
    "%d=A2\n"
    "%d=A3\n"
    "%d=A4\n"
    "%d=A5\n"
    "%d=A6\n"
    "%d=A7\n"
    "%d=A8\n"
    "%d=A9\n"
    "%d=A10\n"
    "%d=A11\n"
    "%d=A12\n"
    "%d=A13\n"
    "%d=A14\n"
    "%d=A15\n"
    "%d=A16\n"
    "%d=A17\n"
    "%d=A18\n"
    "%d=A19\n"
    "%c",
    A0,
    A1,
    A2,
    A3,
    A4,
    A5,
    A6,
    A7,
    A8,
    A9,
    A10,
    A11,
    A12,
    A13,
    A14,
    A15,
    A16,
    A17,
    A18,
    A19,
    13);
  //
  // DOIT ESP32 DEVKIT V1 (esp32)
  // 36=A0
  // 39=A3
  // 32=A4
  // 33=A5
  // 34=A6
  // 35=A7
  // 4=A10
  // 0=A11
  // 2=A12
  // 15=A13
  // 13=A14
  // 12=A15
  // 14=A16
  // 27=A17
  // 25=A18
  // 26=A19
  //
  // MH ET LIVE ESP32DevKIT (esp32)
  // 36=A0
  // 39=A3
  // 32=A4
  // 33=A5
  // 34=A6
  // 35=A7
  // 4=A10
  // 0=A11
  // 2=A12
  // 15=A13
  // 13=A14
  // 12=A15
  // 14=A16
  // 27=A17
  // 25=A18
  // 26=A19
  //
  // OLIMEX ESP32-DevKit-LiPo (esp32)
  // 36=A0
  // 39=A3
  // 32=A4
  // 33=A5
  // 34=A6
  // 35=A7
  // 4=A10
  // 0=A11
  // 2=A12
  // 15=A13
  // 13=A14
  // 12=A15
  // 14=A16
  // 27=A17
  // 25=A18
  // 26=A19
  //
  // ESP32 FM DevKit (esp32)
  // — does not compile: No A«n» symbols defined for the board
  //
  // VintLabs ESP32 Devkit (esp32)
  // 36=A0
  // 39=A3
  // 32=A4
  // 33=A5
  // 34=A6
  // 35=A7
  // 4=A10
  // 0=A11
  // 2=A12
  // 15=A13
  // 13=A14
  // 12=A15
  // 14=A16
  // 27=A17
  // 25=A18
  // 26=A19
  //
  // Adafruit ESP32 Feather (esp32)
  // 26=A0
  // 25=A1
  // 34=A2
  // 39=A3
  // 36=A4
  // 4=A5
  // 14=A6
  // 32=A7
  // 15=A8
  // 33=A9
  // 27=A10
  // 12=A11
  // 13=A12
  // 35=A13
  //
  // SpartFun ESP32 Thin (esp32)
  // 36=A0
  // 39=A3
  // 32=A4
  // 33=A5
  // 34=A6
  // 35=A7
  // 4=A10
  // 0=A11
  // 2=A12
  // 15=A13
  // 13=A14
  // 12=A15
  // 14=A16
  // 27=A17
  // 25=A18
  // 26=A19
}

void loop() {
  delay(500000);
}
