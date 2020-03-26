#pragma once

/**
 * A function-like macro to debounce code to be executed after the press of some
 * button/switch.
 *
 * The code can be used as follows:
 *
 *    debounce_btn(PIN_SWITCH_MODE_A, 250) {
 *      // switch mode to A
 *    }
 *
 * @param BTN address of the button
 * @param DEBOUNCE_MS debouncing time in ms
 */
#define debounce_btn(BTN, DEBOUNCE_MS) \
  static volatile long last_press ##BTN = 0; \
  for ( \
    ; \
    !digitalRead(BTN) && (millis() - last_press ##BTN > DEBOUNCE_MS); \
    last_press ##BTN = millis())
