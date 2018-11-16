#pragma once
#include "Arduino.h"

#define PRINT_INFO	true

//#define PRINT_COMMAND

#ifdef PRINT_COMMAND
	#define USE_ESP
#endif

void printInfo(String text);

#define BUFFER_SIZE 256
class ComBuff{
public:
	ComBuff() {
		nbrVal = 0;
		newCommand = false;
		index_read = 0;
		index_write = 0;
		size = BUFFER_SIZE;
		startNewCmd = false;
		nbrValInCmd = 0;
	}

	void AddValue(uint16_t val) {
		buff[index_write] = val;

		bool add = false;
		if (val == 0xA5 && !startNewCmd) {
			startNewCmd = true;
			nbrValInCmd = 1;
			//Serial.println("################## Start new command");
			add = true;
		}
		else if (startNewCmd && nbrValInCmd < 6) {
			add = true;
			//Serial.println("############# Add in new command");
			nbrValInCmd++;
		}
		else if ( startNewCmd && nbrValInCmd >= 6){
			startNewCmd = false;
			//Serial.println("################## End new command");
			add = true;
			nbrValInCmd++;
		}

		if (add) {
			if (++index_write >= BUFFER_SIZE) {
				index_write = 0;
			}
			nbrVal++;


			/*Serial.println("Add one, iw : " + String(index_write) + " -- " +
				"ir : " + String(index_read) + " -- " +
				"nbVal : " + String(nbrVal) + " -- " +
				"val : " + String(val)
			);*/
		}

		//CheckFirstValue();
	}

	uint8_t GetValue() {
		uint16_t val = buff[index_read];

		/*Serial.println("GetValue, iw : " + String(index_write) + " -- " +
			"ir : " + String(index_read) + " -- " +
			"nbVal : " + String(nbrVal) + " -- " +
			"val : " + String(val)
		);*/

		if (++index_read >= BUFFER_SIZE) {
			index_read = 0;
		}
		nbrVal--;
		return val;
	}

	uint16_t Checkval() {
		return buff[index_read];
	}

	bool RecievedCommand() {
		return nbrVal >= 7;
	}

	void CheckFirstValue() {
		while (nbrVal > 0 && buff[index_read] != 0xA5) {
			uint8_t val = GetValue();
			Serial.println("Remove : " + String(val) + " ind : " + String(index_read));
		}
	}

	byte GetCheck();

	void ClearCommand() {
		CheckFirstValue();
	}

bool newCommand;

bool startNewCmd;
uint16_t nbrValInCmd;
uint16_t nbrVal;
uint16_t index_read;
uint16_t index_write;
uint16_t size;
uint8_t buff[BUFFER_SIZE];
};

String sendData(String command, const int timeout, boolean debug);

String sendCmd(String cmd, bool block = false);
