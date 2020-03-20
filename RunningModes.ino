
// running mode one 
// one to one ratio between inhalation and exhalation
void modeOne(int amplitude){
  myservo.write(0);  // set servo to mid-point
  delay(amplitude/2);
  myservo.write(95);  // set servo to mid-point
  delay(amplitude/2);
}

// running mode two
// one to two ratio between inhalation and exhalation
void modeTwo(int amplitude){
  myservo.write(0);  // set servo to mid-point
  delay(amplitude/3);
  myservo.write(95);  // set servo to mid-point
  delay(amplitude/3*2);
}
