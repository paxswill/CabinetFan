#ifndef FAN_MENU_H
#define FAN_MENU_H

#include <stdint.h>
#include <Arduino.h>
#include "Fan.h"
#include "Thermometer.h"
#include "FanController.h"
#include "Settings.h"

class Menu {
  public:
    Menu(
      Fan *fan,
      Thermometer *thermometer,
      Stream *controlInterface = &Serial
    );

    void control();

  private:
    Fan *fan;

    Thermometer *thermometer;

    Settings settings = Settings();

    FanController *controller;

    Stream *controlInterface;

    bool logEnabled = false;

    unsigned long lastLogTimestamp = 0;

    void drain(bool onlyWhitespace = false);

    void rootMenu(char command);
    void printStatus() const;
    void printHelp() const;
    void editValue();
};
#endif