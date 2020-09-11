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
    /* Create a `Fan` that is able to detect the actual speed with a tachometer
     * pin.
     *
     * ` controlPin` - An Arduino PWM output pin connected to either a 16-bit or
     * 10-bit Timer/Counter.
     * `sensePin` - An Arduino pin number that is able to be used as an external
     * interrupt.
     * `mode` - A `PWMMode` defining the specifics of the PWM signal.
     */
    Fan(
      uint8_t controlPin,
      uint8_t sensePin,
      PWMMode mode = phaseFrequencyCorrect
    );

    /* Create a `Fan` that is *not* able to detect the actual speed of the
     * attached fan.
     *
     * ` controlPin` - An Arduino PWM output pin connected to either a 16-bit or
     * 10-bit Timer/Counter.
     * `maxRPM` - The maximum RPM of the attached fan.
     * `mode` - A `PWMMode` defining the specifics of the PWM signal.
     */
    Fan(
      uint8_t controlPin,
      int maxRPM,
      PWMMode mode = phaseFrequencyCorrect
    );

    // Get the current speed of the attached fan as a range from 0.0 to 1.0.
    float getSpeed() const;

    /* Set the speed of the attached fan.
     *
     * If the requested speed is less than 30% of the maximum and the fan is
     * currently stopped, a ramp up cycle is started. The fan is started at a
     * 30% duty cycle for 2 seconds, then the speed is lowered to the
     * requested level.
     *
     * `fanSpeed` - The requested fan speed as a value between 0.0 and 1.0.
     * Values outside of this range will be clamped to that range.
     */
    void setSpeed(float fanSpeed);

    uint16_t getRPM() const;
    void setRPM(int rpmSpeed);

    void periodic();
    void periodic(unsigned long currentMillis);

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
     * datasheet for details on how it affects the PWM signal. Alternatively,
     * read Ken Shirriff's "Secrets of Arduino PWM" article for a different
     * (shorter) explanation.
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

    // The last known sensed RPM.
    uint16_t lastKnownRPM;

    /* Flags to ensure a timer is not double-configured. It shouldn't hurt
     * anything if it is, but it might be able to disrupt an operating signal.
     *
     * These values are all initialized to 0 (which is equivalent to false) by
     * nature of being declared static.
     */
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
