#pragma once

/* The programm manages a counter for the amount of processed cycle interactions
 * to raise a warning if some threshold is passed. This allows the replacement
 * of some fragile parts before wearing out.
 * The threshold is configured as MAX_CYCLE_COUNT.
 *
 * There are different modes to be operated. Each mode has a different
 * multiplier for the delay before and after operating the servo. This may
 * result in a 1:1 ventilation and a 1:2 ventilation.
 *
 * To maintain a state, those values are stored on the device's EEPROM.
 */

/**
 * Reset the internal cycle counter in the EEPROM.
 */
void cycle_count_reset();

/**
 * Read the cycle counter from the EEPROM.
 *
 * @return cycle counter
 */
long unsigned int cycle_count_read();

/**
 * Read the cycle counter from the EEPROM and writes its increment back.
 *
 * @return cycle counter
 */
long unsigned int cycle_count_increment();

/**
 * Check the cycle counter and optionally update the error.
 */
void cycle_warn_display();

/**
 * Simple struct type to store those two multipliers.
 */
struct mode_t {
  double mult_inhale;
  double mult_exhale;
};

/* The _two_ preconfigured ventilation profiles: 1:1 and 1:2 */
const int mode_sizeof = 2;
const mode_t modes[mode_sizeof] = {
  mode_t{0.33, 0.67},
  mode_t{0.5, 0.5},
};

/**
 * Reset the internal mode in the EEPROM.
 */
void mode_reset();

/**
 * Read the mode from the EEPROM.
 *
 * @return mode's index
 */
int mode_read();

/**
 * Read the mode from the EEPROM and return its mode_t.
 *
 * @return mode
 */
mode_t mode_get();

/**
 * Update the mode to an index of modes, e.g., 0 for 1:1.
 *
 * @param new mode's index
 */
void mode_write(int);
