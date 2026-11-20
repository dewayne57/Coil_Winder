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

## Parts List 

- 2x 2020 extrusions, 12" or longer is recommended.  
- 1x NEMA 23 stepper motor
- 1x NEMA 17 stepper motor 
- 1x ACME lead screw and matching nut (at least 12" long).  The software is 
  set up for a 4-start (8mm pitch) 8mm diameter lead screw.  If the pitch
  is changde, the software must be updated. 
- 1x ball bearing for the far end lead screw support
- 1x Arduino Mega2560 board
- 1x Arduino CNC Shield V3 
- 2x LM2596 buck converter boards (one set to 5V and the other to 12V)
- 1x 20X4 backlit LCD display, any color
- 1x panel mount mechanical rotary encoder with switch + suitable knob
- 1x tie point strip for wire connections
- various pieces of hookup wire
- 1x 24V@4A (minimum) power supply.  
- 1x SPST rocker switch 
- 2x Miniature micro switches (SPST NO or SPDT)
- 1X Roller Micro switch (SPST NO or SPDT) 
- 1X 8mm shaft at least 12" long (this is for the wire guide support, a 6mm 
  shaft can be used, but the flex is much more significant)
- 1X 8mm LM8UU linear bearing (for the wire guide)
- 4X rubber feet to attach to the bottom of the 2020 extrusions
- M3 heat set inserts
- Various springs as needed
- Various M3, M4, and M6 screws, nuts, and washers

## Assembly 

The model files contain several variations of parts to be able to use different 
size shafts, motors, and bearings.  All the things that are likely different 
from one hobbiest to another.  Therefore, the first thing to do is to collect
all the parts that you will need.

1. Print all the parts you will need.  This includes the left, right, and motor 
   ends, the back, front, and top cover, wire guide, alignment bar, left and right
   limit switch brackets, tailstock, and an appropriate drive hub for the NEMA23
   motor you intend to use.  The drive hub is available in 5mm, 6mm, and quarter 
   inch variants.  You will need a matching drive hub for the tailstock.  The 
   tail stock is available with both 6mm and quarter inch variants.  Use whichever
   is appropriate for your shaft and bearing sizes.  You will also need to print 
   the extrusion cover(s), and trim it(them) as needed for your length of extrusion.  
2. Insert the M3 heat set inserts into the left end to mount the arduino, buck 
   converters, and the tie strip. 
3. Insert the M3 heat set inserts into the left and motor ends to attach the 
   covers. 
4. Thread the 2020 extrusion center hole to M6 on all for ends. 
5. Place the 2020 extrusion turn-lock nuts and M4 short screws in the motor end 
   railsattachment points (6 places).  Align the nuts so they slide down the extrusion.
   Align the nuts to the extrusions and slide the motor plate onto both extrusions
   about 6".  DO NOT tighten at this time.
6. Attach the left end to the 2020 extrusions (2 places) using M6 bolts, lock washers,
   and flat washers.  
7. Use the front and top covers to locate the motor end panel.  Slide it down the 
   extrusions until the covers fit correctly and the left end is perpendicular to the
   extrusion rails.  Tighten the motor rail attachment screws at this point. Remove
   the front and top covers.
8. Mount the NEMA 17 and 23 motors. 
9.  On the left end panel, mount the arduino MEGA2560 board and align and snap the 
   CNC shield to the top of it. 
10. Mount both buck converters. 
11. Mount the power input jack. 
12. Mount the tie point strip. 
13. On the front panel, insert the M3 heat set inserts for hte LCD attachment. 
14. Mount the LCD, rocker switch, and rotary encoder on the front panel.
15. Wire the 24V supply in the left end. Run positive lead from the input jack to the 
    front rocker switch, from the switch to the tie point.  You may wish to use an 
    in-line 2-pin connector to allow the front panel to be removed easily in the 
    future.  Run the ground lead from the power jack to another lug on the tie point.
16. Run a hookup wire from the ground lug to the ground input of both buck converters.
17. Run a hookup wire from the +24V lug to the input of both buck converters.
18. IMPORTANT.  Before proceeding, YOU MUST adjust the buck converters output voltages.
    Connect the 24V supply, plug it in, and turn on the rocker switch. Use a multimeter
    and measure the voltage from the ground lug on the tie point to the output of each 
    converter.  Adjust one converter for +5V output, label it the +5V supply.  Adjust 
    the other converter for +12V output, label it the +12V supply.  Turn off the 
    switch, unplug and remove the power supply, and remove the multimeter. 
19. Connect the +5V and ground points to the Arduino using short pieces of hookup wire
    and DuPont style connectors (push on). 
20. Connect the +24V and ground points to the CNC shield using heavier hookup wire 
    (#18-#14).  This is the stepper motor supply for the CNC board. 
21. Connect the +12V output from the converter to an open lug on the tie strip. 
22. Connect the +12V and ground leads to the rear fan.  You may wish to use a 3-pin 
    in-line plug/socket to allow for easier removal of the fan later.  Use of a 3-pin 
    plug/socket will prevent inadvertent connection to the power supply in the future.
23. Fabricate the ribbon cable from the LCD to the arduino and attach it to the LCD
    and plug it into the arduino.
24. Attach a 10K trim pot between pins 1, 2, and 3 of the LCD.  Solder it to the 
    back of the LCD using short leads of small hookup wire. The wiper of the trim 
    pot connects to LCD pin 3. Hot melt glue the trim pot to the back of the LCD 
    or in some location inside the covers that is easily accessed by removing the 
    top cover. 
25. Fabricate the cable from the rotary encoder (and switch) to the arduino. I 
    used 3-pin connectors to attach to each side of the rotary switch.  You could 
    also just solder the wire directly to the switch leads.  Use heat shrink tubing 
    to insulate eash pin if you do that.  Attach the other end to the arduino using 
    the appropriate DuPont style connector and pins. 
26. Insert the LM8UU linear bearing into the wire guide.  
27. Insert the ACME lead screw nut into the wire guide. 
28. Attach the guide bar sense switch to the wire guide and solder a 2-wire small 
    cable to it at C and NO (Normally Open) connections.  Attach the wire out sense
    switch to the guide bar tensioning assembly.  Also solder it at C and NO.  Leave 
    these cables long enough to reach from the far right end, through the motor 
    plate, and to the CNC shield plus extra to allow routing.  
29. Insert the tailstock bearings into the tailstock unit. 
30. Insert an M4 nut into the tail stock extrusion recess for clamping.  Thread an 
    M4 socket head screw into this nut sufficiently long to tighten against the 
    inside of the extrusion and not the tailstock plastic part. 
31. Slip the tailstock extrusion onto the right end of the front 2020 extrusion 
    with the bearings facing the spindle motor.  The tailstock shaft should 
    align with the spindle motor shaft. 
32. Place a coupler on the wire guide motor (NEMA 17) at the back of the motor 
    plate and tighten. 
33. Thread the ACME leadscrew through the leadscrew nut in the wire guide 
    about half way. 
34. Slide the wire guide linear bearing onto the top guide bar (8mm round) also 
    about half way. 
35. Insert the top guide bar into the top guide bar boss and hole in the motor 
    plate and the leadscrew into the coupler.  Tighten the coupler.
36. Insert the leadscrew bearing in the right end panel. 
37. Place the leadscrew through the right end panel support bearing and the top 
    guide in the top guide boss and hole on the right end panel.  Align the right
    end panel with the 2020 extrusions and attach using M6 screws, lock washers, 
    and flat washers. 
38. Attach the left and right limit switches to the motor and right end panels.
    Run the 2-lead hookup wire from both limit switches through the wire holes in the
    motor plate to the CNC board.  Use DuPont 2-pin single row connectors and female
    pins to attach the left limit switch to the X- input, and the right limit switch 
    to the X+ input. route the right end limit switch wire in the 2020 extrusion 
    channel and use the extrusion covers to hide the wire inside the rear extrusion.
39. Run the alignment bar sense wire and the wire out sense cable in the top channel
    of the extrusion cover with the wire guide at the far right hand extreme motion 
    (against the limit switch).  Secure the wires at the left end of the cover, then 
    route them through the motor end and to the CNC shield.  Connect the wire out 
    limit switch to Y- (or Y+) limit switch inputs on the CNC board, and the guide
    bar sense switch to the Z- (or Z+) limit switch input.
40. Assemble the wire guide tensioning assembly.  This uses a long M4 screw anchored
    into one end of the bracket, and then a stack of flat washers covered in felt
    padding, a spring, and thumb nut.  Wire for up to 5 filaments is then placed
    between pairs of felt-lined washers and the tension nut tightened or loosened 
    as needed to set the appropriate wire tension.
41. Attach all covers (front, top, and rear), making sure all connections are tight.
42. Connect a USB cable to the Arduino MEGA2560 through the rear cover and attach
    it to a host computer.
43. Compile the sketch and program the Arduino board. 
44. Once programming is complete, disconnect the USB cable and attach the 24V 
    power supply.  Plug it in and turn on the rocker switch.  The LCD backlight
    should illuminate and the display should guide you.     


## Usage

The general winding process is:

1. Ensure the alignment bar is fully retracted before any motion begins. The firmware will 
   block motion if the bar is not retracted, and the Y and Z sensors are used as safety 
   checks.  If the bar is not retracted, the system will prompt you to retract the bar
   before anything else can be performed.
2. Install the coil form, wire, and guide hardware and verify the machine moves freely.
   Make sure to install the correct form adapters for a round, rectangular, or square 
   form.  Also, ensure that the correct wire guide end is installed for 1, 2, 3, or 4 
   filament winding and the wire(s) are threaded on the appropriate tensioners and 
   pulleys.
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
9.  When the turns are completed, the machine will stop and allow you to create a tap
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


