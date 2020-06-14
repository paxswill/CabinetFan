#include <math.h>
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
  name(name),
  thermometer(thermometer),
  k_p(k_p),
  k_i(k_i),
  k_d(k_d),
  period(period)
{}

void PIDFanController::periodic(unsigned long currentMillis) {
  if (periodPassed(currentMillis, lastUpdate, period)) {
    unsigned long elapsedTime = currentMillis - lastUpdate;
    // Update `lastUpdate` after we have the elapsed time.
    lastUpdate = currentMillis;
    float temp = thermometer->getTemperature();
    /* Turn the fans off if the temperature is lower than the set point (no
     * sense heating up the cabinet just to turn the fans on).
     */
    if (temp < value) {
      /* TODO: if setRPM() is improved at some point, that might allow for more
       * fine-grained control.
       */
      fan->setSpeed(0.0);
    } else {
      // TODO: Is this operation backwards (temp - value)?
      const float error = temp - value;
      // NOTE: the correction 
      float correction = 0.0;
      // Proportional.
      correction += k_p * error;
      // Integral. Only bother updating it if integral control is enabled.
      if (k_i != 0) {
        /* NOTE: Instead of seconds, the time unit is milliseconds as that's
         * how we're keeping track of time anyways.
         */
        // TODO: That part above might not be right.
        errorIntegral += error * elapsedTime;
        correction += k_i * errorIntegral;
      }
      // Derivative.
      correction += k_d * ((error - previousError) / elapsedTime);
      // Apply the correction and set a new speed as needed.
      float currentSpeed = fan->getSpeed();
      // Constrain the new speed to the proper bounds.
      float newSpeed = min(1.0, max(0.0, currentSpeed + correction));
      if (fabsf(currentSpeed - newSpeed) > MIN_SPEED_CHANGE) {
        fan->setSpeed(newSpeed);
      }
    }
  }
}