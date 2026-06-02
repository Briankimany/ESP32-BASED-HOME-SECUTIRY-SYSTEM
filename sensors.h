#ifndef SENSORS_H
#define SENSORS_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MQUnifiedsensor.h>
#include "config.h"

// OLED object
extern Adafruit_SSD1306 display;

// MQ135 object
extern MQUnifiedsensor MQ135;

// Functions
void initOLED();
void initMQ135();
float readBattery();
float readSwitch();
bool isSwitchPressed();
bool isBatteryLow(float voltage);

#endif