#ifndef PUMP8_H
#define PUMP8_H

// Using 'polyfill' library to allow standard `analogWrite()` function use.
// With that, no ESP32 specific api libraries or methods are needed. for PWM
// motor control or analog sensor reading.
#include <analogWrite.h>
#include "smart_time.h"
#include "watering_management.h"
#include "irrigation_state.h"

#endif
