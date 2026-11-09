#ifndef ENCODER_H
#define ENCODER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void encoder_init(void);
/* Call often. Returns detents turned since the last call (+ clockwise, - counter-clockwise). */
int encoder_poll(void);
/* True once per button press (debounced). */
bool encoder_button_pressed(void);

typedef enum { BTN_NONE = 0, BTN_SHORT, BTN_LONG } btn_event_t;
/* BTN_SHORT is reported on release; BTN_LONG fires once while held past ENC_LONG_PRESS_MS. */
btn_event_t encoder_button_event(void);

#ifdef __cplusplus
}
#endif

#endif
