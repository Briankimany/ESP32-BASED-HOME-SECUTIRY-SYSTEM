#include <Arduino.h>
#include "sensors.h"
#include "rfid.h"
#include "states.h"
#include "camera_handler.h"

// ===== Constants =====
const unsigned long AUTH_WINDOW   = 10000;   // 10 s to present RFID after motion
const unsigned long UNLOCKED_TIME = 30000;   // 30 s before re‑arming
const unsigned long MOTION_COOLDOWN = 5000;  // 5 s cooldown after motion trigger
const unsigned long PIR_READ_INTERVAL = 200; // read PIR every 200 ms (non‑blocking)

// ===== State Machine =====
enum SystemState {
  STATE_ARMED,
  STATE_MOTION_AUTH,
  STATE_UNLOCKED
};
SystemState currentState = STATE_ARMED;

// ===== Timers =====
unsigned long stateTimer       = 0;
unsigned long lastPirRead      = 0;
unsigned long lastMotionTime   = 0;
unsigned long lastBeepTime     = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastCameraSend   = 0;

// ===== Motion =====
bool motionTriggered = false;
bool systemReady = false;
unsigned long startTime = 0;

// ===== RFID =====
bool lastRfidValid = false;

// ===== Buzzer / LED =====
bool beepState = false;

// ===== Setup =====
void setupPins() {
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

void setup() {
  Serial.begin(115200);

  // Print chip info
  uint64_t chipId = ESP.getEfuseMac();
  Serial.printf("ESP32 Chip ID: %llX\n", chipId);
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  Serial.printf("Model: %s\n", 
    (chip_info.model == CHIP_ESP32) ? "ESP32" : 
    (chip_info.model == CHIP_ESP32S2) ? "ESP32-S2" : 
    (chip_info.model == CHIP_ESP32S3) ? "ESP32-S3" : 
    (chip_info.model == CHIP_ESP32C3) ? "ESP32-C3" : "Unknown");
  Serial.printf("Revision: %d\n", chip_info.revision);
  Serial.printf("Cores: %d\n", chip_info.cores);
  
  if (psramFound()) {
    size_t psramSize = ESP.getPsramSize();
    Serial.printf("PSRAM detected! Size: %d bytes (%.2f MB)\n", psramSize, psramSize / 1024.0 / 1024.0);
  } else {
    Serial.println("No PSRAM detected.");
  }

  setupPins();
  initOLED();
  initRFID();
  setupCameraAndWiFi();

  display.setCursor(0, 0);
  display.println("SYSTEM STARTING...");
  display.display();

  startTime = millis();
  Serial.println("Security System Initializing...");
  Serial.print("Calibrating for ");
  Serial.print(CALIBRATION_TIME / 1000);
  Serial.println(" seconds...");

  currentState = STATE_ARMED;
}

// ===== Helper: update OLED =====
void updateOLED(const String& line1, const String& line2 = "", const String& line3 = "") {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(line1);
  if (line2.length()) display.println(line2);
  if (line3.length()) display.println(line3);
  display.display();
}

// ===== Helper: short beep =====
void beep(unsigned long duration = 200) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

// ===== Helper: unlock system =====
void unlockSystem() {
  currentState = STATE_UNLOCKED;
  stateTimer = millis();
  digitalWrite(LED_PIN, HIGH);
  Serial.println("System UNLOCKED");
  updateOLED("UNLOCKED", "Door open");
  sendImageToEndpoint();   // capture on unlock
}

// ===== Helper: re‑arm =====
void reArm() {
  currentState = STATE_ARMED;
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("System ARMED");
  updateOLED("ARMED", "System ready");
}

// ===== Helper: motion trigger =====
void onMotion() {
  motionTriggered = true;
  lastMotionTime = millis();
  currentState = STATE_MOTION_AUTH;
  stateTimer = millis();
  Serial.println("Motion detected - authentication window open");
  updateOLED("MOTION DETECTED", "Present RFID card");
  sendImageToEndpoint();   // capture image on motion
}

// ===== Main Loop =====
void loop() {
  unsigned long now = millis();

  // ---- PIR calibration ----
  if (!systemReady && (now - startTime >= CALIBRATION_TIME)) {
    systemReady = true;
    Serial.println("PIR READY");
    updateOLED("ARMED", "System ready");
  }
  if (!systemReady) return;

  // ---- Read RFID (non‑blocking) ----
  bool rfidValid = validRfidCard();   // returns true if a valid card is present

  // Detect change in RFID state (valid -> invalid or invalid -> valid)
  if (rfidValid != lastRfidValid) {
    if (rfidValid) {
      // Valid card presented
      Serial.println("Valid RFID card detected");
      if (currentState == STATE_ARMED || currentState == STATE_MOTION_AUTH) {
        unlockSystem();
      } else if (currentState == STATE_UNLOCKED) {
        // Already unlocked – maybe reset the unlock timer?
        stateTimer = now;   // reset timer
      }
    } else {
      // Card removed – we could log but nothing else
      Serial.println("RFID card removed");
    }
    lastRfidValid = rfidValid;
  }

  // ---- PIR reading with cooldown ----
  if (now - lastPirRead >= PIR_READ_INTERVAL && currentState != STATE_UNLOCKED) {
    lastPirRead = now;
    // Only read if we are not in cooldown after a motion trigger
    if (now - lastMotionTime >= MOTION_COOLDOWN) {
      int pirState = digitalRead(PIR_PIN);
      if (pirState == HIGH ||  1==1) {
        // Motion detected – only act if we are in ARMED state (or maybe also if in MOTION_AUTH? but we don't want to re‑trigger)
        if (currentState == STATE_ARMED) {
          onMotion();
        }
      }
    }
  }

  // ---- State machine ----
  switch (currentState) {

    case STATE_ARMED:
      // Nothing else to do – RFID handled above, PIR handled above
      break;

    case STATE_MOTION_AUTH:
      // Wait for RFID (already handled above) or timeout
      if (now - stateTimer > AUTH_WINDOW) {
        // No valid RFID within window – go back to armed
        Serial.println("Authentication timeout - re‑arming");
        reArm();
      }
      break;

    case STATE_UNLOCKED:
      // Keep door open; after timeout re‑arm
      if (now - stateTimer > UNLOCKED_TIME) {
        reArm();
      }
      // While unlocked, we ignore motion (no re‑trigger)
      break;
  }

  // ---- Update OLED periodically (every 2 seconds) to show state ----
  if (now - lastDisplayUpdate >= 2000) {
    lastDisplayUpdate = now;
    // Optionally update OLED with status, but we already update on state changes.
    // For good measure, we can re‑display the state if it's unchanged but we want to show something.
    // We'll just do nothing here to avoid flicker.
  }

  // ---- Small delay to prevent watchdog ----
  delay(1);
}