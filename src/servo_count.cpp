#include <Arduino.h>
#include <EEPROM.h>

#include "./servo_count.h"

/**
 * Variable used for local caching to not query the EEPROM too much.
 * One MUST NOT use this variable. Just use the functions!
 */
long unsigned int servo_cache = 0;

void servo_count_reset() {
  servo_cache = 0;
  EEPROM.put(0, servo_cache);

  Serial.println("[info] writing servo counter to EEPROM: 0");
}

long unsigned int servo_count_read() {
  if (servo_cache == 0) {
    EEPROM.get(0, servo_cache);
  }

  return servo_cache;
}

long unsigned int servo_count_increment() {
  servo_cache = servo_count_read() + 1;

  // only store every SERVO_MOD_SYNC-nth iteration a value
  if ((servo_cache > 0) && (servo_cache % SERVO_MOD_SYNC == 0)) {
    EEPROM.put(0, servo_cache);

    Serial.print("[info] writing servo counter to EEPROM: ");
    Serial.println(servo_cache);
  }

  return servo_cache - 1;
}
