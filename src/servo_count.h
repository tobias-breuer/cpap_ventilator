#pragma once

/* The programm manages a counter for the amount of processed servo interactions
 * to raise a warning if some threshold is passed. This allows the replacement
 * of the servo before it wears out.
 *
 * The threshold is configured as MAX_SERVO_COUNT.
 *
 * To maintain a state, the value is stored on the device's EEPROM.
 */

/**
 * Reset the internal servo counter in the EEPROM.
 */
void servo_count_reset();

/**
 * Read the servo counter from the EEPROM and writes its increment back.
 *
 * @return incremented servo counter
 */
long unsigned int servo_count_fetch();
