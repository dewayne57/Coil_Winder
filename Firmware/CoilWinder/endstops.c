#include <Arduino.h>
#include "endstops.h"
#include "config.h"

typedef struct {
  uint8_t pin;
  bool stable;
  bool last_raw;
  unsigned long changed_ms;
} limit_t;

static limit_t x_lim = { X_LIMIT_PIN, false, false, 0 };

static bool limit_read(limit_t *l) {
  bool raw = (digitalRead(l->pin) == LIMIT_TRIGGERED_LEVEL);
  unsigned long now = millis();
  if (raw != l->last_raw) {
    l->last_raw = raw;
    l->changed_ms = now;
  } else if (raw != l->stable && (now - l->changed_ms) >= LIMIT_DEBOUNCE_MS) {
    l->stable = raw;
  }
  return l->stable;
}

void endstops_init(void) {
  pinMode(X_LIMIT_PIN, INPUT_PULLUP);
  x_lim.stable = x_lim.last_raw = (digitalRead(X_LIMIT_PIN) == LIMIT_TRIGGERED_LEVEL);
}

bool endstops_x_triggered(void) { return limit_read(&x_lim); }
