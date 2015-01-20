#include <stdlib.h>
#include "Sensors.h"
#include <arduino.h>

//KONSTRUKTORY

Sensors::Sensors(unsigned char* pins, unsigned char num_samples_per_sensor, unsigned char emitter_pin) {
	int i;
	calibrated_minimum_on = 0;
	calibrated_maximum_on = 0;
	calibrated_minimum_off = 0;
	calibrated_maximum_off = 0;

	for (i = 0; i < NUM_SENSORS; i++)
		_pins[i] = pins[i];

	_emitter_pin = emitter_pin;
	_num_samples_per_sensor = num_samples_per_sensor;
	_max_value = 1023;
}

//FUNKCJE

void Sensors::read(unsigned int *sensor_values, unsigned char read_mode) {
	unsigned int off_values[NUM_SENSORS];
	int i;

	if (read_mode == EMITTERS_ON || read_mode == EMITTERS_ON_AND_OFF)
		emitters_on();
	else
		emitters_off();

	read_private(sensor_values);
	emitters_off();

	if (read_mode == EMITTERS_ON_AND_OFF) {
		read_private(off_values);

		for(i = 0; i < NUM_SENSORS; i++)
			sensor_values[i] += _max_value - off_values[i];
	}
}

void Sensors::emitters_off() {
	if (_emitter_pin == NO_EMITTER_PIN) return; //jezeli nie ustawimy LEDON'a to nie mozemy wylaczyc emiterow

	pinMode(_emitter_pin, OUTPUT);
	digitalWrite(_emitter_pin, LOW);
	delayMicroseconds(200);
}

void Sensors::emitters_on() {
	if (_emitter_pin == NO_EMITTER_PIN) return; //jezeli nie ustawimy LEDON'a to nie mozemy wlaczyc emiterow

	pinMode(_emitter_pin, OUTPUT);
	digitalWrite(_emitter_pin, HIGH);
	delayMicroseconds(200);
}

void Sensors::calibrate(unsigned char read_mode) {
	if (read_mode == EMITTERS_ON || read_mode == EMITTERS_ON_AND_OFF) {
		calibrate_on_or_off(&calibrated_minimum_on, &calibrated_maximum_on, EMITTERS_ON);
	}
	if (read_mode == EMITTERS_OFF || read_mode == EMITTERS_ON_AND_OFF) {
		calibrate_on_or_off(&calibrated_minimum_off, &calibrated_maximum_off, EMITTERS_OFF);
	}
}

// void reset_calibration();
void Sensors::read_calibrated(unsigned int *sensor_values, unsigned char read_mode) {
	int i;

	//jezeli nie sa zaalokowane to wracamy
	if(read_mode == EMITTERS_ON || read_mode == EMITTERS_ON_AND_OFF)
		if(!calibrated_minimum_on || !calibrated_maximum_on)
			return;
	if(read_mode == EMITTERS_OFF || read_mode == EMITTERS_ON_AND_OFF)
		if(!calibrated_minimum_off || !calibrated_maximum_off)
			return;

	//jezeli sa to czytamy
	read(sensor_values, read_mode);

	//i poprawiamy
	for (i = 0; i < NUM_SENSORS; i++) {
		unsigned int calmin, calmax;
		unsigned int denominator;

		if (read_mode == EMITTERS_ON) {
			calmin = calibrated_minimum_on[i];
			calmax = calibrated_maximum_on[i];
		}
		else if (read_mode == EMITTERS_OFF) {
			calmin = calibrated_minimum_off[i];
			calmax = calibrated_maximum_off[i];
		}
		else { //if (read_mode == EMITTERS_ON_AND_OFF)
			if(calibrated_minimum_off[i] < calibrated_minimum_on[i]) // male znaczenie sygnalu
				calmin = _max_value;
			else
				calmin = calibrated_minimum_on[i] + _max_value - calibrated_minimum_off[i];

			if(calibrated_maximum_off[i] < calibrated_maximum_on[i]) // male znaczenie sygnalu
				calmax = _max_value;
			else
				calmax = calibrated_maximum_on[i] + _max_value - calibrated_maximum_off[i];
		}
		
		denominator = calmax - calmin;

		signed int x = 0;
		if (denominator != 0)
			x = (((signed long)sensor_values[i]) - calmin) * 1000 / denominator;
		if (x < 0)
			x = 0;
		else if(x > 1000)
			x = 1000;

		sensor_values[i] = x;
	}
}

void Sensors::read_private(unsigned int *sensor_values) {
	int i, j;

	for (i = 0; i < NUM_SENSORS; i++)
		sensor_values[i] = 0;

	for (i = 0; i < _num_samples_per_sensor; i++)
		for (j = 0; j < NUM_SENSORS; j++)
			sensor_values[j] += analogRead(_pins[j]);

	for (i = 0; i < NUM_SENSORS; i++)
		sensor_values[i] = (sensor_values[i] + (_num_samples_per_sensor >> 1)) / _num_samples_per_sensor;
		// sensor_values[i] = sensor_values[i] / _num_samples_per_sensor;
}

void Sensors::calibrate_on_or_off(unsigned int **calibrated_minimum, unsigned int **calibrated_maximum, unsigned char read_mode) {
	int i;
	unsigned int sensor_values[NUM_SENSORS];
	unsigned int max_sensor_values[NUM_SENSORS];
	unsigned int min_sensor_values[NUM_SENSORS];

	//alokacja tablic calibrated...
	if(*calibrated_minimum == 0) {
		*calibrated_minimum = (unsigned int*)malloc(sizeof(unsigned int) * NUM_SENSORS);

		if(*calibrated_minimum == 0) return;

		for(i = 0; i < NUM_SENSORS; i++)
			(*calibrated_minimum)[i] = _max_value;
	}
	if(*calibrated_maximum == 0) {
		*calibrated_maximum = (unsigned int*)malloc(sizeof(unsigned int) * NUM_SENSORS);

		if(*calibrated_maximum == 0) return;

		for(i = 0; i < NUM_SENSORS; i++)
			(*calibrated_maximum)[i] = 0;
	}

	//szukanie min i max na czujnikach przez 10 odczytow
	for (int i = 0; i < 10; i++) {
		read(sensor_values, read_mode);

		for (int j = 0; j < NUM_SENSORS; j++) {
			if (i == 0 || max_sensor_values[j] < sensor_values[j])
				max_sensor_values[j] = sensor_values[j];
		}

		for (int j = 0; j < NUM_SENSORS; j++) {
			if (i == 0 || min_sensor_values[j] > sensor_values[j])
				min_sensor_values[j] = sensor_values[j];
		}
	}

	for (i = 0; i < NUM_SENSORS; i++) {
		if(min_sensor_values[i] > (*calibrated_maximum)[i])
			(*calibrated_maximum)[i] = min_sensor_values[i];
		if(max_sensor_values[i] < (*calibrated_minimum)[i])
			(*calibrated_minimum)[i] = max_sensor_values[i];
	}
}

int Sensors::read_line(unsigned int *sensor_values, unsigned char read_mode, unsigned char white_line) {
	int i, on_line = 0;
	unsigned long avg;
	unsigned int sum;
	static int last_value = 0; //do przechowywania z ktorej strony widziano linie, domyslnie: lewa

	read_calibrated(sensor_values, read_mode);

	avg = 0;
	sum = 0;

	for(i = 0; i < NUM_SENSORS; i++) {
		int value = sensor_values[i];
		if(white_line)
			value = 1000 - value;

		if(value > 200) //sprawdza czy widzimy linie
			on_line = 1;

		if(value > 50) {
			avg += (long) (value) * i * 1000;
			sum += value;
		}
	}

	if(!on_line) {
		if(last_value < (NUM_SENSORS - 1) * 1000 / 2)
			return 0;
		else
			return (NUM_SENSORS - 1) * 1000;
	}

	last_value = avg / sum;

	return last_value;
}

//DESTRUKTOR

Sensors::~Sensors() {
	if (calibrated_minimum_on)
		free(calibrated_minimum_on);
	if (calibrated_maximum_on)
		free(calibrated_maximum_on);
	if (calibrated_minimum_off)
		free(calibrated_minimum_off);
	if (calibrated_maximum_off)
		free(calibrated_maximum_off);
}