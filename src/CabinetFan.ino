#include <limits.h>
#include <util/atomic.h>

#include "Thermometer.h"
#include "Fan.h"
#include "util.h"
#include "Menu.h"

// Pin Definitions are for the 32u4 Adafruit ItsyBitsy
const byte tachPin = 0;     // Digital 0   PD2  INT2
const byte controlPin = 9;  // Digital 9   PB5  TIMER1A
const byte tempPin = A11;    // D12/A11     PD6  ADC9

Thermometer externalThermo = Thermometer(tempPin);
Thermometer internalThermo = Thermometer();
Fan *fan;
Menu *menu;

void setup() {
#if DEBUG
  // If in debug mode, wait for serial.
  Serial.begin(115200);
  while (!Serial);
  while (!Serial.available());
#endif

  /* Fan needs to be constructed *after* Arduino setup has completed, or else
   * the PWM settings will be clobbered.
   */
  /* For reference:
  * Noctua NF-A4x20 speed ranges from 1200 to 5000 rpm
  * Noctua NF-S12A speed ranges from 300 to 1200 rpm
  * Gelid Silent 12 PWM speed ranges from 750 to 1500 rpm
  */
  fan = new Fan(controlPin, tachPin, phaseFrequencyCorrect);
  fan->setSpeed(0.8);
  menu = new Menu(fan, &externalThermo, &Serial);
}

// Implicitly set to 0
unsigned long lastUpdate;

void loop() {
  menu->control();
}