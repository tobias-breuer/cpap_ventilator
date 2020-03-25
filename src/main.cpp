#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "./error.h"
#include "./mode.h"
#include "./cycle_count.h"

#ifdef MODE_AMBU
#include "./ambu.h"
#elif MODE_CPAP
#include "./cpap.h"
#else
#error neither MODE_AMBU nor MODE_CPAP were specified
#endif

/* Signatures of all following functions */
inline void display_error();
inline void display_status();
inline void read_frequency();
inline void read_mode();
inline void reset();
void loop();
void setup();

/* Reference to the LCD */
LiquidCrystal_I2C lcd(0x27);

/* Variable to measure the breaths per minute */
volatile float breaths_per_minute;

/**
 * Setup the CPAP device. This function is run once while booting up.
 */
void setup() {
  Serial.begin(9600);
  Serial.println("[info] ventilator booting up...");

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

  // init display
  Wire.begin();
  lcd.begin(16, 2);

  // mode specific setup
#ifdef MODE_AMBU
  ambu_setup();
#elif MODE_CPAP
  cpap_setup();
#endif
}

/**
 * Reset the device's state to a factory mode.
 *
 * This implies a reset of all LEDs and the cycle counter in the EEPROM.
 * This function should be called after the replacement of some hardware.
 */
inline void reset() {
  Serial.println("[warn] entering reset function, cleaning all data...");

#ifdef MODE_AMBU
  ambu_reset();
#elif MODE_CPAP
  cpap_reset();
#endif

  cycle_count_reset();

  error = err_none;

  digitalWrite(PIN_LED_MODE_ONE, LOW);
  digitalWrite(PIN_LED_MODE_TWO, LOW);
  digitalWrite(PIN_LED_WARN, LOW);
  digitalWrite(PIN_SPEAKER, LOW);

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

    case err_cycle_count:
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
 * Main loop, which is called endlessly during execution.
 */
void loop() {
  read_mode();

  // show state on the two state LEDs
  digitalWrite(PIN_LED_MODE_ONE, mode == 0);
  digitalWrite(PIN_LED_MODE_TWO, mode == 1);

  read_frequency();

  display_status();

  // warn if cycle_count is greater than MAX_CYCLE_COUNT
  cycle_warn_display();

  // A more urgent specific error might overwrite the previous cycle warning.
#ifdef MODE_AMBU
  ambu_loop();
#elif MODE_CPAP
  cpap_loop();
#endif

  display_error();

  // reset the device if the reset button is pressed
  if (!digitalRead(PIN_BUTTON_RESET)) {
    reset();
  }
}
