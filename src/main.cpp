#include <Arduino.h>
#include <Servo.h>

/**
 * There are different modes to be operated. Each mode has a different
 * multiplier for the delay before and after operating the servo.
 */
struct mode_t {
  double mult_before;
  double mult_after;
};

const int mode_sizeof = 2;
const mode_t modes[mode_sizeof] = {
  mode_t{0.5, 0.5},
  mode_t{0.33, 0.66},
};
volatile int mode_sel = 0;

Servo myservo;

// to prevent debouncing define debounce time
const long debouncing_time = 100000;
volatile unsigned long last_micros = 0;

// TODO what is this exactly for?
int interruptCountOne = 0;
int interruptCountTwo = 0;

// TODO why do we want those warnings? will the warning be reset later?
// variable to count main-loops --> we want warning after defined number of loops
long int loopcount = 0;
const long int loopcount_warning = 10;

// if this flag is set by interrupt-routine, LED will blink and signal reset of loopcount
bool blinking_warning_loopcount = false;
// define number of blinking for reset_warning
const int ResetBlinkRepititions = 5;

/**
 * Read PIN_SWITCH1 and sets mode_sel to its value.
 * This requires that sizeof(modes) >= 2 or, even better, == 2.
 *
 * @return new value of mode_sel
 */
int mode_sel_read_switch() {
  const bool switchVal = digitalRead(PIN_SWITCH1);
  mode_sel = (int) switchVal;
  return mode_sel;
}

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


// actual interrupt-routine for second interrupt-button
void InterruptTwo() {
  long time_pressed = millis();

  Serial.print("[warn] interrupt two has fired at ");
  Serial.println(time_pressed);

  // TODO `millis() - time_pressed` is _always_ near 0; the following cannot work
  if ((long)(millis() - time_pressed) < 1000) {
    // Serial.println("Reset Light Barrier");
    reset_lightbarrier_warning();
  }
  // TODO `millis` has advanced, if the first if has been fulfilled; those are heavy side effects!
  if ((long)(millis() - time_pressed) >= 5000) {  // Maybe find better times
    // Serial.println("Reset LoopCounter");
    reset_loopcount_warning();
  }
}

void interrupt_commands() {
  // TODO: unreachable code, interruptCount{One,Two} will never be changed
  if (interruptCountOne == 1 && interruptCountTwo == 1) {
    //digitalWrite(acousticPin, LOW);
    digitalWrite(PIN_LEDWARN, LOW);
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

/**
 * Open and close the tube with the mode's specific delays.
 *
 * @param amplitude to determine the time.
 */
void executeMode(const int amplitude) {
  Serial.print("[info] execute mode (");
  Serial.print(modes[mode_sel].mult_before);
  Serial.print(",");
  Serial.print(modes[mode_sel].mult_after);
  Serial.print(") with amplitude of ");
  Serial.println(amplitude);

  // Open tube and wait. TODO: verify if this opens
  Serial.println("[info] setting servo to 0");
  myservo.write(0);
  delay(amplitude * modes[mode_sel].mult_before);

  // Read light sensor's state.
  // TODO: which value is expected? check also if this matches
  const bool lightBefore = digitalRead(PIN_LIGHT);
  Serial.print("[info] read light sensor: ");
  Serial.println(lightBefore);

  // Close tube and wait again.
  Serial.println("[info] setting servo to 95");
  myservo.write(95);
  delay(amplitude * modes[mode_sel].mult_after);

  const bool lightAfter = digitalRead(PIN_LIGHT);
  Serial.print("[info] read light sensor: ");
  Serial.println(lightAfter);
  //
  // Alert iff light sensor has not changed afterwards.
  if (lightBefore == lightAfter) {
    // TODO: is it possible to reset warnings?
    Serial.println("[warn] light sensor value has NOT changed!");
    acoustic_warning_lightBarrier();
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
  pinMode(PIN_LIGHT, INPUT);  // TODO: will be overwritten later

  digitalWrite(PIN_LEDWARN, LOW);
  digitalWrite(PIN_SPEAKER, LOW);

  // Lege den Interruptpin als Inputpin mit Pullupwiderstand fest
  pinMode(PIN_SWITCH1, INPUT_PULLUP);
  pinMode(PIN_SWITCH2, INPUT_PULLUP);

  pinMode(PIN_LIGHT, INPUT_PULLUP);

  // Lege die ISR 'blink' auf den Interruptpin mit Modus 'CHANGE':
  // "Bei wechselnder Flanke auf dem Interruptpin" --> "Führe die ISR aus"
  attachInterrupt(digitalPinToInterrupt(PIN_SWITCH2), debounceInterruptTwo, FALLING);

  myservo.write(0);

  Serial.begin(9600);
  Serial.println("[info] CPAP booting up...");
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
  // TODO: can this be removed?
  // Reset Interrupt Counters
  interruptCountOne = 0;
  interruptCountTwo = 0;

  Serial.print("[info] read mode from switch: ");
  Serial.println(mode_sel_read_switch());

  // enable LED 1 if first mode is select; same for LED 2 / second mode
  digitalWrite(PIN_LEDONE, mode_sel == 0);
  digitalWrite(PIN_LEDTWO, mode_sel == 1);

  const int amplitude = readAmplitude();
  Serial.print("[info] read amplitude: ");
  Serial.println(amplitude);

  executeMode(amplitude);

  Serial.print("[info] finished loop iteration ");
  Serial.println(++loopcount);

  // as a warning the LED is turned off after defined number of loops
  // TODO: only == or >=, will the warning be removed?
  if (loopcount == loopcount_warning) {
    Serial.println("[warn] reached loop count warning");

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
