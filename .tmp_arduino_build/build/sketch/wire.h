#line 1 "C:\\Users\\Dewayne\\Documents\\GitHub\\Coil_Winder\\.tmp_arduino_build\\CoilWinder\\wire.h"
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
#ifndef WIRE_H
#define WIRE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Overall diameter in micrometres for an AWG size (bare diameter plus
 WIRE_INSULATION_PERCENT). */
unsigned int wire_awg_to_um(int awg);

/* Nearest AWG size (WIRE_AWG_MIN..WIRE_AWG_MAX) for a diameter in
micrometres. */
int wire_um_to_awg(unsigned int um);

#ifdef __cplusplus
}
#endif

#endif
