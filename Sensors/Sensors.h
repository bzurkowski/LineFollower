/*
	Mniejsze wartości sensorów odpowiadają większemu odbiciu (np. kolor biały)
	natomiast większe odpowiadają mniejszemu odbiciu (np. kolor czarny).
	Sensory zwracają wartość pomiędzy 0 a 1023.
*/

#ifndef Sensors.h
#define Sensors.h

//tryb czytania
#define EMITTERS_OFF 0 //mierzenie przy wylaczonych diodach IR (odczyt reprezentuje otaczajacy poziom swiatla w poblizu sensorow)
#define EMITTERS_ON 1 //mierzenie przy wlaczonych diodach IR (odczyt odbicia)
#define EMITTERS_ON_AND_OFF 2 //odczyt w obu stanach

#define NUM_SENSORS 6 //liczba sensorow

#define NO_EMITTER_PIN 255 //jezeli emiter pin nie jest ustawiony

class Sensors {
public:
// konstruktory
	Sensors(unsigned char *pins, unsigned char numSamplesPerSensor = 4, unsigned char emitterPin = NO_EMITTER_PIN);

//funkcje
	void init(unsigned char *pins, unsigned char numSamplesPerSensor = 4, unsigned char emitterPin = NO_EMITTER_PIN);

	void read(unsigned int *sensor_values, unsigned char readMode = EMITTERS_ON);
	void emittersOff();
	void emittersOn();

	void calibrate(unsigned char readMode = EMITTERS_ON);
// void resetCalibration();
	void readCalibrated(unsigned int *sensor_values, unsigned char readMode = EMITTERS_ON);

//atrybuty
	unsigned int* calibratedMinimumOn;
	unsigned int* calibratedMaximumOn;
	unsigned int* calibratedMinimumOff;
	unsigned int* calibratedMaximumOff;

//destruktor
	~Sensors();

protected:

	unsigned char[NUM_SENSORS] _pins; //numery pinow
	unsigned char _emitterPin; //numer LEDON
	unsigned int _maxValue;

private:

	void readPrivate(unsigned int *sensor_values);
	void calibrateOnOrOff(unsigned int **calibratedMinimum, unsigned int **calibratedMaximum, unsigned char readMode);

	unsigned char _numSamplesPerSensor;
};

#endif