#ifndef RFID_H
#define RFID_H

#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>

// RFID SS pin
#define RFID_SS_PIN 5

// Known card UIDs
#define CARD1_UID "43 69 6D 31"
#define CARD2_UID "43 69 6D 31"

extern MFRC522 mfrc522;

void initRFID();
bool checkCardPresent();
String getCardUID();
bool isCardValid(String uid);

#endif