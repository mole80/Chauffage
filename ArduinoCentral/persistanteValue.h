#pragma once
#include "Arduino.h"
#include <eeprom.h>
#include "com.h"

// val 0 -> add 0-1
// val 1 -> add 2-3
// val 2 -> add 4-5
// val 3 -> add 6-7

class PersistValue {
public:	
	PersistValue(byte defaultValue, uint16_t id) {
		def = defaultValue;
		this->add = id*2;
	}

	void Initialize(){
		printInfo( "Init persist : " + String(add) );
		uint8_t tmp = EEPROM.read(add);
		if (tmp == 1) {
			value = EEPROM.read(add + 1);
			printInfo( "Read value : " + String(value) );
		}
		else {
			printInfo( "Default value : " + String(def) );
			value = def;
		}
	}

	void SetValue(byte val) {
		if (val != value) {
			EEPROM.write(add, 1);
			EEPROM.write(add+1, val);
			printInfo( "Set value : " + String(val) );
			value = val;
		}
	}

	byte GetValue() {
		return value;
	}

	uint16_t add;
	byte value;
	byte define;
	byte def;
};

class PersistTempTarget : public PersistValue {
public:
	PersistTempTarget(float defaultTemp, uint16_t id) :
		PersistValue((byte)(defaultTemp * 10), id)
	{}

	void SetTemp(float temp) {
		SetValue( (byte)(temp * 10) );
	}

	float ReadTemp() {
		return (float)GetValue() / 10.0;
	}
};