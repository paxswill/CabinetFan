#include <limits.h>
#include <util/atomic.h>

#include "Thermometer.h"
#include "Fan.h"
#include "util.h"

// Pin Definitions are for the 32u4 Adafruit ItsyBitsy
const byte tachPin = 0;     // Digital 0   PD2  INT2
const byte controlPin = 9;  // Digital 9   PB5  TIMER1A
const byte tempPin = A11;    // D12/A11     PD6  ADC9

Thermometer externalThermo = Thermometer(tempPin);
Thermometer internalThermo = Thermometer();
Fan *fan;

void setup() {
  /* Fan needs to be constructed *after* Arduino setup has completed, or else
   * the PWM settings will be clobbered.
   */
  /* For reference:
  * Noctua NF-A4x20 speed ranges from 1200 to 5000 rpm
  * Noctua NF-S12A speed ranges from 300 to 1200 rpm
  */
  fan = new Fan(controlPin, Fan::NOT_SET, 1200, 5000, phaseFrequencyCorrect);
}

void loop() {
  fan->periodic();
}