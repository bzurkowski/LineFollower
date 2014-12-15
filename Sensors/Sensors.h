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
	Sensors(unsigned char *pins, unsigned char num_samples_per_sensor = 4, unsigned char emitter_pin = NO_EMITTER_PIN);

//funkcje
	void init(unsigned char *pins, unsigned char num_samples_per_sensor = 4, unsigned char emitter_pin = NO_EMITTER_PIN);

	void read(unsigned int *sensor_values, unsigned char read_mode = EMITTERS_ON);
	void emitters_off();
	void emitters_on();

	void calibrate(unsigned char read_mode = EMITTERS_ON);
// void resetCalibration();
	void read_calibrated(unsigned int *sensor_values, unsigned char read_mode = EMITTERS_ON);

//atrybuty
	unsigned int* calibrated_minimum_on;
	unsigned int* calibrated_maximum_on;
	unsigned int* calibrated_minimum_off;
	unsigned int* calibrated_maximum_off;

//destruktor
	~Sensors();

protected:

	unsigned char[NUM_SENSORS] _pins; //numery pinow
	unsigned char _emitter_pin; //numer LEDON
	unsigned int _maxValue;

private:

	void read_private(unsigned int *sensor_values);
	void calibrate_on_or_off(unsigned int **calibrated_minimum, unsigned int **calibrated_maximum, unsigned char read_mode);

	unsigned char _num_samples_per_sensor;
};

#endif