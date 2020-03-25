#ifdef MODE_CPAP

#include <Arduino.h>
#include <Servo.h>

#include "./cpap.h"
#include "./cycle_count.h"
#include "./error.h"
#include "./mode.h"

inline bool cpap_read_light_barrier();
void cpap_state_close();
void cpap_state_open();

extern volatile error_t error;
extern volatile float breaths_per_minute;

Servo servo;

/* Variables for the next state:
 * - timestamp (ms) of next execution
 * - previous value of the light barrier
 * - pointer to next state function
 */
volatile unsigned long cpap_next_state_time = 0;
volatile bool cpap_next_state_light_barrier = false;
void (*cpap_next_state_fun)() = cpap_state_open;

void cpap_loop() {
  // execute next state iff the time threshold was exceeded
  if (cpap_next_state_time <= millis()) {
    Serial.println("[info] executing next state function");

    cpap_next_state_fun();

    // warn if the light barrier has not changed
    // this warning is more urgent than the previous one and overwrites it
    const bool tmp_light_barrier = cpap_read_light_barrier();

    if (tmp_light_barrier == cpap_next_state_light_barrier) {
      Serial.println("[warn] light barrier has not changed");
      error = err_light_barrier;
    }
    cpap_next_state_light_barrier = tmp_light_barrier;
  }
}

void cpap_reset() {
  /* nop */
}

void cpap_setup() {
  pinMode(PIN_LIGHT, INPUT_PULLUP);

  // init servo
  servo.attach(PIN_SERVO);
  servo.write(0);
}

/**
 * Read out the status of the light barrier
 * Differentiate between different types of light barriers
 */
inline bool cpap_read_light_barrier() {
#if (LIGHT_BARRIER_MODEL == 1)
  return analogRead(PIN_LIGHT) > LIGHT_BARRIER_THRESHOLD;
#elif (LIGHT_BARRIER_MODEL == 2)
  return digitalRead(PIN_LIGHT);
#elif (LIGHT_BARRIER_MODEL == 0)
  static volatile bool last_state = false;

  last_state = !last_state;
  return last_state;
#else
#error LIGHT_BARRIER_MODEL has to be defined, if no light barrier is connected use 'NONE'
#endif
}

/**
 * In this state the servo opens the tube.
 */
void cpap_state_open() {
  Serial.println("[info] starting opening servo state");

  servo.write(SERVO_OPEN);

  // Calculate the next state execution time, for closing.
  // Subtract the SERVO_CLOSE_LATENCY because of the delay while closing.
  const float offset = 60000.0 / breaths_per_minute * modes[mode].mult_inhale - SERVO_CLOSE_LATENCY;
  cpap_next_state_time = millis() + (unsigned long) offset;

  cpap_next_state_fun = cpap_state_close;

  Serial.print("[info] scheduled closing state to be executed in ");
  Serial.print(offset);
  Serial.println(" ms");
}

/**
 * In this state the servo closes the tube.
 */
void cpap_state_close() {
  Serial.println("[info] starting closing servo state");

  servo.write(SERVO_CLOSE);

  Serial.print("[info] finished cycle count iteration number ");
  Serial.println(cycle_count_increment());

  // Calculate the next state execution time, for opening.
  const float offset = 60000.0 / breaths_per_minute * modes[mode].mult_exhale;
  cpap_next_state_time = millis() + (unsigned long) offset;

  cpap_next_state_fun = cpap_state_open;

  Serial.print("[info] scheduled opening state to be executed in ");
  Serial.print(offset);
  Serial.println(" ms");
}

#endif  // MODE_CPAP
