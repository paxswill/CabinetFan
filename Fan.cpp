#include <Arduino.h>
#include <util/atomic.h>
#include <limits.h>
#include "Fan.h"
#include "util.h"

// The 4-pin fan spec says the PWM frequency should be 25kHz
#define F_CONTROL_PWM 25000

// Declare the actual "instances" of the static members in the Fan class
bool Fan::isTimer1Setup;
bool Fan::isTimer3Setup;
bool Fan::isTimer4Setup;
bool Fan::isExternalInterruptSetup[NUM_EXTERNAL_INTERRUPTS];

/* Keep track of how many times the fan has "ticked" and when we last
 * calculated the RPMs of the fan.
 */
static volatile uint16_t numTicks[NUM_EXTERNAL_INTERRUPTS];
static unsigned long lastTickUpdate[NUM_EXTERNAL_INTERRUPTS];

// NOTE: within a multiline #define, "//" comments will break things
#define setup16BitPWM(timerN) do {\
  if (!isTimer ## timerN ## Setup) {\
    TCCR ## timerN ## A = _BV(COM ## timerN ## A1);\
    /* Set the clock prescaler to match the system clock one to one. */\
    TCCR ## timerN ## B = _BV(CS ## timerN ## 0);\
    /* Set up PWM using ICR. topValue is a previously set \
    * member of the Fan class. \
    */\
    ICR ## timerN = topValue;\
    switch (mode) {\
      case fast:\
        TCCR ## timerN ## A |= _BV(WGM ## timerN ## 1);\
        TCCR ## timerN ## B |= _BV(WGM ## timerN ## 3) | _BV(WGM ## timerN ## 2);\
        break;\
      case phaseCorrect:\
        TCCR ## timerN ## A |= _BV(WGM ## timerN ## 1);\
        TCCR ## timerN ## B |= _BV(WGM ## timerN ## 3);\
        break;\
      case phaseFrequencyCorrect:\
        TCCR ## timerN ## B |= _BV(WGM ## timerN ## 3);\
        break;\
    }\
    isTimer ## timerN ## Setup = true;\
  }\
} while(0);

/* Helper macro for setting 10-bit values for Timer 4. See section 15.11 in the
 * SRM for more details.
 */
#define set10Bit(reg, value) do{\
  /* NOTE: *Always* set TC4H, as its value will always be used for setting. */ \
  TC4H = value >> 8;\
  reg = (uint8_t)value;\
  TC4H = 0;\
  /* ...and as a corralary, always clear TC4H once you're done. */ \
} while(0);

/* Given an external interrupt vector number (on the 32u4, 1-4, 7), return the
 * index for that vector in the `numTicks` and `isExternalInterruptSetup`
 * arrays.
 */
uint8_t getInterruptIndex(uint8_t interruptNumber) {
  // Just chose 5, 6 and 7 would've worked as well.
  if (interruptNumber < 5) {
    return interruptNumber - 1;
  } else {
    // 4 being the last index number
    return 4;
  }
}

Fan::Fan(
  uint8_t controlPin,
  uint8_t sensePin,
  int minRPM,
  int maxRPM,
  PWMMode mode
):
  controlPin(controlPin),
  sensePin(sensePin),
  minRPM(minRPM),
  maxRPM(maxRPM)
{
  // Set up PWM
  pinMode(controlPin, OUTPUT);
  topValue = F_CPU / F_CONTROL_PWM;
  if (mode == phaseCorrect || mode == phaseFrequencyCorrect) {
    /* The phase (and frequency) correct PWM modes count up and then back
     * down, needing twice as long to complete, which is why the TOP value is
     * half of the value used by "fast" PWM mode.
     */
    topValue /= 2;
  }
  switch (digitalPinToTimer(controlPin)) {
    case TIMER0A:
    case TIMER0B:
      /* Timer 0 is not supported as it's used by Arduino already
       * for millis()/micros() (and delay() I think).
       */
      break;
    case TIMER1A:
    case TIMER1B:
    case TIMER1C:
      setup16BitPWM(1);
      break;
    case TIMER3A:
    case TIMER3B:
    case TIMER3C:
      setup16BitPWM(3);
      break;
    case TIMER4A:
    case TIMER4B:
    case TIMER4C:
    case TIMER4D:
      if (!isTimer4Setup) {
        TCCR4A = _BV(COM4A1);
        // As above, setting the clock scaler to match the system clock
        TCCR4B = _BV(CS40);
        /* TOP is always set in OCR4C (which is conveniently *not* exposed on an
        * outside pin).
        */
        set10Bit(OCR4C, topValue);
        switch (mode) {
          case fast:
            /* Fast PWM is enabled with just the PWM4x bits with the WGM4 bits
            * unset.
            */
            break;
          case phaseCorrect:
          /* There is no phase correct PWM mode for Timer 4, only phase and
          * frequency correct. So instead of erroring out, just default to the
          * one that works.
          */
          case phaseFrequencyCorrect:
            /* Phase and frequency correct PWM is set by just setting the WGM40
            * bit along with the appropriate PWM4x bit(s).
            */
            TCCR4D = _BV(WGM40);
            break;
        }
        isTimer4Setup = true;
      }
      break;
  }
  // Set up external interrupts (if needed)
  if (sensePin != NOT_SET) {
    interruptIndex = digitalPinToInterrupt(sensePin);
    uint8_t vectorNumber = interruptIndex + 1;
    if (!isExternalInterruptSetup[interruptIndex]) {
      switch (vectorNumber) {
        case 1:
        case 2:
        case 3:
        case 4:
          /* Skipping the use of the nice register bit position macros for this.
           * Explanation: the ISC bits are laid out ISCn1, ISCn0, with n having
           * the value 3, 2, 1, 0 (in that order) in the EICRA register. So we
           * take the value 3 (in binary 0b11) and bit shift it enough times
           * over. The interrupt vector number is one more than the external
           * interrupt number, so we subtract one. And since we're shifting
           * everything over by 2 bit positions, we have to multiply by two.
           */
          EICRA |= (3 << ((vectorNumber - 1) * 2));
          break;
        case 7:
          /* As opposed to the other external interrupts, interrupt 6 is all by
           * itself on EICRB.
           */
          EICRB = _BV(ISC60) | _BV(ISC61);
          break;
      }
      /* Enable the interrupt. Again, converting between the interrupt vector
       * number and the external interrupt number.
       */
      EIMSK |= 1 << (vectorNumber - 1);
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        numTicks[interruptIndex] = 0;
      }
      lastTickUpdate[interruptIndex] = millis();
      isExternalInterruptSetup[interruptIndex] = true;
    }
    /* Initialize the last known RPM to 0. It should be updated about 1.5
     * seconds after the fan is spun up.
     */
    lastKnownRPM = 0;
  }
}

/* Get the current speed of the fan as a percentage of the maximum speed.
 * If `sensePin` is `NOT_SET`, the speed is assumed to be equal to the last
 * requested speed.
 */
float Fan::getSpeed() {
  if (sensePin == NOT_SET) {
    return currentSpeed;
  } else {
    return getRPM() / maxRPM;
  }
}

/*
 * Set fan speed as a percentage of the maximum, accounting for ramp up as
 * needed.
 */
void Fan::setSpeed(float fanSpeed) {
  float scaledSpeed = constrain(fanSpeed, 0.0, 1.0);
  // See ramp up notes in `periodic` for explanation of the ramp-up
  bool stopped = getSpeed() == 0.0;
  if (stopped && scaledSpeed < 0.3) {
    // Need to ramp up, then back down
    rampTarget = scaledSpeed;
    rampStartTime = millis();
    _setSpeed(0.3);
  } else if (rampTarget != 0.0 && scaledSpeed >= 0.3) {
    // The new speed doesn't need ramp up, so cancel the one in-progress
    rampTarget = 0.0;
    _setSpeed(scaledSpeed);
  } else if (rampTarget != 0.0 && scaledSpeed < 0.3) {
    // Just a new ramp down target
    rampTarget = scaledSpeed;
  } else {
    _setSpeed(scaledSpeed);
  }
}

/*
 * Directly set fan speed by setting the PWM duty cycle.
 */
void Fan::_setSpeed(float fanSpeed) {
  currentSpeed = constrain(fanSpeed, 0.0, 1.0);
  /* Calculate the closest value for the OCRnx register for the appropriate
   * duty cycle.
   */
  uint16_t ocrSpeed = (uint16_t)(currentSpeed * topValue);
  /* All of the registers in question are either 16-bits (so really two 8-bit
   * registers) or 10-bit registers (so a weird shared high register). In both
   * cases it's possible for them to be clobbered if an interrupt is triggered
   * in between accessing the high and low bits. So interrupts are temporarily
   * disabled while the OCRnx register is being set.
   */
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    switch (digitalPinToTimer(controlPin)) {
      case TIMER1A:
        OCR1A = ocrSpeed;
        break;
      case TIMER1B:
        OCR1B = ocrSpeed;
        break;
      case TIMER1C:
        OCR1C = ocrSpeed;
        break;
      case TIMER3A:
        OCR3A = ocrSpeed;
        break;
      case TIMER3B:
        OCR3B = ocrSpeed;
        break;
      case TIMER3C:
        OCR3C = ocrSpeed;
        break;
      case TIMER4A:
        set10Bit(OCR4A, ocrSpeed);
        break;
      case TIMER4B:
        set10Bit(OCR4B, ocrSpeed);
        break;
      // Skipping OCR4C, as it's used for setting the TOP value
      case TIMER4D:
        set10Bit(OCR4D, ocrSpeed);
        break;
    }
  }
}

/* Get the current fan speed in rotations per minute.
 * If `sensePin` is `NOT_SET`, the speed is estimated based on the last
 * requested fractional speed and the provided maximum speed.
 */
uint16_t Fan::getRPM() {
  if (sensePin != NOT_SET) {
    return lastKnownRPM;
  } else {
    /* If we're not sensing, we don't definitively know how fast we're going.
     * Instead we fudge it based on the requested speed and the provided top
     * speed.
     */
    return maxRPM * currentSpeed;
  }
}

// An inexact method of setting the fan speed by a target RPM.
void Fan::setRPM(int rpmSpeed) {
  setSpeed(rpmSpeed / maxRPM);
}

/* Like `periodic(unsigned long)`, but it fills in the current number of
 * elapsed milliseconds for you. The other method is provided to avoid multiple
 * successive calls to millis().
 */
void Fan::periodic() {
  periodic(millis());
}

/* Handle various period tasks. This mehod should be called roughly every
 * second or so.
 */
void Fan::periodic(unsigned long currentMillis) {
  /* Check if we're doing a ramp-up cycle and finish it if needed.
  * When starting from a dead stop, the fan should be set to a 30% duty cycle
  * for 2 seconds and then move to the final speed.
  */
  if (rampTarget != 0.0 && periodPassed(currentMillis, rampStartTime, 2000)) {
    _setSpeed(rampTarget);
    rampTarget = 0.0;
  }
  if (
    sensePin != NOT_SET &&
    periodPassed(currentMillis, lastTickUpdate[interruptIndex], 1500)
  ) {
    unsigned long period;
    if (currentMillis < lastTickUpdate[interruptIndex]) {
      period = currentMillis - lastTickUpdate[interruptIndex];
    } else {
      // Handle the case where `millis()` has overflowed.
      period = ULONG_MAX - lastTickUpdate[interruptIndex] + currentMillis;
    }
    // Reset lastTickUpdate *after* the period has been calculated.
    lastTickUpdate[interruptIndex] = currentMillis;
    uint16_t tickCount;
    /* Disable interrupts while we're retrieving and resetting the tick counts.
     * It's also important to disable interrupts when accessing these values so
     * they don't get corrupted in between operating on the high and low bits.
     */
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      tickCount = numTicks[interruptIndex];
      numTicks[interruptIndex] = 0;
    }
    /* Check to see if we waited too long between updates (very unlikely). If
     * we have waited too long, just leave the last known RPM value in place
     * and hope we check fast enough next time.
     */
    float periodSeconds = (float)period / 1000.0;
    if (periodSeconds < UINT16_MAX / (maxRPM / 60.)) {
      // The tachometer signal is pulsed *twice* for every rotation.
      lastKnownRPM = (tickCount / 2 / periodSeconds);
    }
  }
}

/* Super simple interrupt handlers, just incrementing the appropriate tick
 * counter. Another option for implementing this would be a single interrupt
 * handler for all of the vectors and then checking EIFR (external interrupt
 * flag register) to see which counter needs to be incremented. That approach
 * spends more time in the hanler as there's a series of branches checking EIFR,
 * while this approach is pretty simple, just a two-ish cycle increment of a
 * 16-bit variable (in addition to the interrupt preamble and teardown).
 */
ISR(INT0_vect) { ++numTicks[0]; }
ISR(INT1_vect) { ++numTicks[1]; }
ISR(INT2_vect) { ++numTicks[2]; }
ISR(INT3_vect) { ++numTicks[3]; }
ISR(INT6_vect) { ++numTicks[4]; }