/*
||
|| @file Keypad_I2C.cpp
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

#include "Keypad_I2C.h"

// <<constructor>> Allows custom keymap, pin configuration, and keypad sizes.
Keypad::Keypad(char *userKeymap, byte *row, byte *col, byte numRows, byte numCols, char *i2ctype) {
	rowPins = row;
	columnPins = col;
	sizeKpd.rows = numRows;
	sizeKpd.columns = numCols;

	begin(userKeymap);

	setDebounceTime(10);
	setHoldTime(500);
	keypadEventListener = 0;

	startTime = 0;
	single_key = false;

	if (i2ctype == "Adafruit_MCP23017") {
		I2Ctype = MCP23017;
		mcp17.begin();
	}
	else if (i2ctype == "Adafruit_MCP23008") {
		I2Ctype = MCP23008;
		mcp8.begin();
	}
}

// I've included a constructor that does not include an i2ctype. This constructor is meant
// to ensure backwards compatibility. New users of the library will probably want to use the
// other constructor.
Keypad::Keypad(char *userKeymap, byte *row, byte *col, byte numRows, byte numCols) {
	Keypad(userKeymap, row, col, numRows, numCols, "Adafruit_MCP23008");
}

// Let the user define a keymap - assume the same row/column count as defined in constructor
void Keypad::begin(char *userKeymap) {
    keymap = userKeymap;
}

// Returns a single key only. Retained for backwards compatibility.
char Keypad::getKey() {
	single_key = true;

	if (getKeys() && key[0].stateChanged && (key[0].kstate==PRESSED))
		return key[0].kchar;

	single_key = false;

	return NO_KEY;
}

// Populate the key list.
bool Keypad::getKeys() {
	bool keyActivity = false;

	// Limit how often the keypad is scanned. This makes the loop() run 10 times as fast.
	if ( (millis()-startTime)>debounceTime ) {
		scanKeys();
		keyActivity = updateList();
		startTime = millis();
	}

	return keyActivity;
}

// Private : Hardware scan
void Keypad::scanKeys() {
	// Re-intialize the row pins. Allows sharing these pins with other hardware.
	for (byte r=0; r<sizeKpd.rows; r++) {
		pin_mode(rowPins[r],INPUT_PULLUP);
	}

	// bitMap stores ALL the keys that are being pressed.
	for (byte c=0; c<sizeKpd.columns; c++) {
		pin_mode(columnPins[c],OUTPUT);
		pin_write(columnPins[c], LOW);	// Begin column pulse output.
		for (byte r=0; r<sizeKpd.rows; r++) {
			bitWrite(bitMap[r], c, !pin_read(rowPins[r]));  // keypress is active low so invert to high.
		}
		// Set pin to high impedance input. Effectively ends column pulse.
		pin_write(columnPins[c],HIGH);
		pin_mode(columnPins[c],INPUT);
	}
}

// Manage the list without rearranging the keys. Returns true if any keys on the list changed state.
bool Keypad::updateList() {

	bool anyActivity = false;

	// Delete any IDLE keys
	for (byte i=0; i<LIST_MAX; i++) {
		if (key[i].kstate==IDLE) {
			key[i].kchar = NO_KEY;
			key[i].kcode = -1;
			key[i].stateChanged = false;
		}
	}

	// Add new keys to empty slots in the key list.
	for (byte r=0; r<sizeKpd.rows; r++) {
		for (byte c=0; c<sizeKpd.columns; c++) {
			bool button = bitRead(bitMap[r],c);
			char keyChar = keymap[r * sizeKpd.columns + c];
			int keyCode = r * sizeKpd.columns + c;
			int idx = findInList (keyCode);
			// MyKey is already on the list so set its next state.
			if (idx > -1)	{
				nextKeyState(idx, button);
			}
			// MyKey is NOT on the list so add it.
			if ((idx == -1) && button) {
				for (byte i=0; i<LIST_MAX; i++) {
					if (key[i].kchar==NO_KEY) {		// Find an empty slot or don't add key to list.
						key[i].kchar = keyChar;
						key[i].kcode = keyCode;
						key[i].kstate = IDLE;		// Keys NOT on the list have an initial state of IDLE.
						nextKeyState (i, button);
						break;	// Don't fill all the empty slots with the same key.
					}
				}
			}
		}
	}

	// Report if the user changed the state of any key.
	for (byte i=0; i<LIST_MAX; i++) {
		if (key[i].stateChanged) anyActivity = true;
	}

	return anyActivity;
}

// Private
// This function is a state machine but is also used for debouncing the keys.
void Keypad::nextKeyState(byte idx, bool button) {
	key[idx].stateChanged = false;

	switch (key[idx].kstate) {
		case IDLE:
			if (button==CLOSED) {
				transitionTo (idx, PRESSED);
				holdTimer = millis(); }		// Get ready for next HOLD state.
			break;
		case PRESSED:
			if ((millis()-holdTimer)>holdTime)	// Waiting for a key HOLD...
				transitionTo (idx, HOLD);
			else if (button==OPEN)				// or for a key to be RELEASED.
				transitionTo (idx, RELEASED);
			break;
		case HOLD:
			if (button==OPEN)
				transitionTo (idx, RELEASED);
			break;
		case RELEASED:
			transitionTo (idx, IDLE);
			break;
	}
}

// New in 2.1
bool Keypad::isPressed(char keyChar) {
	for (byte i=0; i<LIST_MAX; i++) {
		if ( key[i].kchar == keyChar ) {
			if ( (key[i].kstate == PRESSED) && key[i].stateChanged )
				return true;
		}
	}
	return false;	// Not pressed.
}

// Search by character for a key in the list of active keys.
// Returns -1 if not found or the index into the list of active keys.
int Keypad::findInList (char keyChar) {
	for (byte i=0; i<LIST_MAX; i++) {
		if (key[i].kchar == keyChar) {
			return i;
		}
	}
	return -1;
}

// Search by code for a key in the list of active keys.
// Returns -1 if not found or the index into the list of active keys.
int Keypad::findInList (int keyCode) {
	for (byte i=0; i<LIST_MAX; i++) {
		if (key[i].kcode == keyCode) {
			return i;
		}
	}
	return -1;
}

// New in 2.0
char Keypad::waitForKey() {
	char waitKey = NO_KEY;
	while( (waitKey = getKey()) == NO_KEY );	// Block everything while waiting for a keypress.
	return waitKey;
}

// Backwards compatibility function.
KeyState Keypad::getState() {
	return key[0].kstate;
}

// The end user can test for any changes in state before deciding
// if any variables, etc. needs to be updated in their code.
bool Keypad::keyStateChanged() {
	return key[0].stateChanged;
}

// The number of keys on the key list, key[LIST_MAX], equals the number
// of bytes in the key list divided by the number of bytes in a MyKey object.
byte Keypad::numKeys() {
	return sizeof(key)/sizeof(MyKey);
}

// Minimum debounceTime is 1 mS. Any lower *will* slow down the loop().
void Keypad::setDebounceTime(uint debounce) {
	debounce<1 ? debounceTime=1 : debounceTime=debounce;
}

void Keypad::setHoldTime(uint hold) {
    holdTime = hold;
}

void Keypad::addEventListener(void (*listener)(char)){
	keypadEventListener = listener;
}

void Keypad::transitionTo(byte idx, KeyState nextState) {
	key[idx].kstate = nextState;
	key[idx].stateChanged = true;

	// Sketch used the getKey() function.
	// Calls keypadEventListener only when the first key in slot 0 changes state.
	if (single_key)  {
	  	if ( (keypadEventListener!=NULL) && (idx==0) )  {
			keypadEventListener(key[0].kchar);
		}
	}
	// Sketch used the getKeys() function.
	// Calls keypadEventListener on any key that changes state.
	else {
	  	if (keypadEventListener!=NULL)  {
			keypadEventListener(key[idx].kchar);
		}
	}
}

/*
|| @changelog
|| | 0.1.8 2016-6-01 - Evan Simkowitz	: Update to example, turns out it didn't like const char*.
|| | 0.1.7 2016-6-01 - Evan Simkowitz	: Release candidate: Some variable naming has been changed and some reformatting was performed to ensure future
|| |									  ease of adding features. MCP23017 is now fully supported and can be selected by adding an i2ctype parameter to
|| |									  the constructor. For backwards-compatibility, including no i2ctype in the constructor defaults to MCP23008.
|| | 0.1.7 2016-6-01 - Evan Simkowitz	: Trying just importing all the libraries and then deciding which one to use in the code with separate helpers.
|| | 0.1.7 2016-5-31 - Evan Simkowitz	: I am preparing to add MCP23017 compatibility and am setting the groundwork with some reorganization.
|| | 0.1.2 2016-5-20 - Evan Simkowitz	: 0.1.2 published to Particle's library repository.
|| | 0.1.2 2016-5-20 - Evan Simkowitz	: No changes here, but had to update version because of an issue importing to Particle.
|| | 0.1.1 2016-5-20 = Evan Simkowitz	: worked out some issues with reliability
|| | 0.1.0 2016-5-19 - Evan Simkowitz	: Accounted for name change of Keypad-I2C.h from Keypad.h
|| | 0.1.0 2016-5-19 - Evan Simkowitz	: Changed name from Keypad.cpp to Keypad-I2C.cpp
|| | 0.1.0 2016-5-19 - Evan Simkowitz	: Added the Adafruit_MCP23008.h, forked from Keypad-spark
|| #
*/
