# Coil_Winder

A ACNC general purpose coil winder.

## Hardware direction

- Use DM556 stepper drivers for axis motion control.
- Add shaft-mounted rotary encoders for feedback on each driven shaft.
- The planned encoder resolution is 10 degrees per step, which provides 36 positions per full revolution.

## Feedback expectations

- Encoder feedback should be treated as coarse position verification rather than high-resolution closed-loop control.
- The 10 degree encoder resolution is suitable for detecting missed motion, shaft slip, and general alignment errors.
