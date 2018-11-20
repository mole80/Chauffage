
//#include <RadioHead.h>

//#include <eeprom.h>

/*
	Consomation :
	Sleep power down with led : 5.06 mA
	Actif à vide avec led : 15.7 mA
	Actif avec power sur DHT et led 17.2 mA
	Sleep power down without led 2.25 mA
	Regulateur entrée sur VIN pour faire du 5V  : 1.5 mA

	Sans LED, sans régualteur : 240 uA
*/

#include "lib\DHT_sensor_library\DHT.h"

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

volatile int f_timer = 0;


//#define TEST_RADIO


#define SENSOR_ID 15



#define DHTPIN 7     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define DHT_POW	4
#define RADIO_POW	11
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

// #include <RH_ASK.h>
// RH_ASK driver(1000, 5, 1, 6);

ISR(WDT_vect) {
	if (f_timer == 0) {
		f_timer = 1;
	}
}

void setupWdTimer() {
	//https://gist.github.com/stojg/aec2c8c54c29c0fab407
	// The MCU Status Register (MCUSR) is used to tell the cause of the last
	// reset, such as brown-out reset, watchdog reset, etc.
	// NOTE: for security reasons, there is a timed sequence for clearing the
	// WDE and changing the time-out configuration. If you don't use this
	// sequence properly, you'll get unexpected results.

	// Clear the reset flag on the MCUSR, the WDRF bit (bit 3).
	MCUSR &= ~(1 << WDRF);

	// Configure the Watchdog timer Control Register (WDTCSR)
	// The WDTCSR is used for configuring the time-out, mode of operation, etc

	// In order to change WDE or the pre-scaler, we need to set WDCE (This will
	// allow updates for 4 clock cycles).

	// Set the WDCE bit (bit 4) and the WDE bit (bit 3) of the WDTCSR. The WDCE
	// bit must be set in order to change WDE or the watchdog pre-scalers.
	// Setting the WDCE bit will allow updates to the pre-scalers and WDE for 4
	// clock cycles then it will be reset by hardware.
	WDTCSR |= (1 << WDCE) | (1 << WDE);

	/**
	*	Setting the watchdog pre-scaler value with VCC = 5.0V and 16mHZ
	*	WDP3 WDP2 WDP1 WDP0 | Number of WDT | Typical Time-out at Oscillator Cycles
	*	0    0    0    0    |   2K cycles   | 16 ms
	*	0    0    0    1    |   4K cycles   | 32 ms
	*	0    0    1    0    |   8K cycles   | 64 ms
	*	0    0    1    1    |  16K cycles   | 0.125 s
	*	0    1    0    0    |  32K cycles   | 0.25 s
	*	0    1    0    1    |  64K cycles   | 0.5 s
	*	0    1    1    0    |  128K cycles  | 1.0 s
	*	0    1    1    1    |  256K cycles  | 2.0 s
	*	1    0    0    0    |  512K cycles  | 4.0 s
	*	1    0    0    1    | 1024K cycles  | 8.0 s
	*/

#ifndef TEST_RADIO
	WDTCSR = (1 << WDP3) | (0 << WDP2) | (0 << WDP1) | (1 << WDP0);	// 8sec
#else
	WDTCSR = (0 << WDP3) | (1 << WDP2) | (0 << WDP1) | (1 << WDP0); // 0.5 sec
#endif

	// Enable the WD interrupt (note: no reset).
	WDTCSR |= _BV(WDIE);
}

void enterSleepModeWd() {
	
	/*SLEEP_MODE_IDLE - the least power savings
	SLEEP_MODE_ADC
	SLEEP_MODE_PWR_SAVE
	SLEEP_MODE_STANDBY
	SLEEP_MODE_PWR_DOWN - the most power savings*/

	pinMode(0, INPUT);
	pinMode(1, INPUT);

	// Disable ADC
	ADCSRA = ADCSRA & B01111111;
	// Disable comparator
	ACSR = B10000000;
	// Disable analog buffer
	 DIDR0 = DIDR0 | B00111111;

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	//set_sleep_mode(SLEEP_MODE_IDLE);
	
	sleep_enable();

	sleep_mode();

	sleep_disable();

	power_all_enable();
	//power_adc_disable();
	//power_spi_disable();
}

void blinkLed(int nbr) {
	for (int k = 0; k < nbr; k++){
		digitalWrite(LED_BUILTIN, HIGH);
		delay(100);
		digitalWrite(LED_BUILTIN, LOW);
		delay(100);
	}

	delay(500);

	for (int k = 0; k < nbr; k++){
		digitalWrite(LED_BUILTIN, HIGH);
		delay(100);
		digitalWrite(LED_BUILTIN, LOW);
		delay(100);
	}
}

void setup(){

	pinMode(DHT_POW, OUTPUT);
	pinMode(RADIO_POW, OUTPUT);
	pinMode(LED_BUILTIN, OUTPUT);

	Serial.begin(1200);

	dht.begin();

	setupWdTimer();
}

float hum, temp;
int cpt = 0;

void loop() {

	if (f_timer == 1) {
		f_timer = 0;

	#ifdef TEST_RADIO
		if(false){
	#else
		if ( cpt >= 0 && cpt < 3) {
	#endif
			cpt++;
		}
		else {
			cpt = 0;

			digitalWrite(DHT_POW, HIGH);
			
			#ifdef TEST_RADIO
				delay(100);
			#else
				delay(100 + SENSOR_ID * 100);
			#endif 

			digitalWrite(RADIO_POW, HIGH);
			hum = dht.readHumidity();
			temp = dht.readTemperature();

			for (int i = 0; i < 4; i++)
			{
				byte buff[21];

				buff[0] = 0xA5;

				buff[1] = SENSOR_ID;

				int tmp = temp * 100;
				buff[2] = (tmp >> 8) & 0xFF;
				buff[3] = tmp & 0xFF;

				tmp = hum * 100;
				buff[4] = (tmp >> 8) & 0xFF;
				buff[5] = tmp & 0xFF;

				/*buff[0] = 0xA5;
				buff[1] = 0x1;
				buff[2] = 0x2;
				buff[3] = 0x3;
				buff[4] = 0x4;
				buff[5] = 0x5;*/

				buff[6] = buff[0] + buff[1] + buff[2] + buff[3] + buff[4] + buff[5];

				for (int k = 7; k < 21; k++) {
					buff[k] = buff[k % 7];
				}

				Serial.write(buff, 21);
				Serial.flush();

				delay(10);
			}

			digitalWrite(RADIO_POW, LOW);
			digitalWrite(DHT_POW, LOW);

			
			wdt_reset();
		}
	}
	else {
		delay(100);
	}
	
	enterSleepModeWd();
}
