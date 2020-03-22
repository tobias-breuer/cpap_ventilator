#include <Arduino.h>
#include <Servo.h>

/**
 * There are different modes to be operated. Each mode has a different
 * multiplier for the delay before and after operating the servo.
 */
struct mode_t {
  double mult_inhale;
  double mult_exhale;
};

const int mode_sizeof = 2;
const mode_t modes[mode_sizeof] = {
  mode_t{0.5, 0.5},
  mode_t{0.33, 0.66},
};
volatile int mode = 0;

Servo servo;

// Counter for the amount of processed loops. This is necessary to raise a
// warning if some threshold is passed, configured as MAX_LOOP_COUNT.
volatile long int loop_count = 0;


/**
 * Open and close the tube with the mode's specific delays.
 *
 * @param amplitude to determine the time.
 */
void executeMode(const int amplitude) {
  Serial.print("[info] execute mode (");
  Serial.print(modes[mode].mult_inhale);
  Serial.print(", ");
  Serial.print(modes[mode].mult_exhale);
  Serial.print(") with amplitude of ");
  Serial.println(amplitude);

  // Open tube and wait. TODO: verify if this opens
  Serial.println("[info] setting servo to 0");
  servo.write(0);
  delay(amplitude * modes[mode].mult_inhale);

  // Read light sensor's state.
  // TODO: which value is expected? check also if this matches
  const bool lightBefore = digitalRead(PIN_LIGHT);
  Serial.print("[info] read light sensor: ");
  Serial.println(lightBefore);

  // Close tube and wait again.
  Serial.println("[info] setting servo to 95");
  servo.write(95);
  delay(amplitude * modes[mode].mult_exhale);

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
  Serial.begin(9600);
  Serial.println("[info] CPAP ventilator booting up...");

  // init servo
  servo.attach(PIN_SERVO);
  servo.write(0);

  // init pins of LEDs as outputs
  pinMode(PIN_LED_MODE_ONE, OUTPUT);
  pinMode(PIN_LED_MODE_TWO, OUTPUT);
  pinMode(PIN_LED_WARN, OUTPUT);
  pinMode(PIN_SPEAKER, OUTPUT);

  // set output signals to low
  digitalWrite(PIN_LED_MODE_ONE, LOW);
  digitalWrite(PIN_LED_MODE_TWO, LOW);
  digitalWrite(PIN_LED_WARN, LOW);
  digitalWrite(PIN_SPEAKER, LOW);

  // init input pins as input with internal pullups
  pinMode(PIN_SWITCH_MODE, INPUT_PULLUP);
  pinMode(PIN_BUTTON_RESET, INPUT_PULLUP);
  pinMode(PIN_LIGHT, INPUT_PULLUP);
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
  mode = digitalRead(PIN_SWITCH_MODE);
  Serial.println(mode);

  // enable LED 1 if first mode is select; same for LED 2 / second mode
  digitalWrite(PIN_LED_MODE_ONE, mode == 0);
  digitalWrite(PIN_LED_MODE_TWO, mode == 1);

  const int amplitude = readAmplitude();
  Serial.print("[info] read amplitude: ");
  Serial.println(amplitude);

  executeMode(amplitude);

  Serial.print("[info] finished loop iteration ");
  Serial.println(++loop_count);

  // warn if loopcount is greater than MAX_LOOP_COUNT
  if (loop_count >= MAX_LOOP_COUNT) {
    Serial.println("[warn] reached loop count warning");

    digitalWrite(PIN_LED_WARN, HIGH);
  }

}
