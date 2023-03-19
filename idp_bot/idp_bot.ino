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
int PUSH_BUTTON_SWITCH = 3;
int servo_pin = 10;

int lines[4]; // array to store line sensor readings

int low_speed = 0;
int high_speed = 255;

int junction_count = 0;
float last_junc_time = 0;
bool block_picked = false;
bool block_is_blue = false;
bool button_pressed = false;
bool run_once = false;

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
void junction_handler();
void junction(bool left_dir);
void junction_l();
void junction_r();
void grab();
void drop();
void colour_detect();
int median(int lt[]);
int sort_desc(const void *cmp1, const void *cmp2);
void onButtonPress();


void setup() {
  pinMode(blinky_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);
  digitalWrite(blinky_led, LOW);
  myservo.attach(servo_pin);

  Serial.begin(9600);
  Serial.println("Hello world!");
  AFMS.begin();

  // set up push button which will start the robot in a controlled manner
  pinMode(PUSH_BUTTON_SWITCH, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PUSH_BUTTON_SWITCH), onButtonPress, CHANGE);
}


void loop() {
  if (button_pressed) { // button_pressed is set to true by the push button interrupt. This line ensures any code in loop() runs only once the push button has been pressed.
    read_line_sensors();
    
    if (!run_once) { // run the start code only once
      start_l();
    }
    
    line_follow();

    if (block_picked) {
      myservo.write(180); // maintain the servo position when a block is held (in case the servo has moved slightly)
    }

  }
}



void read_line_sensors(){
  lines[0] = digitalRead(red); // extreme left
  lines[1] = digitalRead(blue); // centre-left
  lines[2] = digitalRead(yellow); // centre-right
  lines[3] = digitalRead(green); // extreme right
}

void start(bool left_dir) {
  Serial.println("start code initiated");
  myservo.write(0); // open grabber
  digitalWrite(blinky_led, HIGH);
  straight(high_speed, 2000);
  while (lines[0] == 0 || lines[3] == 0) { // continue moving forward as long as neither the extreme left nor the extreme right sensor sees the white line
    straight();
    read_line_sensors();
  }
  delay(500); // move ahead a little even after finding the white line to ensure better alignment
  if (left_dir) { // turn onto the white line
    turn_ninety_l(1500);
  }
  else {
    turn_ninety_r(1500);
  }
  Serial.println("start code completed");
  run_once = true; // this ensures the start code is run only once eventhough it is in loop()
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
}

void straight(int speed=high_speed, int t=0) {
  left->setSpeed(speed);
  right->setSpeed(speed);
  left->run(FORWARD);
  right->run(FORWARD);
  digitalWrite(blinky_led, HIGH);
  delay(t);
}

void reverse(int speed=high_speed, int t=0) {
  left->setSpeed(speed);
  right->setSpeed(speed);
  left->run(BACKWARD);
  right->run(BACKWARD);
  digitalWrite(blinky_led, HIGH);
  delay(t); 
}

void turn_right(int high=high_speed, int low=low_speed) {
  left->setSpeed(high);
  right->setSpeed(low);
  left->run(FORWARD);
  right->run(FORWARD);
  digitalWrite(blinky_led, HIGH);
}

void turn_left(int high=high_speed, int low=low_speed) {
  left->setSpeed(low);
  right->setSpeed(high);
  left->run(FORWARD);
  right->run(FORWARD);
  digitalWrite(blinky_led, HIGH);
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
  digitalWrite(blinky_led, HIGH);
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
    if (lines[3] == 1) { // robot moves in one direction only and reverses to avoid the ramp so we care only about junctions on the right
      junction_handler();
    }
    else {
      straight();
    }
  }
  else if (lines[1] == 0 && lines[2] == 1) { // turn right if middle-left is on black
    turn_right();
  }
  else if (lines[1] == 1 && lines[2] == 0) { // turn left if middle-right is on black
    turn_left();
  }
}

void junction_handler() {
  Serial.print("Junction count. ");
  Serial.println(junction_count);

  if (millis() - last_junc_time > 1000) {
    junction_count = junction_count + 1;
    last_junc_time = millis();
    // all junctions have a uniqiue index. 1 is where the blocks are picked up from, 2 is the green box, 3 is the starting area and 4 is the red box.
  }

  bool green_box = (junction_count == 2 && block_is_blue && block_picked);
  bool red_box = (junction_count == 4 && !block_is_blue && block_picked);
  if (junction_count == 1 || green_box || red_box) {
    junction_r();
    Serial.print("Turned at junction ");
    Serial.println(junction_count);            
  }

  if (green_box || red_box) {
    junction_count = 0; // reset once the block has been dropped
  }
}

void junction(bool left_dir) {
  int turn = 1850;  // how much it turns into/out of a junction
  int dist = 2000; // how much it moves into/out of a junction
  straight(high_speed, 900); // move straight even after detecting a junction before turning into it
  
  if (left_dir) {
    turn_ninety_r(turn); // turn into the junction
  }
  else {
    turn_ninety_l(turn);
  }
  reverse(high_speed, dist); // reverse into the junction
  stop();
  
  if (block_picked) {
    drop();
  }
  else {
    grab();
  }
  
  straight(high_speed, dist); // leave the junction
  if (left_dir) { // turn out of the junction
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
  delay(1000); // delay to ensure the servo has reached the new position before initiating colour detection
  colour_detect();
}

void drop() {
  myservo.write(0);
  block_picked = false;
  block_is_blue = false;
  digitalWrite(green_led, LOW);
  digitalWrite(red_led, LOW);
}

void colour_detect() {
  int light_readings[10];

  for (int i=0; i<10; i++) {
    light_readings[i] = analogRead(light_sensor);
  }

  int median_light = median(light_readings); // take the median of 10 readings

  if (median_light > 530) { //blue
    Serial.println("Blue");
    block_picked = true;
    block_is_blue = true;
    digitalWrite(green_led, HIGH);
    stop(5000);    
  }
  else if (median_light < 500){ //brown
    Serial.println("Brown");
    block_picked = true;
    block_is_blue = false;
    digitalWrite(red_led, HIGH);
    stop(5000);
  }
  else { //air
    Serial.println("Air");
    block_picked = false;
    block_is_blue = false;
    drop(); // colour detection hasn't worked so we drop the block and pick it up again (hopefully in a slightly different orientation so that colour detection works)
    delay(1000);
    grab();
  }
}


// The below two functions are heavily influenced by this Stack Exchange answer - https://arduino.stackexchange.com/questions/38177/how-to-sort-elements-of-array-in-arduino-code
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

void onButtonPress() {
  button_pressed = true;
}
