#line 1 "C:\\Users\\Dewayne\\Documents\\GitHub\\Coil_Winder\\.tmp_arduino_build\\CoilWinder\\settings.h"
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
#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Settings structure for the Coil_Winder firmware. This structure is persisted
// to EEPROM and contains all user-configurable parameters for winding.
typedef struct {
  uint8_t  awg_mode;
  int16_t  awg;
  int16_t  wire_um;
  int32_t  turns;
  uint8_t  pattern;
  int16_t  univ_quarters;
  int16_t  y_speed_tenths;
  uint8_t  edges_set;
  int32_t  left_pos;    /* X microsteps from the home position */
  int32_t  right_pos;
  uint8_t  start_right; /* 1 = start winding at the right edge */
} settings_t;


/**
 * @brief Loads persisted settings from EEPROM.
 *
 * @param s Pointer to the settings destination.
 * @return True when a valid settings block was found and loaded.
 */
bool settings_load(settings_t *s);

/**
 * @brief Saves the current settings to EEPROM.
 *
 * @param s Settings structure to persist.
 */
void settings_save(const settings_t *s);

#ifdef __cplusplus
}
#endif

#endif
