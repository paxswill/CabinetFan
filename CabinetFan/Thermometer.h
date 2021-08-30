#ifndef FAN_THERMOMETER_H
#define FAN_THERMOMETER_H

#include <stdint.h>
#include "Arduino.h"

class Thermometer: public Printable {
  public:
    static const uint8_t INTERNAL_SENSOR = 255;
    Thermometer(uint8_t pin): pin(pin) {};
    Thermometer(): pin(Thermometer::INTERNAL_SENSOR) {};
    float getTemperature() const;

    // Inheriting from Printable
    virtual size_t printTo(Print& p) const;
  private:
    // The pin the temperature sensor is connected to.
    const uint8_t pin;

    /* Convenience function for determining if the internal temperature sensor
     * is being used.
     */
    bool isInternalSensor() const;
};
#endif
