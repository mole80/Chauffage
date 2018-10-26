
// PROGRAMATION
// Pour pouvoir programmer la carte node, il faut definir la constante suivante dans
// ArduinoCentral pour désactiver le port série
// #define PROG_ESP8266

#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFi.h>

const char *ssid = "Maulaz_Visitor";
//const char *ssid = "AndroidAP";
//const char *pass = "chlu9480";
const char *pass = "Qwer-123";

WiFiServer server(80);

int state;
byte buff[256];
int nbrVal = 0;

String cmd = "";
String cmdTmp = "";

class Room
{
public:
	Room() {
		id = -1;
		target = "";
		meas = "";
		cpt = "";
		timeout = "";
	}
	int id;
	String timeout;
	String cpt;
	//String name;
	String valveOpen;
	String target;
	String meas;
	String tempStart;
	String tempStop;
};

class Sensor
{
public:
	Sensor() {
		id = -1;
	}
	int id;
	//String name;
	String temp;
	String hum;
	String tick;
};

#define NBR_SENSORS_MAX	30
Sensor sensors[NBR_SENSORS_MAX];

#define NBR_ROOM_MAX	10
Room rooms[NBR_ROOM_MAX];

#define DEBUG_INFO false
void printDebug(String text)
{
	if (DEBUG_INFO){
		Serial.println(text);
	}
}

void connect(String ssid, String pass, IPAddress add) {
	printDebug("");
	printDebug("Connecting to ");
	printDebug(ssid);

	IPAddress g(192, 168, 0, 0);
	IPAddress sub(255, 255, 255, 0);
	WiFi.config(add, g, sub);

	WiFi.begin(ssid.c_str(), pass.c_str());
	
	/*while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		printDebug(".");
	}*/

	printDebug("");
	printDebug("WiFi connected");
	server.begin();
	printDebug("Server started");
	printDebug("Use this URL to connect: ");
	printDebug("http://");
	//printDebug(WiFi.localIP());
	printDebug("/");
}

String GetNextParam()
{
	int ind = 0;
	String param = "";
	while (cmdTmp[ind] != '\r' && cmdTmp[ind] != ',') {
		param += (String)cmdTmp[ind];
		//Serial.println("Del : " + (String)cmdTmp[ind]);
		//delay(100);
		ind++;
		//delay(100);
	}
	
	//Serial.println("Del end ind : " + String(ind) + "\r\n");
	cmdTmp.remove(0, ind+1);
	//Serial.println("Rest : " + cmdTmp + "\r\n");
	printDebug("Param : " + param + "\r\n");

	return param;
}

String RemoveCmd()
{
	int ind = 0;
	while (cmdTmp[ind] != ' ' || cmdTmp[ind] != '\r') {
		ind++;
	}
	
	printDebug("Remove : " + cmdTmp + "\r\n");
	
	cmdTmp.remove(0, ind);
	return cmdTmp;
}

void setup()
{
	state = 0;

	Serial.begin(115200);
	Serial.setTimeout(10);
	delay(500);

	pinMode(5, OUTPUT);

	printDebug("Start device");
	WiFi.setAutoConnect(false);
	WiFi.setAutoReconnect(false);
	//Serial.println();
	//Serial.println();
	//Serial.print("Connecting to ");
	//Serial.println(ssid);
	//WiFi.begin(ssid, pass);
	//while (WiFi.status() != WL_CONNECTED) {
	//	delay(500);
	//	Serial.print(".");
	//}
	//Serial.println("");
	//Serial.println("WiFi connected");
	//server.begin();
	//Serial.println("Server started");
	//Serial.print("Use this URL to connect: ");
	//Serial.print("http://");
	//Serial.print(WiFi.localIP());
	//Serial.println("/");

	printDebug("Start watchdog");
	//ESP.wdtDisable();
	//wdt_enable(1000U);
}


String request;

String GetNextValue(String *str) {
	int ind = 0;
	String val = "";
	while ((*str)[ind] != ',' && ind < 6) {
		val += (*str)[ind];
		ind++;
	}

	(*str).remove(0, ind + 1);

	return val;
}

void sendRep(String text) {
	Serial.println(text + "\r\n");
	//ESP.wdtFeed();
}

void endRep() {
	sendRep("cmd_end\r\n");
}

String TestCmd()
{
	String cmd = "";
	if (buff[0] == 'c' && buff[1] == 'm' && buff[2] == 'd' && buff[3] == '_')
	{
		for (int k = 4; k < nbrVal; k++) {
			cmd += String((char)(buff[k]));
		}
	}
	nbrVal = 0;
	return cmd;
}

bool AddChar(char val) {
	printDebug("Add : " + String(val) + " Nb val : " + String(nbrVal) );
	buff[nbrVal] = val;
	nbrVal++;

	bool res = false;

	if (nbrVal > 50) {
		nbrVal = 0;
	}

	if (val == '\r' || val == '\n') {
		if (buff[0] == 'c' && buff[1] == 'm' && buff[2] == 'd' && buff[3] == '_') {
			res = true;
		}
		else {
			nbrVal = 0;
		}
	}
	
	return res;
}

void loop()
{
	int cmdId = -1;
	bool newCmd = false;

	while (Serial.available() > 0){
		if( AddChar(Serial.read()) ){ 
			//TestCmd();
			newCmd = true;
		}
	}

	//if (Serial.available() > 0 && Serial.find("cmd_") && Serial.find('\r') )
	if( newCmd )
	{
		//ESP.wdtFeed();

		//cmd = Serial.readString();
		cmdTmp = TestCmd();
		printDebug("rec : " + cmdTmp);

		String p = GetNextParam();
		if (p.equalsIgnoreCase("rst")) {
			cmdId = -1;
			state = 3;
		}
		else if (p.equalsIgnoreCase("gs")) {
			sendRep(String(state));
		}
		else if ( p.equalsIgnoreCase("ctn") ) {
			cmdId = 1;
			endRep();
		}
		else if ( p.equalsIgnoreCase("ctnm") ) {
			cmdId = 2;
			endRep();
		}
		else if ( p.equalsIgnoreCase("ctna") ) {
			cmdId = 3;
			endRep();
		}
		else if ( p.equalsIgnoreCase("cmdtest") ) {
			sensors[1].id = 1;
			//sensors[1].name = "capt 1";
			sensors[1].temp = "23.5";
			sensors[2].id = 1;
			//sensors[2].name = "capt 1";
			sensors[2].temp = "23.5";
			rooms[1].id = 1;
			//rooms[1].name = "R1";
			rooms[1].target = "18.1";
			rooms[1].valveOpen = "C";
			rooms[2].id = 2;
			//rooms[2].name = "R2";
			rooms[2].target = "28.1";
			rooms[2].valveOpen = "C";
			endRep();
		}
		else if (p.equalsIgnoreCase("ncapt")) {
			uint8_t id = GetNextParam().toInt();
			uint8_t index = id % 10;
			if (index < NBR_SENSORS_MAX) {
				sensors[index].id = id;
				//sensors[id].name = "capt";// GetNextParam();
				sensors[index].temp = "0";
				sensors[index].hum = "0";
				sensors[index].tick = "0";
			}
			endRep();
		}
		else if (p.equalsIgnoreCase("rcapt")) {
			int id = GetNextParam().toInt();
			uint8_t index = id % 10;
			if (index < NBR_SENSORS_MAX) {
				sendRep("Capt," + String(id) + "," + sensors[index].temp + "," + sensors[index].hum + "," + sensors[index].tick);
			}
			else{
				endRep();
			}
		}
		else if (p.equalsIgnoreCase("scapt")) {
			int id = GetNextParam().toInt();
			uint8_t index = id % 10;
			if (index < NBR_SENSORS_MAX) {
				sensors[index].temp = GetNextParam();
				sensors[index].hum = GetNextParam();
				sensors[index].tick = GetNextParam();
			}
			endRep();
		}
		else if (p.equalsIgnoreCase("nroom")) {
			int id = GetNextParam().toInt();
			if (id < NBR_ROOM_MAX) {
				rooms[id].id = id;
				//rooms[id].name = "Room";// GetNextParam();
				rooms[id].target = GetNextParam();
				rooms[id].valveOpen = "C";

			}
			endRep();
		}
		else if (p.equalsIgnoreCase("rroom")) {
			int id = GetNextParam().toInt();
			if (id < NBR_ROOM_MAX) {
				sendRep("Room," + String(id) + "," +
					rooms[id].target + ',' +
					rooms[id].meas + ',' +
					rooms[id].tempStart + ',' +
					rooms[id].tempStop + ',' +
					rooms[id].valveOpen);
			}
			else {
				endRep();
			}
		}
		else if (p.equalsIgnoreCase("srm")) {
			int id = GetNextParam().toInt();
			if (id < NBR_ROOM_MAX) {
				rooms[id].tempStart = GetNextParam();
				rooms[id].tempStop= GetNextParam();
				rooms[id].valveOpen = GetNextParam();
				rooms[id].timeout = GetNextParam();
				rooms[id].target = GetNextParam();
				rooms[id].meas = GetNextParam();
				rooms[id].cpt = GetNextParam();
			}
			endRep();
		}
		else if (p.equalsIgnoreCase("srt")) {
			int id = GetNextParam().toInt();
			if (id < NBR_ROOM_MAX) {
				rooms[id].target = GetNextParam();
			}
			endRep();
		}
	}

	switch (state)
	{
	case 0:
		if (cmdId == 1) {
			String ssid = GetNextParam();
			String pass = GetNextParam();
			String add1 = GetNextParam();
			String add2 = GetNextParam();
			String add3 = GetNextParam();
			String add4 = GetNextParam();
			IPAddress addip(add1.toInt(), add2.toInt(), add3.toInt(), add4.toInt());
			connect( ssid, pass, addip );
			cmdId = 0;
		}
		else if (cmdId == 2) {
			String ssid = "Maulaz_Visitor";
			String pass = "Qwer-123";
			IPAddress add(10, 128, 0, 200);
			connect( ssid, pass, add );
			cmdId = 0;
		}
		else if (cmdId == 3) {
			String ssid = "AndroidAP";
			String pass = "chlu9480";
			IPAddress add(192, 168, 43, 15);
			connect( ssid, pass, add );
			cmdId = 0;
		}

		if (WiFi.isConnected()) {
			printDebug("Wifi connected");
			state = 1;
		}

		//state = 1;
		break;

	case 3:
		if (WiFi.isConnected()) {
			WiFi.disconnect();
		}

		state = 0;
		break;

	case 1:
	{
		if (!WiFi.isConnected()) {
			state = 0;
			return;
		}

		WiFiClient client;
		client = server.available();

		if (!client) {
			return;
		}

		printDebug("new client");
		while (!client.available()) {
			delay(1);
		}

		request = client.readStringUntil('\r');
		printDebug(request);
		client.flush();

		int ind = request.indexOf("targ=");
		if (ind != -1) {
			request.remove(0, ind + 5);
			int id = GetNextValue(&request).toInt();
			String val = GetNextValue(&request);

			rooms[id].target = val;
			printDebug("Room set : " + String(id) + " : " + val);
		}

		client.println("HTTP/1.1 200 OK");
		client.println("Content-Type: text/html");
		client.println("");
		client.println("");
		client.println("<h2>Maulaz's Home</h2>");
		client.println("<h4>");
		client.println("");

		for (int i = 0; i < NBR_SENSORS_MAX; i++) {
			if (sensors[i].id >= 0) {
				client.println("Capt," + String(sensors[i].id) + "," + sensors[i].temp + "," + sensors[i].hum + "," + sensors[i].tick);
				client.println("</br>");
				client.println("");
			}
		}
		
		for (int i = 0; i < NBR_ROOM_MAX; i++) {
			if (rooms[i].id >= 0) {
				client.println(
					"Valve," + 
					String(rooms[i].id) + "," + 
					rooms[i].target  + ',' + 
					rooms[i].meas  + ',' + 
					rooms[i].tempStart + ',' + 
					rooms[i].tempStop + ',' + 
					rooms[i].valveOpen + ',' + 
					rooms[i].timeout + ',' + 
					rooms[i].cpt 
				);
				
				client.println("</br>");
				client.println("");
			}
		}

		client.println(" ");
		client.println("<h4/>");
		delay(1);
		printDebug("Client disonnected");
		printDebug("");
	}
	break;

	default:
		break;
	}
}
