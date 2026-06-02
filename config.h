#ifndef CONFIG_H
#define CONFIG_H

// Pins
#define BATTERY_PIN   34
#define SWITCH_PIN    39
#define LATCH_PIN     28
#define MQ135_PIN     35
#define PIR_PIN       13
#define LED_PIN       4
#define BUZZER_PIN    12

// OLED
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64

// ADC
#define VOLTAGE_RESOLUTION 3.3
#define ADC_RESOLUTION     4095.0
const float SCALE = 2.0;
const float MIN_BATTERY = 4.5;

// Intervals
const unsigned long batteryInterval = 1000;
const unsigned long mq135Interval   = 2000;
const unsigned long displayInterval = 500;
const unsigned long buzzerInterval  = 200;

// PIR
const unsigned long CALIBRATION_TIME = 15000;

// MQ135
#define Board               "ESP32"
#define Type                "MQ-135"
#define Voltage_Resolution  3.3
#define ADC_Bit_Resolution  12

// Security
#define CORRECT_PASSWORD "1234"
const unsigned long DOOR_OPEN_TIME = 5000;  // Door stays open for 5 seconds

#endif