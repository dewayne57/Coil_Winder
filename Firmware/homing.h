#ifndef HOMING_H
#define HOMING_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Blocking X homing. Seeks X- (left), backs off, sets X position 0 just clear of the switch.
   Leaves X stopped at rate X_STEP_RATE_SPS, facing right. Returns false on fault. */
bool homing_home_x(void);

#ifdef __cplusplus
}
#endif

#endif
