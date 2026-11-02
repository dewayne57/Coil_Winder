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
   is busy. */

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

static axis_cfg_t axes[AXIS_COUNT] = {
  { X_STEP_PIN, X_DIR_PIN, X_DIR_RIGHT_LEVEL, 0, 0, false, false, 1, 0, 0, false, DIR_FORWARD },
  { Y_STEP_PIN, Y_DIR_PIN, Y_DIR_FWD_LEVEL,   0, 0, false, false, 1, 0, 0, false, DIR_FORWARD },
};

static volatile bool    f_on = false;
static volatile int32_t f_len = 1;          /* Y steps per segment */
static volatile int32_t f_pos = 0;          /* Y steps into the current segment */
static volatile int32_t f_frac = 0;
static volatile int32_t f_dx = 0;           /* X steps in the current segment */
static volatile int32_t f_next_dx = 0;
static volatile bool    f_cur_loaded = false;
static volatile bool    f_next_valid = false;
static volatile bool    f_starved = false;
static volatile int32_t f_target = 0;       /* where X should be right now */

static inline void apply_dir(axis_cfg_t *a, dir_t dir) {
  a->dir = dir;
  a->sign = (dir == DIR_FORWARD) ? 1 : -1;
  digitalWrite(a->dir_pin, (dir == DIR_FORWARD) ? a->fwd_level : !a->fwd_level);
}

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

void stepper_set_rate(axis_t axis, unsigned long steps_per_sec) {
  uint32_t inc = (uint32_t)(((uint64_t)steps_per_sec << 32) / STEP_TICK_HZ);
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { axes[axis].inc = inc; }
}

void stepper_set_running(axis_t axis, bool running) {
  axes[axis].running = running;
}

void stepper_stop(axis_t axis) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    axes[axis].running = false;
    axes[axis].goto_active = false;
  }
}

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

bool stepper_move_active(axis_t axis) { return axes[axis].goto_active; }

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

void stepper_follow_stop(void) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { f_on = false; }
}

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

bool stepper_follow_starved(void) { return f_starved; }

bool stepper_follow_settled(void) {
  bool s;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { s = (axes[AXIS_X].pos == f_target) && !axes[AXIS_X].pulse_high; }
  return s;
}

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

void stepper_enable(bool on) {
  digitalWrite(STEPPER_EN_PIN, on ? LOW : HIGH);
}

void stepper_set_dir(axis_t axis, dir_t dir) {
  apply_dir(&axes[axis], dir);
}

dir_t stepper_get_dir(axis_t axis) { return axes[axis].dir; }

long stepper_get_position(axis_t axis) {
  int32_t p;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { p = axes[axis].pos; }
  return p;
}

void stepper_set_position(axis_t axis, long pos) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { axes[axis].pos = pos; }
}
