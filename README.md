# Keypad_I2C

_Particle IDE Library for a keypad connected over I2C_

## Typical usage

Connect a Photon to a keypad using either a [MCP23008](https://www.adafruit.com/products/593) or [MCP23017](https://www.adafruit.com/product/732) I2C controller.

```
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
```

## Examples

See [complete example](examples/HelloKeypad/HelloKeypad.ino) in the examples directory.

## Reference

### `Keypad`

`Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS, I2CTYPE );`

Creates an object to interact with the keypad.
The parameters are as follows:
1. A keymap made using the `makeKeymap()` function, which takes a parameter of an array containing the characters and where they appear on the keypad (see the [example](examples/HelloKeypad/HelloKeypad.ino)).
2. An array of the pin numbers (on the I2C controller) of the rows (you may have to play around with the order of the pins depending on your wiring).
3. An array of the pin numbers (on the I2C controller) of the columns (same deal as the row pins).
4. The number of rows on the keypad.
5. The number of columns on the keypad.
6. A char array containing the name of the I2C controller you are using (either Adafruit_MCP23017 or Adafruit_MCP23008).

### `getKey`

`keypad.getKey();`

Returns a char of whatever key is currently pressed. Returns null if no key is pressed.

## Resource Utilization

The I2C controller uses RX, TX. It must also be connected to power and ground. Other than this, the wiring on the I2C side varies depending on the keypad being used.


## References

- [MCP23008 Datasheet](https://cdn-shop.adafruit.com/datasheets/MCP23008.pdf)
- [MCP23017 datasheet](https://cdn-shop.adafruit.com/datasheets/mcp23017.pdf)

## License
Copyright 2017 Evan Simkowitz

Released under the GPLv3 license
