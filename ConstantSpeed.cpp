#include "ConstantSpeed.h"
#include "util.h"

// The period of time between updates to the fan speed, in milliseconds.
static const int FAN_UPDATE_PERIOD = 500;

const char * ConstantSpeedController::valueUnits = "RPM";

/* "Default" constructor that uses the midpoint between the maximum and minimum
 * fan speeds for the initial speed.
 */
ConstantSpeedController::ConstantSpeedController(
  Fan *fan
):
  FanController(fan, fan->minRPM, fan->maxRPM)
{}

ConstantSpeedController::ConstantSpeedController(
  Fan *fan,
  float initialSpeed
):
  FanController(fan, fan->minRPM, fan->maxRPM)
{
  if (value > fan->maxRPM) {
    value = (float)(fan->maxRPM);
  }
}

void ConstantSpeedController::periodic(unsigned long currentMillis) {
  if (periodPassed(currentMillis, lastUpdate, FAN_UPDATE_PERIOD)) {
    lastUpdate = currentMillis;
    fan->setRPM((int)value);
  }
}