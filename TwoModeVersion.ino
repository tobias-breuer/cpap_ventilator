#include <Servo.h> 

Servo myservo;

// Setze den Pin für die LED auf 13
const byte ledPinOne = 13;
const byte ledPinTwo = 6;
const byte ledPinThree = 5;

// Setze Interruptpins
const byte interruptPinOne = 1;
const byte interruptPinTwo = 0;
const byte interruptPinThree = 7;

// Definiere eine globale volatile Variable für den Status der LED
volatile byte stateOne = HIGH;
volatile byte stateTwo = LOW;

long debouncing_time = 50;
volatile unsigned long last_micros;

long timestamp = 0;
long timestamptwo = 0;
long timeone = 1000;
long timetwo = 2000;
long timethree = 5000;

//---------------------------------------------------------------------------------------------------------
void setup() {
  myservo.attach(9);
  
  // Lege den Pin für die LED als Outputpin fest
  pinMode(ledPinOne, OUTPUT);
  pinMode(ledPinTwo, OUTPUT);
  
  // Lege den Interruptpin als Inputpin mit Pullupwiderstand fest
  pinMode(interruptPinOne, INPUT_PULLUP);
  pinMode(interruptPinTwo, INPUT_PULLUP);
  
  // Lege die ISR 'blink' auf den Interruptpin mit Modus 'CHANGE':
  // "Bei wechselnder Flanke auf dem Interruptpin" --> "Führe die ISR aus"
  attachInterrupt(digitalPinToInterrupt(interruptPinOne), debounceInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(interruptPinTwo), debounceInterruptTwo, FALLING);
  myservo.write(0);
  Serial.begin(9600);
  long timestamp = 0;
}


//---------------------------------------------------------------------------------------------------------
void loop() {
  
  // Schalte die LEDs an
  digitalWrite(ledPinOne, stateOne);
  digitalWrite(ledPinTwo, stateTwo);

  //routine for mode 1:
  if(stateOne){
    
    myservo.write(0);  // set servo to mid-point
    delay(1000);
    myservo.write(95);  // set servo to mid-point
    delay(1000);
    
  
  }

  //routine for mode 2:
  if(stateTwo){


    myservo.write(0);  // set servo to mid-point
    delay(1000);
    myservo.write(95);  // set servo to mid-point
    delay(2000);

   
  }
  
}
//---------------------------------------------------------------------------------------------------------
void debounceInterrupt() {
  if ((long)(micros() - last_micros) >= debouncing_time*1){
    Interrupt();
    last_micros = micros();
  }
}

void debounceInterruptTwo() {
  if ((long)(micros() - last_micros) >= debouncing_time*1){
    InterruptTwo();
    last_micros = micros();
  }
}


//---------------------------------------------------------------------------------------------------------
void Interrupt() {
  // Invertiere den Status: "Lass die LED blinken von HIGH auf LOW/ an auf aus"
  stateOne = HIGH;
  stateTwo = LOW;
  
}

void InterruptTwo() {
  // Invertiere den Status: "Lass die LED blinken von HIGH auf LOW/ an auf aus"
  stateTwo = HIGH;
  stateOne = LOW;
}
