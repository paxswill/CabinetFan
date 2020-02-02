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

/* Specify what kind of temperature sensor to use.
 */
#define USE_INT_THERMOMETER 0
#define USE_EXT_THERMOMETER 1

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
volatile float temperature = 0.0;
uint8_t frequencies[] = {0, 0};

unsigned long lastFrequencyUpdate = 0;
unsigned long lastTemperatureUpdate = 0;
unsigned long rampStartTime = 0;
float rampTarget = 0.0;

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
  /* Configure the ADC and Timer/Counter 4 to update the temperature
   * every second. Only Timers 0, 1 and 4 can be used to trigger
   * automatic ADC conversions. Timer 0 is used for Arduino things,
   * and Timer 1 is being used for fan control as it's a 16-bit timer
   * and is exposed on more than one pin (Timer 3 only has one pin
   * exposed). This leaves Timer 4, the 10-bit "high speed" timer.
   * The longest TOP value with the largest prescaler gives is a delay
   * of just over a second.
   */
  // Start with Timer/Counter 4 config. No PWM makes for a lot of 0s.
  TCCR4A = 0;
  // Use the 16384 prescaler, see table 15-14
  TCCR4B = _BV(CS43) | _BV(CS42) | _BV(CS41) | _BV(CS40);
  TCCR4C = 0;
  TCCR4D = 0;
  TCCR4E = 0;
  // All 10-bit registers share a high byte.
  // See section 15.11 in the manual for details.
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    TC4H = 0b11;
    OCR4C = 0xFF;
  }
  /* Set the high bits back to 0 to prevent inadvertently setting high
   * bits later.
   */
  TC4H = 0;
  /* Configure ADC to automatically trigger conversions on
   * Timer 4 overflow. The ADC complete interrupt will be used
   * to retrieve the results.
   */
  // NOTE: ADEN is not set until after all setup is complete.
  ADCSRA = _BV(ADATE) | _BV(ADIE);
  /* Both internal and external thermometers need MUX5 set.
   */
  ADCSRB = _BV(ADTS3) | _BV(MUX5);
  // Configure the thermometer.
#if USE_INT_THERMOMETER
  /* The internal reference must be used with the internal temperature
   * sensor.
   */
  ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX0) | _BV(MUX1) | _BV(MUX2);
#elif USE_EXT_THERMOMETER
  /* Use the internal voltage reference for the external thermometer
   * as it tops out at 1.75V (the internal reference voltage is 2.56v)
   * and we'll get better scaling with that instead of the Arduino
   * default of 5V.
   */
  ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX0);
#else
#error Invalid thermometer selection
#endif
  // Kick off the automatic conversions
  ADCSRA |= ADEN;
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
void setFans(float speed) {
  float scaledSpeed = constrain(speed, 0.0, 1.0);
  // See ramp up notes in loop for explanation of the ramp-up
  if (frequencies[0] == 0 && frequencies[1] == 0) {
    rampTarget = scaledSpeed;
    rampStartTime = millis();
    _setFans(0.3);
  } else {
    _setFans(scaledSpeed);
  }
}

/*
 * Directly set fan speed.
 */
void _setFans(float speed) {
  float scaledSpeed = constrain(speed, 0.0, 1.0);
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

/* ADC conversions are going to be automatically triggered, so this
 * handler will just stash the results in a global variable.
 */
ISR(ADC_vect) {
#if USE_INT_THERMOMETER
  /* Apparently the output of the internal temperature sensor is in
   * Kelvins directly, so let's just convert it to celsius.
   */
  // TODO: see if you need to explicitly discard the first value, or can
  // we just barrel right past it?
  // TODO: The internal sensor can (according to the datasheet) be wildly
  // inaccurate (±10ºC)
  temperature = ADCW - 273;
#elif USE_EXT_THERMOMETER
  // Lifting equation from Adafruit (scaling the ADC value back to volts)
  temperature = 100.0 * ((float)ADC / 2.56) - 50.0;
#else
#error Invalid thermometer selection
#endif
  // For debugging purposes track when the temp is updated
  lastTemperatureUpdate = millis();
}
