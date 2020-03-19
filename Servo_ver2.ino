#include <Servo.h> 

Servo myservo;

void setup() 
{ 
  myservo.attach(9);
  //myservo.write(90);  // set servo to mid-point
} 

void loop() {
  myservo.write(0);  // set servo to mid-point
  delay(1000);
  myservo.write(95);  // set servo to mid-point
  delay(1000);
  } 
