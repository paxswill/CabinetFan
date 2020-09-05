#ifndef FAN_FAN_H
#define FAN_FAN_H

#include <stdint.h>

/* Hardcoding five external interrupts, tying this to the 32u4 pretty hard.
 * Ordering is external interrupts 0, 1, 2, 3, 6 (with interrupt numbers
 * 1, 2, 3, 4, 7).
 */
#define NUM_EXTERNAL_INTERRUPTS 5

enum PWMMode {
  fast,
  phaseCorrect,
  phaseFrequencyCorrect
};

class Fan {
  public:
    Fan(
      uint8_t controlPin,
      uint8_t sensePin,
      PWMMode mode = phaseFrequencyCorrect
    );

    Fan(
      uint8_t controlPin,
      int maxRPM,
      PWMMode mode = phaseFrequencyCorrect
    );

    float getSpeed();
    void setSpeed(float fanSpeed);

    uint16_t getRPM();
    void setRPM(int rpmSpeed);

    void periodic();
    void periodic(unsigned long currentMillis);

    static const uint8_t NOT_SET = UINT8_MAX;
  private:
    // The Arduino pin the PWM signal is generated on.
    const uint8_t controlPin;

    /* The Arduino pin the fan's sense/tachometer signal is connected to. If
     * set to the `NOT_SET` value, the fan speed will be estimated from the
     * most recently requested speed and the maximum fan speed.
     */
    const uint8_t sensePin;

    /* The maximum speed the fan can go, in rotations per minute. If `sensePin`
     * is set, this value is detected automatically.
     */
    int maxRPM;

    /* The TOP value used for the PWM signal. See the appropriate pages in the
     * SRM for details on how it affects the PWM signal. Alternatively, read
     * Ken Shirriff's "Secrets of Arduino PWM" article for a different (shorter)
     * explanation.
     */
    int topValue;

    /* The array index used for tracking tachometer ticks from the interrupt
     * handler.
     */
    uint8_t interruptIndex;

    /* The time (in milliseconds since startup) a ramp up then down cycle was
     * started.
     */
    unsigned long rampStartTime = 0;

    /* The final speed (as a floating point percentage of the maximum speed) to
     * be ramped down to at the end of the ramp up then down cycle.
     */
    float rampTarget = 0.0;

    // The last requested speed (as a percentage of the maximum speed).
    float currentSpeed = 0.0;

    // The last know sensed RPM.
    uint16_t lastKnownRPM;

    // These are all initialized to 0, which is equivalent to false
    static bool isTimer1Setup;
    static bool isTimer3Setup;
    static bool isTimer4Setup;
    static bool isExternalInterruptSetup[NUM_EXTERNAL_INTERRUPTS];

    // Private method for directly setting the duty cycle of the PWM signal.
    void _setSpeed(float fanSpeed);

    // Private setup methods
    void setupPWM(PWMMode mode);
    void setupInterrupts();
};
#endif
