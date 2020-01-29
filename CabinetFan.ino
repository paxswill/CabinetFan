#include <util/atomic.h>

// Pin Definitions are for the 32u4 Adafruit ItsyBitsy
const byte tach1Pin = 0;     // Digital 0   PD2  INT2
const byte tach2Pin = 1;     // Digital 1   PD3  INT3
const byte control1Pin = 9;  // Digital 9   PB5  TIMER1A
const byte control2Pin = 10; // Digital 10  PB6  TIMER1B
const byte tempPin = A11;    // D12/A11     PD6  ADC9

// See setup() where PWM is configured for details.
#define FAN_CONTROL_TOP 320

volatile long tach1LastTick = 0;
volatile uint16_t tach1Frequency = 0;
volatile long tach2LastTick = 0;
volatile uint16_t tach2Frequency = 0;

void setup() {
  // Could probably gang these together so only one tach is read
  // and both fans are controlled by one PWM signal.
  // But this is more fun.
  pinMode(tach1Pin, INPUT_PULLUP);
  pinMode(tach2Pin, INPUT_PULLUP);
  pinMode(control1Pin, OUTPUT);
  pinMode(control2Pin, OUTPUT);
  /*
   * Configure Timer/Counter 1 as follows:
   * Enable phase correct PWM using ICR1 as the TOP value (mode
   * 10 in table 14-4) on pins PD5 and PD6 (digital pins marked
   * above). To hit a 25kHz duty cycle the clock is going to be
   * set to the same value as the system clock (see table 14-5) 
   * and TOP set to 320 (if using fast PWM, TOP should be 639).
   * I'm using ICR1 as I don't intend to use input capture.
   */
  TCCR1A = _BV(COM1A1) | _BV(COM1B1);
  TCCR1B = _BV(CS10) | _BV(WGM13);
  ICR1 = FAN_CONTROL_TOP;
  // Default to no PWM signal
  OCR1A = 0;
  OCR1B = 0;
}

void loop() {
  // put your main code here, to run repeatedly:

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

void tach1Handler() {
  tachHandler(&tach1LastTick, &tach1Frequency);
}

void tach2Handler() {
  tachHandler(&tach2LastTick, &tach2Frequency);
}

void tachHandler(volatile long * tick, volatile uint16_t * frequency) {
  long now = micros();
  long lastTick;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    lastTick = *tick;
  }
  long elapsed = now - lastTick;
  long rawFrequency = 1000000 / elapsed;
  // Clamp the max to a 16-bit unsigned in
  rawFrequency = min(UINT16_MAX, rawFrequency);
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    frequency = (uint16_t)rawFrequency;
  }
}
