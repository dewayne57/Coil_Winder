#include <Arduino.h>
#include <math.h>
#include "wire.h"
#include "config.h"

unsigned int wire_awg_to_um(int awg) {
  /* Standard AWG formula: d(mm) = 0.127 * 92^((36 - n) / 39) */
  float bare_mm = 0.127f * powf(92.0f, (36.0f - (float)awg) / 39.0f);
  float um = bare_mm * 1000.0f * (1.0f + (float)WIRE_INSULATION_PERCENT / 100.0f);
  return (unsigned int)(um + 0.5f);
}

int wire_um_to_awg(unsigned int um) {
  int best = WIRE_AWG_MIN;
  long best_err = 0x7FFFFFFFL;
  for (int n = WIRE_AWG_MIN; n <= WIRE_AWG_MAX; n++) {
    long err = labs((long)wire_awg_to_um(n) - (long)um);
    if (err < best_err) { best_err = err; best = n; }
  }
  return best;
}
