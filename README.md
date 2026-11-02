# Coil_Winder

This project builds upon the project by JGJMatt at https://www.instructables.com/Make-an-Automatic-Coil-Winder/. 
It uses the same general approach but makes several additions/changes.  The summary of these alterations are:

- Use an 8mm axle and LM8UU linear bearing.  Alternate end pieces and wire guide are provided, while the 
  original 6mm versions are retained.
- Replace the wire spindle motor with an NEMA 23 series motor because the NEMA 17 was torque limited.
- Add a wire anchor plate to help keep the wires and any taps organized
- Add both a conic and trapezoidal coil form adapter
- Add limit switches for the X-axis (wire guide)
- Add software for the arduino to support many different winding patterns and options.

## Winding Patterns Supported

This project supports winding coils using one of 4 standard patterns.  These are: 

- Solenoid: The solenoid pattern places turns adjacent to each other on the form.  Spacing can be 
  specified in software, with the default being 0mm (turns are adjacent)
- Scramble: The turns are laid down in a random pattern and overlap each other.  This form provides
  lower coil capacitance and is ideal for RF and audio pickup coils. 
- Orthocyclic: Similar to solenoid, excpet the subsequent layers are offset by 1/2 the diameter
   of the wire to ensure the most compact coil can be wound. 
- Universal: This is a format developed primarily for RF and high frequency coils that lays each 
   turn down at an angle to the previous turn to reduce capacitance.

## Features

The software allows the user to enter the wire size using a standard AWG wire gauge, or the diameter 
of the wire if non-standard or metric size.