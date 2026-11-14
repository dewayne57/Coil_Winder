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
#include <stdlib.h>
#include "homing.h"
#include "config.h"
#include "stepper.h"
#include "endstops.h"

/**
 * @brief Moves the X axis until the desired limit condition is reached or the distance limit expires.
 *
 * @param dir Direction to move.
 * @param target_triggered Desired endstop condition to stop on.
 * @param max_steps Maximum distance in steps before aborting the move.
 * @return True when the target condition was reached, false on timeout.
 */
static bool move_until(dir_t dir, bool target_triggered, long max_steps) {
  stepper_set_dir(AXIS_X, dir);
  delayMicroseconds(5); /* DIR setup time before steps resume */
  long start = stepper_get_position(AXIS_X);
  bool ok = false;
  stepper_set_running(AXIS_X, true);
  while (labs(stepper_get_position(AXIS_X) - start) < max_steps) {
    if (endstops_x_triggered() == target_triggered) { ok = true; break; }
  }
  stepper_set_running(AXIS_X, false);
  return ok;
}

/**
 * @brief Runs a fixed-length X-axis move without checking the limit state.
 *
 * @param dir Direction of travel.
 * @param steps Number of steps to move.
 */
static void move_steps(dir_t dir, long steps) {
  stepper_set_dir(AXIS_X, dir);
  delayMicroseconds(5);
  long start = stepper_get_position(AXIS_X);
  stepper_set_running(AXIS_X, true);
  while (labs(stepper_get_position(AXIS_X) - start) < steps) {}
  stepper_set_running(AXIS_X, false);
}

/**
 * @brief Homes the X axis to its reference endstop and clears the position.
 *
 * @return True when homing completes successfully, false on a fault.
 */
bool homing_home_x(void) {
  bool ok = false;
  stepper_set_rate(AXIS_X, HOME_SEEK_RATE_SPS);

  /* Starting on a switch: it must be X- if moving right releases it. If
   * not, we were on X+, so move left until released instead.
   */
  if (endstops_x_triggered()) {
    if (!move_until(DIR_FORWARD, false, HOME_BACKOFF_MAX_STEPS) &&
        !move_until(DIR_REVERSE, false, 2 * HOME_BACKOFF_MAX_STEPS)) {
      goto done;
    }
  }

  if (!move_until(DIR_REVERSE, true, HOME_SEEK_MAX_STEPS)) goto done;
  if (!move_until(DIR_FORWARD, false, HOME_BACKOFF_MAX_STEPS)) goto done;
  move_steps(DIR_FORWARD, HOME_CLEARANCE_STEPS);
  stepper_set_position(AXIS_X, 0);
  ok = true;

done:
  stepper_set_rate(AXIS_X, X_STEP_RATE_SPS);
  stepper_set_dir(AXIS_X, DIR_FORWARD);
  return ok;
}
