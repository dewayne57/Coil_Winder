#ifndef PATTERN_H
#define PATTERN_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  PAT_SOLENOID = 0,    /* aligned layers, one wire pitch per revolution */
  PAT_ORTHOCYCLIC,     /* alternate layers nest in the grooves of the layer below */
  PAT_SCRAMBLE,        /* random lay */
  PAT_UNIVERSAL,       /* continuous wave traverse, fractional revs per cycle */
  PAT_COUNT
} pattern_t;

const char *pattern_name(pattern_t t);

/* left/right are the guide positions (X microsteps) of the first and last wire centres.
   wire_steps is the wire pitch in X microsteps. univ_quarters is used by PAT_UNIVERSAL only. */
void pattern_begin(pattern_t t, long left, long right, float wire_steps, int univ_quarters);

/* Guide segments per spindle revolution (1, or 4 for universal). */
long pattern_segs_per_rev(void);

/* Largest guide travel in X microsteps during one spindle revolution (for speed limiting). */
float pattern_max_steps_per_rev(void);

/* Absolute guide position at the start of segment `seg`.
   Must be called with consecutive indices 0, 1, 2, ... after pattern_begin. */
long pattern_pos(unsigned long seg);

#ifdef __cplusplus
}
#endif

#endif
