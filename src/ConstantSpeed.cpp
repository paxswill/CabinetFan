#include "ConstantSpeed.h"
#include "util.h"

// The period of time between updates to the fan speed, in milliseconds.
static const int FAN_UPDATE_PERIOD = 500;

const char * ConstantSpeedController::valueUnits = "RPM";

ConstantSpeedController::ConstantSpeedController(
  Fan *fan,
  float initialSpeed
):
  FanController(fan, 0.0, 1.0, initialSpeed)
{
  this->name = "Constant Speed";
}

void ConstantSpeedController::periodic(unsigned long currentMillis) {
  if (periodPassed(currentMillis, lastUpdate, FAN_UPDATE_PERIOD)) {
    controllerDebug("new speed", value);
    lastUpdate = currentMillis;
    fan->setSpeed(value);
  }
}