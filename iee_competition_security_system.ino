#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MQUnifiedsensor.h>

// =====================================================
// OLED
// =====================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

#define BATTERY_PIN   34
#define SWITCH_PIN    32
#define LATCH_PIN     28
#define MQ135_PIN     35
#define BUZZER_PIN    18

// =====================================================
// ESP32 ADC
// =====================================================
#define VOLTAGE_RESOLUTION 3.3
#define ADC_RESOLUTION     4095.0

const float SCALE = 2.0;
const float MIN_BATTERY = 4.5;

// =====================================================
// INTERVALS
// =====================================================
const unsigned long batteryInterval = 1000;
const unsigned long mq135Interval   = 2000;
const unsigned long displayInterval = 500;

unsigned long lastBatteryCheck = 0;
unsigned long lastMQ135Check   = 0;
unsigned long lastDisplayUpdate = 0;

// =====================================================
// GLOBAL VALUES
// =====================================================
float batteryVoltage = 0;
float switchVoltage  = 0;
float ppm            = 0;

bool switchPressed = false;
bool batteryLow    = false;
bool latchState    = false;

// =====================================================
// MQ135
// =====================================================
#define Board               "ESP32"
#define Type                "MQ-135"
#define Voltage_Resolution  3.3
#define ADC_Bit_Resolution  12

MQUnifiedsensor MQ135(
  Board,
  Voltage_Resolution,
  ADC_Bit_Resolution,
  MQ135_PIN,
  Type
);

void setup() {

  Serial.begin(115200);

  pinMode(LATCH_PIN, OUTPUT);
  digitalWrite(LATCH_PIN, LOW);
  pinMode(BUZZER_PIN ,OUTPUT);
  digitalWrite(BUZZER_PIN , LOW);

  // =====================================================
  // OLED INIT
  // =====================================================

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

    Serial.println("OLED FAILED");

    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("System Starting...");
  display.display();


  MQ135.setRegressionMethod(1);

  MQ135.setA(110.47);
  MQ135.setB(-2.862);

  MQ135.init();

  MQ135.setR0(10.22);

  Serial.println("System Ready");
}

void loop() {

  unsigned long now = millis();

  // =====================================================
  // BATTERY TASK
  // =====================================================

  if (now - lastBatteryCheck >= batteryInterval) {

    lastBatteryCheck = now;

    batteryVoltage =
      (analogRead(BATTERY_PIN) *
       VOLTAGE_RESOLUTION /
       ADC_RESOLUTION) * SCALE;

    switchVoltage =
      (analogRead(SWITCH_PIN) *
       VOLTAGE_RESOLUTION /
       ADC_RESOLUTION) * SCALE;

    // to be fixed latter.
    switchPressed = 5 > 3.0;

    batteryLow = batteryVoltage <= MIN_BATTERY;

    latchState = (!batteryLow && switchPressed);

    digitalWrite(
      LATCH_PIN,
      latchState ? HIGH : LOW
    );

    Serial.print("Battery: ");
    Serial.print(batteryVoltage);

    Serial.print(" V | MQ135: ");
    Serial.print(ppm);

    Serial.print(" PPM | Latch: ");
    Serial.println(
      latchState ? "ON" : "OFF"
    );
  }

  // =====================================================
  // MQ135 TASK
  // =====================================================

  if (now - lastMQ135Check >= mq135Interval) {

    lastMQ135Check = now;

    MQ135.update();

    ppm = MQ135.readSensor();
    if (ppm > 400) {
      digitalWrite(BUZZER_PIN ,HIGH);
    } else{
      digitalWrite(BUZZER_PIN ,LOW) ;

    }

  }

  // =====================================================
  // OLED TASK
  // =====================================================

  if (now - lastDisplayUpdate >= displayInterval) {

    lastDisplayUpdate = now;

    display.clearDisplay();

    display.setCursor(0, 0);
    display.println("AIR MONITOR");

    display.println("----------------");

    display.print("Battery: ");
    display.print(batteryVoltage, 2);
    display.println(" V");

    display.print("MQ135: ");
    display.print(ppm, 1);
    display.println(" PPM");

    display.print("Switch: ");
    display.println(
      switchPressed ? "PRESSED" : "OFF"
    );

    display.print("Latch: ");
    display.println(
      latchState ? "ON" : "OFF"
    );

    if (batteryLow) {

      display.println("LOW BATTERY");
    }

    display.display();
  }
}