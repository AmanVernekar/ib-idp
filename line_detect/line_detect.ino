#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include <Servo.h>

Servo myservo;
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_DCMotor *left = AFMS.getMotor(1);
Adafruit_DCMotor *right = AFMS.getMotor(2);
//default_dir = FORWARD

int blue = 8;
int green = 9;
int red = 10;
int yellow = 11;

int blinky_led = 2;

int lines[4];

int mid_val = 500;
int low_speed = 100;
int high_speed = 255;

bool prev_dir_is_right;
bool junction_ahead = false;
bool ramp_control = false;
int single_junc_count = 0;
int junc_count = 0;
long last_junc_time = 0;

void read_line_sensors();
void start(bool left_dir=true);
void straight(int speed=high_speed);
void turn_right(int high=high_speed, int low=low_speed);
void turn_left(int high=high_speed, int low=low_speed);
void junction(bool left_dir=true);
void stop(int t);
void turn_ninety(bool left_dir, int t);
void uturn();

void setup() {
  pinMode(blinky_led, OUTPUT);
  digitalWrite(blinky_led, LOW);
  myservo.attach(10);
  
  Serial.begin(9600);
  Serial.println("Hello world!");

  AFMS.begin();
  read_line_sensors();
  start();
}

void loop() {
  if (millis() - last_junc_time > 1){
    single_junc_count = 0;
  }
  read_line_sensors();
  line_follow();
  //delay(1000);

  if (junc_count == 1) {
    junc_count = 0;
    uturn();
  }
}


void read_line_sensors(){
  lines[0] = digitalRead(red);
  lines[1] = digitalRead(blue);
  lines[2] = digitalRead(yellow);
  lines[3] = digitalRead(green);
  
  // Serial.print(lines[0]);
  // Serial.print(lines[1]);
  // Serial.print(lines[2]);
  // Serial.print(lines[3]);
  // Serial.println("");
}

void start(bool left_dir=true) {
  Serial.println("start code initiated");
  //myservo.write(0);
  //delay(1000);
  myservo.write(0);
  stop(2000);
  digitalWrite(blinky_led, HIGH);
  straight();
  delay(2000);
  while (lines[0] == 0 || lines[3] == 0) {
    straight();
    read_line_sensors();
  }
  delay(500);
  turn_ninety(left_dir, 1500);
  Serial.println("start code completed");
}

void stop(int t) {
  left->setSpeed(0);
  right->setSpeed(0);
  digitalWrite(blinky_led, LOW);
  delay(t);
  digitalWrite(blinky_led, HIGH);
}

void straight(int speed=high_speed) {
  left->setSpeed(speed);
  right->setSpeed(speed);
  left->run(FORWARD);
  right->run(FORWARD);
}

void turn_right(int high=high_speed, int low=low_speed) {
  left->setSpeed(high);
  right->setSpeed(low);
  left->run(FORWARD);
  right->run(FORWARD);
  prev_dir_is_right = true;
}

void turn_left(int high=high_speed, int low=low_speed) {
  left->setSpeed(low);
  right->setSpeed(high);
  left->run(FORWARD);
  right->run(FORWARD);
  prev_dir_is_right = false;
}

void junction(bool left_dir=true) {
  Serial.print("RUNNING JUNCTION CODE AT ");
  Serial.println(millis());
  left->setSpeed(high_speed);
  right->setSpeed(high_speed);
  delay(900);
  turn_ninety(true, 1800);
  left->setSpeed(high_speed);
  right->setSpeed(high_speed);
  left->run(BACKWARD);
  right->run(BACKWARD);
  delay(2000);
  myservo.write(180);
  stop(5000);
  straight();
  delay(2000);
  // while (lines[0] == 0 || lines[3] == 0) {
  //   straight();
  //   read_line_sensors();
  // }
  turn_ninety(false, 1800);
  //stop(1000);
}

void line_follow() {
  if (lines[1] == 1 && lines[2] == 1){ //middle two sensors are on white line
    straight();
    if (lines[3] == 1) {
      single_junc_count++;
      last_junc_time = millis();
      if (true) { //(single_junc_count == 100) { //&& millis() > 30000){
        junction();
        single_junc_count = 0;
        junc_count++;
      }
    }
  }
  else if (lines[1] == 0 && lines[2] == 1) { // || (lines[0] == 1)){ //turn right if middle-left is on black
    turn_right();
  }
  else if (lines[1] == 1 && lines[2] == 0) { // || (lines[3] == 1)){ //turn left if middle-right is on black
    turn_left();
  }

  else {
    if (ramp_control){
      if (prev_dir_is_right){
        //messed up by turning too much to the right so turn left
        turn_left();
      }
    else {
        //messed up by turning too much to the left so turn right
        turn_right();
      }
    }
  }  
}

void turn_ninety(bool left_dir, int t) {
  if (left_dir) {
    left->setSpeed(high_speed);
    right->setSpeed(high_speed);
    left->run(BACKWARD);
    right->run(FORWARD);
  }
  else {
    left->setSpeed(high_speed);
    right->setSpeed(high_speed);
    left->run(FORWARD);
    right->run(BACKWARD);
  }
  delay(t);
}

void uturn() {
  turn_ninety(true, 3000);
}
