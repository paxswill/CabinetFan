#include <math.h>
#include <Arduino.h>
#include "PIDFanController.h"
#include "util.h"

// The minimum change in speed needed before actually changing the speed.
static const float MIN_SPEED_CHANGE = 0.01;

// It'd be nice to include the degree symbol at some point later.
const char * PIDFanController::valueUnits = "C";

PIDFanController::PIDFanController(
  Fan *fan,
  Thermometer * thermometer,
  const char * name,
  float target,
  float k_p,
  float k_i,
  float k_d,
  unsigned long period
):
  FanController(fan, 0.0, 100, target),
  thermometer(thermometer),
  k_p(k_p),
  k_i(k_i),
  k_d(k_d),
  period(period)
{
  this->name = name;
  controllerDebug("Name", name);
  controllerDebug("Kp", k_p);
  controllerDebug("Ki", k_i);
  controllerDebug("Kd", k_d);
  controllerDebug("Period", period);
}

void PIDFanController::periodic(unsigned long currentMillis) {
  if (periodPassed(currentMillis, lastUpdate, period)) {
    controllerDebug("Updating controller", "");
    float elapsedSeconds = ((float)(currentMillis - lastUpdate)) / 1000.;
    controllerDebug("Elapsed seconds", elapsedSeconds);
    // Update `lastUpdate` after we have the elapsed time.
    lastUpdate = currentMillis;
    float temp = thermometer->getTemperature();
    controllerDebug("Current temp", temp);
    const float error = temp - value;
    controllerDebug("error", error);
    float correction = 0.0;
    // Proportional.
    correction += k_p * error;
    controllerDebug("Correction after Kp", correction);
    // Derivative.
    correction += k_d * ((error - previousError) / elapsedSeconds);
    controllerDebug("Correction after Kd", correction);
    // When the error crosses the setpoint, reset the integral
    if (signbit(previousError) != signbit(error)) {
      errorIntegral = 0.0;
    }
    previousError = error;
    // Integral. Only bother updating it if integral control is enabled.
    if (k_i != 0.0) {
      errorIntegral += error * elapsedSeconds;
      // Constrain the error integral to 300
      errorIntegral = max(min(errorIntegral, 300.0), -300.0);
      controllerDebug("Error integral", errorIntegral);
      correction += k_i * errorIntegral;
      controllerDebug("Correction after Ki", correction);
    }
    // Apply the correction and set a new speed as needed.
    float currentSpeed = fan->getSpeed();
    controllerDebug("Current Speed", currentSpeed);
    // Constrain the new speed to the proper bounds.
    float newSpeed = min(1.0, max(0.0, currentSpeed + correction));
    // If the speed would be less than 5%, just stop the fan.
    newSpeed = newSpeed < 0.05 ? 0.0 : newSpeed;
    controllerDebug("New speed", newSpeed);
    fan->setSpeed(newSpeed);
  }
}