#ifndef smart_time_h
#define smart_time_h

#include <Arduino.h>

/**
 * data structures and functions needed for handling time intervals when the
 * clock can change, and values can wrap from maximum back to zero.
 *
 * Time-Of-Day can change when updated through NTF
 * offsets from timing values based on millis() can wrap around to zero
 * daylight savings time changes *should* be ok, as long as all TOD references
 *   are based on `epoch`, since that does not shift
 *
 * This is still a shell. It only works on millis values, without checking for
 * wrapping. It should be turned into a class.
 *
 * @member epoch linux epoch time reference
 * @member millis milliseconds
 *   initially, the raw millis() time reference
 *   evolve to offset delta to compare epoch values before and after clock adjustments
 */
struct smart_time_t {
  unsigned long epoch;
  unsigned long millis;
};

const struct smart_time_t NULL_TIME = { 0, 0 };

smart_time_t getSmartTime(void);
smart_time_t smartOffsetMillis(const smart_time_t, const unsigned long);
int smartTimeCompare(const smart_time_t, const smart_time_t);
unsigned long smartDeltaMillis(smart_time_t, smart_time_t);
// smartTo«date¦string¦utf¦iso»

#endif
