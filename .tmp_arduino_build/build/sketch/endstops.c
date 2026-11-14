#line 1 "C:\\Users\\Dewayne\\Documents\\GitHub\\Coil_Winder\\.tmp_arduino_build\\CoilWinder\\endstops.c"
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
#include "endstops.h"
#include "config.h"

// Structure to hold the state of a single limit switch, including debounce tracking.
typedef struct {
  uint8_t pin;
  bool stable;
  bool last_raw;
  unsigned long changed_ms;
} limit_t;

static limit_t x_lim = { X_LIMIT_PIN, false, false, 0 };
static limit_t y_lim = { Y_LIMIT_PIN, false, false, 0 };
static limit_t z_lim = { Z_LIMIT_PIN, false, false, 0 };

/**
 * @brief Debounces a single endstop input and returns its stable state.
 *
 * @param l Limit state structure to evaluate.
 * @return True when the limit is triggered and stable.
 * @note This function implements software debounce for the limit switch.
 * @note The function returns the stable state only after the debounce period has elapsed.
 * @note The debounce period is defined by LIMIT_DEBOUNCE_MS.
 * @note The function should be called regularly to maintain accurate debounce tracking.
 */
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

/**
 * @brief Initializes the X, Y, and Z limit inputs with pull-ups and stable state.
 * @note This function sets up the hardware pins and initializes the debounce state for each limit switch.
 * @note The function should be called once during system initialization.
 * @note The initial stable state is determined by reading the hardware pins at initialization.
 * @note The function configures the pins as INPUT_PULLUP to match the expected hardware setup.
 * @note The function should be called before any calls to endstops_x_triggered, endstops_y_triggered, or endstops_z_triggered.
 */
void endstops_init(void) {
  pinMode(X_LIMIT_PIN, INPUT_PULLUP);
  pinMode(Y_LIMIT_PIN, INPUT_PULLUP);
  pinMode(Z_LIMIT_PIN, INPUT_PULLUP);
  x_lim.stable = x_lim.last_raw = (digitalRead(X_LIMIT_PIN) == LIMIT_TRIGGERED_LEVEL);
  y_lim.stable = y_lim.last_raw = (digitalRead(Y_LIMIT_PIN) == LIMIT_TRIGGERED_LEVEL);
  z_lim.stable = z_lim.last_raw = (digitalRead(Z_LIMIT_PIN) == LIMIT_TRIGGERED_LEVEL);
}

/**
 * @brief Returns the debounced state of the X axis limit.
 *
 * @return True if the X limit is currently triggered.
 * @note The returned state is debounced and reflects the stable state of the limit switch.
 */
bool endstops_x_triggered(void) { return limit_read(&x_lim); }

/**
 * @brief Returns the debounced state of the Y axis wire-out sensor.
 *
 * @return True if the Y sensor is triggered.
 * @note The returned state is debounced and reflects the stable state of the limit switch.
 */
bool endstops_y_triggered(void) { return limit_read(&y_lim); }

/**
 * @brief Returns the debounced state of the Z axis alignment-bar sensor.
 *
 * @return True if the Z sensor indicates the bar is not retracted.
 * @note The returned state is debounced and reflects the stable state of the limit switch.
 */
bool endstops_z_triggered(void) { return limit_read(&z_lim); }

/**
 * @brief Reports whether the alignment bar is fully retracted.
 *
 * @return True when the Z sensor indicates a safe retracted state.
 * @note The returned state is debounced and reflects the stable state of the limit switch.
 */
bool endstops_alignment_bar_retracted(void) {
  return !endstops_z_triggered();
}
