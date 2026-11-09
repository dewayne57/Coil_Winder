#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

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

/* Returns false if nothing valid is stored (the struct is left untouched). */
bool settings_load(settings_t *s);
void settings_save(const settings_t *s);

#ifdef __cplusplus
}
#endif

#endif
