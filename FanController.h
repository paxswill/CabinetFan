#ifndef FAN_FAN_CONTROLLER_H
#define FAN_FAN_CONTROLLER_H

#include <float.h>
#include <math.h>
#include "Arduino.h"
#include "Fan.h"

class FanController {
  public:
    FanController(
      Fan *fan,
      float minValue = FLT_MIN,
      float maxValue = FLT_MAX,
      float initialValue = NAN
    );

    // A short display name describing this controller type.
    const char * name;

    // The abbreviation for the units for the set point.
    static const char * valueUnits;

    // A suggested amount to increment the set point value by.
    const float valueStep = 0;

    // The minimum value for the set point.
    const float minValue;

    // The maximum value for the set point.
    const float maxValue;

    // Get the current set point.
    virtual float getValue();

    // Set a new set point.
    virtual void setValue(float newValue);

    /* Convenience function that calls `periodic()` with the result of
     * `millis()`
     */
    void periodic();

    // Called each iteration of the run loop.
    virtual void periodic(unsigned long currentMillis) = 0;
  protected:
    Fan *fan;
    float value;
};
#endif