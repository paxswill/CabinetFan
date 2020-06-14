#include <limits.h>
#include <util/atomic.h>

#include "Thermometer.h"
#include "Fan.h"
#include "util.h"
#include "FanController.h"
#include "ConstantSpeed.h"

// Pin Definitions are for the 32u4 Adafruit ItsyBitsy
const byte tachPin = 0;     // Digital 0   PD2  INT2
const byte controlPin = 9;  // Digital 9   PB5  TIMER1A
const byte tempPin = A11;    // D12/A11     PD6  ADC9

Thermometer externalThermo = Thermometer(tempPin);
Thermometer internalThermo = Thermometer();
Fan *fan;
FanController *currentController;
ConstantSpeedController *speedController;

void setup() {
  /* Fan needs to be constructed *after* Arduino setup has completed, or else
   * the PWM settings will be clobbered.
   */
  /* For reference:
  * Noctua NF-A4x20 speed ranges from 1200 to 5000 rpm
  * Noctua NF-S12A speed ranges from 300 to 1200 rpm
  */
  fan = new Fan(controlPin, Fan::NOT_SET, 1200, 5000, phaseFrequencyCorrect);
  fan->setSpeed(0.8);
  /* Create all fan controllers ahead of time, and then set `currentController`
   * as needed.
   */
  speedController = new ConstantSpeedController(fan);
  // Right now there's only one controller implemented.
  currentController = speedController;
}

// Implicitly set to 0
unsigned long lastUpdate;

void loop() {
  unsigned long currentMillis = millis();
  if (periodPassed(currentMillis, lastUpdate, 3000)) {

    Serial.print("Current speed: ");
    Serial.println(fan->getSpeed());
    Serial.print("Current RPM:   ");
    Serial.println(fan->getRPM());
    lastUpdate = currentMillis;
  }
  fan->periodic(currentMillis);
  currentController->periodic(currentMillis);
}