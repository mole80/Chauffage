#include "com.h"

void printInfo(String text) {
	if (PRINT_INFO) {
		Serial.println(text);
		Serial.flush();
	}
}

/*void getReply(int wait)
{
	int tempPos = 0;
	long int time = millis();
	while ((time + wait) > millis())
	{
		while (Serial2.available())
		{
			char c = Serial2.read();
			if (tempPos < 500) { reply[tempPos] = c; tempPos++; }
		}
		reply[tempPos] = 0;
	}

	if (printReply) { Serial.println(reply);  Serial.println(line); }
}*/


String sendData(String command, const uint16_t timeout, boolean debug)
{
	String response = "";
	Serial2.print(command);
	uint32_t time = millis();
	while ((time + timeout) > millis())
	{
		while (Serial2.available())
		{
			char c = Serial2.read(); // read the next character.
			response += c;
		}
	}

	if (debug)
	{
		Serial.print(response); //displays the esp response messages in arduino Serial monitor
	}
	return response;
}

String sendCmd(String cmd, bool block = false) {
	bool res = false;

		int retry = 100;

		cmd += '\r';

		while ( Serial2.available() > 0 )
			Serial2.read();

		Serial.println("Send : " + cmd + "\r");
		Serial.flush();

		//delay(100);

		String tmp = "";
		bool end = false;
			Serial2.println("cmd_" + cmd);
		Serial2.flush();
		do {
			if (Serial2.available() > 0)
			{
				tmp += Serial2.readString();
				if (tmp[tmp.length() - 1] == '\n' || tmp[tmp.length() - 1] == '\r') {
					end = true;
				}
			}
			delay(10);
			if (!block) {
				retry--;
			}
		} while (!end && retry > 0);

		//Serial.println("Retry : " + retry);

		//String rep = "Rep ---";
		//if(Serial2.available() > 0)
		//	rep = Serial2.readString();
		//delay(100);
		if (retry <= 0) {
			Serial.print("Rep : Timeout\r\n");
		}
		else {
			Serial.print("Rep : " + tmp);
			res = true;
		}
		
		Serial.flush();
		//delay(100);

		if (res) {
			return tmp;
		}

		return "\r\n";
}


/*bool ComBuff::AddChar(char val) {
	newCommand = false;
	static int nbrMax = 0;

	if ( nbrVal == 0 && (byte)val != 0xA5 ) {
			nbrVal = 0;
	}	
	else if( nbrVal < 30 ) {
		buff[nbrVal] = val;
		nbrVal++;
	}

	
	if (nbrVal >= 7 || nbrVal < 0) {
		if (buff[0] == 0xA5) {
			newCommand = true;
		}
		else {
			nbrVal = 0;
		}
	}

	return newCommand;
}*/

byte ComBuff::GetCheck() {
	return buff[0] + buff[1] + buff[2] + buff[3] + buff[4] + buff[5];
}

/*void ComBuff::ClearCommand() {
	nbrVal = 0;
	newCommand = false;
}*/
