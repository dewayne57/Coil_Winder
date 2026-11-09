#ifndef STEPPER_H
#define STEPPER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { AXIS_X = 0, AXIS_Y = 1, AXIS_COUNT } axis_t;

/* Direction is axis-relative: X: reverse=left, forward=right. Y: spindle reverse/forward. */
typedef enum { DIR_REVERSE = 0, DIR_FORWARD = 1 } dir_t;

/* Sets up pins and the Timer1 interrupt that generates all step pulses. */
void stepper_init(void);
void stepper_enable(bool on);
void stepper_set_dir(axis_t axis, dir_t dir);
dir_t stepper_get_dir(axis_t axis);
/* Step rate in (micro)steps per second. */
void stepper_set_rate(axis_t axis, unsigned long steps_per_sec);
/* Start/stop step generation for an axis. */
void stepper_set_running(axis_t axis, bool running);
/* Position in (micro)steps since stepper_set_position; forward is positive. */
long stepper_get_position(axis_t axis);
void stepper_set_position(axis_t axis, long pos);
/* Move to an absolute position at the current rate and stop exactly there (non-blocking). */
void stepper_move_to(axis_t axis, long target);
bool stepper_move_active(axis_t axis);
/* Stops the axis and cancels any move in progress. */
void stepper_stop(axis_t axis);

/* Follow mode: X is slaved to Y. The path is queued as equal-length segments (seg_len Y steps
   each); each segment moves X by dx steps in proportion to Y's progress. Two segments can be
   queued ahead. Start with Y stopped, queue segments, then start Y. */
void stepper_follow_start(long seg_len_y_steps);
void stepper_follow_stop(void);
/* Returns false if both queue slots are full. */
bool stepper_follow_queue(long dx);
/* True if Y ran out of queued segments (set when a segment ends with none queued). */
bool stepper_follow_starved(void);
/* True when X has caught up to where Y says it should be. */
bool stepper_follow_settled(void);

#ifdef __cplusplus
}
#endif

#endif
