// comments on lines 91, 101, 242-244, 281 should tell you what to play around with to make it work. use trial and error to make sure it turns back onto the line after a junction


#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include <Servo.h>

Servo myservo;
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_DCMotor *left = AFMS.getMotor(1);
Adafruit_DCMotor *right = AFMS.getMotor(2);

// declare line sensors below
int blue = 8;
int green = 6;
int red = 7;
int yellow = 11;

int blinky_led = 2;
int green_led = 5;
int red_led = 4;
int light_sensor = A1;

int lines[4];

int low_speed = 100;
int high_speed = 255;

int junction_count = 0;
int last_junc_time = 0;
bool block_picked = false;
bool block_is_blue = false;
bool returning_home = false;

void read_line_sensors();
void start(bool left_dir);
void start_l();
void start_r();
void stop(int t=0);
void straight(int speed=high_speed, int t=0);
void reverse(int speed=high_speed, int t=0);
void turn_right(int high=high_speed, int low=low_speed);
void turn_left(int high=high_speed, int low=low_speed);
void turn_ninety(bool left_dir, int t);
void turn_ninety_l(int t);
void turn_ninety_r(int t);
void uturn();
void line_follow();
void junction(bool left_dir);
void junction_l();
void junction_r();
void grab();
void drop();
void colour_detect();
int median(int lt[]);
int sort_desc(const void *cmp1, const void *cmp2);


void setup() {
  pinMode(blinky_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);
  digitalWrite(blinky_led, LOW);
  myservo.attach(10);
  
  Serial.begin(9600);
  Serial.println("Hello world!");

  AFMS.begin();
  read_line_sensors();
  start_l();
}


void loop() {
  read_line_sensors();
  // if (millis() > 4.5*60*1000) {
  //   return_home();
  // }

  bool junction_detected = (lines[1] == 1 && lines[2] == 1 && lines[3] == 1);
  if (junction_detected) {
    if (junction_count == 0) {
      junction_r();      
    }
    else if (junction_count == 2) {
      junction_r();    
      junction_count = -1;  
    }
    if (millis() - last_junc_time > 2000) { // this is to ensure the same junction isn't counted multiple times. change 2000 if 2s seems too little/too much
      junction_count++;
    }
    last_junc_time = millis();
  }
  else {
    line_follow();
  }

  bool inside_box = (lines[0] == 0 && lines[1] == 0 && lines[2] == 0 && lines[3] == 0);
  if (inside_box && (millis() - last_junc_time > 3000)) { // the 3000 is to stop it from dropping the block soon after grabbing it. 
    if (junction_count == 1) {
      stop(100);
      uturn();
      stop(100);
      if (block_is_blue && block_picked) {
        drop();
        start_l();
      }
      else {
        start_r();
      }
    }
  }

  if (!block_picked) {
    digitalWrite(green_led, LOW);
    digitalWrite(red_led, LOW);
  }
  else {
    myservo.write(180);
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

void start(bool left_dir) {
  Serial.println("start code initiated");
  myservo.write(0);
  stop(2000);
  digitalWrite(blinky_led, HIGH);
  straight(high_speed, 2000);
  while (lines[0] == 0 || lines[3] == 0) {
    straight();
    read_line_sensors();
  }
  delay(500);
  if (left_dir) {
    turn_ninety_l(1500);
  }
  else {
    turn_ninety_r(1500);
  }
  Serial.println("start code completed");
}

void start_l() {
  start(true);
}

void start_r() {
  start(false);
}

void stop(int t=0) {
  left->setSpeed(0);
  right->setSpeed(0);
  digitalWrite(blinky_led, LOW);
  delay(t);
  digitalWrite(blinky_led, HIGH);
}

void straight(int speed=high_speed, int t=0) {
  left->setSpeed(speed);
  right->setSpeed(speed);
  left->run(FORWARD);
  right->run(FORWARD);
  delay(t);
}

void reverse(int speed=high_speed, int t=0) {
  left->setSpeed(speed);
  right->setSpeed(speed);
  left->run(BACKWARD);
  right->run(BACKWARD);
  delay(t); 
}

void turn_right(int high=high_speed, int low=low_speed) {
  left->setSpeed(high);
  right->setSpeed(low);
  left->run(FORWARD);
  right->run(FORWARD);
}

void turn_left(int high=high_speed, int low=low_speed) {
  left->setSpeed(low);
  right->setSpeed(high);
  left->run(FORWARD);
  right->run(FORWARD);
}

void turn_ninety(bool left_dir, int t) {
  left->setSpeed(high_speed);
  right->setSpeed(high_speed);
  if (left_dir) {
    left->run(BACKWARD);
    right->run(FORWARD);
  }
  else {
    left->run(FORWARD);
    right->run(BACKWARD);
  }
  delay(t);
}

void turn_ninety_l(int t) {
  turn_ninety(true, t);
}

void turn_ninety_r(int t) {
  turn_ninety(false, t);
}

void uturn() {
  turn_ninety(true, 3000);
}

void line_follow() {
  if (lines[1] == 1 && lines[2] == 1){ //middle two sensors are on white line
    straight();
  }
  else if (lines[1] == 0 && lines[2] == 1) { // || (lines[0] == 1)){ //turn right if middle-left is on black
    turn_right();
  }
  else if (lines[1] == 1 && lines[2] == 0) { // || (lines[3] == 1)){ //turn left if middle-right is on black
    turn_left();
  }
}

void junction(bool left_dir) {
  int turn = 1850;  // change this if you want to change how much it turns into/out of a junction
  int dist = 2000; // change this if you want to change how much it moves into/out of a junction
  straight(high_speed, 900); // use this to change how much it moves before turning into a junction
  
  if (left_dir) {
    turn_ninety_r(turn);
  }
  else {
    turn_ninety_l(turn);
  }
  reverse(high_speed, 2000);
  stop();
  
  if (block_picked) {
    drop();
  }
  else {
    grab();
  }
  
  straight(high_speed, 2000);
  if (left_dir) {
    turn_ninety_r(turn);
  }
  else {
    turn_ninety_l(turn);
  }
}

void junction_l() {
  junction(true);
}

void junction_r() {
  junction(false);
}

void grab() {
  myservo.write(180);
  delay(500); // increase the delay if it still seems to be detecting colour too early
  colour_detect();
}

void drop() {
  myservo.write(0);
  block_picked = false;
  block_is_blue = false;
}

void colour_detect() {
  int light_readings[10];

  for (int i=0; i<10; i++) {
    light_readings[i] = analogRead(light_sensor);
  }

  int median_light = median(light_readings);

  if (median_light > 530) { //blue
    Serial.println("Blue");
    block_picked = true;
    block_is_blue = true;
    digitalWrite(green_led, HIGH);
    stop(5000);    
  }
  else if (median_light < 500) { //air
    Serial.println("Air");
    drop();
    delay(100);
    grab(); // try again
  }
  else { //brown
    Serial.println("Brown");
    block_picked = true;
    block_is_blue = false;
    digitalWrite(red_led, HIGH);
    stop(5000);
  }
}

int median(int lt[]) {
  // Number of items in the array
  int lt_length = sizeof(lt) / sizeof(lt[0]);
  // qsort - last parameter is a function pointer to the sort function
  qsort(lt, lt_length, sizeof(lt[0]), sort_desc);
  // lt is now sorted
  return lt[lt_length/2];
}

// qsort requires you to create a sort function
int sort_desc(const void *cmp1, const void *cmp2) {
  // Need to cast the void * to int *
  int a = *((int *)cmp1);
  int b = *((int *)cmp2);
  // The comparison
  return a > b ? -1 : (a < b ? 1 : 0);
  // A simpler, probably faster way:
  //return b - a;
}
