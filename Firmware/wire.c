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
#include "wire.h"
#include "config.h"

/**
 * @brief Converts an AWG value to the equivalent wire diameter in micrometres.
 *
 * @param awg Wire gauge to convert.
 * @return Wire diameter including insulation allowance in micrometres.
 */
unsigned int wire_awg_to_um(int awg) {
  /* Standard AWG formula: d(mm) = 0.127 * 92^((36 - n) / 39) */
  float bare_mm = 0.127f * powf(92.0f, (36.0f - (float)awg) / 39.0f);
  float um = bare_mm * 1000.0f * (1.0f + (float)WIRE_INSULATION_PERCENT / 100.0f);
  return (unsigned int)(um + 0.5f);
}

/**
 * @brief Finds the nearest AWG value for a target wire diameter.
 *
 * @param um Diameter in micrometres.
 * @return Closest available AWG value.
 */
int wire_um_to_awg(unsigned int um) {
  int best = WIRE_AWG_MIN;
  long best_err = 0x7FFFFFFFL;
  for (int n = WIRE_AWG_MIN; n <= WIRE_AWG_MAX; n++) {
    long err = labs((long)wire_awg_to_um(n) - (long)um);
    if (err < best_err) { best_err = err; best = n; }
  }
  return best;
}
