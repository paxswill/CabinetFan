#include "Thermometer.h"
#include <Arduino.h>

float Thermometer::getTemperature() const {
  // For external TMP35 sensors this is easy
  if (this->pin != Thermometer::INTERNAL_SENSOR) {
    analogReference(INTERNAL);
    int adcValue = analogRead(this->pin);
    return 100.0 * ((float)adcValue / 1023.0 * 2.56) - 50.0;
  } else {
    // NOTE: this is only implemented for the 32u4
    /* Overview of the process:
     * Select the internal voltage source, select the internal temperature
     * sensor from the ADC muxer, then do _two_ conversions, as it takes
     * that long to switch over to the internal temperature sensor. The
     * value from second conversion is then used to calculate the
     * temperature.
     */
    ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0);
    ADCSRB |= _BV(MUX5);
    // Double conversion time
    ADCSRA |= _BV(ADSC);
    loop_until_bit_is_clear(ADCSRA, ADSC);
    ADCSRA |= _BV(ADSC);
    loop_until_bit_is_clear(ADCSRA, ADSC);
    /* Apparently the output of the internal temperature sensor is in
     * Kelvins directly, so let's just convert it to celsius.
     */
    // TODO: see if you need to explicitly discard the first value, or can
    // we just barrel right past it?
    // TODO: The internal sensor can (according to the datasheet) be wildly
    // inaccurate (±10ºC)
    int adcValue = ADCW;
    return adcValue - 273.0;
  }
}

size_t Thermometer::printTo(Print& p) const {
  return p.print(getTemperature());
}