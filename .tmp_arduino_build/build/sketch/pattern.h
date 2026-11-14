#line 1 "C:\\Users\\Dewayne\\Documents\\GitHub\\Coil_Winder\\.tmp_arduino_build\\CoilWinder\\pattern.h"
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
#ifndef PATTERN_H
#define PATTERN_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  PAT_SOLENOID = 0,    /* aligned layers, one wire pitch per revolution */
  PAT_ORTHOCYCLIC,     /* alternate layers nest in the grooves of the layer below */
  PAT_SCRAMBLE,        /* random lay */
  PAT_UNIVERSAL,       /* continuous wave traverse, fractional revs per cycle */
  PAT_COUNT
} pattern_t;

#include <stdbool.h>

/**
 * @brief Returns the human-readable name of a winding pattern.
 *
 * @param t Pattern to describe.
 * @return Name string for the selected pattern.
 */
const char *pattern_name(pattern_t t);

/* When set, the pattern is mirrored so winding starts at the right edge
 * and proceeds leftward. Call before pattern_begin.
 */
/**
 * @brief Enables or disables mirrored winding direction.
 *
 * @param mirror True to reverse winding start direction.
 */
void pattern_set_mirror(bool mirror);

/**
 * @brief Initializes a winding pattern over the selected coil span.
 *
 * @param t Pattern type.
 * @param left Left coil-edge position in microsteps.
 * @param right Right coil-edge position in microsteps.
 * @param wire_steps Nominal wire pitch in X microsteps.
 * @param univ_quarters Universal pattern quarter-turn count.
 */
void pattern_begin(pattern_t t, long left, long right, float wire_steps, int univ_quarters);

/**
 * @brief Returns the number of pattern segments per spindle revolution.
 *
 * @return Segment count for the active pattern.
 */
long pattern_segs_per_rev(void);

/**
 * @brief Returns the maximum guide travel during one spindle revolution.
 *
 * @return Largest guide excursion in microsteps.
 */
float pattern_max_steps_per_rev(void);

/**
 * @brief Computes the X guide position for a selected pattern segment.
 *
 * @param seg Segment index.
 * @return Guide position in microsteps at the start of that segment.
 */
long pattern_pos(unsigned long seg);

#ifdef __cplusplus
}
#endif

#endif
