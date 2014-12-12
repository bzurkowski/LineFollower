/*
	Mniejsze wartości sensorów odpowiadają większemu odbiciu (np. kolor biały)
	natomiast większe odpowiadają mniejszemu odbiciu (np. kolor czarny).
	Sensory zwracają wartość pomiędzy 0 a 1023.
*/

#ifndef QTRSensors.h
#define QTRSensors.h

//tryb czytania
#define QTR_EMITTERS_OFF 0 //mierzenie przy wylaczonych diodach IR (odczyt reprezentuje otaczajacy poziom swiatla w poblizu sensorow)
#define QTR_EMITTERS_ON 1 //mierzenie przy wlaczonych diodach IR (odczyt odbicia)
#define QTR_EMITTERS_ON_AND_OFF 2 //odczyt w obu stanach

#define NUM_QTR_SENSORS 6 //liczba sensorow

#define QTR_NO_EMITTER_PIN 255 //jezeli emiter pin nie jest ustawiony

class QTRSensors {
public:
// konstruktory
	QTRSensors(unsigned char *pins, unsigned char numSamplesPerSensor = 4, unsigned char emitterPin = QTR_NO_EMITTER_PIN);

//funkcje
	void init(unsigned char *pins, unsigned char numSamplesPerSensor = 4, unsigned char emitterPin = QTR_NO_EMITTER_PIN);

	void read(unsigned int *sensor_values, unsigned char readMode = QTR_EMITTERS_ON);
	void emittersOff();
	void emittersOn();

	void calibrate(unsigned char readMode = QTR_EMITTERS_ON);
// void resetCalibration();
	void readCalibrated(unsigned int *sensor_values, unsigned char readMode = QTR_EMITTERS_ON);

//atrybuty
	unsigned int* calibratedMinimumOn;
	unsigned int* calibratedMaximumOn;
	unsigned int* calibratedMinimumOff;
	unsigned int* calibratedMaximumOff;

//destruktor
	~QTRSensors();

protected:

	unsigned char[NUM_QTR_SENSORS] _pins; //numery pinow
	unsigned char _emitterPin; //numer LEDON
	unsigned int _maxValue;

private:

	void readPrivate(unsigned int *sensor_values);
	void calibrateOnOrOff(unsigned int **calibratedMinimum, unsigned int **calibratedMaximum, unsigned char readMode);

	unsigned char _numSamplesPerSensor;
};

#endif