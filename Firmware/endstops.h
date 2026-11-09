#ifndef ENDSTOPS_H
#define ENDSTOPS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void endstops_init(void);
/* Debounced state of the shared X limit input (X- and X+ combined). */
bool endstops_x_triggered(void);

#ifdef __cplusplus
}
#endif

#endif
