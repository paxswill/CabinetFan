#include <limits.h>
/*
 * Check if enough time has elapsed, accounting for millis() overflow.
 */
bool periodPassed(
    unsigned long currentMillis,
    unsigned long lastUpdate,
    unsigned long period
) {
  unsigned long elapsed;
  if (currentMillis < lastUpdate) {
    // millis overflow happened
    elapsed = ULONG_MAX - lastUpdate;
    elapsed += currentMillis;
  } else {
    elapsed = currentMillis - lastUpdate;
  }
  return elapsed > period;
}
