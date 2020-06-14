#ifndef FAN_CONSTANT_SPEED_H
#define FAN_CONSTANT_SPEED_H

#include "FanController.h"
class ConstantSpeedController: public FanController {
  public:
    ConstantSpeedController(Fan *fan);
    ConstantSpeedController(Fan *fan, float initialSpeed);

    // A short display name describing this controller type.
    const char * name = "Constant Speed";

    // The abbreviation for the units for the set point.
    static const char * valueUnits;

    // A suggested amount to increment the set point value by.
    const float valueStep = 100.0;

    // Called each iteration of the run loop.
    virtual void periodic(unsigned long currentMillis);
  private:
    // When the fan speed was last checked.
    unsigned long lastUpdate = 0;
};
#endif