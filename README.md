---
title: Irrigation Automation
...
<!-- cSpell:enable -->
<!-- # ESP32 Irrigation automation code -->

This repository contains a series of arduino sketches created while exploring the use of an ESP32 board to manage plant watering based on windshield washer pumps and capacitive soil moisture sensors. Each sketch, with supporting files, is stored in its own folder, so that the complete folder can be downloaded directly to an Arduino sketchbook folder, then opened with the IDE. The irrigation repo folder can be treated like an Arduino sketchbook. This is a bit different that the usual git repository, in that "versions" are new folders. Previous versions are still visible in there own folders. This is done to make it easy to browse through the progression, and look at intermediate research.

The initial sketches loaded here are around reading the sensors and controlling the pump motor. Other exploration sketches exist, but are not uploaded here yet. For connecting to WiFi, setting the ESP32 RTC from internet time, and sending email notifications. SMS notifications work as well, as long as an email to SMS gate is available. Some telephone service providers have an Internet portal available for that, to send SMS to their own customers phones.

Some code blocks «will» also have examples and additional backround information in the [esduino](https://github.com/mMerlin/esduino) repository.

* [send text](#link_send_text)
* [capacitive soil moisture sensor](#link_sensor)
* [capacitive soil moisture sensor with pump](#link_pump1)
* [mcpwm explore](#link_esp_motor)
* [capacitive soil moisture sensor with pump2](#link_pump2)
* [capacitive soil moisture sensor with pump3](#link_pump3)
* [capacitive soil moisture sensor with pump4](#link_pump4)
* [capacitive soil moisture sensor with pump5](#link_pump5)
* [pump5](#link_just_pump5)
* [analog test](#link_analog_test)
* [watering](#link_watering)
* [pump6](#link_pump6)
* [pump7](#link_pump7)
* [pump8](#link_pump8)
* [pump9](#link_pump9)

<!--
* [Link](#link_link)
## <a name="link_link">⚓</a> Link
-->

* Features to be implemented, tested, merged
  * water level reservoir sensor
  * wifi provisioning
  * ntp updating (and repeating)
  * sending email/sms notifications
  * «secure access» web site
    * certificate
    * authentication
    * authorization by function
    * configuration
    * status
    * history¦log

## <a name="link_send_text">⚓</a> send text

Exploration of sample code to connect to a local wifi access point, set the time from an NTP server, and send notification reports to email and sms targets. This code is an extension and refactoring of the [mobizt ESP Mail Client](https://github.com/mobizt/ESP-Mail-Client) [send text](https://github.com/mobizt/ESP-Mail-Client/blob/master/examples/Send_Text/Send_Text.ino) example code, combined with code from the [getting data & time from NTP server with esp32](https://lastminuteengineers.com/esp32-ntp-server-date-time-tutorial/) tutorial.

* secrets.h plus template_secrets.h
  * see [esduino](https://github.com/mMerlin/esduino) for information about creating and hiding secrets, template, and .gitignore
* refactored to external .cpp and .h files
* access ntp time server
* send email to multiple targets (including SMS)
* gmail key with 'less secure' setting

## <a name="link_sensor">⚓</a> capacitive soil moisture sensor

Reading values from the capacitive soil moisture sensor. This sketch is for a standard 5V UNO class Arduino board. It is used to test access to the sensor. The sensor calibration values are based on 0 to 1023 analog values, and the code from [Capacitive Soil Moisture Sensor](https://www.sigmaelectronica.net/wp-content/uploads/2018/04/sen0193-humedad-de-suelos.pdf). That is different part than being used in the project (different voltage), but the same connection and features.

## <a name="link_pump1">⚓</a> capacitive soil moisture sensor with pump

PWM speed control of the pump motor, triggered by low moisture levels. This sketch is for a standard 5V UNO class Arduino board. It is used to test the hardware motor control driver (a FET) circuit.

## <a name="link_esp_motor">⚓</a> mcpwm explore

Explore configuration and usage of mcpwm library for motor speed control, without using an H bridge. [Espressif esp idf api mcpwm](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/mcpwm.html) reference and [brushed dc motor control](https://github.com/espressif/esp-idf/tree/1ca2afd/examples/peripherals/mcpwm/mcpwm_brushed_dc_control) example.

## <a name="link_pump2">⚓</a> capacitive soil moisture sensor with pump2

Change to using an ESP32 DevKit board. This uses 3.3 volts, while the sensor works with 5V.  An op-amp with gain locked to 1 is being used to buffer (condition) the signal, and a voltage divider is added (after the op-amp) to reduce the range to match to what the esp32 DevKit board can handle.

Use information from [Espressif esp idf api adc](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/adc.html) and [Espressif esp idf api mcpwm](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/mcpwm.html) references to setup initial analog sensor reading, plus pwm speed control for a pump motor. Initial testing found the adc values to be varying considerably, so multiple readings are used and averaged.

* adc library
* mcpwm library

## <a name="link_pump3">⚓</a> capacitive soil moisture sensor with pump3

Refactor to move detail code to descriptive named functions.

* Same basic code logic as pump2, just structured a bit to move details into functions, keeping the top level flow easier to follow
* More hard-coded literal values moved to global constants
* rename some variables to match conventions

## <a name="link_pump4">⚓</a> capacitive soil moisture sensor with pump4

Convert sensor and control code to a basic state machine that does not use delay. This is preparation for handling multiple sensor and pump pairs.

## <a name="link_pump5">⚓</a> capacitive soil moisture sensor with pump5

Initial handling of multiple sensors and pumps.

* Expanded data structures to handle multiple sensors and water pumps
* Each sensor plus pump pair is handled by a separate state machine
* data structure ideas for using multiple factors to determine timing and amount of water to supply
  * Implementing is well beyond the scope of the current project
* move transition handling code to a callable function that takes a data structure for the specific state machine
* move individual state processing code to separate functions
* add shared resource for pump power
* add minimal logging for pump power resource hogging

## <a name="link_just_pump5">⚓</a> pump5

Implement a shell that will later properly handle time of day and relative time references across system time adjustments from NTP updates, and system time counters wrapping from maximum values to zero.

* implement shell for expanded time handling
* move struct and enum definitions into an include file
* move default state machine data initialization to functions

## <a name="link_analog_test">⚓</a> analog test

[ESP32 AnalogWrite](https://github.com/ERROPiX/ESP32_AnalogWrite) polyfill library encountered while exploring ESP32 example code and tutorials. Reviewing the hardware level functionality of the existing code found no reason that the expanded features of the idf library calls were needed. This sketch verifies that, with the polyfill, standard Arduino IDE code has the capabilities needed.

* [ESP32 AnalogWrite](https://github.com/ERROPiX/ESP32_AnalogWrite) polyfill library
* no IDF / ESP32 specific library references, beyond the polyfill, and what is 'builtin' to the ESP32 boards definitions.
* test `analogRead()` and `analogWrite()` functionality
* `analogReadResolution()` to configure for full 12 bits ADC

ISSUES:

Issues were found with the underlying library code, but the scenarios that trigger problems are not needed for the code in this project.

* [polyfill library](issue_documentation/analogWrite_polyfill.md)
* [analogRead](issue_documentation/analogRead_espressif.md)

## <a name="link_watering">⚓</a> watering

 This sketch starts from [capacitive soil moisture sensor with pump3](#link_pump3), adds the `analogWrite()` pollyfill, then re-implements the hardware level sensing and control logic using 'standard' Arduino IDE functions.

* [ESP32 AnalogWrite](https://github.com/ERROPiX/ESP32_AnalogWrite) polyfill library
* no IDF / ESP32 specific library references, beyond the polyfill, and what is 'builtin' to the ESP32 boards definitions.
* refactor to move control data for a sensor+pump pair into a struct that is passed to functions
  * allow generic code, without a complex set of parameters, to be used for multiple pairs

## <a name="link_pump6">⚓</a> pump6

Incorporate `analogWrite()` pollyfill test code from [watering](#link_watering) into a simplified non state machine version of the irrigation code.

* [ESP32 AnalogWrite](https://github.com/ERROPiX/ESP32_AnalogWrite) polyfill library
* no IDF / ESP32 specific library references, beyond the polyfill, and what is 'builtin' to the ESP32 boards definitions.
* typedefs to add type information for common irrigation specific data usage
* named watering zone struct instance
* array of watering zones, although it contains only a single entry

## <a name="link_pump7">⚓</a> pump7

Re-implement state machine logic on top of the polyfill code.

* `analogWrite()` polyfill
* loop over array of irrigation zone state machines
* refactor data structures
* move structure and enum definitions into include file
* expand state machine states
* extended state debug reporting
* expand doxygen function documentation
* shell for smart time handling

## <a name="link_pump8">⚓</a> pump8

refactor time, hardware watering access, most irrigation state machine code to separate .cpp and .h files.

* shell smart time handling in smart_time.cpp and .h
* hardware accessing watering related code moved to watering_management.cpp and .h
* irrigation state transition code moved to irrigation_state.cpp and .h
* implement state transition logging shell

## <a name="link_pump9">⚓</a> pump9

Adjust state transition processing logic to remove unneeded transient states. Simplify the state machine.

* remove most transient states
* refactor to more generic 'resource' wait, instead of power wait.
* implement emergency shutdown. use when code detects inconsistent state.

<!-- cSpell:disable -->
<!-- cSpell:enable -->
<!--
# cSpell:disable
# cSpell:enable
cSpell:words
cSpell:ignore
cSpell:enableCompoundWords
-->
