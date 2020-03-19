#include <Servo.h> 

Servo myservo;


int potpin = A0; // Analog Pin, an dem das Potentiometer angeschlossen ist
int val; // Variable um den Vert des Analogen Pin zwischen zu speichern

// Setze den Pin für die LED auf 13
const byte ledPinOne = 13;
const byte ledPinTwo = 6;
const byte ledPinWarning = 5;

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

long amplitude;

long int loopcount = 0;
long int loopcount_warning = 10;

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

//---------------------------------------------------------------------------------------------------------
void setup() {
  myservo.attach(9);
  
  // Lege den Pin für die LED als Outputpin fest
  pinMode(ledPinOne, OUTPUT);
  pinMode(ledPinTwo, OUTPUT);
  pinMode(ledPinWarning, OUTPUT);
  digitalWrite(ledPinWarning, LOW);
 
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

  val = analogRead(potpin); // liest das Potentiometer aus (Wert zwischen 0 und 1023)
  val = map(val, 0, 1023, 12, 26); // rechnet den Wert in den Wertebereich des Servomotors (zwischen 0 und180)

  amplitude = 60000/val;
  Serial.print(amplitude);
  //routine for mode 1:
  if(stateOne){
    
    myservo.write(0);  // set servo to mid-point
    delay(amplitude/2);
    myservo.write(95);  // set servo to mid-point
    delay(amplitude/2);
  
  }

  //routine for mode 2:
  if(stateTwo){


    myservo.write(0);  // set servo to mid-point
    delay(amplitude/3);
    myservo.write(95);  // set servo to mid-point
    delay(amplitude/3*2);
   
  }

  loopcount++;

  if (loopcount == loopcount_warning){
    digitalWrite(ledPinWarning, HIGH);
  }
  
}
