# Coil_Winder

This project builds upon the work by JGJMatt at
https://www.instructables.com/Make-an-Automatic-Coil-Winder/.
It uses the same general approach but adds several improvements and
changes.

## Licensing

This project is released under the Coil_Winder Non-Commercial License.
The repository is open for personal, educational, research, and hobby
use, but commercial use requires a separate written license from the
copyright holder.

For the full text, see [LICENSE.md](./LICENSE.md).

Commercial use is not permitted without prior written permission,
including selling, manufacturing, licensing, or distributing this
project or derived products for direct or indirect financial gain.

## Commercial Licensing

Commercial use of this project, including the sale, production,
sublicensing, or distribution of hardware, firmware, software, or
derivative works for direct or indirect financial gain, requires a
separate written commercial license from Dewayne L. Hafenstein.

To request a commercial license, contact:

   dewayne@hafenstein.net

Please include:

- your company or organization name
- the intended product, service, or use case
- whether you plan to sell, manufacture, distribute, or integrate the
design
- the expected scope and volume of commercial use
- any requested license term or territory

We will respond with licensing terms and pricing appropriate to the
requested use.

## Winding Patterns Supported

This project supports winding coils using one of four standard
patterns. These are:

- Solenoid: The solenoid pattern places turns adjacent to each other on
  the form. Spacing can be specified in software, with the default
  being 0mm (turns are adjacent).
- Scramble: The turns are laid down in a random pattern and overlap each
  other. This form provides lower coil capacitance and is ideal for RF
  and audio pickup coils.
- Orthocyclic: Similar to solenoid, except the subsequent layers are
  offset by 1/2 the diameter of the wire to ensure the most compact
  coil can be wound.
- Universal: This is a format developed primarily for RF and high
  frequency coils that lays each turn down at an angle to the previous
  turn to reduce capacitance.

## Features

The software allows the user to enter the wire size using a standard
AWG wire gauge, or the diameter of the wire if non-standard or metric
size.

## Usage

The general winding process is:

1. Ensure the alignment bar is fully retracted before any motion begins. The firmware will 
   block motion if the bar is not retracted, and the Y and Z sensors are used as safety 
   checks.  If the bar is not retracted, the system will prompt you to retract the bar
   before anything else can be performed.
2. Install the coil form, wire, and guide hardware and verify the machine moves freely.
3. Power on the system.  When the system first powers on (assuming the alignment bar 
   is retracted), the system will automatically home the wire guide to the left edge.)
4. Select to set the left and right edges.  Jog the wire guide until the wire guide 
   is within the boundaries of the form.  Use the alignment bar to set the left and 
   right edges.  The alignment bar is 4mm wide.  Jog it left until the left edge is 
   adjacent to the furthest left extent of the coil (this may or may not be the left
   edge of the form). Set the left edge.  Proceed to adjust the right edge using the 
   right side of the alignment bar.  Set the right edge.  Retract the alignment bar 
   and select proceed.
   The firmware accounts for the alignment bar width when recording these edge positions.
5. Set the wire feed, number of filaments (1-4), and spindle parameters for the wire 
   gauge and winding speed.
6. Choose to start the winding from the left edge, or right edge.
7. Choose the winding pattern and the number of turns, then start the winding sequence.
8. The machine will move the X guide while the spindle rotates the form, keeping the 
   required relationship between the axes for the selected pattern.
9. When the turns are completed, the machine will stop and allow you to create a tap
   and enter a new number of turns, or to complete the winding.  While stopped, you 
   can jog the form either clockwise or counter-clockwise as needed for taping or 
   to anchor the wire for a tap. 
10. If desired, you can enter a new number of turns and resume winding from the 
   current position and direction, start at the left edge, or start at the right 
   edge (ina reversed winding direction). Basically, you can repeat steps 7-10 
   as many times as needed to create a multi-tap winding.
11. When the run completes, the machine stops automatically and returns to the menu 
   or wait state for the next coil.

For best results, verify the alignment bar retracts completely at startup, confirm 
the Y wire-out sensor is clear before running, and validate the left/right edge 
offsets on the actual machine before production use.


