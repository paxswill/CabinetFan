#include <avr/pgmspace.h>
#include "Menu.h"
#include "util.h"

// The timeout when waiting for input from the user over serial
static int INPUT_TIMEOUT = 5000;

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
  thermometer->periodic(currentMillis);
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
      changeController();
      break;
    case 'e':
    case 'E':
      // _E_dit values
      editValue();
      break;
    case 'r':
    case 'R':
      // _R_ecalibrate fan limits
      break;
    case 'd':
    case 'D':
      // Toggle fan controller _D_ebug
      controlInterface->println("Toggling controller debug logging");
      controller->toggleDebug();
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
      __attribute__ ((fallthrough));
    case '\n':
    case '\r':
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
    "Available commands:\r\n"
    "s - Save current values to EEPROM.\r\n"
    "p - Print current values.\r\n"
    "l - Continuously log fan RPM once per second.\r\n"
    "c - Change the current controller.\r\n"
    "e - Change the current controller value.\r\n"
    "r - Recalibrate fan limits.\r\n"
    "d - Toggle fan controller debug logging.\r\n"
    "\r\n"
    "Any unknown command shows this help text."
  ));
}

void Menu::editValue() {
  // Print the current value
  controlInterface->print("Current value: ");
  controlInterface->print(controller->getValue());
  controlInterface->print(" ");
  controlInterface->println(controller->valueUnits);
  // Give the limits
  float minValue = controller->minValue;
  float maxValue = controller->maxValue;
  controlInterface->print("Enter a value between ");
  controlInterface->print(minValue);
  controlInterface->print(" and ");
  controlInterface->print(maxValue);
  controlInterface->print(": ");
  /* Start reading in a new value. Give a larger timeout to read the prompt and
   * enter a number.
   */
  controlInterface->setTimeout(INPUT_TIMEOUT);
  float newValue = controlInterface->parseFloat();
  if (newValue < minValue || newValue > maxValue) {
    controlInterface->println("Out of range value entered. Ignoring.");
  } else {
    settings.setValue(newValue);
    controller->setValue(newValue);
    controlInterface->println("New value set. Settings have NOT been saved.");
  }
}

void Menu::changeController() {
  // Print the current controller and what the options are.
  controlInterface->print("Current controller: ");
  controlInterface->println(*controller);
  controlInterface->println("Available controllers:");
  controlInterface->println("\tconstant");
  controlInterface->println("\tproportional");
  controlInterface->println("\tpid");
  // Get the new controller
  controlInterface->setTimeout(INPUT_TIMEOUT);
  String controllerInput = controlInterface->readStringUntil('\r');
  controllerInput.trim();
  ControllerType newController;
  if (controllerInput.equalsIgnoreCase(String("constant"))) {
    newController = ControllerType::constant;
    controlInterface->println("Changing to constant speed controller");
  } else if (controllerInput.equalsIgnoreCase(String("proportional"))) {
    newController = ControllerType::proportional;
    controlInterface->println("Changing to proportional speed controller");
  } else if (controllerInput.equalsIgnoreCase(String("pid"))) {
    newController = ControllerType::pid;
    controlInterface->println("Changing to PID controller");
  } else {
    controlInterface->print("Unknown controller type \"");
    controlInterface->print(controllerInput);
    controlInterface->println("\"");
    return;
  }
  settings.setController(newController);
  delete controller;
  controller = settings.createCurrentController(fan, thermometer);
}
