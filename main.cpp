// #include <QTRSensors.h>
#include "lcd.h"

#define MAX_SPEED 30
#define KP 0.0085
#define KD 0

LCD lcd(2, 1, 13, 12, 11, 10);  // TODO: Where is enable pin?
// QTRSensorsAnalog qtra((unsigned char[]) { 0, 1, 2, 3, 4, 5 }, 6);

// pins
int pwm_motor_a = 5;  // right
int pwm_motor_b = 6;  // left
int in_motor_a1 = 4;
int in_motor_a2 = 3;
int in_motor_b1 = 7;
int in_motor_b2 = 8;
int pwm_ledon = 9;

// movement, line following
int pos = 0;
int error_val = 0;
int last_error = 0;
int motor_speed;
int left_speed = 0;
int right_speed = 0;
unsigned int sensors[6];

void initialize_lcd();
void create_bars();
void display_debug_info();
void say_hello();

void initialize_sensors();
void calibrate_sensors();
void initialize_motors();
void calculate_movement();
void drive_motors();

byte sensor_bars[8][8] = {
  { B00000, B00000, B00000, B00000, B00000, B00000, B00000, B11111 },
  { B00000, B00000, B00000, B00000, B00000, B00000, B11111, B11111 },
  { B00000, B00000, B00000, B00000, B00000, B11111, B11111, B11111 },
  { B00000, B00000, B00000, B00000, B11111, B11111, B11111, B11111 },
  { B00000, B00000, B00000, B11111, B11111, B11111, B11111, B11111 },
  { B00000, B00000, B11111, B11111, B11111, B11111, B11111, B11111 },
  { B00000, B11111, B11111, B11111, B11111, B11111, B11111, B11111 },
  { B11111, B11111, B11111, B11111, B11111, B11111, B11111, B11111 }  
};

void setup() {
  initialize_lcd();  
  create_bars();
  say_hello();
  initialize_sensors();
  initialize_motors();
  calibrate_sensors();
}

void loop() {
  read_sensors();
  calculate_movement();
  drive_motors();
  display_debug_info();
  
  delay(10);
}

void initialize_lcd() {
   lcd.begin(16, 2);  
}

void create_bars() {
  for (int i = 0; i < 8; i++)
    lcd.create_char(i, sensor_bars[i]); 
}

void say_hello() {
  lcd.clear();
  
  lcd.set_cursor(0, 0); 
  lcd.print("LINE FOLLOWER");
}

void initialize_sensors() {
  digitalWrite(pwm_ledon, 128);
}

void initialize_motors() {
  pinMode(pwm_motor_a, OUTPUT); 
  pinMode(pwm_motor_b, OUTPUT);
  pinMode(in_motor_a1, OUTPUT); 
  pinMode(in_motor_a2, OUTPUT);   
  pinMode(in_motor_b1, OUTPUT);
  pinMode(in_motor_b2, OUTPUT);
  
  digitalWrite(in_motor_a1, HIGH); 
  digitalWrite(in_motor_a2, LOW);
  
  digitalWrite(in_motor_b1, LOW); 
  digitalWrite(in_motor_b2, HIGH);
  
  analogWrite(pwm_motor_a, 0);
  analogWrite(pwm_motor_b, 0);
}

void calibrate_sensors() {
  lcd.clear(); 
  lcd.set_cursor(0, 0);
  lcd.print("Calibrating...");
  
  for (int i = 0; i < 250; i++) {
    qtra.calibrate();
    delay(10);
  }
}

void read_sensors() {
  for (int i = 0; i < 6; i++) 
    sensors[i] = analogRead(i);
}

void calculate_movement() { 
  pos = qtra.readLine(sensors);
  error_val = pos - 2500;
  
  motor_speed = KP * error_val + KD * (error_val - last_error);
  last_error = error_val;
  
  left_speed = MAX_SPEED + motor_speed;
  right_speed = MAX_SPEED - motor_speed;
  
  if (left_speed < 0) left_speed = 0;
  if (right_speed < 0) right_speed = 0;
}

void drive_motors() {
  analogWrite(pwm_motor_a, right_speed);
  analogWrite(pwm_motor_b, left_speed); 
  
  delay(50);  // Reconsider this delay
}

void display_debug_info() {
  int num_printed = 0;
  
  lcd.set_cursor(0, 0);
  num_printed = lcd.print(left_speed);
  lcd.clear_line(0, num_printed, 8);
  
  num_printed = lcd.print(right_speed);
  lcd.clear_line(0, 8 + num_printed, 16);
  
  lcd.set_cursor(8, 0);
  lcd.clear_line(0, 8 + lcd.print(right_speed), 16);

  for (int i = 0; i < 6; i++) {
    lcd.set_cursor(i, 1);
    lcd.write(byte(sensors[i] / 128 % 8));
  }

  lcd.clear_line(1, 8, 16);
  
  lcd.set_cursor(8, 1);
  lcd.print(error_val);
}
