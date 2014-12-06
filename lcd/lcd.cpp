#include "lcd.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
// #include "Arduino.g"


LCD::LCD(uint8_t rs, uint8_t rw, uint8_t enable,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
	init(1, rs, rw, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}

void LCD::init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	_rs_pin = rs;
	_rw_pin = rw;
	_enable_pin = enable;

	_data_pins[0] = d0;
	_data_pins[1] = d1;
	_data_pins[2] = d2;
	_data_pins[3] = d3;
	_data_pins[4] = d4;
	_data_pins[5] = d5;
	_data_pins[6] = d6;
	_data_pins[7] = d7;

	pinMode(_rs_pin, OUTPUT);
	pinMode(_rw_pin, OUTPUT);
	pinMode(_enable_pin, OUTPUT);

	_displayfunction = LCD_1LINE | LCD_5x8DOTS;

	if (fourbitmode) {
		_displayfunction |= LCD_4BITMODE;
	} else {
		_displayfunction |= LCD_8BITMODE;
	}

	begin(16, 1);
}

void LCD::begin(uint8_t cols, uint8_t lines, uint8_t dotsize)
{
	if (lines > 1) {
		_displayfunction |= LCD_2LINE;
	}
	_numlines = lines;
	_currline = 0;
	
	delayMicroseconds(50000);

	digitalWrite(_rs_pin, LOW);
	digitalWrite(_enable_pin, LOW);
	digitalWrite(_rw_pin, LOW);

	if (! (_displayfunction & LCD_8BITMODE)) {
		// try to set 4 bit mode
		for (int i = 0; i < 3; i++) {
			write4bits(0x03);
			delayMicroseconds(4500);
		}

		// try to set 4-bit interface
		write4bits(0x02);
	}

	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOTOFF | LCD_BLINKOFF;
	
	// init to default text direction (from left to right)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

	command(LCD_FUNCTIONSET | _displayfunction);
	command(LCD_ENTRYMODESET | _displaymode);

	display();
	clear();
}

void LCD::clear()
{
	command(LCD_CLEARDISPLAY);
	delayMicroseconds(2000);  // long lasting commmand
}

void LCD::home()
{
	command(LCD_RETURNHOME);
	delayMicroseconds(2000);  // long lasting commmand
}

void LCD::set_cursor(uint8_t col, uint8_t row)
{
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };

	if (row >= _numlines)
		row = _numlines - 1;

	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void LCD::no_display() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LCD::display() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LCD::create_char(uint8_t location, uint8_t charmap[])
{
	location &= 0x7;  // we only have 8 locations 0-7

	command(LCD_SETCGRAMADDR | (location << 3));

	for (int i = 0; i < 8; i++) {
		write(charmap[i]);
	}
}

size_t LCD::print(const char str[])
{
	return write(str);
}

size_t LCD::print(char c)
{
	return write(c);
}

size_t LCD::print(int n)
{
	return print_number(n, 10);
}

size_t LCD::print_number(unsigned long n, uint8_t base) {
	char buf[8 * sizeof(long) + 1];
	char *str = &buf[sizeof(buf) - 1];
	*str = '\0';
	
	if (base < 2) base = 10;
	do {
		unsigned long m = n;
		n /= base;
		char c = m - base * n;
		*--str = c < 10 ? c + '0' : c + 'A' - 10;
	} while(n);
	
	return write(str);
}

void LCD::command(uint8_t value)
{
	send(value, LOW);
}

size_t LCD::write(uint8_t value)
{
	send(value, HIGH);
	return 1;
}

size_t LCD::write(const uint8_t *buffer, size_t size)
{
	if (buffer == NULL) return 0;

	size_t n = 0;	
	while (size--) {
		n += write(*buffer++);
	}

	return n;
}

size_t LCD::write(const char *buffer)
{
	if (str == NULL) return 0;
	return write((const uint8_t *)buffer, strlen(buffer));
}

void LCD::send(uint8_t value, uint8_t mode)
{
	digitalWrite(_rs_pin, mode);

	digitalWrite(_rw_pin, LOW);

	if (! (_displayfunction & LCD_8BITMODE)) {
		write4bits(value >> 4);
		write(value);
	}
}

void LCD::write4bits(uint8_t value)
{
	for (int i = 0; i < 4; i++) {
		pinMode(_data_pins[i], OUTPUT);
		digitalWrite(_data_pins[i], (value >> i) & 0x01);
	}

	pulse_enable();
}

void LCD::pulse_enable()
{
	/*
	When data is supplied to data pins, a high-t-low pulse must be applied to this pin
	in order for LCD to latch in the data present at the data pins. This pulse must be 
	a minimum of 450ns wide.
	*/

	digitalWrite(_enable_pin, LOW);
	delayMicroseconds(1);

	digitalWrite(_enable_pin, HIGH);
	delayMicroseconds(1);

	digitalWrite(_enable_pin, LOW);
	delayMicroseconds(100);  // commands need > 37ns to settle
}
