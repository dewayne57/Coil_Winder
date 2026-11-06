#include <Arduino.h>
#include <stdlib.h>
#include "homing.h"
#include "config.h"
#include "stepper.h"
#include "endstops.h"

/* Runs X in `dir` until the limit state equals `target_triggered`, or max_steps is travelled. */
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

static void move_steps(dir_t dir, long steps) {
  stepper_set_dir(AXIS_X, dir);
  delayMicroseconds(5);
  long start = stepper_get_position(AXIS_X);
  stepper_set_running(AXIS_X, true);
  while (labs(stepper_get_position(AXIS_X) - start) < steps) {}
  stepper_set_running(AXIS_X, false);
}

bool homing_home_x(void) {
  bool ok = false;
  stepper_set_rate(AXIS_X, HOME_SEEK_RATE_SPS);

  /* Starting on a switch: it must be X- if moving right releases it. If not, we were on X+,
     so move left until released instead. */
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
