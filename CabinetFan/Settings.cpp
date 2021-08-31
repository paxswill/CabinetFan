#include <util/crc16.h>
#include <avr/eeprom.h>
#include "Settings.h"
#include "ConstantSpeed.h"
#include "PIDFanController.h"

// Default values for the PID controllers
/* Right now there isn't any facility for changing these values in the
 * controllers, but I'm hoping to add it (or add some sort of automatic,
 * on-line tuning).
 */
const float DEFAULT_K_P = 0.02;
const float DEFAULT_K_I = 0.02;
const float DEFAULT_K_D = 0.05;
const float DEFAULT_SET_POINT = 30.8;
const unsigned long DEFAULT_PERIOD = 60000;
// Default value for the constant speed controller
const float DEFAULT_SPEED = 1500;

Settings::Settings() {
  // Attempt to read everything from EEPROM first.
  eeprom_busy_wait();
  // the dirty flag isn't stored in EEPROM
  size_t readSize = sizeof(this) - sizeof(dirty);
  eeprom_read_block(this, 0x0, readSize);
  /* dirty is a status flag of the object instance, and it starts off as "clean"
   * (aka "not-dirty"). Any modifications will make the object dirty until it is
   * saved.
   */
  dirty = false;
  if (!checkCRC()) {
    // CRC failed, reset all values
    maxRPM = 0;
    minRPM = 0;
    currentType = constant;
    constantSpeedValues = { .value = DEFAULT_SPEED };
    proportionalValues = {
      .K_p = DEFAULT_K_P,
      .period = DEFAULT_PERIOD,
      .value = DEFAULT_SET_POINT
    };
    pidValues = {
      .K_p = DEFAULT_K_P,
      .K_i = DEFAULT_K_I,
      .K_d = DEFAULT_K_D,
      .period = DEFAULT_PERIOD,
      .value = DEFAULT_SET_POINT
    };
    dirty = true;
    // As we've reset all values, recalculate the CRC and save everything.
    save();
  }
}

void Settings::setController(ControllerType newType) {
  dirty = currentType != newType;
  currentType = newType;
}

ControllerType Settings::getController() const {
  return currentType;
}

void Settings::setMaxRPM(uint16_t newMax) {
  dirty = maxRPM != newMax;
  maxRPM = newMax;
}

uint16_t Settings::getMaxRPM() const {
  return maxRPM;
}

void Settings::setMinRPM(uint8_t newMin) {
  dirty = minRPM != newMin;
  minRPM = newMin;
}

uint8_t Settings::getMinRPM() const {
  return minRPM;
}

void Settings::setValue(float newValue) {
  setValue(newValue, currentType);
}

void Settings::setValue(float newValue, ControllerType type) {
  dirty = getValue(type) != newValue;
  switch (type) {
    case constant:
      constantSpeedValues.value = newValue;
      break;
    case proportional:
      proportionalValues.value = newValue;
      break;
    case pid:
      pidValues.value = newValue;
      break;
  }
}

float Settings::getValue() const {
  return getValue(currentType);
}

float Settings::getValue(ControllerType type) const {
  switch (type) {
    case constant:
      return constantSpeedValues.value;
    case proportional:
      return proportionalValues.value;
    case pid:
      return pidValues.value;
    // Silence a compiler warning
    default:
      return 0.0;
  }
}

bool Settings::isDirty() const {
  return dirty;
}

void Settings::save() {
  if (!isDirty()) {
    return;
  }
  crc = calculateCRC();
  // As mentioned in the constructor, dirty isn't saved to EEPROM
  size_t updateSize = sizeof(this) - sizeof(dirty);
  eeprom_update_block(this, 0x0, updateSize);
  // TODO: reimplement using EEPROM interrupt handlers so life can go on while
  // saving.
  // Reset dirty, as we've now all the values.
  dirty = false;
}

// Not every 
FanController * Settings::createCurrentController(
  Fan *fan,
  Thermometer *thermometer
) {
  switch (currentType) {
    case constant:
      return new ConstantSpeedController(
        fan,
        constantSpeedValues.value
      );
    case proportional:
      return new PIDFanController(
        fan,
        thermometer,
        "Proportional Controller",
        proportionalValues.value,
        proportionalValues.K_p,
        0,
        0,
        proportionalValues.period
      );
    case pid:
      return new PIDFanController(
        fan,
        thermometer,
        "PID Controller",
        pidValues.value,
        pidValues.K_p,
        pidValues.K_i,
        pidValues.K_d,
        pidValues.period
      );
    // Just to silence a compiler warning
    default:
      return NULL;
  }
}

uint16_t Settings::calculateCRC() const {
  uint16_t newCRC = 0xffff;
  uint8_t * valuesBytes = ((uint8_t *)this) + offsetof(Settings, maxRPM);
  uint8_t * dirtyAddr = ((uint8_t *)this) + offsetof(Settings, dirty);
  for (; valuesBytes < dirtyAddr; valuesBytes++) {
    newCRC = _crc16_update(newCRC, *valuesBytes);
  }
  return newCRC;
}

bool Settings::checkCRC() const {
  return crc == calculateCRC();
}