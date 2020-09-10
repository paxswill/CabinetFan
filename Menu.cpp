#include <avr/pgmspace.h>
#include "Menu.h"
#include "util.h"

Menu::Menu(
  Fan *fan,
  Thermometer *thermometer,
  Stream *controlInterface
):
  fan(fan),
  thermometer(thermometer),
  controlInterface(controlInterface)
{
  controller = settings.createCurrentController(fan, thermometer);
  // Drain the serial buffer
  drain();
  // Show the menu
  printHelp();
}

void Menu::control() {
  unsigned long currentMillis = millis();
  fan->periodic(currentMillis);
  controller->periodic(currentMillis);
  if (logEnabled) {
    if (controlInterface->available()) {
      // Clear out the buffer
      drain();
      controlInterface->println("Stopping logging.");
      logEnabled = false;
    } else if (periodPassed(currentMillis, lastLogTimestamp, 1000)) {
      controlInterface->print(currentMillis);
      controlInterface->print('\t');
      controlInterface->print(fan->getRPM());
      controlInterface->print('\t');
      controlInterface->println(thermometer->getTemperature());
      lastLogTimestamp = currentMillis;
    }
  } else {
    if (controlInterface->available()) {
      char command = controlInterface->read();
      drain(true);
      rootMenu(command);
    }
  }
}

void Menu::drain(bool onlyWhitespace) {
  if (onlyWhitespace) {
    char next = 0;
    // Using the expansive definition of "non-printable" for whitespace
    while (controlInterface->available() && next < 32) {
      controlInterface->read();
      next = controlInterface->peek();
    }
  } else {
    while (controlInterface->available()) {
      controlInterface->read();
    }
  }
}

void Menu::rootMenu(char command) {
  // print a newline to clear the entered character
  controlInterface->println();
  switch (command) {
    case 's':
    case 'S':
      // _S_ave
      if (settings.isDirty()) {
        controlInterface->print("Saving...");
        settings.save();
        controlInterface->println("done!");
      } else {
        controlInterface->println("No settings have changed, skipping save.");
      }
      break;
    case 'p':
    case 'P':
      printStatus();
      break;
    case 'l':
    case 'L':
      // _L_og
      // This is a somewhat large string, so store it in flash.
      controlInterface->println(F(
        "Logging timestamp, RPM, and temperature once a second until any other "
        "character is entered."
      ));
      logEnabled = true;
      break;
    case 'c':
    case 'C':
      // _C_hange controller
      break;
    case 'e':
    case 'E':
      // _E_dit values
      break;
    case 'r':
    case 'R':
      // _R_ecalibrate fan limits
      break;
    default:
      // Print help
      controlInterface->print("Unrecognized command ");
      if (command < 32) {
        controlInterface->print("0x");
        controlInterface->println(command, HEX);
      } else {
        controlInterface->print("'");
        controlInterface->print(command);
        controlInterface->println("'");
      }
      printHelp();
  }
}

void Menu::printStatus() const {
  // Controller status
  controlInterface->print("Current controller: ");
  controlInterface->println(*controller);
  // Fan RPM
  controlInterface->print("RPM: ");
  controlInterface->println(fan->getRPM());
  // temperature
  controlInterface->print("Temperature: ");
  controlInterface->println(*thermometer);
}

void Menu::printHelp() const {
  // This is a big string, so store it in flash
  controlInterface->println(F(
    "Available commands:\n"
    "s - Save current values to EEPROM.\n"
    "p - Print current values.\n"
    "l - Continuously log fan RPM once per second.\n"
    "c - Change the current controller.\n"
    "e - Change the current controller value.\n"
    "r - Recalibrate fan limits.\n"
    "\n"
    "Any unknown command shows this help text."
  ));
}