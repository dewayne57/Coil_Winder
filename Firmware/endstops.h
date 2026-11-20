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
#ifndef ENDSTOPS_H
#define ENDSTOPS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes all limit switch inputs and their debounced state.
 */
void endstops_init(void);

/**
 * @brief Returns the debounced state of the X axis limit input.
 *
 * @return True when the X limit is active.
 * @note The returned state is debounced and reflects the stable state of the limit switch.
 */
bool endstops_x_triggered(void);

/**
 * @brief Returns the debounced state of the Y axis wire-out sensor.
 *
 * @return True when the Y sensor is triggered.
 * @note The returned state is debounced and reflects the stable state of the limit switch.
 */
bool endstops_y_triggered(void);

/**
 * @brief Returns the debounced state of the Z axis alignment-bar sensor.
 *
 * @return True when the Z sensor indicates the bar is not retracted.
 * @note The returned state is debounced and reflects the stable state of the limit switch.
 */
bool endstops_z_triggered(void);

/**
 * @brief Reports whether the alignment bar is fully retracted.
 *
 * @return True when the alignment bar is in the safe retracted state.
 * @note The returned state is debounced and reflects the stable state of the limit switch.
 */
bool endstops_alignment_bar_retracted(void);

#ifdef __cplusplus
}
#endif

#endif
