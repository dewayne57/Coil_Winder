#include <Arduino.h>
#include <math.h>
#include "pattern.h"
#include "config.h"

static pattern_t type;
static float left, right, width, wire_d, pitch;
static long turns_per_layer;   /* N: wire turns across one full layer */
static long univ_q;
static uint32_t rng;
static float scr_x;
static bool mirrored;

void pattern_set_mirror(bool m) { mirrored = m; }

static const char *names[PAT_COUNT] = { "Solenoid", "Orthocyclic", "Scramble", "Universal" };

const char *pattern_name(pattern_t t) { return names[t]; }

/* xorshift32, returns -1.0 .. +1.0 */
static float rnd_signed(void) {
  rng ^= rng << 13;
  rng ^= rng >> 17;
  rng ^= rng << 5;
  return ((float)(rng & 0xFFFFUL) / 32767.5f) - 1.0f;
}

void pattern_begin(pattern_t t, long l, long r, float wire_steps, int univ_quarters) {
  type = t;
  left = (float)l;
  right = (float)r;
  width = right - left;
  wire_d = (wire_steps > 1.0f) ? wire_steps : 1.0f;
  /* Fit a whole number of turns across the width; pitch is never tighter than the wire */
  turns_per_layer = (long)floorf(width / wire_d) + 1;
  if (turns_per_layer < 1) turns_per_layer = 1;
  pitch = (turns_per_layer > 1) ? width / (float)(turns_per_layer - 1) : 0.0f;
  univ_q = (univ_quarters < 2) ? 2 : univ_quarters;
  rng = micros() | 1UL;
  scr_x = left;
}

long pattern_segs_per_rev(void) { return (type == PAT_UNIVERSAL) ? 4 : 1; }

float pattern_max_steps_per_rev(void) {
  switch (type) {
    case PAT_SCRAMBLE:  return SCRAMBLE_MAX_PITCHES * (pitch > 0 ? pitch : wire_d);
    case PAT_UNIVERSAL: return 8.0f * width / (float)univ_q;
    default:            return pitch;
  }
}

long pattern_pos(unsigned long seg) {
  float x = left;
  long N = turns_per_layer;

  switch (type) {
    case PAT_SOLENOID:
      if (N > 1) {
        /* Ramp up N turns, then a one-turn dwell at each end so layers stack in line */
        long m = (long)(seg % (unsigned long)(2 * N));
        long j = (m < N) ? m : (2 * N - 1 - m);
        x = left + pitch * (float)j;
      }
      break;

    case PAT_ORTHOCYCLIC:
      if (N > 1) {
        /* Odd layer: N turns left to right. Even layer: N-1 turns, half a pitch inside,
           right to left. The half-pitch shifts happen during the crossover turns. */
        long m = (long)(seg % (unsigned long)(2 * N - 1));
        if (m < N) x = left + pitch * (float)m;
        else       x = right - pitch * 0.5f - pitch * (float)(m - N);
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
      /* Triangle wave across the width; period is univ_q quarter-revolutions */
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
