#pragma once
#include "Arduino.h"

#define PRINT_INFO	true

void printInfo(String text);

	class ComBuff{
public:
	ComBuff() {
		nbrVal = 0;
		newCommand = false;
	}

	bool AddChar(char val);

	byte GetCheck();

	void ClearCommand();

byte buff[30];
uint16_t nbrVal = 0;
bool newCommand;
};

String sendData(String command, const int timeout, boolean debug);

String sendCmd(String cmd, bool block = false);
