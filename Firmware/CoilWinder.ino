// Coil winder firmware for controlling X (wire guide) and Y (form spindle) axes.  This 
// handles homing, jogging, and automated winding based on user settings.  The user has
// control over wire type, number of turns, winding pattern, speed, coil edges, and 
// positioning. The firmware manages the coordination between the X and Y axes to ensure 
// accurate winding.
// 
// It also provides a user interface for configuring these settings and monitoring the 
// winding process. Supported capabilities include:
// - Homing of the X axis (wire guide)
// - Jogging of the X and Y axes
// - Automated winding based on user-defined settings
// - Configuration of wire type, number of turns, winding pattern, and speed
// - Setting and using coil edges for precise winding
// - Multi-tap winding with section management
// - User interface for monitoring and adjusting settings
// 
// This project is an open-source and open-hardware initiative for building a coil winder, 
// and is freely available for modification and distribution under the terms of the MIT License.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// The original author of this firmware is Dewayne Hafenstein.  This entire project 
// is based on the works of JGJMatt's project in Instructables at 
// https://www.instructables.com/Make-an-Automatic-Coil-Winder/.  The original 
// project was GRBL-based, but this project replaces that control mechanism 
// with direct stepper control using an Arduino-compatible microcontroller, user
// interface, and additional features for precise coil winding.  It is intended 
// for hobbyists, educators, and anyone interested in automated coil winding, 
// using a complete self-contained and stand-alone coil winding system.
//
// Significant changes include:
// - Replacement of GRBL-based control with direct stepper control.
// - Implementation of a user interface for setting coil parameters.
// - Addition of precise coil edge setting and jogging functionality.
// - Support for multiple winding patterns and universal winding.
// - Enhanced speed control and safety features for the stepper motors.
// - Multi-tap functionality for complex winding sequences.
// - Improved error handling and recovery mechanisms.
// - Integration of homing and endstop detection for accurate positioning.
// - Support for jogging the Y axis independently for setup and adjustments.
// - Improved user interface with menu navigation and real-time feedback.
// - Enhanced support for different wire gauges and manual wire diameter input.
// - Improved handling of multi-tap winding sequences with section tracking and resumption. 
// - Larger stepper motor (23 instead of 17 series) for the form motor for increased torque and reliability.
// - Improved synchronization between X and Y axes for consistent winding quality.
// - Enhanced support for different coil form sizes and shapes.
// - Larger winding sizes accommodated with improved X axis travel range.
// - improved tensioning mechanism with wire out sensing
// - addition of both left and right edge limit switches 
// - Ability to wind single and multiple filaments simultaneously (bifilar, trifilar, etc.)
// - Support for both left-handed and right-handed winding configurations.
//
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "stepper.h"
#include "endstops.h"
#include "lcd.h"
#include "encoder.h"
#include "homing.h"
#include "wire.h"
#include "pattern.h"
#include "settings.h"

// States for the main state machine of the coil winder
typedef enum { 
  ST_MENU = 0,         //< Main menu state
  ST_SET_LEFT = 1,     //< Setting the left edge of the coil
  ST_SET_RIGHT = 2,    //< Setting the right edge of the coil
  ST_TO_START = 3,     //< Moving to the start position for winding
  ST_RUN = 4,          //< Actively winding the coil
  ST_PAUSED = 5,       //< Winding paused
  ST_DONE = 6,         //< Winding completed
  ST_ERROR = 7,        //< Error state
  ST_JOG_Y = 8         //< Jogging the Y axis independently
} state_t;

// Menu item identifiers for the coil winder interface
enum { IT_UNITS, IT_WIRE, IT_TURNS, IT_TYPE, IT_UNIV, IT_SPEED, IT_EDGES, IT_SIDE, IT_JOGY, IT_START, IT_REHOME, IT_NEWCOIL, IT_SAVE, IT_COUNT };

static state_t state = ST_MENU;
static const char *msg = "";

// Settings
static bool wire_awg_mode = true;
static int awg = 30;
static int wire_um_manual = 250;
static long turns = TURNS_DEFAULT;
static int pat = PAT_SOLENOID;
static int univ_quarters = UNIV_QUARTERS_DEFAULT;
static int y_speed_tenths = Y_SPEED_DEFAULT_TENTHS_RPS;

// Coil edges (X microsteps from home)
static bool edges_set = false;
static long left_pos = 0;
static long right_pos = 0;
static bool start_right = false;  // wind from the right edge toward the left
static long jog_target = 0;
static long jog_y_target = 0;
static long x_max_limit = 0;
static bool x_max_known = false;

// Menu
static int cursor = 0;
static bool editing = false;
static int visible[IT_COUNT];
static int visible_count = 0;

// Winding run
static int y_cap_tenths = Y_SPEED_MAX_TENTHS_RPS;  // speed cap from the guide's max step rate
static long seg_total = 0, seg_queued = 0, plan_x = 0, pending_nx = 0;
static bool have_pending = false;
static long total_y_steps = 0;

// Multi-tap: after a section finishes the guide stays put (resume_ok) or is rehomed
static bool tap_mode = false;
static bool resume_ok = false;
static int section = 1;
static long seg_base = 0;

static bool ui_dirty = true;
static unsigned long last_ui_ms = 0;
static unsigned long last_detent_ms = 0;

static int wire_um(void) {
  return wire_awg_mode ? (int)wire_awg_to_um(awg) : wire_um_manual;
}

static long steps_to_um(long steps) {
  float um = (float)steps * 1000.0f / X_USTEPS_PER_MM;
  return (long)(um + (um >= 0 ? 0.5f : -0.5f));
}

static void fmt_mm(char *buf, size_t n, long um, int dec) {
  if (dec == 2) um = (um >= 0 ? um + 5 : um - 5) / 10;
  unsigned long a = (unsigned long)labs(um);
  const char *sign = (um < 0) ? "-" : "";
  if (dec == 2) snprintf(buf, n, "%s%lu.%02lu", sign, a / 100, a % 100);
  else          snprintf(buf, n, "%s%lu.%03lu", sign, a / 1000, a % 1000);
}

static void apply_y_speed(void) {
  int t = (y_speed_tenths < y_cap_tenths) ? y_speed_tenths : y_cap_tenths;
  if (t < 1) t = 1;
  stepper_set_rate(AXIS_Y, (unsigned long)t * MOTOR_FULL_STEPS_PER_REV * Y_MICROSTEPS / 10);
}

static int y_speed_effective(void) {
  return (y_speed_tenths < y_cap_tenths) ? y_speed_tenths : y_cap_tenths;
}

// Knob acceleration: fast turning multiplies the step
static long accel_mult(void) {
  unsigned long now = millis();
  unsigned long dt = now - last_detent_ms;
  last_detent_ms = now;
  if (dt < 40) return 100;
  if (dt < 120) return 10;
  return 1;
}

static int clamp_int(long v, int lo, int hi) {
  return (v < lo) ? lo : (v > hi) ? hi : (int)v;
}

static void build_visible(void) {
  visible_count = 0;
  for (int i = 0; i < IT_COUNT; i++) {
    if (tap_mode) {
      if (i != IT_TURNS && i != IT_SPEED && i != IT_JOGY && i != IT_START && i != IT_REHOME && i != IT_NEWCOIL) continue;
    } else {
      if (i == IT_REHOME || i == IT_NEWCOIL) continue;
      if (i == IT_UNIV && pat != PAT_UNIVERSAL) continue;
    }
    visible[visible_count++] = i;
  }
  if (cursor >= visible_count) cursor = visible_count - 1;
}

static void item_text(int it, char *buf, size_t n) {
  char t[14];
  switch (it) {
    case IT_UNITS:
      snprintf(buf, n, "Units: %s", wire_awg_mode ? "AWG" : "mm");
      break;
    case IT_WIRE:
      fmt_mm(t, sizeof(t), wire_um(), 3);
      if (wire_awg_mode) snprintf(buf, n, "Wire: #%d  %smm", awg, t);
      else               snprintf(buf, n, "Wire: %s mm", t);
      break;
    case IT_TURNS:
      snprintf(buf, n, "Turns: %ld", turns);
      break;
    case IT_TYPE:
      snprintf(buf, n, "Type: %s", pattern_name((pattern_t)pat));
      break;
    case IT_UNIV:
      snprintf(buf, n, "Revs/cycle: %d.%02d", univ_quarters / 4, (univ_quarters % 4) * 25);
      break;
    case IT_SPEED:
      snprintf(buf, n, "Y speed: %d.%d rps", y_speed_tenths / 10, y_speed_tenths % 10);
      break;
    case IT_EDGES:
      if (edges_set) {
        fmt_mm(t, sizeof(t), steps_to_um(right_pos - left_pos), 2);
        snprintf(buf, n, "Edges: %s mm", t);
      } else {
        snprintf(buf, n, "Edges: not set");
      }
      break;
    case IT_SIDE:
      snprintf(buf, n, "Start side: %s", start_right ? "Right" : "Left");
      break;
    case IT_JOGY:
      snprintf(buf, n, "Jog form motor");
      break;
    case IT_START:
      snprintf(buf, n, (tap_mode && resume_ok) ? "** CONTINUE **" : "** START **");
      break;
    case IT_REHOME:
      snprintf(buf, n, "Rehome X");
      break;
    case IT_NEWCOIL:
      snprintf(buf, n, "Finish coil");
      break;
    case IT_SAVE:
      snprintf(buf, n, "Save settings");
      break;
    default:
      buf[0] = '\0';
  }
}

static void edit_item(int it, int delta) {
  switch (it) {
    case IT_UNITS:
      if (wire_awg_mode) wire_um_manual = (int)wire_awg_to_um(awg);
      else               awg = wire_um_to_awg((unsigned int)wire_um_manual);
      wire_awg_mode = !wire_awg_mode;
      break;
    case IT_WIRE:
      if (wire_awg_mode) {
        awg = clamp_int((long)awg + delta, WIRE_AWG_MIN, WIRE_AWG_MAX);
      } else {
        long step = (wire_um_manual < 100) ? 1 : 5;
        wire_um_manual = clamp_int((long)wire_um_manual + (long)delta * step * accel_mult(),
                                   WIRE_MIN_UM, WIRE_MAX_UM);
      }
      break;
    case IT_TURNS: {
      long v = turns + (long)delta * accel_mult();
      turns = (v < 1) ? 1 : (v > TURNS_MAX) ? TURNS_MAX : v;
      break;
    }
    case IT_TYPE:
      pat = (pat + delta) % PAT_COUNT;
      if (pat < 0) pat += PAT_COUNT;
      break;
    case IT_UNIV:
      univ_quarters = clamp_int((long)univ_quarters + delta, UNIV_QUARTERS_MIN, UNIV_QUARTERS_MAX);
      break;
    case IT_SPEED:
      y_speed_tenths = clamp_int((long)y_speed_tenths + delta, Y_SPEED_MIN_TENTHS_RPS, Y_SPEED_MAX_TENTHS_RPS);
      break;
    case IT_SIDE:
      start_right = !start_right;
      break;
  }
}

static void save_settings(void) {
  settings_t s;
  s.awg_mode = wire_awg_mode;
  s.awg = awg;
  s.wire_um = wire_um_manual;
  s.turns = turns;
  s.pattern = pat;
  s.univ_quarters = univ_quarters;
  s.y_speed_tenths = y_speed_tenths;
  s.edges_set = edges_set;
  s.left_pos = left_pos;
  s.right_pos = right_pos;
  s.start_right = start_right;
  settings_save(&s);
  Serial.println(F("Settings saved"));
}

static bool load_settings(void) {
  settings_t s;
  if (!settings_load(&s)) return false;
  wire_awg_mode = s.awg_mode != 0;
  awg = clamp_int(s.awg, WIRE_AWG_MIN, WIRE_AWG_MAX);
  wire_um_manual = clamp_int(s.wire_um, WIRE_MIN_UM, WIRE_MAX_UM);
  turns = (s.turns < 1) ? 1 : (s.turns > TURNS_MAX) ? TURNS_MAX : s.turns;
  pat = (s.pattern < PAT_COUNT) ? s.pattern : PAT_SOLENOID;
  univ_quarters = clamp_int(s.univ_quarters, UNIV_QUARTERS_MIN, UNIV_QUARTERS_MAX);
  y_speed_tenths = clamp_int(s.y_speed_tenths, Y_SPEED_MIN_TENTHS_RPS, Y_SPEED_MAX_TENTHS_RPS);
  start_right = s.start_right != 0;
  // Edges are measured from home, so they remain valid after the boot homing
  if (s.edges_set && s.left_pos >= 0 && s.right_pos >= s.left_pos + (long)MIN_COIL_WIDTH_STEPS) {
    edges_set = true;
    left_pos = s.left_pos;
    right_pos = s.right_pos;
  }
  return true;
}

static void enter_menu(const char *m) {
  state = ST_MENU;
  msg = m;
  editing = false;
  stepper_follow_stop();
  stepper_stop(AXIS_Y);
  stepper_stop(AXIS_X);
  ui_dirty = true;
}

static void goto_left_edge(void) {
  if (!edges_set) return;
  stepper_set_rate(AXIS_X, JOG_RATE_SPS);
  stepper_move_to(AXIS_X, start_right ? right_pos : left_pos);
}

static void enter_jog_y(void) {
  state = ST_JOG_Y;
  msg = "";
  stepper_follow_stop();
  stepper_stop(AXIS_Y);
  stepper_stop(AXIS_X);
  stepper_set_rate(AXIS_Y, JOG_Y_RATE_SPS);
  jog_y_target = stepper_get_position(AXIS_Y);
  ui_dirty = true;
}

static void enter_setup(int s) {
  state = (state_t)s;
  msg = "";
  stepper_follow_stop();
  stepper_stop(AXIS_Y);
  stepper_stop(AXIS_X);
  stepper_set_rate(AXIS_X, JOG_RATE_SPS);
  jog_target = stepper_get_position(AXIS_X);
  ui_dirty = true;
}

static void jog(int detents) {
  long target = jog_target + (long)detents * (long)JOG_STEPS_PER_DETENT;
  if (target < 0) target = 0;
  if (x_max_known && target > x_max_limit) target = x_max_limit;
  jog_target = target;
  stepper_move_to(AXIS_X, jog_target);
}

// The X+ switch stops a jog; back off so the guide never rests on the switch.
static void handle_limit_while_jogging(void) {
  if (stepper_move_active(AXIS_X) && stepper_get_dir(AXIS_X) == DIR_FORWARD && endstops_x_triggered()) {
    stepper_stop(AXIS_X);
    long back = stepper_get_position(AXIS_X) - HOME_CLEARANCE_STEPS;
    if (back < 0) back = 0;
    x_max_limit = back;
    x_max_known = true;
    jog_target = back;
    stepper_move_to(AXIS_X, back);
    msg = "X+ limit reached";
    ui_dirty = true;
    Serial.println(F("X+ limit hit while jogging, backing off"));
  }
}

static void refill_segments(void) {
  while (seg_queued < seg_total) {
    if (!have_pending) {
      pending_nx = pattern_pos((unsigned long)(seg_base + seg_queued + 1));
      have_pending = true;
    }
    if (!stepper_follow_queue(pending_nx - plan_x)) break;
    plan_x = pending_nx;
    have_pending = false;
    seg_queued++;
  }
}

static void begin_run(void);

static void start_winding(void) {
  if (tap_mode && resume_ok) {
    // Same pattern, guide stays where it is; the pattern continues from its current phase
    section++;
    Serial.print(F("Continue section ")); Serial.println(section);
    begin_run();
    return;
  }
  if (!edges_set) { msg = "Set edges first"; return; }
  float d = (float)wire_um() * X_USTEPS_PER_MM / 1000.0f;
  if ((float)(right_pos - left_pos) < d) { msg = "Edges < wire dia"; return; }

  section = tap_mode ? section + 1 : 1;
  seg_base = 0;
  resume_ok = false;
  pattern_set_mirror(start_right);
  pattern_begin((pattern_t)pat, left_pos, right_pos, d, univ_quarters);
  float mspr = pattern_max_steps_per_rev();
  y_cap_tenths = Y_SPEED_MAX_TENTHS_RPS;
  if (mspr > 0.0f) {
    long cap = (long)(10.0f * (float)X_MAX_STEP_RATE_SPS / mspr);
    if (cap < y_cap_tenths) y_cap_tenths = (cap < 1) ? 1 : (int)cap;
  }
  apply_y_speed();

  Serial.print(F("Start: ")); Serial.print(pattern_name((pattern_t)pat));
  Serial.print(F(", wire um=")); Serial.print(wire_um());
  Serial.print(F(", turns=")); Serial.print(turns);
  Serial.print(F(", Y tenths rps=")); Serial.println(y_speed_effective());

  stepper_set_rate(AXIS_X, JOG_RATE_SPS);
  stepper_move_to(AXIS_X, start_right ? right_pos : left_pos);
  state = ST_TO_START;
  msg = "";
  ui_dirty = true;
}

static void begin_run(void) {
  long segs_per_rev = pattern_segs_per_rev();
  seg_total = turns * segs_per_rev;
  seg_queued = 0;
  have_pending = false;
  total_y_steps = turns * (long)Y_STEPS_PER_REV;
  plan_x = (tap_mode && resume_ok) ? stepper_get_position(AXIS_X) : pattern_pos(0);

  stepper_set_position(AXIS_Y, 0);
  stepper_follow_start((long)Y_STEPS_PER_REV / segs_per_rev);
  refill_segments();
  apply_y_speed();
  stepper_move_to(AXIS_Y, total_y_steps);
  state = ST_RUN;
  ui_dirty = true;
}

static void fail(const char *m) {
  stepper_stop(AXIS_Y);
  stepper_follow_stop();
  stepper_stop(AXIS_X);
  state = ST_ERROR;
  msg = m;
  ui_dirty = true;
  Serial.print(F("ERROR: ")); Serial.println(m);
}

static void ui_update(void) {
  char buf[LCD_COLS + 1];
  char a[14];
  long pos = stepper_get_position(AXIS_X);

  switch (state) {
    case ST_MENU: {
      int top = (cursor < 3) ? 0 : cursor - 2;
      for (int r = 0; r < 3; r++) {
        int idx = top + r;
        if (idx < visible_count) {
          char line[LCD_COLS + 1];
          item_text(visible[idx], line, sizeof(line));
          snprintf(buf, sizeof(buf), "%c%s", idx == cursor ? (editing ? '*' : '>') : ' ', line);
        } else {
          buf[0] = '\0';
        }
        lcd_print_row(r, buf);
      }
      lcd_print_row(3, msg[0] ? msg : (editing ? "Turn:change Click:OK" : "Turn:move Click:edit"));
      break;
    }
    case ST_SET_LEFT:
    case ST_SET_RIGHT:
      lcd_print_row(0, state == ST_SET_LEFT ? "Set LEFT edge" : "Set RIGHT edge");
      fmt_mm(a, sizeof(a), steps_to_um(pos), 2);
      snprintf(buf, sizeof(buf), "Guide: %s mm", a);
      lcd_print_row(1, buf);
      lcd_print_row(2, msg[0] ? msg : "Knob: jog guide");
      lcd_print_row(3, state == ST_SET_LEFT ? "Click:set Hold:cancel" : "Click:set Hold:cancel");
      break;
    case ST_JOG_Y:
      lcd_print_row(0, "Jog form motor");
      snprintf(buf, sizeof(buf), "Angle: %ld deg", (stepper_get_position(AXIS_Y) * 360L / (long)Y_STEPS_PER_REV) % 360L);
      lcd_print_row(1, buf);
      lcd_print_row(2, "Knob: rotate form");
      lcd_print_row(3, "Click: done");
      break;
    case ST_TO_START:
      lcd_print_row(0, "Moving to start...");
      lcd_print_row(1, "");
      lcd_print_row(2, "");
      lcd_print_row(3, "");
      break;
    case ST_RUN:
    case ST_PAUSED: {
      long done = stepper_get_position(AXIS_Y) / (long)Y_STEPS_PER_REV;
      snprintf(buf, sizeof(buf), "%s  Sect %d", pattern_name((pattern_t)pat), section);
      lcd_print_row(0, buf);
      snprintf(buf, sizeof(buf), "Turns %ld/%ld", done, turns);
      lcd_print_row(1, buf);
      fmt_mm(a, sizeof(a), steps_to_um(pos), 2);
      int ye = y_speed_effective();
      snprintf(buf, sizeof(buf), "X:%smm Y:%d.%d%s", a, ye / 10, ye % 10, ye < y_speed_tenths ? "*" : "rps");
      lcd_print_row(2, buf);
      lcd_print_row(3, state == ST_PAUSED ? "PAUSED Click:resume" : "Click:pause Hold:end");
      break;
    }
    case ST_DONE:
      lcd_print_row(0, "Winding complete");
      snprintf(buf, sizeof(buf), "Turns: %ld", turns);
      lcd_print_row(1, buf);
      lcd_print_row(2, "");
      lcd_print_row(3, "Click:tap Hold:end");
      break;
    case ST_ERROR:
      lcd_print_row(0, "ERROR");
      lcd_print_row(1, msg);
      lcd_print_row(2, "");
      lcd_print_row(3, "Click: menu");
      break;
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  endstops_init();
  stepper_init();
  lcd_init();
  encoder_init();
  apply_y_speed();

  stepper_enable(true);
  stepper_set_dir(AXIS_Y, DIR_FORWARD);

  lcd_print_row(0, "Homing X...");
  Serial.println(F("Homing X..."));
  if (!homing_home_x()) {
    stepper_enable(false);
    lcd_print_row(0, "HOMING FAILED");
    lcd_print_row(1, "Check X limit switch");
    lcd_print_row(2, "and motor wiring");
    Serial.println(F("ERROR: homing failed (limit not found / not released)"));
    while (true) {}
  }
  Serial.println(F("Homed."));
  if (load_settings()) { Serial.println(F("Settings loaded")); apply_y_speed(); enter_menu("Settings loaded"); }
  else enter_menu("");
  goto_left_edge();
}

void loop() {
  int delta = encoder_poll();
  btn_event_t btn = encoder_button_event();

  switch (state) {
    case ST_MENU:
      build_visible();
      if (delta) {
        msg = "";
        if (editing) {
          edit_item(visible[cursor], delta);
        } else {
          cursor += delta;
          if (cursor < 0) cursor = 0;
          if (cursor >= visible_count) cursor = visible_count - 1;
        }
        ui_dirty = true;
      }
      if (btn == BTN_SHORT) {
        msg = "";
        int it = visible[cursor];
        if (it == IT_EDGES) {
          editing = false;
          enter_setup(ST_SET_LEFT);
        } else if (it == IT_JOGY) {
          editing = false;
          enter_jog_y();
        } else if (it == IT_START) {
          start_winding();
        } else if (it == IT_SAVE) {
          save_settings();
          msg = "Settings saved";
        } else if (it == IT_REHOME) {
          lcd_print_row(3, "Homing X...");
          Serial.println(F("Rehoming X"));
          if (homing_home_x()) {
            resume_ok = false;
            goto_left_edge();
            msg = edges_set ? "Homed; at left edge" : "Homed; START restarts";
          } else {
            msg = "Homing failed";
          }
        } else if (it == IT_NEWCOIL) {
          tap_mode = false;
          resume_ok = false;
          cursor = 0;
          msg = "Coil finished";
        } else {
          editing = !editing;
        }
        ui_dirty = true;
      }
      break;

    case ST_SET_LEFT:
    case ST_SET_RIGHT:
      handle_limit_while_jogging();
      if (delta) { msg = ""; jog(delta); }
      if (btn == BTN_LONG) {
        enter_menu("Edges unchanged");
      } else if (btn == BTN_SHORT && state == ST_SET_LEFT) {
        left_pos = stepper_get_position(AXIS_X);
        Serial.print(F("Left edge set: ")); Serial.println(left_pos);
        enter_setup(ST_SET_RIGHT);
      } else if (btn == BTN_SHORT) {
        long p = stepper_get_position(AXIS_X);
        if (p < left_pos + (long)MIN_COIL_WIDTH_STEPS) {
          msg = "Right must be > left";
          ui_dirty = true;
        } else {
          right_pos = p;
          edges_set = true;
          Serial.print(F("Right edge set: ")); Serial.println(right_pos);
          enter_menu("Edges set");
        }
      }
      if (state == ST_SET_LEFT || state == ST_SET_RIGHT) ui_dirty = true;
      break;

    case ST_JOG_Y:
      if (delta) {
        jog_y_target += (long)delta * (long)JOG_Y_STEPS_PER_DETENT;
        stepper_move_to(AXIS_Y, jog_y_target);
      }
      if (btn == BTN_SHORT || btn == BTN_LONG) {
        stepper_stop(AXIS_Y);
        apply_y_speed();
        enter_menu("");
        break;
      }
      if ((millis() - last_ui_ms) >= 250) ui_dirty = true;
      break;

    case ST_TO_START:
      if (btn == BTN_LONG) { enter_menu("Cancelled"); break; }
      if (!stepper_move_active(AXIS_X)) begin_run();
      break;

    case ST_RUN:
      refill_segments();
      if (endstops_x_triggered()) { fail("X limit switch hit"); break; }
      if (!stepper_move_active(AXIS_Y)) {
        // Spindle finished; let the guide catch up with its last steps
        if (stepper_follow_settled()) {
          stepper_follow_stop();
          seg_base += seg_total;
          state = ST_DONE;
          Serial.println(F("Winding complete"));
          ui_dirty = true;
        }
        break;
      }
      if (stepper_follow_starved()) { fail("Pattern underrun"); break; }
      if (delta) {
        y_speed_tenths = clamp_int((long)y_speed_tenths + delta, Y_SPEED_MIN_TENTHS_RPS, Y_SPEED_MAX_TENTHS_RPS);
        apply_y_speed();
      }
      if (btn == BTN_SHORT) {
        stepper_stop(AXIS_Y);
        state = ST_PAUSED;
        ui_dirty = true;
      } else if (btn == BTN_LONG) {
        enter_menu("Winding stopped");
      }
      if ((millis() - last_ui_ms) >= 250) ui_dirty = true;
      break;

    case ST_PAUSED:
      if (btn == BTN_SHORT) {
        stepper_move_to(AXIS_Y, total_y_steps);
        state = ST_RUN;
        ui_dirty = true;
      } else if (btn == BTN_LONG) {
        enter_menu("Winding stopped");
      }
      break;

    case ST_DONE:
      if (btn == BTN_SHORT) {
        tap_mode = true;
        resume_ok = true;
        cursor = 0;
        enter_menu("Make tap; set turns");
      } else if (btn == BTN_LONG) {
        tap_mode = false;
        resume_ok = false;
        enter_menu("");
      }
      break;

    case ST_ERROR:
      if (btn == BTN_SHORT) {
        tap_mode = false;
        resume_ok = false;
        enter_menu("");
      }
      break;
  }

  // LCD writes block briefly, so refresh at most every 100 ms and only when something changed
  if (ui_dirty && (millis() - last_ui_ms) >= 100) {
    last_ui_ms = millis();
    ui_dirty = false;
    ui_update();
  }
}
