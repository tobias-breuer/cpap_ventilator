#include <Arduino.h>
#include <Servo.h>

#include "./mode.h"
#include "./servo_count.h"

float breaths_per_minute;

Servo servo;

/**
 * Reset the device's state to a factory mode.
 *
 * This implies a reset of all LEDs and the servo counter in the EEPROM.
 * This function should be called after the replacement of some hardware.
 */
void reset_cpap() {
  Serial.println("[warn] entering reset_cpap function, cleaning all data...");

  servo_count_reset();

  digitalWrite(PIN_LED_MODE_ONE, 0);
  digitalWrite(PIN_LED_MODE_TWO, 0);
  digitalWrite(PIN_LED_WARN, 0);
  digitalWrite(PIN_SPEAKER, 0);

  Serial.println("[warn] finished reset_cpap function");
}

/**
 * Open and close the tube with the mode's specific delays.
 */
void executeCycle() {
  Serial.print("[info] execute mode (");
  Serial.print(modes[mode].mult_inhale);
  Serial.print(", ");
  Serial.print(modes[mode].mult_exhale);
  Serial.print(") with breathing frequency of ");
  Serial.println(breaths_per_minute);

  // Open tube and wait. TODO: verify if this opens
  float inhale_delay_ms = 60000 / breaths_per_minute * modes[mode].mult_inhale;
  Serial.print("[info] setting servo to 0 for ");
  Serial.print(inhale_delay_ms);
  Serial.println("ms");
  servo.write(0);
  delay(inhale_delay_ms);

  // Read light sensor's state.
  // TODO: which value is expected? check also if this matches
  const bool lightBefore = digitalRead(PIN_LIGHT);
  Serial.print("[info] read light sensor: ");
  Serial.println(lightBefore);

  // Close tube and wait again.
  float exhale_delay_ms = 60000 / breaths_per_minute * modes[mode].mult_exhale;
  Serial.print("[info] setting servo to 95 for ");
  Serial.print(exhale_delay_ms);
  Serial.println("ms");
  servo.write(95);
  delay(exhale_delay_ms);

  const bool lightAfter = digitalRead(PIN_LIGHT);
  Serial.print("[info] read light sensor: ");
  Serial.println(lightAfter);

  // Alert iff light sensor has not changed afterwards.
  if (lightBefore == lightAfter) {
    Serial.println("[warn] light sensor value has NOT changed!");
    digitalWrite(PIN_LED_WARN, HIGH);
  }
}

/**
 * Setup the CPAP device. This function is run once while booting up.
 */
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
inline float read_frequency() {
  Serial.print("[info] Reading potentiometer: ");

  float poti = analogRead(PIN_POTI);
  Serial.print(poti);
  Serial.print("/");
  Serial.print(POTI_MAX);

  Serial.print(" resulting in ");
  // map the values of the poti to the breaths per minute
  // map() cannot be used, since it only works for int values
  breaths_per_minute = (poti - POTI_MIN) * (BPM_MAX - BPM_MIN) / (POTI_MAX - POTI_MIN) + BPM_MIN;
  Serial.print(breaths_per_minute);
  Serial.println(" breaths per minute.");

  return breaths_per_minute;
}

/**
 * Main loop, which is called endlessly during execution.
 */
void loop() {
  long unsigned int servo_count = servo_count_fetch();
  Serial.print("[info] start servo count iteration ");
  Serial.println(servo_count);

  Serial.print("[info] read mode from switch: ");
  mode = digitalRead(PIN_SWITCH_MODE);
  Serial.println(mode);

  // enable LED 1 if first mode is select; same for LED 2 / second mode
  digitalWrite(PIN_LED_MODE_ONE, mode == 0);
  digitalWrite(PIN_LED_MODE_TWO, mode == 1);

  read_frequency();
  executeCycle();

  // warn if servo_count is greater than MAX_SERVO_COUNT
  if (servo_count >= MAX_SERVO_COUNT) {
    Serial.println("[warn] reached servo count threshold");
    digitalWrite(PIN_LED_WARN, HIGH);
  }

  // reset the device if the reset button is pressed
  // TODO: check if it pressed longer, e.g., for two samples
  if (digitalRead(PIN_BUTTON_RESET)) {
    reset_cpap();
  }
}
