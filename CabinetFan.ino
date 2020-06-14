#include <limits.h>
#include <util/atomic.h>

#include "Thermometer.h"
#include "Fan.h"
#include "util.h"
#include "FanController.h"
#include "ConstantSpeed.h"
#include "PIDFanController.h"

// Pin Definitions are for the 32u4 Adafruit ItsyBitsy
const byte tachPin = 0;     // Digital 0   PD2  INT2
const byte controlPin = 9;  // Digital 9   PB5  TIMER1A
const byte tempPin = A11;    // D12/A11     PD6  ADC9

Thermometer externalThermo = Thermometer(tempPin);
Thermometer internalThermo = Thermometer();
Fan *fan;
FanController *currentController;
ConstantSpeedController *speedController;
PIDFanController *pController;
PIDFanController * pidController;

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
  //fan = new Fan(controlPin, Fan::NOT_SET, 1200, 5000, phaseFrequencyCorrect);
  fan = new Fan(controlPin, tachPin, 750, 1500, phaseFrequencyCorrect);
  fan->setSpeed(0.8);
  /* Create all fan controllers ahead of time, and then set `currentController`
   * as needed.
   */
  speedController = new ConstantSpeedController(fan);
  Thermometer * currentThermometer = &externalThermo;
  float targetTemp = 24.0;
  pController = new PIDFanController(
    fan,
    currentThermometer,
    "P Controller",
    targetTemp,
    // TODO: These values are garbage!
    6, 0, 0
  );
  pidController = new PIDFanController(
    fan,
    currentThermometer,
    "PID Controller",
    targetTemp,
    // TODO: These values are also garbage and need tuning!
    6, 2.5, 1.3
  );
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