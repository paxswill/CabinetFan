#include <Arduino.h>
#include "FanController.h"

FanController::FanController(
  Fan * fan,
  float minValue,
  float maxValue,
  float initialValue
):
  minValue(minValue),
  maxValue(maxValue),
  fan(fan)
{
  /* Default to the middle of the acceptable range if an invalid initial value
   * is given.
   */
  if (initialValue == NAN) {
    value = (maxValue - minValue) / 2.;
  } else {
    value = min(maxValue, max(minValue, initialValue));
  }
}

float FanController::getValue() {
  return value;
}

void FanController::setValue(float newValue) {
  value = min(maxValue, max(minValue, newValue));
}

void FanController::periodic() {
  periodic(millis());
}

void FanController::controllerDebug(const char * message) {
  if (debug) {
    Serial.print("\t");
    Serial.print(name);
    Serial.print(": ");
    Serial.println(message);
  }
}

void FanController::controllerDebug(const char * message, const char * value) {
  if (debug) {
    Serial.print("\t");
    Serial.print(name);
    Serial.print(": ");
    Serial.print(message);
    Serial.print(": ");
    Serial.println(value);
  }
}

void FanController::controllerDebug(const char * message, float value) {
  if (debug) {
    Serial.print("\t");
    Serial.print(name);
    Serial.print(": ");
    Serial.print(message);
    Serial.print(": ");
    Serial.println(value);
  }
}

void FanController::controllerDebug(const char * message, unsigned long value) {
  if (debug) {
    Serial.print("\t");
    Serial.print(name);
    Serial.print(": ");
    Serial.print(message);
    Serial.print(": ");
    Serial.println(value);
  }
}

void FanController::controllerDebug(const char * message, uint16_t value) {
  if (debug) {
    Serial.print("\t");
    Serial.print(name);
    Serial.print(": ");
    Serial.print(message);
    Serial.print(": ");
    Serial.println(value);
  }
}