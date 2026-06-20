#include "rfid.h"

MFRC522DriverPinSimple ss_pin(RFID_SS_PIN);
MFRC522DriverSPI driver{ss_pin};
MFRC522 mfrc522{driver};

void initRFID() {
  mfrc522.PCD_Init();
  Serial.println("RFID Reader initialized");
}

bool checkCardPresent() {
  return mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial();
}

String getCardUID() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      uid += "0";
    }
    uid += String(mfrc522.uid.uidByte[i], HEX);
    if (i < mfrc522.uid.size - 1) {
      uid += " ";
    }
  }
  uid.toUpperCase();
  return uid;
}

bool isCardValid(String uid) {
  if (uid == CARD1_UID || uid == CARD2_UID) {
    return true;
  }
  return false;
}