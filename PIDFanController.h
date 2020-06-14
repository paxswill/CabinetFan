#ifndef FAN_PID_CONTROLLER_H
#define FAN_PID_CONTROLLER_H

#include "FanController.h"
#include "Thermometer.h"

/* A PID Controller that adjusts the fan speed to achieve a set temperature.
 * The temperature is set in degrees Celsius, and can be set between 0 and 100.
 */
class PIDFanController: public FanController {
  public:
    /* Setting any of the tuning constants (`k_p`, `k_i`, `k_p`) to 0 will
     * disable that portion of the controller.
     * The default set point temperature is 27 degrees Celsius, about 80 degrees
     * Fahrenheit.
     */
    PIDFanController(
      Fan *fan,
      Thermometer * thermometer,
      const char * name = "PID Controller",
      float target = 27.0,
      float k_p = 1,
      float k_i = 0,
      float k_d = 0,
      unsigned long period = 1000
    );

    // A short display name describing this controller type.
    const char * name;

    // The abbreviation for the units for the set point.
    static const char * valueUnits;

    // A suggested amount to increment the set point value by.
    const float valueStep = 0.2;

    // 0 degrees Celsius is the minimum value.
    const float minValue = 0.0;

    // 100 degrees Celsius is the maximum value.
    const float maxValue = 100.0;

    virtual void periodic(unsigned long currentMillis);
  private:
    // The thermometer used for determining the process variable
    Thermometer * thermometer;

    // The tuning constants.
    const float k_p;
    const float k_i;
    const float k_d;

    // The period over which change is measured.
    const unsigned long period;

    // The last time the calculations were done.
    unsigned long lastUpdate = 0;

    // The running values.
    float errorIntegral = 0.0;
    float previousError = 0.0;
};
#endif