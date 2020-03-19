  
#define enA 9
#define in1 6
#define in2 7

int potPin = A1;


const byte interruptPinOne = 0;
int rotDirection = 0;

// to prevent debouncing define debounce time
long debouncing_time = 50;
volatile unsigned long last_micros;



//-------------------------------------------------------------------------------------------------------
void debounceInterrupt() {
  if ((long)(micros() - last_micros) >= debouncing_time*1000){
    Interrupt();
    last_micros = micros();
  }
}

// actual interrupt-routine for first interrupt-button
void Interrupt() {
  Serial.print("BUTTON FUCK YEAH    ");

  //turn direction
  bool in1_old = digitalRead(in1);
  digitalWrite(in1, !in1_old);
  digitalWrite(in2, in1_old);
  
}
//-------------------------------------------------------------------------------------------------------


void setup() {
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(interruptPinOne, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPinOne), debounceInterrupt, FALLING);
  // Set initial rotation direction
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  Serial.begin(9600);
}



void loop() {
  int potValue = analogRead(potPin); // Read potentiometer value
  int pwmOutput = map(potValue, 0, 1023, 40 , 255); // Map the potentiometer value from 0 to 255
  analogWrite(enA, potValue); // Send PWM signal to L298N Enable pin
  
}



void loop() {
 potValue = analogRead(potPin);  
 motorValue = map(potValue, 0, 1023, 0, 255);
 analogWrite(motorPin, motorValue);  
 Serial.print("potentiometer = " );     
 Serial.print(potValue);
 Serial.print("t motor = ");
 Serial.println(motorValue);
 delay(2);    
}
