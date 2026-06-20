#include <Arduino.h>
#include "sensors.h"
#include "rfid.h"

#include "states.h"

// Timers

unsigned long lastDisplayUpdate = 0;
unsigned long previousBuzzerMillis = 0;
unsigned long pirCheckInterval = 100;
unsigned long lastPirCheck =0;
unsigned long lastKeyPadCheck = 0;
unsigned long keyPadCheckInterval=5;
unsigned long lastPasswordInput =0;
unsigned long passwordInputWindow = 30000;


bool passwordInputActive= false;


// PIR
bool buzzerState = false;
bool motionDetected = false;
unsigned long lastMotionTime = 0;
bool systemReady = false;
unsigned long startTime = 0;

// Security system
String enteredPassword = "";
String currentPass = "";
bool doorOpen = false;
unsigned long doorOpenTime = 0;
bool alarmArmed = true;

void setupPins(){
  //Set the input output pins.

  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

}


void setup() {
  Serial.begin(115200);

  setupPins();

  initOLED();
  display.setCursor(0, 0);

  display.println("SYSTEM STARTING...");
  display.display();

  initRFID();

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

  // Sample inputs from sensors.
  bool verifiedRfidTag = validRfidCard();
  if (now-lastKeyPadCheck >=keyPadCheckInterval){
     currentPass = getKeypadInputs(enteredPassword);
     lastKeyPadCheck = now;

  }
 
  // Monitor the pir for motion.
  if (systemReady && alarmArmed  && now - lastPirCheck  >= pirCheckInterval) {
    int pirState = digitalRead(PIR_PIN);
    lastPirCheck = now;

    if (pirState == HIGH) {
      motionDetected = true;
      pirCheckInterval = 5000;
    
    } else {
      motionDetected = false;
      pirCheckInterval = 10;
    }
  }

  // Determing which state the   system is currently in.
  // Idle  = ( no entered password, no valid rfid card and there is no motion detected.)
  // Logging = (activated when the user is inputing something using the keypad or an invalid rfid car was used.)

  if (currentPass == "" && !verifiedRfidTag && !motionDetected){
    idle(); 
  }

  else {
    if (verifiedRfidTag){
      // Transfer to unlocked state and log.
      Serial.println("SYSTEM UNLOCKED BY AN RFID CARD.");

    } else if (currentPass != "WAIT" && currentPass != ""){
        if (enteredPassword == CORRECT_PASSWORD){
          // Transfer to unlocked state and log.
          Serial.println("SYSTEM UNLOCKED USING KEYPAD.");
          
        } else {
          // Transfer to logging state and log the failed authorization attempt.
          Serial.println("FAILED PASSWORD AUTHENTICATOIN.");
          informFailedAuthorization(false ,true);

        }

        enteredPassword = "";
        currentPass = "";

    } else if (motionDetected){
      // Motion triggered authorization or logging. 
      Serial.println("MOTION DETECTED.");
      motionDetected = false;
      if (!passwordInputActive){
        displaypromptForCredentials();
      } else{
        display.clearDisplay();
        display.println("INVALID PASSWORD.");
        display.println(enteredPassword);
        display.display();
      }
    }    
  }
}
