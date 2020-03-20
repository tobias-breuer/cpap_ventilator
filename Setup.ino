
// define starting setup
void setup() {
  myservo.attach(9);
  
  // define pins of LEDs as outputpins
  pinMode(ledPinOne, OUTPUT);
  pinMode(ledPinTwo, OUTPUT);
  pinMode(ledPinWarning, OUTPUT);
  pinMode(acousticPin, OUTPUT);
  
  digitalWrite(ledPinWarning, LOW);
  digitalWrite(acousticPin, LOW);
 
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
