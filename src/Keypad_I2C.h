/*
||
|| @file Keypad_I2C.h
|| @version 0.1.8
|| @author Mark Stanley, Alexander Brevig, Evan Simkowitz
|| @contact mstanley@technologist.com, alexanderbrevig@gmail.com, esimkowitz@wustl.edu
||
|| @description
|| | This library provides a simple interface for using matrix
|| | keypads over an I2C interface. It supports multiple
|| | keypresses while maintaining backwards compatibility with
|| | the old single key library. It also supports user selectable
|| | pins and definable keymaps.
|| #
||
|| @license
|| | This library is free software; you can redistribute it and/or
|| | modify it under the terms of the GNU General Public
|| | License as published by the Free Software Foundation; version
|| | 3 of the License.
|| |
|| | This library is distributed in the hope that it will be useful,
|| | but WITHOUT ANY WARRANTY; without even the implied warranty of
|| | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
|| | General Public License for more details.
|| |
|| | You should have received a copy of the GNU General Public
|| | License along with this library; if not, write to the Free Software
|| | Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
|| #
||
*/

/*
Currently, I have implemented the MCP23008 and MCP23017 chips for I2C port expansion.
To pick which one you want to use, put a char* with contents "Adafruit_MCP230XX" where
XX is the corresponding number (either 08 or 17). This should be the last parameter in
your constructor.
*/

#ifndef KEYPAD_I2C_H
#define KEYPAD_I2C_H

#include "Keypad_I2C/Key.h"

// #include "application.h"

#include "Adafruit_MCP23008.h"
#include "Adafruit_MCP23017.h"

#define OPEN LOW
#define CLOSED HIGH

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

typedef char KeypadEvent;
typedef unsigned int uint;
typedef unsigned long ulong;

// Made changes according to this post http://arduino.cc/forum/index.php?topic=58337.0
// by Nick Gammon. Thanks for the input Nick. It actually saved 78 bytes for me. :)
typedef struct {
    byte rows;
    byte columns;
} KeypadSize;

#define LIST_MAX 10		// Max number of keys on the active list.
#define MAPSIZE 10		// MAPSIZE is the number of rows (times 16 columns)
#define makeKeymap(x) ((char*)x)

enum I2C {MCP23017, MCP23008};

//class Keypad : public Key, public HAL_obj {
class Keypad : public MyKey {
public:

	I2C I2Ctype;

	Adafruit_MCP23008 mcp8;
	Adafruit_MCP23017 mcp17;

	Keypad(char *userKeymap, byte *row, byte *col, byte numRows, byte numCols, char *i2ctype);

	// for backwards compatibility, calling the constructor without the i2ctype parameter will
	// default to the MCP23008 chip.
	Keypad(char *userKeymap, byte *row, byte *col, byte numRows, byte numCols);

	virtual void pin_mode(byte pinNum, PinMode mode) {
		if (mode == INPUT_PULLUP) {
			switch(I2Ctype) {
				case MCP23008:
				{
					mcp8.pinMode(pinNum, INPUT);
					mcp8.pullUp(pinNum, HIGH);
					break;
				}
				case MCP23017:
				{
					mcp17.pinMode(pinNum, INPUT);
					mcp17.pullUp(pinNum, HIGH);
					break;
				}
				default:
					break;
			}
			return;
		}
		switch(I2Ctype) {
			case MCP23008:
			{
				mcp8.pinMode(pinNum, mode);
				break;
			}
			case MCP23017:
			{
				mcp17.pinMode(pinNum, mode);
				break;
			}
			default:
				break;
		}
	}
	virtual void pin_write(byte pinNum, bool level) {
		switch(I2Ctype) {
			case MCP23008:
			{
				mcp8.digitalWrite(pinNum, level);
				break;
			}
			case MCP23017:
			{
				mcp17.digitalWrite(pinNum, level);
				break;
			}
			default:
				break;
		}
	}
	virtual int  pin_read(byte pinNum) {
		switch(I2Ctype) {
			case MCP23008:
			{
				return mcp8.digitalRead(pinNum);
			}
			case MCP23017:
			{
				return mcp17.digitalRead(pinNum);
			}
			default:
			{
				return -1;
			}
		}
	}

	uint bitMap[MAPSIZE];	// 10 row x 16 column array of bits. Except Due which has 32 columns.
	MyKey key[LIST_MAX];
	unsigned long holdTimer;

	char getKey();
	bool getKeys();
	KeyState getState();
	void begin(char *userKeymap);
	bool isPressed(char keyChar);
	void setDebounceTime(uint);
	void setHoldTime(uint);
	void addEventListener(void (*listener)(char));
	int findInList(char keyChar);
	int findInList(int keyCode);
	char waitForKey();
	bool keyStateChanged();
	byte numKeys();

private:
	unsigned long startTime;
	char *keymap;
	byte *rowPins;
    	byte *columnPins;
	KeypadSize sizeKpd;
	uint debounceTime;
	uint holdTime;
	bool single_key;

	void scanKeys();
	bool updateList();
	void nextKeyState(byte n, bool button);
	void transitionTo(byte n, KeyState nextState);
	void (*keypadEventListener)(char);
};

#endif

/*
|| @changelog
|| | 0.1.8 2016-6-06 - Evan Simkowitz	: Added some comments to make the declaration process clearer.
|| | 0.1.8 2016-6-01 - Evan Simkowitz	: Update to example, turns out it didn't like const char*.
|| | 0.1.7 2016-6-01 - Evan Simkowitz	: Release candidate: Some variable naming has been changed and some reformatting was performed to ensure future
|| |									  ease of adding features. MCP23017 is now fully supported and can be selected by adding an i2ctype parameter to
|| |									  the constructor. For backwards-compatibility, including no i2ctype in the constructor defaults to MCP23008.
|| | 0.1.7 2016-6-01 - Evan Simkowitz	: Trying just importing all the libraries and then deciding which one to use in the code with separate helpers.
|| | 0.1.7 2016-5-31 - Evan Simkowitz	: I am preparing to add MCP23017 compatibility and am setting the groundwork with some reorganization.
|| | 0.1.2 2016-5-20 - Evan Simkowitz	: 0.1.2 published to Particle's library repository.
|| | 0.1.2 2016-5-20 - Evan Simkowitz	: No changes here, but had to update version because of an issue importing to Particle.
|| | 0.1.1 2016-5-20 = Evan Simkowitz	: worked out some issues with reliability
|| | 0.1.0 2016-5-19 - Evan Simkowitz	: Changed name from Keypad.h to Keypad-I2C.h, added integration of I2C
|| | 0.1.0 2016-5-19 - Evan Simkowitz	: Added declaration for Adafruit_MCP23008.h
|| #
*/
