#include <limits.h>
#include <util/atomic.h>

#include "Thermometer.h"

// Pin Definitions are for the 32u4 Adafruit ItsyBitsy
const byte tachPin = 0;     // Digital 0   PD2  INT2
const byte controlPin = 9;  // Digital 9   PB5  TIMER1A
const byte tempPin = A11;    // D12/A11     PD6  ADC9

// 25kHz is the normal PWM frequency for 4-pin fan control
#define FAN_CONTROL_FREQ 25000

/* Specify what kind of PWM to use: fast, phase correct, or
 * phase and frequency correct.
 */
#define USE_FAST_PWM       0
#define USE_PHASE_PWM      0
#define USE_PHASE_FREQ_PWM 1

/* Define the TOP value in terms of the kind of PWM and the
 * clock speed (see Timer/Counter 1 configuration in setup()).
 */
#if USE_FAST_PWM
#define FAN_CONTROL_TOP (F_CPU / FAN_CONTROL_FREQ)
#elif USE_PHASE_PWM || USE_PHASE_FREQ_PWM
#define FAN_CONTROL_TOP (F_CPU / FAN_CONTROL_FREQ / 2)
#else
#error Invalid PWM Mode
#endif

volatile uint8_t currentTachTicks = 0;
uint8_t frequencies = 0;

unsigned long lastFrequencyUpdate = 0;
unsigned long rampStartTime = 0;
float rampTarget = 0.0;
Thermometer externalThermo = Thermometer(tempPin);
Thermometer internalThermo = Thermometer();

void setup() {

  pinMode(controlPin, OUTPUT);
  /*
   * Configure Timer/Counter 1 for fan control as follows:
   * Enable phase correct PWM using ICR1 as the TOP value (mode
   * 10 in table 14-4) on pins PD5 and PD6 (digital pins marked
   * above). To hit a 25kHz duty cycle the clock is going to be
   * set to the same value as the system clock (see table 14-5) 
   * and TOP set to 320 (if using fast PWM, TOP should be 639).
   * I'm using ICR1 as I don't intend to use input capture.
   */
  TCCR1A = _BV(COM1A1);
  TCCR1B = _BV(CS10);
  // Set the waveform generator depending on the PWM mode
#ifdef USE_FAST_PWM
  TCCR1A |= _BV(WGM11);
  TCCR1B |= _BV(WGM13) | _BV(WGM12);
#elif USE_PHASE_PWM
  TCCR1A |= _BV(WGM11);
  TCCR1B |= _BV(WGM13);
#elif USE_PHASE_FREQ_PWM
  TCCR1B |= _BV(WGM13);
#else
#error Invalid PWM mode
#endif
  ICR1 = FAN_CONTROL_TOP;
  // Default to no PWM signal
  OCR1A = 0;
  /* Configure external interrupts for the tach pins.
   */
  pinMode(tachPin, INPUT_PULLUP);
  // Disable interrupts, configure, then re-enable
  uint8_t eimsk = EIMSK;
  // Trigger on rising edge
  EICRA = _BV(ISC31) | _BV(ISC30) | _BV(ISC21) | _BV(ISC20);
  EIMSK = eimsk | _BV(INT2);
}

void loop() {
  unsigned long currentMillis = millis();
  // Update the freqency every second
  if (periodPassed(currentMillis, lastFrequencyUpdate, 1000)) {
    unsigned long period = currentMillis - lastFrequencyUpdate;
    lastFrequencyUpdate = currentMillis;
    uint8_t tachTicks;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      tachTicks = currentTachTicks;
      currentTachTicks = 0;
    }
    unsigned long scaledTicks;
    /* Multiply by 100 to get milliticks, then divide by 2 as
     * there are two ticks per revolution.
     */
    scaledTicks = tachTicks * 500;
    // Convert to frequency (milliticks/millisecond == tick/second)
    scaledTicks /= period;
    // Clamp to 255 Hz, which is 15360RPM
    frequencies = min(scaledTicks, UINT8_MAX);
  }
  /* Check if we're doing a ramp-up cycle and finish it if needed.
   * When starting from a dead stop, the fan should be set to a 30%
   * duty cycle for 2 seconds and then move to the final speed.
   */
  if (rampTarget != 0.0 && periodPassed(currentMillis, rampStartTime, 2000)) {
    _setFans(rampTarget);
    rampTarget = 0.0;
  }
}

/*
 * Check if enough time has elapsed, accounting for millis() overflow.
 */
bool periodPassed(
    unsigned long currentMillis,
    unsigned long lastUpdate,
    unsigned long period
) {
  unsigned long elapsed;
  if (currentMillis < lastUpdate) {
    // millis overflow happened
    elapsed = ULONG_MAX - lastUpdate;
    elapsed += currentMillis;
  } else {
    elapsed = currentMillis - lastUpdate;
  }
  return elapsed > period;
}

/*
 * Set fan speed, accounting for ramp up as needed.
 */
void setFans(float fanSpeed) {
  float scaledSpeed = constrain(fanSpeed, 0.0, 1.0);
  // See ramp up notes in loop for explanation of the ramp-up
  bool stopped = frequencies == 0;
  if (stopped && scaledSpeed < 0.3) {
    // Need to ramp up, then back down
    rampTarget = scaledSpeed;
    rampStartTime = millis();
    _setFans(0.3);
  } else if (rampTarget != 0.0 && scaledSpeed >= 0.3) {
    // Skip ramp up now
    rampTarget = 0.0;
    _setFans(scaledSpeed);
  } else if (rampTarget != 0.0 && scaledSpeed < 0.3) {
    // Just a new ramp down target
    rampTarget = scaledSpeed;
  } else {
    _setFans(scaledSpeed);
  }
}

/*
 * Directly set fan speed.
 */
void _setFans(float fanSpeed) {
  float scaledSpeed = constrain(fanSpeed, 0.0, 1.0);
  uint16_t ocrSpeed = (int)(scaledSpeed * FAN_CONTROL_TOP);
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    OCR1A = ocrSpeed;
  }
}

ISR(INT2_vect) {
  currentTachTicks += 1;
}
