#ifdef MODE_AMBU

#include <Arduino.h>

#include "./ambu.h"

inline void ambu_motor_measure_cycle();
long ambu_motor_read_stop();
void ambu_motor_write_direction();

extern volatile float breaths_per_minute;

enum direction_t {
  backward = -1,
  stop = 0,
  forward = 1,
};
direction_t direction = forward;

long breathing_cycle_ms = 0;

void ambu_loop() {
  ambu_motor_read_stop();
  ambu_motor_write_direction();
}

void ambu_reset() {
  /* nop */
}

void ambu_setup() {
  pinMode(PIN_MOTOR_IN3, OUTPUT);
  pinMode(PIN_MOTOR_IN4, OUTPUT);
  pinMode(PIN_MOTOR_STOP, INPUT_PULLUP);

  ambu_motor_measure_cycle();
}

long ambu_motor_read_stop() {
  static long last_stop = millis();

  if (!digitalRead(PIN_MOTOR_STOP)) {
    const long now = millis();
    if (now - last_stop > MOTOR_STOP_DEBOUNCE_MS){
      long breathing_cycle_ms = now - last_stop;

      last_stop = now;
      direction = (direction_t) -direction;

      Serial.print("[info] reached stop pin after ");
      Serial.print(breathing_cycle_ms);
      Serial.print(" ms, setting direction ");
      Serial.println(direction);

      return breathing_cycle_ms;
    }
  }

  return 0;
}

inline void ambu_motor_measure_cycle() {
  Serial.println("[info] measuring motor cycle duration");
  ambu_motor_write_direction();

  while (breathing_cycle_ms == 0) {
    breathing_cycle_ms = ambu_motor_read_stop();
  }
  Serial.print("[info] first cycle took ");
  Serial.print(breathing_cycle_ms);
  Serial.println("ms");
  breathing_cycle_ms = 0;

  while (breathing_cycle_ms == 0) {
    breathing_cycle_ms = ambu_motor_read_stop();
  }
  Serial.print("[info] a breathing cycle takes ");
  Serial.print(breathing_cycle_ms);
  Serial.println("ms");
}

void ambu_motor_write_direction() {
  switch (direction) {
    case forward:
      analogWrite(PIN_MOTOR_IN3, 0);
      analogWrite(PIN_MOTOR_IN4, 255);
      break;
    case backward:
      analogWrite(PIN_MOTOR_IN3, 255);
      analogWrite(PIN_MOTOR_IN4, 0);
      break;
    default:
      analogWrite(PIN_MOTOR_IN3, 0);
      analogWrite(PIN_MOTOR_IN4, 0);
      break;
  }
}

#endif  // MODE_AMBU
