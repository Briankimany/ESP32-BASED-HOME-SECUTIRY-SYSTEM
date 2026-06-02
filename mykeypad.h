#ifndef MYKEYPAD_H
#define MYKEYPAD_H

#include <Arduino.h>
#include <Keypad.h>

// Keypad configuration
#define ROWS 4
#define COLS 4

// Row and column pins
#define ROW1_PIN 32
#define ROW2_PIN 33
#define ROW3_PIN 25
#define ROW4_PIN 26
#define COL1_PIN 27
#define COL2_PIN 14
#define COL3_PIN 12
#define COL4_PIN 13

extern Keypad keypad;

void initKeypad();
char getKeyPress();

#endif