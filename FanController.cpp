#include <Arduino.h>
#include "FanController.h"

float FanController::getValue() {
  return value;
}

void FanController::setValue(float newValue) {
  value = min(maxValue, max(minValue, newValue));
}

void FanController::periodic() {
  periodic(millis());
}