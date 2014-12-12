#include <stdlib.h>
#include "QTRSensors.h"
#include <arduino.h>

//KONSTRUKTORY

QTRSensors::QTRSensors(unsigned char* pins, unsigned char numSamplesPerSensor, unsigned char emitterPin) {
	init(pins, numSamplesPerSensor, emitterPin);
}

//FUNKCJE

void QTRSensors::init(unsigned char *pins, unsigned char numSamplesPerSensor, unsigned char emitterPin) {
	int i;
	calibratedMinimumOn = 0;
	calibratedMaximumOn = 0;
	calibratedMinimumOff = 0;
	calibratedMaximumOff = 0;

	for (i = 0; i < NUM_QTR_SENSORS; i++)
		_pins[i] = pins[i];

	_emitterPin = emitterPin;
	_numSamplesPerSensor = numSamplesPerSensor;
	_maxValue = 1023;
}

void QTRSensors::read(unsigned int *sensor_values, unsigned char readMode = QTR_EMITTERS_ON) {
	unsigned int off_values[NUM_QTR_SENSORS];
	int i;

	if (readMode == QTR_EMITTERS_ON || readMode == QTR_EMITTERS_ON_AND_OFF)
		emittersOn();
	else
		emittersOff();

	readPrivate(sensor_values);
	emittersOff();

	if (readMode == QTR_EMITTERS_ON_AND_OFF) {
		readPrivate(off_values);

		for(i = 0; i < NUM_QTR_SENSORS, i++)
			sensor_values[i] += _maxValue - off_values[i];
	}
}

void QTRSensors::emittersOff() {
	if (_emitterPin == QTR_NO_EMITTER_PIN) return; //jezeli nie ustawimy LEDON'a to nie mozemy wylaczyc emiterow

	pinMode(_emitterPin, OUTPUT);
	digitalWtire(_emitterPin, LOW);
	delayMicroseconds(200);
}

void QTRSensors::emittersOn() {
	if (_emitterPin == QTR_NO_EMITTER_PIN) return; //jezeli nie ustawimy LEDON'a to nie mozemy wlaczyc emiterow

	pinMode(_emitterPin, OUTPUT);
	digitalWtire(_emitterPin, HIGH);
	delayMicroseconds(200);
}

void QTRSensors::calibrate(unsigned char readMode) {
	if (readMode == QTR_EMITTERS_ON || readMode == QTR_EMITTERS_ON_AND_OFF) {
		calibrateOnOrOff(&calibratedMinimumOn, &calibratedMaximumOn, QTR_EMITTERS_ON);
	}
	if (readMode == QTR_EMITTERS_OFF || readMode == QTR_EMITTERS_ON_AND_OFF) {
		calibrateOnOrOff(&calibratedMinimumOff, &calibratedMaximumOff, QTR_EMITTERS_OFF);
	}
}

// void resetCalibration();
void QTRSensors::readCalibrated(unsigned int *sensor_values, unsigned char readMode) {
	int i;

	//jezeli nie sa zaalokowane to wracamy
	if(readMode == QTR_EMITTERS_ON || readMode == QTR_EMITTERS_ON_AND_OFF)
		if(!calibratedMinimumOn || !calibratedMaximumOn)
			return;
	if(readMode == QTR_EMITTERS_OFF || readMode == QTR_EMITTERS_ON_AND_OFF)
		if(!calibratedMinimumOff || !calibratedMaximumOff)
			return;

	//jezeli sa to czytamy
	read(sensor_values, readMode);

	//i poprawiamy
	for (i = 0; i < NUM_QTR_SENSORS; i++) {
		unsigned int calmin, calmax;
		unsigned int denominator;

		if (readMode == QTR_EMITTERS_ON) {
			calmin = calibratedMinimumOn[i];
			calmax = calibratedMaximumOn[i];
		}
		else if (readMode == QTR_EMITTERS_OFF) {
			calmin = calibratedMinimumOff[i];
			calmax = calibratedMaximumOff[i];
		}
		else { //if (readMode == QTR_EMITTERS_ON_AND_OFF)
			if(calibratedMinimumOff[i] < calibratedMinimumOn[i]) // male znaczenie sygnalu
				calmin = _maxValue;
			else
				calmin = calibratedMinimumOn[i] + _maxValue - calibratedMinimumOff[i];

			if(calibratedMaximumOff[i] < calibratedMaximumOn[i]) // male znaczenie sygnalu
				calmax = _maxValue;
			else
				calmax = calibratedMaximumOn[i] + _maxValue - calibratedMaximumOff[i];
		}
		
		denominator = calmax - calmin;

		int x = 0;
		if (denominator != 0)
			x = (sensor_values[i] - calmin) * 1000 / denominator;
		if (x < 0)
			x = 0;
		else if(x > 1000)
			x = 1000;

		sensor_values[i] = x;
	}
}

void QTRSensors::readPrivate(unsigned int *sensor_values) {
	int i, j;

	for (i = 0; i < NUM_QTR_SENSORS; i++)
		sensor_values[i] = 0;

	for (i = 0; i < _numSamplesPerSensor; i++)
		for (j = 0; j < NUM_QTR_SENSORS; j++)
			sensor_values[j] += analogRead(_pins[i])

	for (i = 0; i < NUM_QTR_SENSORS; i++)
		sensor_values[i] = sensor_values[i] / _numSamplesPerSensor; //sensor_values[i] = (sensor_values[i] + (_numSamplesPerSensor >> 1)) / _numSamplesPerSensor;
}

void QTRSensors::calibrateOnOrOff(unsigned int **calibratedMinimum, unsigned int **calibratedMaximum, unsigned char readMode) {
	int i;
	unsigned int sensor_values[];
	unsigned int max_sensor_values[];
	unsigned int min_sensor_values[];

	//alokacja tablic calibrated...
	if(*calibratedMinimum == 0) {
		*calibratedMinimum = (unsigned int*)malloc(sizeof(unsigned int) * NUM_QTR_SENSORS);

		if(*calibratedMinimum == 0) return;

		for(i = 0; i < NUM_QTR_SENSORS; i++)
			(*calibratedMinimum)[i] == _maxValue;
	}
	if(*calibratedMaximum == 0) {
		*calibratedMaximum = (unsigned int*)malloc(sizeof(unsigned int) * NUM_QTR_SENSORS);

		if(*calibratedMaximum == 0) return;

		for(i = 0; i < NUM_QTR_SENSORS; i++)
			(*calibratedMaximum)[i] == 0;
	}

	//szukanie min i max na czujnikach przez 10 odczytow
	for (i = 0; i < 10; i++) {
		read(sensor_values, readMode);

		for (j = 0; j < NUM_QTR_SENSORS; j++) {
			if (i == 0 || max_sensor_values[j] < sensor_values[j])
				max_sensor_values[j] = sensor_values[j];

		for (j = 0; j < NUM_QTR_SENSORS; j++) {
			if (i == 0 || min_sensor_values[j] > sensor_values[j])
				min_sensor_values[j] = sensor_values[j];
		}
	}

	for (i = 0; i < NUM_QTR_SENSORS; i++) {
		if(min_sensor_values[i] > (*calibratedMaximum)[i])
			(*calibratedMaximum)[i] = min_sensor_values[i];
		if(max_sensor_values[i] < (*calibratedMaximum)[i])
			(*calibratedMinimum)[i] = max_sensor_values[i];
	}
}

//DESTRUKTOR

QTRSensors::~QTRSensors() {
	if (calibratedMinimumOn)
		free(calibratedMinimumOn);
	if (calibratedMaximumOn)
		free(calibratedMaximumOn);
	if (calibratedMinimumOff)
		free(calibratedMinimumOff);
	if (calibratedMaximumOff)
		free(calibratedMaximumOff);
}