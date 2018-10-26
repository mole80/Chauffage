
#define DEB false
#include "Sensor.h"
#include "com.h"
#include "persistanteValue.h"
#include <eeprom.h>
#include <avr/wdt.h>

const bool printReply = true;
const char line[] = "-----\n\r";
int loopCount = 0;

#define BOARD_FLOOR_1		0U	// Etage
//#define BOARD_FLOOR_0		10U	// Rez de chaussé
//#define BOARD_FLOOR_M1	20U  // Sous sol

#ifdef  BOARD_FLOOR_1
	#define FLOOR	BOARD_FLOOR_1
#elif  BOARD_FLOOR_0
	#define FLOOR	BOARD_FLOOR_0
#elif  BOARD_FLOOR_M1
	#define FLOOR	BOARD_FLOOR_M1
#endif

RadioSensor radioSensors[] = {
	RadioSensor(0 + FLOOR),
	RadioSensor(1 + FLOOR),
	RadioSensor(2 + FLOOR),
	RadioSensor(3 + FLOOR),
	RadioSensor(4 + FLOOR),
	RadioSensor(5 + FLOOR),
	RadioSensor(6 + FLOOR),
	RadioSensor(7 + FLOOR),
	RadioSensor(8 + FLOOR),
	RadioSensor(9 + FLOOR),
};
uint16_t nbrSensor = 10;

uint16_t tick_meas;

#ifdef  BOARD_FLOOR_1
Room rooms[] = {
	Room(0, radioSensors[1], 18, 0),
	Room(1, radioSensors[2], 19, 1),
	Room(2, radioSensors[3], 21, 2),
	Room(3, radioSensors[4], 21, 3),
	Room(4, radioSensors[5], 21, 4),
	Room(5, radioSensors[6], 21, 5)
};
uint16_t nbrRoom = 5;
#endif 

#ifdef  BOARD_FLOOR_0
Room rooms[] = {
	Room(0, radioSensors[1], 18, 0),
	Room(1, radioSensors[2], 19, 1),
	Room(2, radioSensors[3], 21, 2),
	Room(3, radioSensors[4], 21, 3),
	Room(4, radioSensors[5], 21, 4),
	Room(5, radioSensors[6], 21, 5)
};
uint16_t nbrRoom = 5;
#endif 

#ifdef  BOARD_FLOOR_M1
Room rooms[] = {
	Room(0, radioSensors[1], 18, 0),
	Room(1, radioSensors[2], 19, 1),
	Room(2, radioSensors[3], 21, 2),
	Room(3, radioSensors[4], 21, 3),
	Room(4, radioSensors[5], 21, 4),
	Room(5, radioSensors[6], 21, 5)
};
uint16_t nbrRoom = 5;
#endif 

uint8_t GetRadioSensorIndex(uint16_t id) {
	for (uint8_t k = 0; k < nbrSensor; k++) {
		if (radioSensors[k].GetId() == id) {
			return k;
		}
	}

	while (1) {}
}

bool TestEndCommand(String cmd) {
	if (cmd[0] == 'c' && cmd[1] == 'm' && cmd[2] == 'd' && cmd[3] == '_') {
		return true;
	}
	else {
		return false;
	}
}

void ConfigureESP() {

	digitalWrite(22, LOW);
	delay(100);
	digitalWrite(22, HIGH);
	delay(1000);

	printInfo("Add new capteur");

	
	String rep = "";
	for (uint16_t k = 0; k < nbrSensor; k++) {
		do {
			rep = sendCmd("ncapt," + String(radioSensors[k].GetId()) + ",sens", false);
			delay(100);
		} while ( !TestEndCommand(rep) );
	}

	for (uint16_t k = 0; k < nbrRoom; k++) {
		rooms[k].tempTarget.Initialize();
		rooms[k].SetDefaultPwm(1200, 600);
		do {
			rep = sendCmd(  "nroom," + 
							String(rooms[k].GetId()) + "," + 
							String(rooms[k].GetTargetAsString())
							, false);
			delay(100);
		} while ( !TestEndCommand(rep) );
	}

	Serial.println("Connect WIFI");
	delay(100);
	sendCmd("ctnm", false);
	delay(100);
	Serial.println("Start config");
	delay(100);

	printInfo("Get State");
	sendCmd("gs", false);
	delay(100);	

	tick_meas = 0;
}

void setup()
{
//#define PROG_ESP8266
#ifndef PROG_ESP8266
	Serial2.begin(115200);
	Serial2.setTimeout(10);
	//wdt_enable(4000);
	//Mettre pin comm TX sur esp en input pour éviter le conflit pour 
	//re-programmer le module ESP8266
	pinMode(22, OUTPUT); // RESET
	digitalWrite(22, LOW);
#else
	pinMode(16, INPUT);
#endif

	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);
	pinMode(4, OUTPUT);
	pinMode(5, OUTPUT);
	pinMode(6, OUTPUT);

	Serial1.begin(1200);
	Serial.begin(115200);

	delay(500);

	printInfo("Start application : " );

	ConfigureESP();
}

ComBuff comBuff;

float temp, hum;
int cpt_meas;
int lastSensorId = -1;

uint8_t cpt_update_meas_temp;

#define TIMEOUT_WIFI 60	// Second
int16_t cpt_wifi = TIMEOUT_WIFI;
void CheckConnexionWifi() {

	String status = sendCmd("gs", false);
	if ( status[0] == '1' ) {
		if (cpt_wifi < TIMEOUT_WIFI ) {
			cpt_wifi++;
		}
	}
	else {
		if (cpt_wifi <= 0) {
			ConfigureESP();
			cpt_wifi = TIMEOUT_WIFI;
		}
		else {
			cpt_wifi--;
			printInfo("Wifi error, timeout : " + 
				String(cpt_wifi) + 
				" - Status : " + 
				String(status));
		}
	}
}

void loop(){
	wdt_reset();
	int adcTempStart = analogRead(0);

	if (cpt_update_meas_temp <= 0) {
		cpt_update_meas_temp = 10;
		
		CheckConnexionWifi();
		
		for (uint16_t k = 0; k < nbrRoom; k++) {
			if (rooms[k].GetId() >= 0) {
				rooms[k].UpdateTempValue(adcTempStart);
				rooms[k].ExecuteRegulation();
			}

				Room& r = rooms[k];				
				//printInfo("Rooms : " + String(r3.GetId()) + "  :  " + String(rooms[k]->GetId()) + "  ->  " + String(r->IsValveOpenAsString()) );
				//printInfo("srm," + String(r.GetId()) + "," + r.GetTempStartAsString() + "," + r.GetTempEndAsString() + "," + String(r.IsValveOpenAsString()));
				
				sendCmd("srm," + 
					String(r.GetId()) + "," + 
					r.GetTempStartAsString() + "," + 
					r.GetTempEndAsString() + "," + 
					String(r.IsValveOpenAsString()) + "," + 
					String(r.GetSensor().GetTimeout()) + "," +
					String(r.GetTargetAsString()) + "," + 
					String(r.GetMeasAsString()) + "," +
					String(r.GetCpt())
					, false
				);

				//printInfo("srm," + String(rooms[k]->GetId()) + "," + String(rooms[k]->GetTempStart()) + "," + String(rooms[k]->GetTempEnd()) + "," + String(rooms[k]->IsValveOpen()));
			//}
		}

		for (uint16_t k = 0; k < nbrSensor; k++) {
			radioSensors[k].UpdateTimeout();
		}
	}
	else {
		cpt_update_meas_temp--;
	}


	while (Serial1.available() > 0) {		
		comBuff.AddChar( Serial1.read() );
	}

	if(comBuff.newCommand){
			int id = comBuff.buff[1];
			if (id > 0 && id < 30) {
				uint8_t index = GetRadioSensorIndex(id);

				//if ( check == buff[6] && id != lastSensorId )
				if (comBuff.GetCheck() == comBuff.buff[6] && index < 10 && index >= 0)
				{
					RadioSensor& r = radioSensors[index];

					lastSensorId = id;
					temp = ((comBuff.buff[2] << 8) + comBuff.buff[3]) / 100.0;
					hum = ((comBuff.buff[4] << 8) + comBuff.buff[5]) / 100.0;

					tick_meas++;
					r.SetMeasure(temp, hum);
					sendCmd("scapt," + String(id) + "," + String(temp) + "," + String(hum) + "," + String(tick_meas), false);

					Serial.println("New temp " + String(r.GetCptMeas()) + " id " + String(id) + " : " + String(temp) + " -- " + String(hum));
					Serial.flush();
				}

				comBuff.ClearCommand();
			}
	}

	delay(100);
}
