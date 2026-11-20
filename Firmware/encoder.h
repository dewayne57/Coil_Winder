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
#ifndef ENCODER_H
#define ENCODER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the rotary encoder and button hardware.
 */
void encoder_init(void);

/* Call often. Returns detents turned since the last call (+ clockwise,
 * - counter-clockwise). 
 * @brief Reads the encoder delta since the last call.
 * @return Signed number of detents, positive for clockwise motion.
 */
int encoder_poll(void);

/* True once per button press (debounced).
 * @brief Returns a debounced button press event.
 *
 * @return True once for a valid press, otherwise false.
 */
bool encoder_button_pressed(void);

typedef enum { BTN_NONE = 0, BTN_SHORT, BTN_LONG } btn_event_t;

/**
 * @brief Reports the current encoder button state transition.
 *
 * @return BTN_NONE, BTN_SHORT, or BTN_LONG as appropriate.
 * @note This function should be called regularly to detect button events.
 * @note BTN_SHORT is reported on release; BTN_LONG fires once while held past ENC_LONG_PRESS_MS.
 */
btn_event_t encoder_button_event(void);

#ifdef __cplusplus
}
#endif

#endif
