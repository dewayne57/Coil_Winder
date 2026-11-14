#line 1 "C:\\Users\\Dewayne\\Documents\\GitHub\\Coil_Winder\\.tmp_arduino_build\\CoilWinder\\pattern.c"
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
#include <Arduino.h>
#include <math.h>
#include "pattern.h"
#include "config.h"

// Pattern engine state and configuration variables.
static pattern_t type;   /* Current pattern type */
static float left, right, width, wire_d, pitch;   /* Wire and guide geometry: left/right guide edges, width, wire diameter, and pitch */
static long turns_per_layer;   /* N: wire turns across one full layer (computed from width and wire diameter) */
static long univ_q;   /* Number of quarter turns for universal pattern */
static uint32_t rng;   /* Random number generator state */
static float scr_x;   /* Scramble pattern X position */
static bool mirrored;   /* Mirrored winding direction flag */

/**
 * @brief Enables or disables mirrored winding direction.
 *
 * @param m True to reverse the winding direction.
 */
void pattern_set_mirror(bool m) { mirrored = m; }

// Pattern type display names.
// Pattern type display names. Each entry corresponds to a pattern_t enum value.
static const char *names[PAT_COUNT] = { 
  "Solenoid",       // Solenoid winding pattern
  "Orthocyclic",    // Orthocyclic winding pattern
  "Scramble",       // Scramble winding pattern
  "Universal"       // Universal winding pattern
};   

/**
 * @brief Returns the display name for a pattern type.
 *
 * @param t Pattern to describe.
 * @return String name for the selected pattern.
 */
const char *pattern_name(pattern_t t) { return names[t]; }

/**
 * @brief Produces a signed pseudo-random value for scramble modulation.  This is 
 * used to vary the X position in the scramble pattern.
 *
 * @return Pseudo-random value in the range [-1, 1).
 */
static float rnd_signed(void) {
  rng ^= rng << 13;
  rng ^= rng >> 17;
  rng ^= rng << 5;
  return ((float)(rng & 0xFFFFUL) / 32767.5f) - 1.0f;
}

/**
 * @brief Initializes the pattern engine for a coil span.
 *
 * @param t Pattern type.
 * @param l Left guide edge in microsteps.
 * @param r Right guide edge in microsteps.
 * @param wire_steps Wire pitch in microsteps.
 * @param univ_quarters Number of quarter turns used by the universal pattern.
 */
void pattern_begin(pattern_t t, long l, long r, float wire_steps, int univ_quarters) {
  type = t;
  left = (float)l;
  right = (float)r;
  width = right - left;
  wire_d = (wire_steps > 1.0f) ? wire_steps : 1.0f;
  /* Fit a whole number of turns across the width; pitch is never
   * tighter than the wire.
   */
  turns_per_layer = (long)floorf(width / wire_d) + 1;
  if (turns_per_layer < 1) turns_per_layer = 1;
  pitch = (turns_per_layer > 1) ? width / (float)(turns_per_layer - 1) : 0.0f;
  univ_q = (univ_quarters < 2) ? 2 : univ_quarters;
  rng = micros() | 1UL;
  scr_x = left;
}

/**
 * @brief Returns the number of pattern segments used for one spindle revolution.
 *
 * @return 1 for normal patterns or 4 for universal winding.
 */
long pattern_segs_per_rev(void) { return (type == PAT_UNIVERSAL) ? 4 : 1; }

/**
 * @brief Returns the maximum X travel needed for one spindle revolution.
 *
 * @return Maximum travel in microsteps.
 */
float pattern_max_steps_per_rev(void) {
  switch (type) {
    case PAT_SCRAMBLE:  return SCRAMBLE_MAX_PITCHES * (pitch > 0 ? pitch : wire_d);
    case PAT_UNIVERSAL: return 8.0f * width / (float)univ_q;
    default:            return pitch;
  }
}

/**
 * @brief Returns the X guide position at the start of a pattern segment.
 *
 * @param seg Segment index.
 * @return X position in microsteps.
 */
long pattern_pos(unsigned long seg) {
  float x = left;
  long N = turns_per_layer;

  switch (type) {
    case PAT_SOLENOID:
      if (N > 1) {
        /* Ramp up N turns, then a one-turn dwell at each end so layers
         * stack in line.
         */
        long m = (long)(seg % (unsigned long)(2 * N));
        long j = (m < N) ? m : (2 * N - 1 - m);
        x = left + pitch * (float)j;
      }
      break;

    case PAT_ORTHOCYCLIC:
      if (N > 1) {
        /* Odd layer: N turns left to right. Even layer: N-1 turns, half a pitch
         * inside, right to left. The half-pitch shifts happen during the crossover turns.
         */
        long m = (long)(seg % (unsigned long)(2 * N - 1));
        if (m < N) x = left + pitch * (float)m;
        else x = right - pitch * 0.5f - pitch * (float)(m - N);
      }
      break;

    case PAT_SCRAMBLE:
      if (seg == 0) {
        scr_x = left;
      } else {
        float span = SCRAMBLE_MAX_PITCHES * (pitch > 0 ? pitch : wire_d);
        scr_x += span * rnd_signed();
        if (scr_x > right) scr_x = 2.0f * right - scr_x;
        if (scr_x < left)  scr_x = 2.0f * left - scr_x;
        if (scr_x > right) scr_x = right;
        if (scr_x < left)  scr_x = left;
      }
      x = scr_x;
      break;

    case PAT_UNIVERSAL: {
      /* Triangle wave across the width; period is univ_q quarter-revolutions. */
      float phase = (float)(seg % (unsigned long)univ_q) / (float)univ_q;
      float tri = (phase < 0.5f) ? 2.0f * phase : 2.0f - 2.0f * phase;
      x = left + width * tri;
      break;
    }

    default:
      break;
  }

  if (mirrored) x = left + right - x;
  long xi = lroundf(x);
  if (xi < (long)left)  xi = (long)left;
  if (xi > (long)right) xi = (long)right;
  return xi;
}
