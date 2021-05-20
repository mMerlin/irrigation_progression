<!-- cSpell:enable -->
# analogWrite ESP32 polyfill library issues

The `analogWrite()` function provided by the polyfill supports `analogWriteResolution()` and other configuration functions to add extra control of PWM signal. However, "really" expanding the duty cycle range requires the use of an extra, optional, non-standard parameter for `analogWrite()`. This project does not need more then 255 steps of PWM, so ignoring that at this time.

Reviewing the polyfill library code, it looks like the extended resolution could have been handled the same was as was done for `analogReadResolution()`. Automatically rescaling values supplied to match hardware capabilities, while allowing the full range specified by the `analogWriteResolution()` to be used in calls to `analogWrite()`. Configuration information is already being stored internally for each pwm channel, so adding the resolution to that should allow `analogWrite()` to use the full range without the extra parameter.

* This looks like an opportunity to re-implement the polyfill library
  * It appears that the design was intended to allow existing Arduino sketch code to transfer as seemlessly as possible to esp32. The choice was made to limit the standard call to `analogWrite()` to the 0 to 255 range, to match normal Arduino use.
  * Implementing scaling would allow `analogWrite()` to auto-adjust to match the `analogWriteResolution()`. If that was set (or defaulted) to 7 bits, the same seamless migration to ESP32 works, but explicitly setting a higher (or lower) resolution would allow `analogWrite()` to use the full specified range, without using that extra parameter, and regardless of the range actually supported by the hardware.
  * Library is small
  * [library README]($HOME/Arduino/libraries/ESP32_AnalogWrite/README.md)
  * [github](https://github.com/ERROPiX/ESP32_AnalogWrite)

<!-- cSpell:disable -->
<!-- cSpell:enable -->
<!--
# cSpell:disable
# cSpell:enable
cSpell:words
cSpell:ignore
cSpell:enableCompoundWords
-->
