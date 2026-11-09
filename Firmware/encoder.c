#include <Arduino.h>
#include "encoder.h"
#include "config.h"

/* Quadrature state table: index = (prev_state << 2) | new_state, value = -1/0/+1 */
static const int8_t transition[16] = {
   0, -1,  1,  0,
   1,  0,  0, -1,
  -1,  0,  0,  1,
   0,  1, -1,  0
};

static uint8_t prev_state;
static int accum;

static bool btn_stable = false;   /* pressed = true */
static bool btn_last_raw = false;
static unsigned long btn_changed_ms = 0;

static uint8_t read_state(void) {
  return (digitalRead(ENC_A_PIN) ? 2 : 0) | (digitalRead(ENC_B_PIN) ? 1 : 0);
}

void encoder_init(void) {
  pinMode(ENC_A_PIN, INPUT_PULLUP);
  pinMode(ENC_B_PIN, INPUT_PULLUP);
  pinMode(ENC_SW_PIN, INPUT_PULLUP);
  prev_state = read_state();
  accum = 0;
  btn_stable = btn_last_raw = (digitalRead(ENC_SW_PIN) == LOW);
}

int encoder_poll(void) {
  uint8_t s = read_state();
  if (s != prev_state) {
    accum += transition[(prev_state << 2) | s];
    prev_state = s;
  }
  int detents = 0;
  while (accum >= ENC_TRANSITIONS_PER_DETENT)  { detents++; accum -= ENC_TRANSITIONS_PER_DETENT; }
  while (accum <= -ENC_TRANSITIONS_PER_DETENT) { detents--; accum += ENC_TRANSITIONS_PER_DETENT; }
  return detents;
}

bool encoder_button_pressed(void) {
  bool raw = (digitalRead(ENC_SW_PIN) == LOW);
  unsigned long now = millis();
  if (raw != btn_last_raw) {
    btn_last_raw = raw;
    btn_changed_ms = now;
  } else if (raw != btn_stable && (now - btn_changed_ms) >= ENC_BUTTON_DEBOUNCE_MS) {
    btn_stable = raw;
    return raw;
  }
  return false;
}

static unsigned long press_start_ms = 0;
static bool long_fired = false;

btn_event_t encoder_button_event(void) {
  bool was = btn_stable;
  bool raw = (digitalRead(ENC_SW_PIN) == LOW);
  unsigned long now = millis();
  if (raw != btn_last_raw) {
    btn_last_raw = raw;
    btn_changed_ms = now;
  } else if (raw != btn_stable && (now - btn_changed_ms) >= ENC_BUTTON_DEBOUNCE_MS) {
    btn_stable = raw;
  }

  if (btn_stable && !was) {
    press_start_ms = now;
    long_fired = false;
  } else if (!btn_stable && was) {
    return long_fired ? BTN_NONE : BTN_SHORT;
  } else if (btn_stable && !long_fired && (now - press_start_ms) >= ENC_LONG_PRESS_MS) {
    long_fired = true;
    return BTN_LONG;
  }
  return BTN_NONE;
}
