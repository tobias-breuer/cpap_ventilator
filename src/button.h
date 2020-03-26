#pragma once

/**
 * A function-like macro to both delay and debounce code to be executed while
 * pressing some button/switch. The button must be pressed for at least the
 * configured time and is locked for the same time after being executed.
 *
 * The code can be used as follows:
 *
 *    debounce_btn(PIN_SWITCH_MODE_A, 250) {
 *      // switch mode to A
 *    }
 *
 * @param BTN address of the button
 * @param DEBOUNCE_MS debouncing/lock time in ms
 */
#define debounce_btn(BTN, DEBOUNCE_MS) \
  static volatile long last_press ##BTN = millis(); \
  if (digitalRead(BTN)) { \
    last_press ##BTN = millis(); \
  } else \
    for (; millis() - last_press ##BTN > DEBOUNCE_MS; last_press ##BTN = millis())
