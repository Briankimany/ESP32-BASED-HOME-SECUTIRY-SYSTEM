#ifndef SENSORS_H
#define SENSORS_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

// OLED object
extern Adafruit_SSD1306 display;

void initOLED();

// Sensors utility functions. 
bool validRfidCard();
String getKeypadInputs(String& enteredPassword);

// Functions executed during the off state.
void displayHomeScreen();
void turnOffAlarm();
void displaypromptForCredentials();
void informFailedAuthorization(bool rfid ,bool password);

#endif