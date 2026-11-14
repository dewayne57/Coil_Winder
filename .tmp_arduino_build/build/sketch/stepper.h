#line 1 "C:\\Users\\Dewayne\\Documents\\GitHub\\Coil_Winder\\.tmp_arduino_build\\CoilWinder\\stepper.h"
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
#ifndef STEPPER_H
#define STEPPER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { AXIS_X = 0, AXIS_Y = 1, AXIS_COUNT } axis_t;

/* Direction is axis-relative: X: reverse=left, forward=right. 
 * Y: spindle reverse/forward. 
 */
typedef enum { DIR_REVERSE = 0, DIR_FORWARD = 1 } dir_t;

/**
 * @brief Initializes the stepper driver peripherals and timer-based pulse generator.
 */
void stepper_init(void);

/**
 * @brief Enables or disables the shared stepper driver enable pin.
 *
 * @param on True to enable the driver, false to disable it.
 */
void stepper_enable(bool on);

/**
 * @brief Sets the motion direction for a selected axis.
 *
 * @param axis Axis to update.
 * @param dir Requested direction.
 */
void stepper_set_dir(axis_t axis, dir_t dir);

/**
 * @brief Reads the currently configured direction for an axis.
 *
 * @param axis Axis to query.
 * @return Current axis direction.
 */
dir_t stepper_get_dir(axis_t axis);

/**
 * @brief Configures the step rate for a selected axis.
 *
 * @param axis Axis to update.
 * @param steps_per_sec Desired step rate in steps per second.
 */
void stepper_set_rate(axis_t axis, unsigned long steps_per_sec);

/**
 * @brief Enables or disables step generation for an axis.
 *
 * @param axis Axis to update.
 * @param running True to start stepping, false to hold the axis.
 */
void stepper_set_running(axis_t axis, bool running);

/**
 * @brief Returns the current absolute position of an axis.
 *
 * @param axis Axis to query.
 * @return Current position in microsteps.
 */
long stepper_get_position(axis_t axis);

/**
 * @brief Sets the stored position for one axis.
 *
 * @param axis Axis to update.
 * @param pos New position in microsteps.
 */
void stepper_set_position(axis_t axis, long pos);

/* (non-blocking). */
/**
 * @brief Starts a non-blocking absolute move for an axis.
 *
 * @param axis Axis to move.
 * @param target Desired absolute position in microsteps.
 */
void stepper_move_to(axis_t axis, long target);

/**
 * @brief Reports whether a move is still active for an axis.
 *
 * @param axis Axis to query.
 * @return True while the absolute move is in progress.
 */
bool stepper_move_active(axis_t axis);

/**
 * @brief Stops a selected axis and cancels any pending absolute move.
 *
 * @param axis Axis to stop.
 */
void stepper_stop(axis_t axis);

/**
 * @brief Enables Y/X follow mode with a fixed segment length.
 *
 * @param seg_len_y_steps Number of Y microsteps per follow segment.
 */
void stepper_follow_start(long seg_len_y_steps);

/**
 * @brief Disables the X/Y follow interpolator.
 */
void stepper_follow_stop(void);

/**
 * @brief Queues the next X-axis delta for the active follow segment.
 *
 * @param dx X delta in microsteps for the next queued segment.
 * @return True when the segment was accepted, false when both queue slots are full.
 */
bool stepper_follow_queue(long dx);

/**
 * @brief Reports whether the follow interpolator ran out of queued data.
 *
 * @return True when follow mode is starved and waiting for more segments.
 */
bool stepper_follow_starved(void);

/**
 * @brief Reports whether the X axis has caught up to the follow target.
 *
 * @return True when follow mode is settled.
 */
bool stepper_follow_settled(void);

#ifdef __cplusplus
}
#endif

#endif
