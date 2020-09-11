#ifndef FAN_FAN_CONTROLLER_H
#define FAN_FAN_CONTROLLER_H

#include <float.h>
#include <math.h>
#include "Arduino.h"
#include "Fan.h"

class FanController: public Printable {
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

    // When enabled, debug statements are logged to the serial console.
#if DEBUG
#warning Debug Mode Enabled!
    bool debug = true;
#else
    bool debug = false;
#endif

    // Get the current set point.
    virtual float getValue() const;

    // Set a new set point.
    virtual void setValue(float newValue);

    /* Convenience function that calls `periodic()` with the result of
     * `millis()`
     */
    void periodic();

    // Called each iteration of the run loop.
    virtual void periodic(unsigned long currentMillis) = 0;

    void controllerDebug(const char * message);
    void controllerDebug(const char * message, const char * value);
    void controllerDebug(const char * message, float value);
    void controllerDebug(const char * message, unsigned long value);
    void controllerDebug(const char * message, uint16_t value);

    // Inheriting from Printable
    virtual size_t printTo(Print& p) const;
  protected:
    Fan *fan;
    float value;
};
#endif