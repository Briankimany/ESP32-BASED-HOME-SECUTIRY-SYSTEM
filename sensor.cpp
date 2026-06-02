#include "sensors.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

MQUnifiedsensor MQ135(Board, Voltage_Resolution, ADC_Bit_Resolution, MQ135_PIN, Type);

void initOLED() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void initMQ135() {
  MQ135.setRegressionMethod(1);
  MQ135.setA(110.47);
  MQ135.setB(-2.862);
  MQ135.init();
  MQ135.setR0(10.22);
}

float readBattery() {
  return (analogRead(BATTERY_PIN) * VOLTAGE_RESOLUTION / ADC_RESOLUTION) * SCALE;
}

float readSwitch() {
  return (analogRead(SWITCH_PIN) * VOLTAGE_RESOLUTION / ADC_RESOLUTION) * SCALE;
}

bool isSwitchPressed() {
  return readSwitch() > 3.0;
}

bool isBatteryLow(float voltage) {
  return voltage <= MIN_BATTERY;
}