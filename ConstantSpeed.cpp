#include "ConstantSpeed.h"
#include "util.h"

// The period of time between updates to the fan speed, in milliseconds.
static const int FAN_UPDATE_PERIOD = 500;

const char * ConstantSpeedController::valueUnits = "RPM";

ConstantSpeedController::ConstantSpeedController(
  Fan *fan,
  float initialSpeed
):
  FanController(fan, fan->minRPM, fan->maxRPM, initialSpeed)
{}

void ConstantSpeedController::periodic(unsigned long currentMillis) {
  if (periodPassed(currentMillis, lastUpdate, FAN_UPDATE_PERIOD)) {
    controllerDebug("new speed", value);
    lastUpdate = currentMillis;
    fan->setRPM((int)value);
  }
}