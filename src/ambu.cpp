#ifdef MODE_AMBU

#include <Arduino.h>

#include "./ambu.h"
#include "./state.h"

inline long ambu_motor_state_read();
void ambu_motor_write_direction(double);
inline double ambu_state_fun(double);

extern volatile float breaths_per_minute;

// Motor's direction as a type and a state variable.
enum direction_t {
  backward = -1,
  stop = 0,
  forward = 1,
};
direction_t direction = forward;

// Variable to determine the time (s) for each breath cycle.
double breath_time_s = 0;

void ambu_loop() {
  // Restrict the bpm value to the ambu bag limits.
  breaths_per_minute = min(max(breaths_per_minute, AMBU_MIN_CYCLE), AMBU_MAX_CYCLE);
  breath_time_s = 60.0 / (breaths_per_minute);

  // Read the current state and adjust the motor speed.
  const double breathing_cycle_s = (double) ambu_motor_state_read() / 1000.0;
  const double speed = ambu_state_fun(breathing_cycle_s);

  ambu_motor_write_direction(speed);
}

void ambu_reset() {
  ambu_setup();
}

void ambu_setup() {
  pinMode(PIN_MOTOR_IN3, OUTPUT);
  pinMode(PIN_MOTOR_IN4, OUTPUT);
  pinMode(PIN_MOTOR_STOP, INPUT_PULLUP);
}

/**
 * Reads the current motor state and determines the time (ms) of the current
 * state. This value is within [0, max_time_for_bpm_mode].
 *
 * Furthermore, this funciton checks if the motor stop button was pressed or a
 * timeout was reached to change the motor's direction.
 *
 * @return time in the current cycle in ms
 */
inline long ambu_motor_state_read() {
  static long last_stop = millis();

  const long now = millis();
  const long breathing_cycle_ms = now - last_stop;

  const double mode_mult = (direction == backward) ? mode_get().mult_exhale : mode_get().mult_inhale;

  const bool is_motor_stop = !digitalRead(PIN_MOTOR_STOP);
  const bool is_motor_debounced = breathing_cycle_ms > MOTOR_STOP_DEBOUNCE_MS;
  const bool is_timeout = breathing_cycle_ms >= (breath_time_s * 1000 * mode_mult);

  // update last_stop iff button is pressed and debounced or timeout is reached
  if ((is_motor_stop && is_motor_debounced) || is_timeout) {
    last_stop = now;
    direction = (direction_t) -direction;

    Serial.print("[info] reached stop after ");
    Serial.print(breathing_cycle_ms);
    Serial.print(" ms, setting direction ");
    Serial.println(direction);

    // increment cycle counter if we switched back to forward mode
    if (direction == forward) {
      const long unsigned int cycle_count = cycle_count_increment();

      Serial.print("[info] finished cycle count iteration number ");
      Serial.println(cycle_count);
    }

    // after switching the mode, last_stop was updated and recalculating the
    // breathing cycle results in zero.
    return 0;
  }

  return breathing_cycle_ms;
}

/**
 * Set the motor's speed as calculated by ambu_state_fun. The direction is
 * determined by the global direction variable.
 *
 * @param speed motor speed
 */
void ambu_motor_write_direction(double speed) {
  switch (direction) {
    case forward:
      analogWrite(PIN_MOTOR_IN3, 0);
      analogWrite(PIN_MOTOR_IN4, speed);
      break;

    case backward:
      analogWrite(PIN_MOTOR_IN3, speed);
      analogWrite(PIN_MOTOR_IN4, 0);
      break;

    case stop:
    default:
      analogWrite(PIN_MOTOR_IN3, 0);
      analogWrite(PIN_MOTOR_IN4, 0);
  }
}

/**
 * Calculate the current motor speed based on the bpm, maximum/minium bpm and
 * the current time position.
 *
 * This function calculates two points in a 2D area between time (x) and motor
 * speed (y). The first point is fixed at x=0 and its y value is determined by
 * the factor of the maximum speed to the current one. The other point is fixed
 * at y=255 and its x value is based on the product of the factor of the minimum
 * time - maximum bpm.
 *
 * A gradient between those two points effects the acceleration and
 * decceleration of the motor. For the fastest bpm value, there is only full
 * speed. However, the slowest bpm value results in longer edges.
 *
 * @param pos_s current time position in seconds; [0, max_time_for_bpm_mode]
 * @return speed as an value of (0, 255]
 */
inline double ambu_state_fun(double pos_s) {
  const double factor = breath_time_s / AMBU_MIN_BREATH_S;

  const double mode_mult = (direction == backward) ? mode_get().mult_exhale : mode_get().mult_inhale;
  const int pt_min_y = (int) (255.0 / factor);
  const int pt_max_x = AMBU_MIN_BREATH_S * factor * mode_mult;

  const double gradient = (255.0 - pt_min_y) / (pt_max_x);

  const double t_s = (direction == backward) ? pt_max_x - pos_s : pos_s;

  return gradient * t_s + pt_min_y;
}

#endif  // MODE_AMBU
