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
#include "encoder.h"
#include "config.h"

// The quadrature state transition table maps the previous and current states
// of the encoder to a movement value: -1 for counter-clockwise, +1 for clockwise, 
// 0 for no movement.  
static const int8_t transition[16] = {
   0, -1,  1,  0,
   1,  0,  0, -1,
  -1,  0,  0,  1,
   0,  1, -1,  0
};

static uint8_t prev_state;  // Previous quadrature state of the encoder
static int accum;  // Accumulated quadrature transitions

static bool btn_stable = false;   /* pressed = true */
static bool btn_last_raw = false; // Last raw button state
static unsigned long btn_changed_ms = 0; // Timestamp of last raw state change

/**
 * @brief Reads the current quadrature encoder state from the A/B pins.
 *
 * @return State value encoding the current A and B levels.
 * The returned value encodes the A and B pin levels as follows:
 *   0 = A=0, B=0
 *   1 = A=0, B=1
 *   2 = A=1, B=0
 *   3 = A=1, B=1
 * The A pin corresponds to PC5 and the B pin corresponds to PC6.
 */
static uint8_t read_state(void) {
  uint8_t port = PINC;
  return ((port & _BV(PC5)) ? 2 : 0) | ((port & _BV(PC6)) ? 1 : 0);
}

/**
 * @brief Initializes the encoder inputs and button debounce state.
 * Configures the A/B pins as inputs with pull-ups and initializes the button state.
 * This function should be called once during system initialization.
 * @note The switch pin is PC7 with an internal pull-up.
 * The A/B pins are PC5 and PC6 with internal pull-ups enabled.
 * @note The encoder uses quadrature encoding to detect rotation direction and detents.
 * @note The button supports short and long press detection with debounce.
 * @note The encoder should be polled regularly to detect rotation and button events.
 * @note The encoder button is active-low and uses PC7 with an internal pull-up.
 * 
 */
void encoder_init(void) {
  DDRC &= ~(_BV(PC5) | _BV(PC6) | _BV(PC7));
  PORTC |= _BV(PC5) | _BV(PC6) | _BV(PC7); /* pull-ups for A/B/SW */
  prev_state = read_state();
  accum = 0;
  btn_stable = btn_last_raw = ((PINC & _BV(PC7)) == 0);
}

/**
 * @brief Polls the encoder and returns signed detent movement.
 *
 * @return Signed detents since the last poll; positive is clockwise.
 * @note This function should be called regularly to track encoder rotation.
 */
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

/**
 * @brief Debounces and reports a single button press event.
 *
 * @return True on a valid debounced press, otherwise false.
 * @note This function should be called regularly to detect button presses.
 */
bool encoder_button_pressed(void) {
  bool raw = ((PINC & _BV(PC7)) == 0);
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

static unsigned long press_start_ms = 0; // Timestamp when the button was pressed down
static bool long_fired = false; // Flag indicating if a long press event has been fired for the current press

/**
 * @brief Reports short or long button events from the encoder switch.
 *
 * @return BTN_NONE, BTN_SHORT, or BTN_LONG depending on the switch state.
 * @note This function should be called regularly to detect button events.
 * @note The function differentiates between short and long presses based on ENC_LONG_PRESS_MS.
 */
btn_event_t encoder_button_event(void) {
  bool was = btn_stable;
  bool raw = ((PINC & _BV(PC7)) == 0);
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
