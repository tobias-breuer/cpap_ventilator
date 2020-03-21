#include <Arduino.h>
#include <Servo.h>

Servo myservo;

// Define globale volatile variables for the status of the LEDs
volatile byte stateOne = HIGH;
volatile byte stateTwo = LOW;

// to prevent debouncing define debounce time
const long debouncing_time = 100000;
volatile unsigned long last_micros = 0;

int interruptCountOne = 0;
int interruptCountTwo = 0;

/* Light barrier */
bool lbBoolOne;
bool lbBoolTwo;

// variable to count main-loops --> we want warning after defined number of loops
long int loopcount = 0;
const long int loopcount_warning = 10;
// if this flag is set by interrupt-routine, LED will blink and signal reset of loopcount
bool blinking_warning_loopcount = false;
// define number of blinking for reset_warning
const int ResetBlinkRepititions = 5;


void acoustic_warning_loopcount() {
  for (int i = 0; i < ResetBlinkRepititions; i++) {
    digitalWrite(PIN_SPEAKER, HIGH);
    delay(300);
    digitalWrite(PIN_SPEAKER, LOW);
    delay(300);
  }
}

void acoustic_warning_lightBarrier() {
  digitalWrite(PIN_SPEAKER, HIGH);
  digitalWrite(PIN_LEDWARN, HIGH);
}

void reset_lightbarrier_warning() {
  digitalWrite(PIN_LEDWARN, LOW);
  digitalWrite(PIN_SPEAKER, LOW);
}

void reset_loopcount_warning() {
  digitalWrite(PIN_SPEAKER, LOW);
  // reset global loopcount variable to 0
  loopcount = 0;

  // blink LED to signal resetting
  blinking_warning_loopcount = true;
}


//---------------------------------------------------------------------------------------------------------
// actual interrupt-routine for first interrupt-button
void Interrupt() {
  // Invert status: LED from HIGH to LOW
  stateOne = ! stateOne;  // invert state
  stateTwo = ! stateTwo;

  // Schalte die LEDs an
  digitalWrite(PIN_LEDONE, stateOne);
  digitalWrite(PIN_LEDTWO, stateTwo);
}

// actual interrupt-routine for second interrupt-button
void InterruptTwo() {
  long time_pressed = millis();

  if ((long)(millis() - time_pressed) < 1000) {
    // Serial.println("Reset Light Barrier");
    reset_lightbarrier_warning();
  }
  if ((long)(millis() - time_pressed) >= 5000){  // Maybe find better times
    // Serial.println("Reset LoopCounter");
    reset_loopcount_warning();
  }
}

void interrupt_commands() {
  // XXX: unreachable code, interruptCount{One,Two} will never be changed
  if (interruptCountOne == 1 && interruptCountTwo == 1) {
    //digitalWrite(acousticPin, LOW);
    digitalWrite(PIN_LEDWARN, LOW);
  }
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
    digitalWrite(PIN_LEDWARN, HIGH);
    digitalWrite(PIN_SPEAKER, HIGH);

    delay(300);
    digitalWrite(PIN_LEDWARN, LOW);
    digitalWrite(PIN_SPEAKER, LOW);
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
  lbBoolOne = digitalRead(PIN_LIGHT);  // read boolean of light barrier
  myservo.write(95);  // set servo to mid-point
  delay(amplitude/2);
  if (lbBoolOne == digitalRead(PIN_LIGHT)) {  // compare if boolean of light barrier
    acoustic_warning_lightBarrier();  // make some noise if the light barrier didn't change
  }
}

// running mode two
// one to two ratio between inhalation and exhalation
void modeTwo(int amplitude) {
  myservo.write(0);  // set servo to mid-point
  delay(amplitude/3);
  lbBoolTwo = digitalRead(PIN_LIGHT);  // read boolean of light barrier
  myservo.write(95);  // set servo to mid-point
  delay(amplitude/3*2);
  if (lbBoolTwo == digitalRead(PIN_LIGHT)) {  // compare if boolean of light barrier
    acoustic_warning_lightBarrier();  // make some noise if the light barrier didn't change
  }
}

// define starting setup
void setup() {
  myservo.attach(PIN_SERVO);

  // define pins of LEDs as outputpins
  pinMode(PIN_LEDONE, OUTPUT);
  pinMode(PIN_LEDTWO, OUTPUT);
  pinMode(PIN_LEDWARN, OUTPUT);
  pinMode(PIN_SPEAKER, OUTPUT);
  pinMode(PIN_LIGHT, INPUT);  // XXX: will be overwritten later

  digitalWrite(PIN_LEDWARN, LOW);
  digitalWrite(PIN_SPEAKER, LOW);

  // Lege den Interruptpin als Inputpin mit Pullupwiderstand fest
  pinMode(PIN_INTERRUPT1, INPUT_PULLUP);
  pinMode(PIN_INTERRUPT2, INPUT_PULLUP);

  pinMode(PIN_LIGHT, INPUT_PULLUP);

  // Lege die ISR 'blink' auf den Interruptpin mit Modus 'CHANGE':
  // "Bei wechselnder Flanke auf dem Interruptpin" --> "Führe die ISR aus"
  attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT1), debounceInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT2), debounceInterruptTwo, FALLING);
  myservo.write(0);
}

/**
 * Read the potentiometer's value, map it between 8 and 30 and convert it to
 * the amplitude.
 *
 * TODO: check and motivate the const values below
 *
 * @return amplitude calculated from the potentiometer
 */
inline int readAmplitude() {
  int rawVal = analogRead(PIN_POTI);
  return 60000/map(rawVal, 0, 1023, 8, 30);
}

//---------------------------------------------------------------------------------------------------------
// main loop of the program
void loop() {
  // Reset Interrupt Counters
  interruptCountOne = 0;
  interruptCountTwo = 0;

  // Schalte die LEDs an
  digitalWrite(PIN_LEDONE, stateOne);
  digitalWrite(PIN_LEDTWO, stateTwo);

  int amplitude = readAmplitude();

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
    digitalWrite(PIN_LEDWARN, HIGH);
    acoustic_warning_loopcount();
  }

  // let LED blink warning if reset was done
  if (blinking_warning_loopcount) {
    blinkLED_loopcount();
    blinking_warning_loopcount = false;
  }

  interrupt_commands();
}
