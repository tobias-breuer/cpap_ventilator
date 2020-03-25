#include <Arduino.h>
#include <EEPROM.h>

#include "./error.h"
#include "./state.h"

const int eeprom_cycle_count = 0x00;
const int eeprom_mode = eeprom_cycle_count + sizeof(long unsigned int);

/**
 * Variable used for local caching to not query the EEPROM too much.
 * One MUST NOT use this variable. Just use the functions!
 */
long unsigned int cycle_cache = 0;
int mode_cache = -1;

void cycle_count_reset() {
  cycle_cache = 0;
  EEPROM.put(eeprom_cycle_count, cycle_cache);

  Serial.println("[info] writing cycle counter to EEPROM: 0");
}

long unsigned int cycle_count_read() {
  if (cycle_cache == 0) {
    EEPROM.get(eeprom_cycle_count, cycle_cache);
  }

  return cycle_cache;
}

long unsigned int cycle_count_increment() {
  cycle_cache = cycle_count_read() + 1;

  // only store every CYCLE_MOD_SYNC-nth iteration a value
  if ((cycle_cache > 0) && (cycle_cache % CYCLE_MOD_SYNC == 0)) {
    EEPROM.put(eeprom_cycle_count, cycle_cache);

    Serial.print("[info] writing cycle counter to EEPROM: ");
    Serial.println(cycle_cache);
  }

  return cycle_cache - 1;
}

void cycle_warn_display() {
  static bool cycle_warn_raised = false;

  if (cycle_warn_raised) {
    return;
  }

  if (cycle_count_read() >= MAX_CYCLE_COUNT) {
    Serial.println("[warn] reached cycle count threshold");
    error = err_cycle_count;

    cycle_warn_raised = true;
  }
}

void mode_reset() {
  mode_write(0);
}

int mode_read() {
  if (mode_cache == -1) {
    EEPROM.get(eeprom_mode, mode_cache);
  }

  return mode_cache;
}

mode_t mode_get() {
  // update mode_cache, if necessary
  mode_read();

  // return a default iff the value is invalid
  if (mode_cache < 0 || mode_cache >= mode_sizeof) {
    return modes[0];
  }

  return modes[mode_cache];
}

void mode_write(int index) {
  mode_cache = index;
  EEPROM.put(eeprom_mode, mode_cache);

  Serial.print("[info] writing mode to EEPROM: ");
  Serial.println(mode_cache);
}
