#ifndef WIRE_H
#define WIRE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Overall diameter in micrometres for an AWG size (bare diameter plus WIRE_INSULATION_PERCENT). */
unsigned int wire_awg_to_um(int awg);
/* Nearest AWG size (WIRE_AWG_MIN..WIRE_AWG_MAX) for a diameter in micrometres. */
int wire_um_to_awg(unsigned int um);

#ifdef __cplusplus
}
#endif

#endif
