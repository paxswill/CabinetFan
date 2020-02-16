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
/* For reference:
 * Noctua NF-A4x20 speed ranges from 1200 to 5000 rpm
 * Noctua NF-S12A speed ranges from 300 to 1200 rpm
 */
Fan fan = Fan(controlPin, tachPin, 1200, 5000);

void setup() {

  fan.setSpeed(0.75);
}

void loop() {
  fan.periodic();
}