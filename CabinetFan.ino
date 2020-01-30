#include <limits.h>
#include <util/atomic.h>

// Pin Definitions are for the 32u4 Adafruit ItsyBitsy
const byte tach1Pin = 0;     // Digital 0   PD2  INT2
const byte tach2Pin = 1;     // Digital 1   PD3  INT3
const byte control1Pin = 9;  // Digital 9   PB5  TIMER1A
const byte control2Pin = 10; // Digital 10  PB6  TIMER1B
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

volatile uint8_t currentTachTicks[] = {0, 0};
uint8_t frequencies[] = {0, 0};

unsigned long lastFrequencyUpdate = 0;

void setup() {
  // Could probably gang these together so only one tach is read
  // and both fans are controlled by one PWM signal.
  // But this is more fun.

  pinMode(control1Pin, OUTPUT);
  pinMode(control2Pin, OUTPUT);
  /*
   * Configure Timer/Counter 1 for fan control as follows:
   * Enable phase correct PWM using ICR1 as the TOP value (mode
   * 10 in table 14-4) on pins PD5 and PD6 (digital pins marked
   * above). To hit a 25kHz duty cycle the clock is going to be
   * set to the same value as the system clock (see table 14-5) 
   * and TOP set to 320 (if using fast PWM, TOP should be 639).
   * I'm using ICR1 as I don't intend to use input capture.
   */
  TCCR1A = _BV(COM1A1) | _BV(COM1B1);
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
  OCR1B = 0;
  /* Configure external interrupts for the tach pins.
   */
  pinMode(tach1Pin, INPUT_PULLUP);
  pinMode(tach2Pin, INPUT_PULLUP);
  // Disable interrupts, configure, then re-enable
  uint8_t eimsk = EIMSK;
  // Trigger on rising edge
  EICRA = _BV(ISC31) | _BV(ISC30) | _BV(ISC21) | _BV(ISC20);
  EIMSK = eimsk | _BV(INT2) | _BV(INT3);
}

void loop() {
  unsigned long currentMillis = millis();
  // Update the freqency every second
  if (periodPassed(currentMillis, lastFrequencyUpdate, 1000)) {
    lastFrequencyUpdate = currentMillis;
    uint8_t tachTicks[2];
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      for (int i = 0; i < 2; i++) {
        tachTicks[i] = currentTachTicks[i];
        currentTachTicks[i] = 0;
      }
    }
    unsigned long period = currentMillis - lastFrequencyUpdate;
    unsigned long scaledTicks[2];
    for (int i = 0; i < 2; i++) {
      /* Multiply by 100 to get milliticks, then divide by 2 as
       * there are two ticks per revolution.
       */
      scaledTicks[i] = tachTicks[i] * 500;
      // Convert to frequency (milliticks/millisecond == tick/second)
      scaledTicks[i] /= period;
      // Clamp to 256 Hz, which is 15360RPM
      frequencies[i] = min(scaledTicks[i], UINT8_MAX);
    }
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

void setFans(float speed) {
  float scaledSpeed = constrain(speed, 0.0, 1.0);
  // TODO: Fix this so it actually sets low speed properly
  // Clamp speeds to a minimum of 30% to avoid having to ramp up
  scaledSpeed = max(scaledSpeed, 0.3);

  uint16_t ocrSpeed = (int)(scaledSpeed * FAN_CONTROL_TOP);
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    OCR1A = ocrSpeed;
    OCR1B = ocrSpeed;
  }
}

// Using a combined external interrupt handler for tach ticks
ISR(INT2_vect) {
  volatile uint8_t * ticker = NULL;
  // tach 1 is int 2, tach 2 is int 3
  if (bit_is_set(EIFR, INTF2)) {
    ticker = currentTachTicks;
  } else if (bit_is_set(EIFR, INTF3)) {
    ticker = currentTachTicks + 1;
  }
  (*ticker)++;
}
ISR(INT3_vect, ISR_ALIASOF(INT2_vect));

}
