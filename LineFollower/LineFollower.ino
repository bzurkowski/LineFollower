#include <Arduino.h>
#include <Sensors.h>
#include <CrystalDisplay.h>


// PID params - slow
#define MAX_SPEED 80
#define BASE_SPEED 50
#define KP 0.0095
#define KI 0.0001
#define KD 3.0

// PID params - medium
//#define MAX_SPEED 90
//#define BASE_SPEED 80
//#define KP 0.022
//#define KI 0
//#define KD 9

// motor status
#define STOPPED 0
#define IN_MOVE 1
#define ROBOT_IN_AIR_THRESHOLD 700


LCD lcd(2, 1, 13, 12, 11, 10);
Sensors qtra((unsigned char[]) { 0, 1, 2, 3, 4, 5 }, 6);

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

// pins
int pwm_motor_a = 5;  // right
int pwm_motor_b = 6;  // left
int in_motor_a1 = 4;
int in_motor_a2 = 3;
int in_motor_b1 = 7;
int in_motor_b2 = 8;
int pwm_led_on = 9;

unsigned int sensors[6];

int position = 0;
int error_val = 0;
int left_speed = 0;
int right_speed = 0;
int proportional = 0;
int last_proportional = 0;
int integral = 0;
int derivative = 0;

int motor_status = STOPPED;


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

  if (motor_status != STOPPED)
    display_debug_info(1);
       
  if (robot_in_air()) {
    stop_motors();
    display_status("STOPPED");
  } else {
    if (motor_status == STOPPED) {
      delay(1500);
      start_motors(); 
    }

    calculate_movement();
    drive_motors();
  }

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
  
  for (int i = 0; i < 16; i++) {
    lcd.set_cursor(i, 1);
    lcd.print("="); 
    delay(100);
  }
  
  delay(1500);
  
  lcd.clear();
  lcd.set_cursor(0, 0);
  lcd.print("Bartek Zurkowski");
  lcd.set_cursor(0, 1);
  lcd.print("Maciek Gawel");

  delay(1800);  
}

void initialize_sensors() {
  digitalWrite(pwm_led_on, 128);
}

void initialize_motors() {
  pinMode(pwm_motor_a, OUTPUT); 
  pinMode(pwm_motor_b, OUTPUT);
  pinMode(in_motor_a1, OUTPUT); 
  pinMode(in_motor_a2, OUTPUT);   
  pinMode(in_motor_b1, OUTPUT);
  pinMode(in_motor_b2, OUTPUT);

  analogWrite(pwm_motor_a, 0);
  analogWrite(pwm_motor_b, 0);

  digitalWrite(in_motor_a2, LOW);
  digitalWrite(in_motor_b1, LOW); 
  
  start_motors();
}

void calibrate_sensors() {
  lcd.clear(); 
  lcd.set_cursor(0, 0);
  lcd.print("Calibrating...");

  int i = 0;
  while (i < 250) {
    qtra.calibrate();
    delay(10);
    i++; 
  }
}

void read_sensors() {
  for (int i = 0; i < 6; i++) 
    sensors[i] = analogRead(i);
}

void calculate_movement() { 
  position = qtra.read_line(sensors);
  
  proportional = position - 2500;
  integral += proportional;
  derivative = proportional - last_proportional;
  last_proportional = proportional;
  
  error_val = int(KP * proportional + KI * integral + KP * derivative);
  
  left_speed = BASE_SPEED + error_val;
  right_speed = BASE_SPEED - error_val;
  
  if (left_speed < 0) left_speed = 0;
  if (right_speed < 0) right_speed = 0;
  
  if (left_speed > MAX_SPEED)
    left_speed = MAX_SPEED;
  if (right_speed > MAX_SPEED)
    right_speed = MAX_SPEED;
}

void drive_motors() {
  analogWrite(pwm_motor_a, right_speed);
  analogWrite(pwm_motor_b, left_speed); 
  
  delay(50);
}

int robot_in_air() {
  for (int i = 0; i < 6; i++)
    if (sensors[i] < ROBOT_IN_AIR_THRESHOLD)
      return 0;
  return 1;
}

void stop_motors() {
  digitalWrite(in_motor_a1, LOW); 
  digitalWrite(in_motor_b2, LOW); 
  motor_status = STOPPED;
}

void start_motors() {
  digitalWrite(in_motor_a1, HIGH); 
  digitalWrite(in_motor_b2, HIGH);    
  motor_status = IN_MOVE;
}

void display_status(char *status) {
  lcd.set_cursor(8, 1);
  clear_line(1, 8 + lcd.print(status), 16);
}

void display_debug_info(int reverted) {
  int numPrinted = 0;
  
  lcd.set_cursor(0, 0);
  numPrinted = lcd.print(right_speed);
  clear_line(0, numPrinted, 8);
  
  numPrinted = lcd.print(left_speed);
  clear_line(0, 8 + numPrinted, 16);
  
  lcd.set_cursor(8, 0);
  clear_line(0, 8 + lcd.print(left_speed), 16);

  for (int i = 0; i < 6; i++) {
    lcd.set_cursor(i, 1);
    lcd.write(byte(sensors[reverted ? (5 - i) : i] / 128 % 8));
  }

  clear_line(1, 8, 16);
  
  lcd.set_cursor(8, 1);
  lcd.print(error_val);
}

void clear_line(int row, int start, int end) {
  for (int i = start; i < end; i++) {
    lcd.set_cursor(i, row);
    lcd.print(" ");
  }   
}


