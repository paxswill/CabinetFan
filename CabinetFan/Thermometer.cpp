#include "Thermometer.h"
#include <Arduino.h>
#include <avr/sleep.h>

// The internal reference voltage for the ADC (in mV).
static const float V_REF = 2560;

// The resolution of the ADC (10 bits).
static const float ADC_RESOLUTION = 1023.0;

// Conversion offset for Kelvins to degrees Celsius.
static const float KELVIN_CELSIUS = 273.15;

/* External temperature sensor output voltage scaling (in mV per degree
 * Celsius). This value is for the TMP36 (and TMP35).
 */
static const float EXTERNAL_SENSOR_SCALING = 10.0;

// Offset voltage (in mV) for the external temperature sensor (TMP36).
static const float EXTERNAL_SENSOR_OFFSET = 500.0;

// Global use for retrieving the ADC value via an interrupt handler.
static volatile uint16_t rawAdcValue;

uint16_t runLowNoiseAdc();

float Thermometer::getTemperature() const {
  // Doing this manually for a bit more control over how the ADC conversion is
  // performed.
  /* The ADC channels are split between 0-7 and 8-15. The upper channels have
    * MUX5 set, while the lower ones have it unset. MUX0 through MUX2 then
    * encode the channel within the range. The internal temperature sensor is
    * equivalent to channel 15.
    */
  uint8_t adcChannel;
  if (isInternalSensor()) {
    adcChannel = 15;
  } else {
    /* The mapping between pins and channels is weird for the 32u4, and there's
     * some hacks in the Arduino source to allow using pinx or channel numbers.
     */
    adcChannel = analogPinToChannel(pin >= 18 ? pin - 18 : pin);
  }
  /* Check which channel the muxer is currently set to (if it's set to the
   * internal temperature sensor, we can skip a conversion).
   * Because the upper channels are encoded in bit 5 (which is stored in
   * ADCSRB), that bit is shifted over by 2 so it corresponds to the channel
   * numbers.
   */
  uint8_t oldChannel = (ADMUX & 0x7) | ((ADCSRB & 0x20) >> 2);
  // Set ADMUX to use the internal voltage reference and the correct channel.
  // Set the muxer to correct channel, and use the internal voltage source.
  uint8_t newADMUX = _BV(REFS1) | _BV(REFS0);
  newADMUX |= adcChannel & 0x7;
  // Set MUX5 along with the other MUX bits
  ADMUX = newADMUX;
  if (adcChannel > 7) {
    ADCSRB |= _BV(MUX5);
  } else {
    ADCSRB &= ~_BV(MUX5);
  }
  /* Setting all the bits in ADCSRA. Setting ADPS2 and ADPS1 sets the 64x
   * prescaler. Also enable the ADC interrupt so that we can wake from ADC sleep
   * later.
   */
  ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1);
  /* The ADC frequency needs to be between 50kHz and 200kHz, and the ItsyBitsy
   * 32u4 5v has a 16MHz clock. The 128x prescaler gets us down to 125kHz, which
   * works, but apparently setting the ADHSM bit in ADCSRB lets us use higher
   * clocks, in which case the 64x divisor should work which gets us down to
   * 250kHz. If the 128x prescaler is being used, ADPS0 needs to be set as well.
   */
  // Save the value of MUX5 that was set before.
  ADCSRB = (ADCSRB & _BV(MUX5)) | _BV(ADHSM);
  /* Trigger an ADC measurement and wait for it to complete. If this is the
   * first measurement of the internal temperature sensor we need to do two
   * samples, discarding the first value.
   */
  uint8_t numSamples = isInternalSensor() && oldChannel != adcChannel ? 2 : 1;
  for (int i = 0; i < numSamples; i++) {
    ADCSRA |= _BV(ADSC);
    loop_until_bit_is_clear(ADCSRA, ADSC);
  }
  float adcValue = float(runLowNoiseAdc());
  if (isInternalSensor()) {
    /* Apparently the output of the internal temperature sensor is in
     * Kelvins directly, so let's just convert it to celsius.
     */
    return adcValue - KELVIN_CELSIUS;
  } else {
    // Only supporting the TMP36 for the external temperature sensor.
    float milliVolts = adcValue * V_REF / ADC_RESOLUTION;
    return (milliVolts - EXTERNAL_SENSOR_OFFSET) / EXTERNAL_SENSOR_SCALING;
  }
}

size_t Thermometer::printTo(Print& p) const {
  return p.print(getTemperature());
}

bool Thermometer::isInternalSensor() const {
  return pin == Thermometer::INTERNAL_SENSOR;
}

uint16_t runLowNoiseAdc() {
  /* Assume everything (channel, voltage reference, etc) is set up prior to this
   * function, so all that's needed is to kick off a conversion and wait for it
   * to complete. Using idle sleep to try to get a little less noise than out of
   * sleep, but can't use ADC noise reduction sleep as that disables outputs
   * (which are driving the fan).
   */
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_mode();
  /* Need this loop in case the CPU is woken up early (ex: by an external
   * interrupt, like the one used for the fan tachometer). The ADIF flag is
   * reset when the interrupt handler is run.
   */
  loop_until_bit_is_clear(ADCSRA, ADIF);
  return rawAdcValue;
}

ISR(ADC_vect) {
  rawAdcValue = ADCW;
}