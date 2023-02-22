#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 

Adafruit_DCMotor *left = AFMS.getMotor(1);
Adafruit_DCMotor *right = AFMS.getMotor(2);

int green = A0;
int orange = A1;
int blue = A2;
int yellow = A3;

int lines[4];

int mid_val = 500;
int low_speed = 50;
int high_speed = 255;

bool prev_dir_is_right;

int ledPin = 13;
int sensorValue = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Hello world!");

  AFMS.begin();
  //left->setSpeed(high_speed);
  //right->setSpeed(high_speed);
}

void loop() {
  // put your main code here, to run repeatedly:
  lines[0] = (analogRead(orange) > mid_val);
  lines[1] = (analogRead(yellow) > mid_val);
  lines[2] = (analogRead(green) > mid_val);
  lines[3] = (analogRead(blue) > mid_val);
  
  Serial.print(lines[0]);
  Serial.print(lines[1]);
  Serial.print(lines[2]);
  Serial.print(lines[3]);
  Serial.println("");

  if (lines[1] == 1 && lines[2] == 1){ //middle two sensors are on white line
    left->setSpeed(high_speed);
    right->setSpeed(high_speed);
    left->run(BACKWARD);
    right->run(BACKWARD);
  }

  else if ((lines[1] == 0 && lines[2] == 1) || (lines[0] == 1)){ //turn right if middle-left is on black or outer-left is on white
    left->setSpeed(high_speed);
    right->setSpeed(low_speed);
    left->run(BACKWARD);
    right->run(BACKWARD);
    prev_dir_is_right = true; //set to 0 if it has already turned right
  }

  else if ((lines[1] == 1 && lines[2] == 0) || (lines[3] == 1)){ //turn left if middle-right is on black or outer-right is on white
    left->setSpeed(low_speed);
    right->setSpeed(high_speed);
    left->run(BACKWARD);
    right->run(BACKWARD);
    prev_dir_is_right = false; //set to 1 if it has already turned left
  }

  else {
    if (prev_dir_is_right){
      //messed up by turning too much to the right so turn left
      left->setSpeed(low_speed);
      right->setSpeed(high_speed);
      left->run(BACKWARD);
      right->run(BACKWARD);
      prev_dir_is_right = false;
    }
    else {
      //messed up by turning too much to the left so turn right
      left->setSpeed(high_speed);
      right->setSpeed(low_speed);
      left->run(BACKWARD);
      right->run(BACKWARD);
      prev_dir_is_right = true;
    }
  }
  //delay(2000);

}
