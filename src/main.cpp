#include <Arduino.h>
#include <Servo.h>

Servo myservo;

/* Potentiometer */
int potpin = A0;  // analog Pin
int val;  // variable for value of potentiometer

/* LED pins */
const byte ledPinOne = 13;  // define pin for LED 13
const byte ledPinTwo = 6;  // define pin for LED 6
const byte ledPinWarning = 5;  // define pin for LED 5

const byte acousticPin = 3;  // define pin for acoustic signal

/* Interrupt pins */
const byte interruptPinOne = 1;
const byte interruptPinTwo = 0;
const byte interruptPinThree = 7;


// Define globale volatile variables for the status of the LEDs
volatile byte stateOne = HIGH;
volatile byte stateTwo = LOW;


// to prevent debouncing define debounce time
long debouncing_time = 50;
volatile unsigned long last_micros;
// in debouncing routine we need to use several timestamps
long timestamp = 0;
long timestamptwo = 0;
long timeone = 1000;
long timetwo = 2000;


// variable to define the duration of a breath-cycle
long amplitude;

// variable to count main-loops --> we want warning after defined number of loops
long int loopcount = 0;
long int loopcount_warning = 10;
// if this flag is set by interrupt-routine, LED will blink and signal reset of loopcount
bool blinking_warning_loopcount = false;
// define number of blinking for reset_warning
int ResetBlinkRepititions = 5;



//---------------------------------------------------------------------------------------------------------
// if both interrupts are pushed, this reset loop will reset loopcount variable and set the blinking flag
void reset_loop_warning() {
  // Serial.print("start reset loop");

  // reset global loopcount variable to 0
  loopcount = 0;

  // blink LED to signal resetting
  blinking_warning_loopcount = true;
}


//---------------------------------------------------------------------------------------------------------
// actual interrupt-routine for first interrupt-button
void Interrupt() {
  // Invert status: LED from HIGH to LOW
  stateOne = HIGH;
  stateTwo = LOW;

  // Serial.print("I1 started...");

  // check, if second interrupt pin is also pushed (for reset mode)
  if (digitalRead(interruptPinTwo) == LOW) {
    reset_loop_warning();
    // Serial.print("worked!");
  }
}

// actual interrupt-routine for second interrupt-button
void InterruptTwo() {
  stateTwo = HIGH;
  stateOne = LOW;
}


//---------------------------------------------------------------------------------------------------------
// interrupt-routine for first interrupt-button
// has a debouncing routine and starts the actual interrupt function
void debounceInterrupt() {
  if ((long)(micros() - last_micros) >= debouncing_time*1) {
    Interrupt();
    last_micros = micros();
  }
}


// interrupt-routine for second interrupt-button
// has a debouncing routine and starts the actual interrupt function
void debounceInterruptTwo() {
  if ((long)(micros() - last_micros) >= debouncing_time*1) {
    InterruptTwo();
    last_micros = micros();
  }
}


// routine to led LED blink
void blinkLED_loopcount() {
  for (int i = 0; i < ResetBlinkRepititions; i++) {
    digitalWrite(ledPinWarning, HIGH);
    digitalWrite(acousticPin, HIGH);

    delay(300);
    digitalWrite(ledPinWarning, LOW);
    digitalWrite(acousticPin, LOW);
    delay(300);
  }

  // reset blinking_warning flag to false
  blinking_warning_loopcount = false;

  // Serial.print("blinkLED finished");
}

// running mode one
// one to one ratio between inhalation and exhalation
void modeOne(int amplitude) {
  myservo.write(0);  // set servo to mid-point
  delay(amplitude/2);
  myservo.write(95);  // set servo to mid-point
  delay(amplitude/2);
}

// running mode two
// one to two ratio between inhalation and exhalation
void modeTwo(int amplitude) {
  myservo.write(0);  // set servo to mid-point
  delay(amplitude/3);
  myservo.write(95);  // set servo to mid-point
  delay(amplitude/3*2);
}

void acoustic_warning_loopcount() {
  for (int i = 0; i < ResetBlinkRepititions; i++) {
    digitalWrite(acousticPin, HIGH);
    delay(300);
    digitalWrite(acousticPin, LOW);
    delay(300);
  }
}

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
  // Serial.begin(9600);
  // long timestamp = 0;
}

//---------------------------------------------------------------------------------------------------------
// main loop of the program
void loop() {
  // Schalte die LEDs an
  digitalWrite(ledPinOne, stateOne);
  digitalWrite(ledPinTwo, stateTwo);

  val = analogRead(potpin);  // liest das Potentiometer aus (Wert zwischen 0 und 1023)
  val = map(val, 0, 1023, 8, 30);  // rechnet den Wert in den Wertebereich des Servomotors (zwischen 0 und180)

  // duration of one breath-cycles per minute
  // is adjusted with the potentiometer value between 12 and 26 cycles per minute
  amplitude = 60000/val;


  // routine for mode 1:
  if (stateOne) {
    modeOne(amplitude);
  }

  // routine for mode 2:
  if (stateTwo) {
    modeTwo(amplitude);
  }

  // number of completed loops is increased after every loop
  loopcount++;
  // as a warning the LED is turned off after defined number of loops
  if (loopcount == loopcount_warning) {
    digitalWrite(ledPinWarning, HIGH);
    acoustic_warning_loopcount();
  }

  // let LED blink warning if reset was done
  if (blinking_warning_loopcount == true) {
    blinkLED_loopcount();
    blinking_warning_loopcount = false;
  }
}
