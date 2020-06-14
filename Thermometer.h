#ifndef FAN_THERMOMETER_H
#define FAN_THERMOMETER_H

#include <stdint.h>

class Thermometer {
  public:
    static const uint8_t INTERNAL_SENSOR = 255;
    Thermometer(uint8_t pin): pin(pin) {};
    Thermometer(): pin(Thermometer::INTERNAL_SENSOR) {};
    float getTemperature();
  private:
    const uint8_t pin;
};
#endif
