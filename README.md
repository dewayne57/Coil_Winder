# Coil_Winder

Arduino Mega 2560 coil-winder sketch with a 4x20 LCD and 4x4 keypad driven menu system.

## Included sketch

- `/home/runner/work/Coil_Winder/Coil_Winder/Coil_Winder.ino`

The sketch provides menu-based entry and review screens for:

- wire size
- form shape (`Round`, `Rectangular`, `Oval`)
- form diameter or width/height depending on shape
- target mode (`Turns` or `Inductance`)
- target turns or target inductance
- number of layers
- winding mode (`Single`, `Bifilar`, `Multi-filar`)
- filar count for multi-filar windings

## Controls

The sketch expects a 4x4 keypad with this layout:

| 1 | 2 | 3 | Up |
|---|---|---|----|
| 4 | 5 | 6 | Down |
| 7 | 8 | 9 | Yes |
| . | 0 | No | Enter |

- `Up` / `Down`: move through the menu
- `Enter`: edit or confirm a numeric field
- `Yes`: advance choice fields or save numeric edits
- `No`: reverse choice fields, erase a digit while editing, or reset on the review screen
- `0-9` / `.`: direct numeric entry for editable fields

## Hardware defaults

The sketch is configured for:

- LCD pins: `RS=22`, `EN=23`, `D4=24`, `D5=25`, `D6=26`, `D7=27`
- keypad row pins: `30, 31, 32, 33`
- keypad column pins: `34, 35, 36, 37`

Adjust those constants in `Coil_Winder.ino` to match the actual wiring.
