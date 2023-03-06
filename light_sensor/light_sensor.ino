int light_sensor = A1;
int trigPin = 11;    // Trigger
int echoPin = 12;    // Echo
long duration, cm, inches, cm_prev;

bool block_held = false;
bool block_is_blue = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Hello world!");
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);

  // Convert the time into a distance
  cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  inches = (duration/2) / 74;   // Divide by 74 or multiply by 0.0135
  
  // Serial.print(inches);
  // Serial.print("in, ");
  // Serial.print(cm);
  // Serial.print("cm, ");
  // Serial.print(analogRead(light_sensor));
  // Serial.println();
  
  if (abs(cm - cm_prev) > 1000){
    block_held = !block_held;
    if (!block_held){
      block_is_blue = false;
    }
  }
  cm_prev = cm;
  
  if (analogRead(light_sensor) > 500){
    block_is_blue = true;
  }

  if (block_held){
    if(block_is_blue){
      Serial.println("Blue");   
    }
    else{
      Serial.println("Brown");
    }
  }
  else{
    Serial.println("No block");    
  }

  //delay(1000);
}
//above 500 for blue, else brown 

