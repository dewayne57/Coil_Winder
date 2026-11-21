/** Coil_Winder Firmware
 * Copyright (c) 2026 Dewayne L. Hafenstein
 *
 * This file is part of the Coil_Winder project.
 *
 * License: Coil_Winder Non-Commercial License
 * Permission is granted to use, copy, modify, and redistribute this
 * firmware for personal, educational, research, and non-commercial
 * purposes only.
 * Commercial use, including sale, manufacturing, sublicensing, or
 * distribution for direct or indirect financial gain, is prohibited
 * without a separate written commercial license from the copyright
 * holder.
 * Any modified or redistributed copies must retain this notice.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE
 * FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF CONNECTION WITH
 * THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <avr/eeprom.h>
#include "settings.h"

#define SETTINGS_MAGIC   0xC017
#define SETTINGS_VERSION 2
#define SETTINGS_ADDR    0

// EEPROM record structure for storing settings with versioning and checksum.
typedef struct {
  uint16_t   magic;
  uint8_t    version;
  settings_t data;
  uint8_t    checksum;
} record_t;

/**
 * @brief Computes the CRC-like checksum that protects the settings record.
 *
 * @param s Settings structure to checksum.
 * @return 8-bit checksum value.
 */
static uint8_t checksum(const settings_t *s) {
  const uint8_t *p = (const uint8_t *)s;
  uint8_t sum = SETTINGS_VERSION;
  for (uint8_t i = 0; i < sizeof(settings_t); i++) sum = (uint8_t)((sum << 1 | sum >> 7) ^ p[i]);
  return sum;
}

/**
 * @brief Loads persisted settings from EEPROM.
 *
 * @param s Pointer to the settings destination.
 * @return True when a valid settings block was found and loaded.
 */
bool settings_load(settings_t *s) {
  record_t r;
  eeprom_read_block(&r, (const void *)SETTINGS_ADDR, sizeof(r));
  if (r.magic != SETTINGS_MAGIC || r.version != SETTINGS_VERSION) return false;
  if (r.checksum != checksum(&r.data)) return false;
  *s = r.data;
  return true;
}

/**
 * @brief Saves the current settings to EEPROM.
 *
 * @param s Settings structure to persist.
 */
void settings_save(const settings_t *s) {
  record_t r;
  r.magic = SETTINGS_MAGIC;
  r.version = SETTINGS_VERSION;
  r.data = *s;
  r.checksum = checksum(s);
  eeprom_update_block(&r, (void *)SETTINGS_ADDR, sizeof(r));
}
