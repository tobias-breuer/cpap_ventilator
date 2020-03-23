#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#include "./mode.h"
#include "./servo_count.h"

/* Signatures of all following functions */
inline void display_error();
inline void display_status();
inline void read_frequency();
inline void read_mode();
inline void reset_cpap();
void loop();
void setup();
void state_close();
void state_open();

/* Reference to the LCD and servo */
LiquidCrystal_I2C lcd(0x27);
Servo servo;

/* Variable to measure the breaths per minute */
volatile float breaths_per_minute;

/* Variables for the next state:
 * - timestamp (ms) of next execution
 * - previous value of the light barrier
 * - pointer to next state function
 */
volatile unsigned long next_state_time = 0;
volatile bool next_state_light_barrier = false;
void (*next_state_fun)() = state_open;

/* Error types to distinguish between different errors */
enum error_t { err_none, err_servo_count, err_light_barrier };
volatile error_t error = err_none;

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

  // init display
  Wire.begin();
  lcd.begin(16, 2);
}

/**
 * Reset the device's state to a factory mode.
 *
 * This implies a reset of all LEDs and the servo counter in the EEPROM.
 * This function should be called after the replacement of some hardware.
 */
inline void reset_cpap() {
  Serial.println("[warn] entering reset_cpap function, cleaning all data...");

  servo_count_reset();

  error = err_none;

  digitalWrite(PIN_LED_MODE_ONE, LOW);
  digitalWrite(PIN_LED_MODE_TWO, LOW);
  digitalWrite(PIN_LED_WARN, LOW);
  digitalWrite(PIN_SPEAKER, LOW);

  Serial.println("[warn] finished reset_cpap function");

  // "debounce" button
  delay(1000);
}

/**
 * Read the mode from the slide switch and notify potential updates.
 */
inline void read_mode() {
  const int tmp_mode = digitalRead(PIN_SWITCH_MODE);
  if (tmp_mode != mode) {
    mode = tmp_mode;

    Serial.print("[info] update mode to ");
    Serial.println(mode);
  }
}

/**
 * Read the potentiometer's value and convert it to breaths per minute.
 */
inline void read_frequency() {
  // map the values of the poti to the breaths per minute
  // map() cannot be used, since it only works for int values

  const float poti = analogRead(PIN_POTI);
  breaths_per_minute = (poti - POTI_MIN) * (BPM_MAX - BPM_MIN) / (POTI_MAX - POTI_MIN) + BPM_MIN;
}

/**
 * Display the state on an external LCD display.
 */
inline void display_status() {
  lcd.home();
  lcd.print("Breaths/Min: ");
  lcd.print(breaths_per_minute);
  lcd.setCursor(0, 1);
  lcd.print("In-/Exhale: ");
  lcd.print(modes[mode].mult_inhale);
  lcd.print(" ");
  lcd.print(modes[mode].mult_exhale);
}

/**
 * Display the error with different blinkings on the warning LED.
 */
inline void display_error() {
  static volatile unsigned long next_blink_switch = 0;

  switch (error) {
    case err_light_barrier:
      // A defect light barrier is critical and results in a permanent light
      digitalWrite(PIN_LED_WARN, HIGH);
      break;

    case err_servo_count:
      // Exceeding the threshold results in a flashing
      if (next_blink_switch == 0 || (millis() >= next_blink_switch)) {
        next_blink_switch = millis() + WARN_BLINK_MS;
        digitalWrite(PIN_LED_WARN, !digitalRead(PIN_LED_WARN));
      }
      break;

    case err_none:
    default:
      digitalWrite(PIN_LED_WARN, LOW);
  }
}

/**
 * In this state the servo opens the tube.
 */
void state_open() {
  Serial.println("[info] starting opening servo state");

  servo.write(SERVO_OPEN);

  // Calculate the next state execution time, for closing.
  // Subtract the SERVO_CLOSE_LATENCY because of the delay while closing.
  const float offset = 60000.0 / breaths_per_minute * modes[mode].mult_inhale - SERVO_CLOSE_LATENCY;
  next_state_time = millis() + (unsigned long) offset;

  next_state_fun = state_close;

  Serial.print("[info] scheduled closing state to be executed in ");
  Serial.print(offset);
  Serial.println(" ms");
}

/**
 * In this state the servo closes the tube.
 */
void state_close() {
  Serial.println("[info] starting closing servo state");

  servo.write(SERVO_CLOSE);

  const long unsigned int servo_count = servo_count_increment();
  Serial.print("[info] finished servo count iteration number ");
  Serial.println(servo_count);

  // Calculate the next state execution time, for opening.
  const float offset = 60000.0 / breaths_per_minute * modes[mode].mult_exhale;
  next_state_time = millis() + (unsigned long) offset;

  next_state_fun = state_open;

  Serial.print("[info] scheduled opening state to be executed in ");
  Serial.print(offset);
  Serial.println(" ms");
}

/**
 * Main loop, which is called endlessly during execution.
 */
void loop() {
  read_mode();

  // show state on the two state LEDs
  digitalWrite(PIN_LED_MODE_ONE, mode == 0);
  digitalWrite(PIN_LED_MODE_TWO, mode == 1);

  read_frequency();

  display_status();

  // execute next state iff the time threshold was exceeded
  if (next_state_time <= millis()) {
    Serial.println("[info] executing next state function");

    next_state_fun();

    // warn if servo_count is greater than MAX_SERVO_COUNT
    if (servo_count_read() >= MAX_SERVO_COUNT) {
      Serial.println("[warn] reached servo count threshold");
      error = err_servo_count;
    }

    // warn if the light barrier has not changed
    // this warning is more urgent than the previous one and overwrites it
    const bool tmp_light_barrier = digitalRead(PIN_LIGHT);
    if (tmp_light_barrier == next_state_light_barrier) {
      Serial.println("[warn] light barrier has not changed");
      error = err_light_barrier;
    }
    next_state_light_barrier = tmp_light_barrier;
  }

  display_error();

  // reset the device if the reset button is pressed
  if (!digitalRead(PIN_BUTTON_RESET)) {
    reset_cpap();
  }
}
