/**
 * metods for manipulating smart time values
*/
#include "smart_time.h"

/**
 * get the smart time version of `now`
 *
 * @return current smart time value
 */
smart_time_t getSmartTime()
{
  smart_time_t sTime;
  sTime.epoch = 0;
  sTime.millis = millis();
  return sTime;
} // end getSmartTime()

/**
 * add milliseconds to a smart time
 *
 * @param[in] base existing smart time
 * @param[in] millis offset (delta) time
 * @return smart time plus millis
 */
smart_time_t smartOffsetMillis(const smart_time_t base, const unsigned long millis)
{
  smart_time_t sTime;
  sTime.epoch = base.epoch;
  sTime.millis = base.millis + millis;
  return sTime;
} // end smartOffsetMillis()

/**
 * compare 2 smart time values
 *
 * @param[in] base base smart time reference
 * @param[in] other another smart time reference
 * @return -1,0,1 when base is <, =, > other time
 */
int smartTimeCompare(const smart_time_t base, const smart_time_t other)
{
  // return base.millis <=> other.millis;
  if (base.millis < other.millis) {
    return -1;
  }
  if (base.millis > other.millis) {
    return 1;
  }
  return 0;
} // end smartTimeCompare()

/**
 * get milliseconds difference between smart time values
 *
 * NOTE: returns unsigned value, so if end < start, a very large value will
 *   be returned
 *
 * @param start starting time reference
 * @param end ending time reference
 * @return milliseconds difference
 */
unsigned long smartDeltaMillis(smart_time_t start, smart_time_t end)
{
  return end.millis - start.millis;
} // end smartDeltaMillis
