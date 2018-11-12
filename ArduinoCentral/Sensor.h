#pragma once
#include "Arduino.h"
#include "persistanteValue.h"

class BaseComp {
public:
	BaseComp() {}
	BaseComp(int16_t pid) {
		id = pid;
		isUsed = false;
	}

	void SetIsUSed() {
		isUsed = true;
	}

	bool IsUsed() {
		return isUsed;
	}

	int16_t GetId() {
		return id;
	}

private:
	int16_t id;
	bool isUsed;
};

/*class BaseComp {
public:
	BaseComp() {}
	String name;
	BaseComp(int16_t pid, String pname) {
		name = pname;
		id = pid;
	}

	int16_t GetId() {
		return id;
	}

private:
	int16_t id;
};*/


class BaseSensor : public BaseComp{
public:
	BaseSensor() {}
//	BaseSensor(int16_t id, String name) :
//		BaseComp(id,name)
	BaseSensor(int16_t id) :
		BaseComp(id){
		cpt_meas = 0;
	}

	//String name;

	virtual float ConvValue(float meas)=0;

	float GetValue() {
		newValue = false;
		return value;
	}

	int16_t GetCptMeas() {
		return cpt_meas;
	}

	String GatValueAsString() {
		return String(GetValue());
	}

	void SetValue(float val) {
		value = ConvValue( val );
		newValue = true;
	}

protected:
	int16_t cpt_meas;
	float value;
	bool newValue;
};


class RadioSensor : public BaseSensor
{
public:
	RadioSensor() {}
//	RadioSensor(int16_t id, String name) :
//		BaseSensor(id, name)
	RadioSensor(int16_t id) :
		BaseSensor(id)
	{
		timeout = 0;
	}

	uint32_t GetTimeout() {
		return timeout;
	}

	float GetTemp() {
		return temp;
	}

	String GetTempAsString() {
		return String(temp);
	}

	void SetMeasure(float temp, float hum) {
		this->temp = temp;
		this->hum = hum;
		cpt_meas++;
		timeout = 0;
	}

	void UpdateTimeout() {
		if (timeout < 0x7FFF) {
			timeout++;
		}
	}

	float ConvValue(float meas) {
		return meas;
	}
private:
	uint32_t timeout;
	float temp;
	float hum;
};


class Valve {
public:
	Valve(uint8_t id) :
		id(id)
	{
		switch(id)
		{
		case 0:
			num_io = 5;
			break;
		case 1:
			num_io = 6;
			break;
		case 2:
			num_io = 4;
			break;
		case 3:
			num_io = 3;
			break;
		case 4:
			num_io = 2;
			break;
		default:
			break;
		}
	}

	void OpenValve() {
		digitalWrite(num_io, LOW);
		isOpen = true;
	}

	void CloseValve() {
		digitalWrite(num_io, HIGH);
		isOpen = false;
	}

	bool IsOpen() {
		return isOpen;
	}

private:
	uint8_t num_io;
	uint8_t id;
	bool isOpen;
};

float ConvToTemp(int adc);

class Room : public BaseComp
{
public:
	//Room(int16_t id, String name, RadioSensor rad, float defTarget, uint16_t idTempTarget) :
	Room(int16_t id, RadioSensor& rad, float defTarget, uint16_t idTempTarget) :
		BaseComp(id),
		tempTarget(defTarget, idTempTarget),
		tempSensor(rad),
		valve( Valve(id) )
	{
		target = 0.0;

		switch(id)
		{
		case 0:
			adcChannelTemp = 7;
			break;
		case 1:
			adcChannelTemp = 6;
			break;
		case 2:
			adcChannelTemp = 5;
			break;
		case 3:
			adcChannelTemp = 4;
			break;
		case 4:
			adcChannelTemp = 3;
			break;
		default:
			break;
		}
	}

	void SetDefaultPwm(uint32_t freq, uint32_t duty) {
		this->freq = freq;
		this->duty = duty;
		this->cpt = 0;
	}

	void UpdateTempValue(int adcValueStartWater) {
		tempWaterStart = ConvToTemp(adcValueStartWater);
		tempWaterEnd = ConvToTemp(analogRead(adcChannelTemp));
	}

	void ExecuteRegulation() {
		float meas = tempSensor.GetTemp();
		float tar = tempTarget.ReadTemp();

		if (target <= 0.0) {
			if ( meas < tar ) {
				target = tar + 0.2;
			}
			else {
				target = tar - 0.2;
			}
		}

		// Security mode PWM fixe
		// timeout 1s @600 = 10min sans signal
		if ( tempSensor.GetTimeout() > 600 ) {
			cpt++;
			if (cpt >= freq) {
				cpt = 0;
			}

			if (cpt > duty) {
				valve.OpenValve();
			}
			else {
				valve.CloseValve();
			}
		}
		else {

			if (meas > tar + 0.2) {
				target = tar - 0.2;
			}
			else if (meas < tar - 0.2) {
				target = tar + 0.2;
			}

			cpt = 0;
			if (tempSensor.GetTemp() < target ) {
				valve.OpenValve();
			}
			else {
				valve.CloseValve();
			}
		}
	}

	uint32_t GetCpt() {
		return cpt;
	}

	float GetTempStart() {
		return tempWaterStart;
	}

	float GetTempEnd() {
		return tempWaterEnd;
	}

	String GetMeasAsString() {
		return tempSensor.GetTempAsString();
	}

	String GetTargetAsString() {
		//return String(tempTarget.ReadTemp());
		return String(target);
	}

	String GetTempStartAsString() {
		return String(tempWaterStart);
	}

	String GetTempEndAsString() {
		return String(tempWaterEnd);
	}

	bool IsValveOpen() {
		return valve.IsOpen();
	}

	String IsValveOpenAsString() {
		if (IsValveOpen())
			return "O";
		else
			return "C";
	}

	PersistTempTarget tempTarget;

	RadioSensor& GetSensor() {
		return tempSensor;
	}

private:
	float target;
	uint32_t duty;
	uint32_t freq;
	uint32_t cpt;
	RadioSensor& tempSensor;
	Valve valve;
	uint8_t adcChannelTemp;
	//float target;
	float tempWaterStart;
	float tempWaterEnd;
	RadioSensor* tempSensorSupl;
};

