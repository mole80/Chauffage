#include "Sensor.h"

float ConvToTemp(int adc) {
	// Gain analogique : 7.8 V/V
	// Mesure temps : 10 °C/V
	// Conversion : digit * 5 / 1023
	float val = adc * 0.06266;
	return val;
}

