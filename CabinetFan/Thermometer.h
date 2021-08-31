#ifndef FAN_THERMOMETER_H
#define FAN_THERMOMETER_H

#include <stdint.h>
#include "Arduino.h"
#include "MovingAverage.h"

class Thermometer: public Printable {
  public:
    static const uint8_t INTERNAL_SENSOR = 255;

    Thermometer(uint8_t pin = Thermometer::INTERNAL_SENSOR);
    ~Thermometer();

    float getTemperature() const;

    void periodic();
    void periodic(unsigned long currentMillis);

    // Inheriting from Printable
    virtual size_t printTo(Print& p) const;
  private:
    // The pin the temperature sensor is connected to.
    const uint8_t pin;

    unsigned long lastUpdate = 0;

    MovingAverage<float, 10> * average;

    // Measure the temperature and update the internal moving average.
    void updateTemperature();

    /* Convenience function for determining if the internal temperature sensor
     * is being used.
     */
    bool isInternalSensor() const;
};
#endif
