#ifndef FAN_SETTINGS_H
#define FAN_SETTINGS_H

#include "Fan.h"
#include "Thermometer.h"
#include "FanController.h"

enum ControllerType: uint8_t {
  constant = 0,
  proportional = 1,
  pid = 2
};

/* The follow structs are all packed because they're being used both as an
 * in-memory representation and for storage in EEPROM.
 */

struct __attribute__((packed)) constantSpeedValues {
  float value;
};

struct __attribute__((packed)) proportionalValues {
  float K_p;
  unsigned long period;
  float value;
};

struct __attribute__((packed)) pidValues {
  float K_p;
  float K_i;
  float K_d;
  unsigned long period;
  float value;
};

class __attribute__((packed)) Settings {
  public:
    Settings();

    void setController(ControllerType newType);
    ControllerType getController() const;

    void setMaxRPM(uint16_t newMax);
    uint16_t getMaxRPM() const;

    void setMinRPM(uint8_t newMin);
    uint8_t getMinRPM() const;

    void setValue(float newValue);
    void setValue(float newValue, ControllerType type);
    float getValue() const;
    float getValue(ControllerType type) const;
    // TODO: At some point expose the PID controller knobs

    bool isDirty() const;
    void save();

    FanController * createCurrentController(
      Fan *fan,
      Thermometer *thermometer
    );
  private:
    // This is also the order for how the values are laid in the EEPROM.
    uint16_t crc;
    uint16_t maxRPM;
    uint8_t minRPM;
    ControllerType currentType;
    struct constantSpeedValues constantSpeedValues;
    struct proportionalValues proportionalValues;
    struct pidValues pidValues;

    bool dirty;

    uint16_t calculateCRC() const;
    bool checkCRC() const;
};
#endif