/* @file HelloKeypad_MCP17.ino
|| @version 1.1.5
|| @author Alexander Brevig, adapted for Particle IDE by Evan Simkowitz
|| @contact alexanderbrevig@gmail.com, esimkowitz@wustl.edu
||
|| @description
|| | Demonstrates the simplest use of the matrix Keypad library. If you want to use the
|| | MCP23008 library, change I2CTYPE to be "Adafruit_MCP23008".
|| #
*/

#include "Keypad_I2C.h"

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char* I2CTYPE = "Adafruit_MCP23017";

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {0, 1, 2, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5, 6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CTYPE );

void setup() {
    Serial.begin(9600);
}

void loop() {
    char key = keypad.getKey();
    if (key) {
        Serial.println(key);
    }
}
