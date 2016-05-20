/* @file HelloKeypad.ino
|| @version 1.1
|| @author Alexander Brevig, Evan Simkowitz
|| @contact alexanderbrevig@gmail.com, esimkowitz@wustl.edu
||
|| @description
|| | Demonstrates the simplest use of the matrix Keypad library over I2C on the Photon, Electron, P1, or Core.
|| #
*/
#include "Keypad/Keypad.h"

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {0, 1, 2, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5, 6}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup(){
  Serial.begin(9600);
}
  
void loop(){
  char key = keypad.getKey();
  
  if (key){
    Serial.println(key);
  }
}
