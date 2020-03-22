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

// Counter for the amount of processed loops. This is necessary to raise a
// warning if some threshold is passed, configured as MAX_LOOP_COUNT.
volatile long int loop_count = 0;

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
  Serial.println(++loop_count);

  // warn if loopcount is greater than MAX_LOOP_COUNT
  if (loop_count >= MAX_LOOP_COUNT) {
    Serial.println("[warn] reached loop count warning");

    digitalWrite(PIN_LEDWARN, HIGH);
  }

}
