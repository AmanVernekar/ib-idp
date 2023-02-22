#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 

Adafruit_DCMotor *left = AFMS.getMotor(1);
Adafruit_DCMotor *right = AFMS.getMotor(2);

int lineFollower1 = A0;
int lineFollower2 = A1;
int lineFollower3 = A2;
int lineFollower4 = A3;

int lines[4];

int mid_val = 500;
int low_speed = 50;
int high_speed = 200;

int ledPin = 13;
int sensorValue = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Hello world!");

  AFMS.begin();
  left->setSpeed(high_speed);
  right->setSpeed(high_speed);
}

void loop() {
  // put your main code here, to run repeatedly:
  lines[0] = (analogRead(lineFollower1) > mid_val);
  lines[1] = (analogRead(lineFollower2) > mid_val);
  lines[2] = (analogRead(lineFollower3) > mid_val);
  lines[3] = (analogRead(lineFollower4) > mid_val);
  
  Serial.print(lines[0]);
  Serial.print(lines[1]);
  Serial.print(lines[2]);
  Serial.print(lines[3]);
  Serial.println("");

  if (1==1) { // centre is on the white line (0 means black)
    
    if (lines[0] == 0 && lines[3] == 0){
      left->setSpeed(high_speed);
      right->setSpeed(high_speed);
      left->run(BACKWARD);
      right->run(BACKWARD);
    }

    else if (lines[0] == 0 && lines[3] == 1){ //right is on white
      left->setSpeed(high_speed);
      right->setSpeed(low_speed);
      left->run(BACKWARD);
      right->run(BACKWARD);
    }

    else if (lines[0] == 1 && lines[3] == 0){ //left is on white
      left->setSpeed(low_speed);
      right->setSpeed(high_speed);
      left->run(BACKWARD);
      right->run(BACKWARD);
    }
  }
  //delay(2000);

}
