#ifdef MODE_AMBU

#include <Arduino.h>

#include "./ambu.h"
#include "./state.h"

inline long ambu_motor_state_read();
void ambu_motor_write_direction(double);
inline double ambu_state_fun(double);

extern volatile float breaths_per_minute;

enum direction_t {
  backward = -1,
  stop = 0,
  forward = 1,
};
direction_t direction = forward;

double breath_time_s = 0;

void ambu_loop() {
  breaths_per_minute = min(max(breaths_per_minute, AMBU_MIN_CYCLE), AMBU_MAX_CYCLE);
  breath_time_s = 60.0 / (breaths_per_minute);

  const double breathing_cycle_s = (double) ambu_motor_state_read() / 1000.0;
  const double speed = ambu_state_fun(breathing_cycle_s);  // TODO: read correct mode

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

inline long ambu_motor_state_read() {
  static long last_stop = millis();

  const long now = millis();
  const long breathing_cycle_ms = now - last_stop;

  const bool is_motor_stop = !digitalRead(PIN_MOTOR_STOP);
  const bool is_motor_debounced = breathing_cycle_ms > MOTOR_STOP_DEBOUNCE_MS;
  const bool is_timeout = breathing_cycle_ms >= (breath_time_s * 1000);

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
      Serial.print("[info] finished cycle count iteration number ");
      Serial.println(cycle_count_increment());
    }

    // after switching the mode, last_stop was updated and recalculating the
    // breathing cycle results in zero.
    return 0;
  }

  return breathing_cycle_ms;
}

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

inline double ambu_state_fun(double pos_s) {
  // TODO mode_multi

  const double factor = breath_time_s / AMBU_MIN_BREATH_S;

  const int pt_min_y = 255 - (int) (255.0 / factor);
  const int pt_max_x = AMBU_MIN_BREATH_S * factor;

  const double gradient = (255.0 - pt_min_y) / (pt_max_x);

  const double x_s = (direction == backward) ? pt_max_x - pos_s : pos_s;

  return gradient * x_s + pt_min_y;
}

#endif  // MODE_AMBU
