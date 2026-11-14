#line 1 "C:\\Users\\Dewayne\\Documents\\GitHub\\Coil_Winder\\.tmp_arduino_build\\CoilWinder\\stepper.c"
/**
 * Coil_Winder Firmware
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
#include <util/atomic.h>
#include "stepper.h"
#include "config.h"

/* Step generation: Timer1 fires at STEP_TICK_HZ. Each axis has a 32-bit phase accumulator
   that advances by `inc` per tick; an overflow emits a step. This gives evenly spaced pulses
   regardless of what the main loop (LCD, encoder, serial) is doing. The pulse goes high on
   one tick and low on the next (~50 us wide).

   Follow mode: X does not use its own accumulator. Every Y step advances a Bresenham
   interpolator through the current segment (dx X-steps spread over seg_len Y-steps) and
   moves the X target; the interrupt then steps X toward that target. The main loop only has
   to queue one segment ahead, so X stays locked to the spindle however long the main loop
   is busy. 
  
   The Bresenham algorithm ensures that X steps are distributed evenly over the Y steps of 
   the current segment, maintaining accurate follow behavior.  This algorithm is used a 
   great deal in CNC and 3D printing applications for precise coordinated motion.
  */

// Axis configuration structure for each stepper motor. Contains pin assignments,
// phase accumulator, stepping state, position, and direction information.
typedef struct {
  uint8_t step_pin;
  uint8_t dir_pin;
  uint8_t fwd_level;
  volatile uint32_t inc;
  volatile uint32_t acc;
  volatile bool running;
  volatile bool pulse_high;
  volatile int8_t sign;
  volatile int32_t pos;
  volatile int32_t target;
  volatile bool goto_active;
  volatile dir_t dir;
} axis_cfg_t;

// Array of axis configurations for each stepper motor. Initialized with pin assignments and default states.
static axis_cfg_t axes[AXIS_COUNT] = {
  { X_STEP_PIN, X_DIR_PIN, X_DIR_RIGHT_LEVEL, 0, 0, false, false, 1, 0, 0, false, DIR_FORWARD },
  { Y_STEP_PIN, Y_DIR_PIN, Y_DIR_FWD_LEVEL,   0, 0, false, false, 1, 0, 0, false, DIR_FORWARD },
};

static volatile bool    f_on = false;       // Follow mode active flag
static volatile int32_t f_len = 1;          // Y steps per segment
static volatile int32_t f_pos = 0;          // Y steps into the current segment
static volatile int32_t f_frac = 0;         // Fractional part of the current segment
static volatile int32_t f_dx = 0;           // X steps in the current segment
static volatile int32_t f_next_dx = 0;      // X steps in the next segment
static volatile bool    f_cur_loaded = false; // Current segment loaded flag
static volatile bool    f_next_valid = false; // Next segment valid flag
static volatile bool    f_starved = false;   // Follow mode starved flag
static volatile int32_t f_target = 0;       // where X should be right now

/**
 * @brief Applies the requested direction to the axis and updates the sign used for stepping.
 *
 * @param a Axis configuration to modify.
 * @param dir Requested direction.
 */
static inline void apply_dir(axis_cfg_t *a, dir_t dir) {
  a->dir = dir;
  a->sign = (dir == DIR_FORWARD) ? 1 : -1;
  digitalWrite(a->dir_pin, (dir == DIR_FORWARD) ? a->fwd_level : !a->fwd_level);
}

/**
 * @brief Advances the Bresenham-style follow interpolator for the X/Y path.
 */
static inline void follow_advance(void) {
  int32_t dx = f_dx;
  int32_t adx = (dx < 0) ? -dx : dx;
  f_frac += adx;
  while (f_frac >= f_len) {
    f_frac -= f_len;
    f_target += (dx < 0) ? -1 : 1;
  }
  if (++f_pos >= f_len) {
    f_pos = 0;
    f_frac = 0;
    if (f_next_valid) {
      f_dx = f_next_dx;
      f_next_valid = false;
    } else {
      f_dx = 0;
      f_starved = true;
    }
  }
}

/**
 * @brief Generates step pulses for all active axes and updates follow-mode tracking.
 */
ISR(TIMER1_COMPA_vect) {
  bool lowered[AXIS_COUNT];
  for (uint8_t i = 0; i < AXIS_COUNT; i++) {
    axis_cfg_t *a = &axes[i];
    lowered[i] = a->pulse_high;
    if (a->pulse_high) {
      digitalWrite(a->step_pin, LOW);
      a->pulse_high = false;
    }
  }

  for (uint8_t i = 0; i < AXIS_COUNT; i++) {
    axis_cfg_t *a = &axes[i];
    if (!a->running) continue;
    uint32_t prev = a->acc;
    a->acc = prev + a->inc;
    if (a->acc < prev) {  /* accumulator wrapped: time for a step */
      digitalWrite(a->step_pin, HIGH);
      a->pulse_high = true;
      a->pos += a->sign;
      if (a->goto_active && a->pos == a->target) {
        a->running = false;
        a->goto_active = false;
      }
      if (i == AXIS_Y && f_on) follow_advance();
    }
  }

  if (f_on && !lowered[AXIS_X]) {
    axis_cfg_t *x = &axes[AXIS_X];
    int32_t diff = f_target - x->pos;
    if (diff != 0) {
      dir_t want = (diff > 0) ? DIR_FORWARD : DIR_REVERSE;
      if (x->dir != want) {
        apply_dir(x, want);   /* step on a later tick so DIR is settled */
      } else {
        digitalWrite(x->step_pin, HIGH);
        x->pulse_high = true;
        x->pos += x->sign;
      }
    }
  }
}

/**
 * @brief Sets the step rate for a selected axis.
 *
 * @param axis Axis to update.
 * @param steps_per_sec Desired step rate in steps per second.
 */
void stepper_set_rate(axis_t axis, unsigned long steps_per_sec) {
  uint32_t inc = (uint32_t)(((uint64_t)steps_per_sec << 32) / STEP_TICK_HZ);
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { axes[axis].inc = inc; }
}

/**
 * @brief Enables or disables stepping for a given axis.
 *
 * @param axis Axis to update.
 * @param running True to enable pulses, false to hold the axis.
 */
void stepper_set_running(axis_t axis, bool running) {
  axes[axis].running = running;
}

/**
 * @brief Stops the selected axis and cancels any in-flight move.
 *
 * @param axis Axis to stop.
 */
void stepper_stop(axis_t axis) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    axes[axis].running = false;
    axes[axis].goto_active = false;
  }
}

/**
 * @brief Starts a non-blocking move to a target position.
 *
 * @param axis Axis to move.
 * @param target Absolute target in microsteps.
 */
void stepper_move_to(axis_t axis, long target) {
  long cur = stepper_get_position(axis);
  stepper_stop(axis);
  if (target == cur) return;
  stepper_set_dir(axis, target > cur ? DIR_FORWARD : DIR_REVERSE);
  delayMicroseconds(5); /* DIR setup time before steps resume */
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    axes[axis].target = target;
    axes[axis].goto_active = true;
    axes[axis].running = true;
  }
}

/**
 * @brief Reports whether the selected axis is still moving to an absolute target.
 *
 * @param axis Axis to query.
 * @return True while the move is active.
 */
bool stepper_move_active(axis_t axis) { return axes[axis].goto_active; }

/**
 * @brief Starts follow mode using a fixed Y segment length.
 *
 * @param seg_len_y_steps Number of Y steps in each segment.
 */
void stepper_follow_start(long seg_len_y_steps) {
  stepper_stop(AXIS_X);
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    f_len = seg_len_y_steps;
    f_pos = 0;
    f_frac = 0;
    f_dx = 0;
    f_next_dx = 0;
    f_cur_loaded = false;
    f_next_valid = false;
    f_starved = false;
    f_target = axes[AXIS_X].pos;
    f_on = true;
  }
}

/**
 * @brief Disables follow mode and leaves the interpolator idle.
 */
void stepper_follow_stop(void) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { f_on = false; }
}

/**
 * @brief Queues a follow-mode X delta for the active segment.
 *
 * @param dx X delta in microsteps for the queued segment.
 * @return True if the queue accepted the segment.
 */
bool stepper_follow_queue(long dx) {
  bool ok = false;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if (!f_cur_loaded) {
      f_dx = dx;
      f_cur_loaded = true;
      ok = true;
    } else if (!f_next_valid) {
      f_next_dx = dx;
      f_next_valid = true;
      ok = true;
    }
  }
  return ok;
}

/**
 * @brief Reports whether follow mode has fallen behind due to missing queued segments.
 *
 * @return True if the interpolator is starved.
 */
bool stepper_follow_starved(void) { return f_starved; }

/**
 * @brief Returns whether the X axis has caught up with the queued follow target.
 *
 * @return True when the X axis has settled against the target.
 */
bool stepper_follow_settled(void) {
  bool s;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { s = (axes[AXIS_X].pos == f_target) && !axes[AXIS_X].pulse_high; }
  return s;
}

/**
 * @brief Initializes all axis outputs, default directions, and the Timer1 step generator.
 */
void stepper_init(void) {
  pinMode(STEPPER_EN_PIN, OUTPUT);
  for (uint8_t i = 0; i < AXIS_COUNT; i++) {
    pinMode(axes[i].step_pin, OUTPUT);
    pinMode(axes[i].dir_pin, OUTPUT);
    digitalWrite(axes[i].step_pin, LOW);
    stepper_set_dir((axis_t)i, DIR_FORWARD);
  }
  stepper_set_rate(AXIS_X, X_STEP_RATE_SPS);
  stepper_enable(false);

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    TCCR1A = 0;
    TCCR1B = _BV(WGM12) | _BV(CS10);   /* CTC mode, no prescaler */
    OCR1A = (F_CPU / STEP_TICK_HZ) - 1;
    TCNT1 = 0;
    TIMSK1 = _BV(OCIE1A);
  }
}

/**
 * @brief Enables or disables the shared stepper driver stage.
 *
 * @param on True to enable the driver, false to disable it.
 */
void stepper_enable(bool on) {
  digitalWrite(STEPPER_EN_PIN, on ? LOW : HIGH);
}

/**
 * @brief Sets the direction pin for the selected axis.
 *
 * @param axis Axis to update.
 * @param dir Desired direction.
 */
void stepper_set_dir(axis_t axis, dir_t dir) {
  apply_dir(&axes[axis], dir);
}

/**
 * @brief Returns the configured direction for the selected axis.
 *
 * @param axis Axis to query.
 * @return Current direction state.
 */
dir_t stepper_get_dir(axis_t axis) { return axes[axis].dir; }

/**
 * @brief Returns the stored absolute position of one axis.
 *
 * @param axis Axis to query.
 * @return Axis position in microsteps.
 */
long stepper_get_position(axis_t axis) {
  int32_t p;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { p = axes[axis].pos; }
  return p;
}

/**
 * @brief Writes the stored position for an axis.
 *
 * @param axis Axis to update.
 * @param pos New absolute position in microsteps.
 */
void stepper_set_position(axis_t axis, long pos) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { axes[axis].pos = pos; }
}
