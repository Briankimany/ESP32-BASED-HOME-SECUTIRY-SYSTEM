#include "sensors.h"
#include "config.h"
#include "mykeypad.h"
#include "rfid.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void initOLED() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

// Monitor the keypad for user's inputs.
String getKeypadInputs(String& enteredPassword){
  char key = getKeyPress();

  if (key != NO_KEY) {
    Serial.print("Key pressed: ");
    Serial.println(key);
    
    if (key == '#') {
      // verify the current password.
      return enteredPassword;

    } else if (key == '*') {
      // Clear password
      enteredPassword = "";
      return "";

    } else {
      // Add digit to password
      enteredPassword += key;
      Serial.print("Current password input: ");
      Serial.println(enteredPassword);
      return "WAIT";
    }
  }
  return "WAIT";
}


// Check for keycards. 
boolean validRfidCard(){
  bool verifed = false;
  if (checkCardPresent()) {
    String cardUID = getCardUID();
    Serial.print("Card detected: ");
    Serial.println(cardUID);
    
    if (isCardValid(cardUID)) {
      Serial.println("Access Granted!");
      verifed = true;
    } else {
      Serial.println("Access Denied!");
    }
    
    mfrc522.PICC_HaltA(); 
    mfrc522.PCD_StopCrypto1();  
    
  }

  return verifed;
}







// Function exectuded during the idle state.
void displayHomeScreen(){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("AIR SECURITY SYS");
    display.setCursor(0,8);
    display.println("YOUR PROTECTION IS OUR TOP PRIORITY");
    display.display();

}
void turnOffAlarm(){
  digitalWrite(LED_PIN ,LOW);
}

// Display a message on the oled prompting the user for a card or a password.
void displaypromptForCredentials(){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("ENTER PASSWORD OR USE YOUR RFID CARD.");
    display.setCursor(0,8);
    display.display();
}

// Inform the user of a failed authorization attempt.
void informFailedAuthorization(bool rfid ,bool password){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("FAILED AUTHORIZATION");
    display.setCursor(0,8);
    if (rfid){
      display.println("INVALID CARD"); 
    } else if (password){
      display.println("INVALID PASSWORD.");
    } else {
      display.println("UNKNOWN STATE");
    }
    
    display.display();
}