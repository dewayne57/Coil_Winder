#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "pattern.h"

static unsigned long seed;

unsigned long micros(void) { return seed; }

static void check_scramble(long left, long right, float wire_steps, int mirror,
                           unsigned long initial_seed) {
  enum { TURNS = 500, BINS = 10, SAMPLES = 20 };
  long counts[BINS] = {0};
  int reached_left = 0, reached_right = 0;
  seed = initial_seed;
  pattern_set_mirror(mirror);
  pattern_begin(PAT_SCRAMBLE, left, right, wire_steps, 13);
  long previous = pattern_pos(0);
  assert(previous == (mirror ? right : left));
  float max_travel = pattern_max_steps_per_rev();
  for (unsigned long seg = 1; seg <= TURNS; ++seg) {
    long next = pattern_pos(seg);
    assert(next >= left && next <= right);
    assert(fabs((double)next - previous) <= ceil(max_travel));
    if (next == left) reached_left++;
    if (next == right) reached_right++;
    for (int i = 0; i < SAMPLES; ++i) {
      double x = previous + (next - previous) * (i + 0.5) / SAMPLES;
      int bin = (int)((x - left) * BINS / (right - left));
      if (bin == BINS) bin--;
      counts[bin]++;
    }
    previous = next;
  }
  assert(reached_left && reached_right);
  for (int bin = 0; bin < BINS; ++bin) {
    assert(counts[bin] > TURNS * SAMPLES / BINS * 0.7);
    assert(counts[bin] < TURNS * SAMPLES / BINS * 1.3);
  }
}

int main(void) {
  for (unsigned long s = 1; s <= 20; ++s) {
    check_scramble(100, 10100, 100.0f, 0, s);
    check_scramble(100, 10100, 100.0f, 1, s);
    check_scramble(100, 200, 100.0f, 0, s);
    check_scramble(100, 500, 100.0f, 0, s);
  }
  puts("Scramble pattern tests passed");
  return 0;
}
