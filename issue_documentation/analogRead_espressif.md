<!-- cSpell:enable -->
# ESP32 Espressif analogRead implementation

The `analogReadResolution()` function from the espressif esp32 base library was found to not implement scaling (bit shifting) correctly for values outside of the ADC range directly supported by the hardware. The [Arduino IDE documentation](https://www.arduino.cc/reference/en/language/functions/zero-due-mkr-family/analogreadresolution/) indicates that the intent was to scale values for settings outside the range the board supports, such that swapping to a different board that did support a different range would 'just work'. No code changes needed to get the higher resolution. Filed bug report [AnalogReadResolution and AnalogRead not handling low and high resolutions correctly](https://github.com/espressif/arduino-esp32/issues/5163) to the Espressif library repository. That includes code snippets for a patch.

<!-- cSpell:disable -->
<!-- cSpell:enable -->
<!--
# cSpell:disable
# cSpell:enable
cSpell:words
cSpell:ignore
cSpell:enableCompoundWords
-->
