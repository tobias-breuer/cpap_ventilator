#include <Arduino.h>
#include <EEPROM.h>

#include "./cycle_count.h"
#include "./error.h"

/**
 * Variable used for local caching to not query the EEPROM too much.
 * One MUST NOT use this variable. Just use the functions!
 */
long unsigned int cycle_cache = 0;

void cycle_count_reset() {
  cycle_cache = 0;
  EEPROM.put(0, cycle_cache);

  Serial.println("[info] writing cycle counter to EEPROM: 0");
}

long unsigned int cycle_count_read() {
  if (cycle_cache == 0) {
    EEPROM.get(0, cycle_cache);
  }

  return cycle_cache;
}

long unsigned int cycle_count_increment() {
  cycle_cache = cycle_count_read() + 1;

  // only store every CYCLE_MOD_SYNC-nth iteration a value
  if ((cycle_cache > 0) && (cycle_cache % CYCLE_MOD_SYNC == 0)) {
    EEPROM.put(0, cycle_cache);

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
