#include "sensors.h"
#include "rfid.h"
#include "mykeypad.h"

// Timers
unsigned long lastBatteryCheck = 0;
unsigned long lastMQ135Check   = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long previousBuzzerMillis = 0;

// Values
float batteryVoltage = 0;
float ppm = 0;
bool latchState = false;

// PIR
bool buzzerState = false;
bool motionDetected = false;
unsigned long lastMotionTime = 0;
bool systemReady = false;
unsigned long startTime = 0;

// Security system
String enteredPassword = "";
bool doorOpen = false;
unsigned long doorOpenTime = 0;
bool alarmArmed = true;

void setup() {
  Serial.begin(115200);

  pinMode(LATCH_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LATCH_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  initOLED();
  display.setCursor(0, 0);
  display.println("SYSTEM STARTING...");
  display.display();

  initMQ135();
  initRFID();
  initKeypad();

  startTime = millis();
  Serial.println("Security System Initializing...");
  Serial.print("Calibrating for ");
  Serial.print(CALIBRATION_TIME / 1000);
  Serial.println(" seconds...");
}

void loop() {
  unsigned long now = millis();

  // PIR calibration
  if (!systemReady && (now - startTime >= CALIBRATION_TIME)) {
    systemReady = true;
    Serial.println("PIR READY");
  }

  // Battery task
  if (now - lastBatteryCheck >= batteryInterval) {
    lastBatteryCheck = now;

    batteryVoltage = readBattery();
    bool switchPressed = isSwitchPressed();
    bool batteryLow = isBatteryLow(batteryVoltage);
    
    // Only close latch if battery is good AND (switch pressed OR door is opened by RFID/keypad)
    if (!batteryLow) {
      if (switchPressed || doorOpen) {
        latchState = true;
      }
    } else {
      latchState = false;
    }

    digitalWrite(LATCH_PIN, latchState ? HIGH : LOW);

    Serial.print("Battery: ");
    Serial.print(batteryVoltage);
    Serial.print(" V | MQ135: ");
    Serial.print(ppm);
    Serial.print(" PPM | Motion: ");
    Serial.print(motionDetected ? "YES" : "NO");
    Serial.print(" | Latch: ");
    Serial.println(latchState ? "ON" : "OFF");
  }

  // MQ135 task
  if (now - lastMQ135Check >= mq135Interval) {
    lastMQ135Check = now;
    MQ135.update();
    ppm = MQ135.readSensor();
  }

  // RFID task
  if (checkCardPresent()) {
    String cardUID = getCardUID();
    Serial.print("Card detected: ");
    Serial.println(cardUID);
    
    if (isCardValid(cardUID)) {
      Serial.println("Access Granted!");
      openDoor();
    } else {
      Serial.println("Access Denied!");
    }
    
    mfrc522.PICC_HaltA();  // Halt PICC
    mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
  }

  // Keypad task
  char key = getKeyPress();
  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);
    
    if (key == '#') {
      // Check password
      if (enteredPassword == CORRECT_PASSWORD) {
        Serial.println("Password Correct! Access Granted!");
        openDoor();
      } else {
        Serial.println("Wrong Password! Access Denied!");
      }
      enteredPassword = "";
    } else if (key == '*') {
      // Clear password
      enteredPassword = "";
      Serial.println("Password cleared");
    } else {
      // Add digit to password
      enteredPassword += key;
      Serial.print("Current password input: ");
      Serial.println(enteredPassword);
    }
  }

  // PIR security task
  if (systemReady && alarmArmed) {
    int pirState = digitalRead(PIR_PIN);

    if (pirState == HIGH) {
      lastMotionTime = now;

      if (!motionDetected) {
        motionDetected = true;
        Serial.println("MOTION DETECTED! ALARM ACTIVE");
      }

      digitalWrite(LED_PIN, HIGH);

      if (now - previousBuzzerMillis >= buzzerInterval) {
        previousBuzzerMillis = now;
        buzzerState = !buzzerState;
        digitalWrite(BUZZER_PIN, buzzerState);
      }
    } else {
      if (motionDetected && (now - lastMotionTime > 2000)) {
        motionDetected = false;
        digitalWrite(LED_PIN, LOW);
        digitalWrite(BUZZER_PIN, LOW);
        buzzerState = false;
        Serial.println("Motion Ended");
      }
    }
  }

  // Door open timer
  if (doorOpen && (now - doorOpenTime >= DOOR_OPEN_TIME)) {
    doorOpen = false;
    Serial.println("Door auto-locked");
  }

  // OLED task
  if (now - lastDisplayUpdate >= displayInterval) {
    lastDisplayUpdate = now;

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("AIR SECURITY SYS");
    display.println("----------------");
    display.print("Battery: ");
    display.print(batteryVoltage, 2);
    display.println("V");
    display.print("MQ135: ");
    display.print(ppm, 1);
    display.println(" PPM");
    display.print("Latch: ");
    display.println(latchState ? "ON" : "OFF");
    display.print("Motion: ");
    display.println(motionDetected ? "DETECTED" : "CLEAR");
    display.print("Door: ");
    display.println(doorOpen ? "OPEN" : "LOCKED");

    if (!systemReady) {
      display.println("CALIBRATING...");
    }

    if (isBatteryLow(batteryVoltage)) {
      display.println("LOW BATTERY");
    }

    display.display();
  }
}

void openDoor() {
  doorOpen = true;
  doorOpenTime = millis();
  alarmArmed = false;  // Disarm alarm when door is opened
  
  if (motionDetected) {
    motionDetected = false;
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    buzzerState = false;
  }
  
  Serial.println("Door opened!");
}