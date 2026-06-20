#include <Keypad.h>
#include "mykeypad.h"

char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {ROW1_PIN, ROW2_PIN, ROW3_PIN,ROW4_PIN};
byte colPins[COLS] = {COL1_PIN, COL2_PIN, COL3_PIN};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

char getKeyPress() {
  char k = keypad.getKey();
  return k;
}