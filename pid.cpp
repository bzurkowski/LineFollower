#include "Sensors.h" 

int read_line(unsigned int *sensor_values, unsigned char read_mode, unsigned char white_line) {
	int i, on_line = 0;
	unsigned long avg = 0;
	unsigned int sum = 0;
	static int last_value = 0; //do przechowywania z ktorej strony widziano linie, domyslnie: lewa

	read_calibrated(sensor_values, read_mode);

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