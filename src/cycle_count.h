#pragma once

/* The programm manages a counter for the amount of processed cycle interactions
 * to raise a warning if some threshold is passed. This allows the replacement
 * of some fragile parts before wearing out.
 *
 * The threshold is configured as MAX_CYCLE_COUNT.
 *
 * To maintain a state, the value is stored on the device's EEPROM.
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
