#include <EEPROM.h>

#include "./servo_count.h"

void servo_count_reset() {
  long unsigned int servo_count = 0;
  EEPROM.put(0, servo_count);
}

long unsigned int servo_count_fetch() {
  long unsigned int servo_count = 0;
  EEPROM.get(0, servo_count);
  EEPROM.put(0, servo_count + 1);
  return servo_count;
}
