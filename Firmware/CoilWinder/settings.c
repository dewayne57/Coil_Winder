#include <avr/eeprom.h>
#include "settings.h"

#define SETTINGS_MAGIC   0xC017
<<<<<<< HEAD
#define SETTINGS_VERSION 1
=======
#define SETTINGS_VERSION 2
>>>>>>> ecaf5c5b57e6cfd2ef8d7d9244e5381016d40631
#define SETTINGS_ADDR    0

typedef struct {
  uint16_t   magic;
  uint8_t    version;
  settings_t data;
  uint8_t    checksum;
} record_t;

static uint8_t checksum(const settings_t *s) {
  const uint8_t *p = (const uint8_t *)s;
  uint8_t sum = SETTINGS_VERSION;
  for (uint8_t i = 0; i < sizeof(settings_t); i++) sum = (uint8_t)((sum << 1 | sum >> 7) ^ p[i]);
  return sum;
}

bool settings_load(settings_t *s) {
  record_t r;
  eeprom_read_block(&r, (const void *)SETTINGS_ADDR, sizeof(r));
  if (r.magic != SETTINGS_MAGIC || r.version != SETTINGS_VERSION) return false;
  if (r.checksum != checksum(&r.data)) return false;
  *s = r.data;
  return true;
}

void settings_save(const settings_t *s) {
  record_t r;
  r.magic = SETTINGS_MAGIC;
  r.version = SETTINGS_VERSION;
  r.data = *s;
  r.checksum = checksum(s);
  eeprom_update_block(&r, (void *)SETTINGS_ADDR, sizeof(r));
}
