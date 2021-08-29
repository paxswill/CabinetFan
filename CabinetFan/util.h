#ifndef FAN_UTIL_H
#define FAN_UTIL_H

bool periodPassed(
    unsigned long currentMillis,
    unsigned long lastUpdate,
    unsigned long period
);

#endif