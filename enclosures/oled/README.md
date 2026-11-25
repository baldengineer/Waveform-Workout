# Flush Adafruit 128×64 OLED lid

![Adafruit OLED glass seated flush in the trainer lid beside the command sticker](assembled.png)

This lid mounts the Adafruit 0.96-inch 128×64 STEMMA QT OLED (product 326) from the underside. The outer viewing surface and the broad top face share one plane. Both enclosure versions retain their original 86.8 × 51.6 mm footprint and height. The command sticker fits inside the engraved outline to the right of the glass. Cut it to 30 × 10 mm. [Adafruit module](https://www.adafruit.com/product/326) · [STEMMA QT revision board drawing](https://github.com/adafruit/Adafruit-128x64-Monochrome-OLED-PCB/blob/51ef8e242dcf460d5effc298d8565110624e6b23/Adafruit%200.96in%20128x64%20OLED%20STEMMA%20QT.brd)

- [Print the flush lid and RESET plunger](Flush%20OLED%20lid%20and%20reset.3mf)
- [Edit the SolidCraft assembly](Waveform%20Workout%20R1%20-%20Adafruit%20326.sc)

## Setting the glass height

Measure from the display-side surface of your breakout PCB to the finished viewing surface, with the temporary protective film removed. Set `glassRise` to this measurement before printing. The default **1.80 mm** is an assumed assembled height including adhesive. A larger value seats the board deeper. If the glass stands 0.15 mm proud on a first print, for example, increase `glassRise` by 0.15 mm. The four seating pads bear on the PCB around its mounting holes and leave the glass unloaded.

## Print and assembly

1. Print the supplied plate with a 0.4 mm nozzle, 0.20 mm layers and four perimeters. The exterior of the lid and the finger end of RESET face the bed. A smooth bed gives a smooth face beside the glass. Use a small brim for RESET and local support under its short flange if needed. Clear any first-layer flare from the window before fitting the glass.
2. Fit the headerless module from underneath. Point its eight-hole header row toward the nine settings indicators. Align the fourth mounting ear with the printed locating pin. Secure the other three holes with **M1.6 × 5 mm, 90° countersunk screws** with heads up to 3.0 mm across, and standard M1.6 hex nuts. Tighten lightly against the PCB. Check the glass with a straightedge across the lid.
3. Insert the new rectangular RESET plunger from inside. Its offset foot points toward the center of the lid and aligns with SW4. Transfer the original MODE and VALUE plungers and all eleven light guides. Their positions and lengths are shared with the [standard lid](../README.md#fit-and-assembly).
4. With power disconnected, connect a flexible four-wire STEMMA QT cable between the module socket nearest the sticker and trainer J6. Seat both plugs before closing the case. Dress the leads clear of RESET, the case pillars and the glass. Fit the original frame and four case screws. Check all three buttons for a free click and release. OLED operation requires firmware that powers J6 and drives the display over I²C.

## Editable dimensions

X and Y below are lid coordinates in millimetres. The origin is the center of the PCB outline. Positive X points toward USB-C and positive Y toward the power selector.

| Parameters | Default (mm) | Effect |
| --- | --- | --- |
| `glassRise` | 1.80 | Distance from the display PCB seating plane to the lid top |
| `windowW`, `windowH` | 27.30, 19.76 | Through-opening for the entire 26.70 × 19.26 mm panel |
| `windowX`, `windowY` | −12.295, 3.897 | Opening center, independent of the mounting pattern |
| `pitchX`, `pitchY` | 24.13, 26.67 | Hole spacing |
| `holeX`, `holeY` | −12.295, 5.325 | Center of the three screw locations and fourth locating pin |
| `mountBore`, `headDia` | 1.80, 3.20 | Fastener clearance and countersink diameters |

The SolidCraft assembly includes the USB tray and reference bodies for the module and screws. The 3MF contains only the replacement lid and RESET plunger.

## Verification

The modeled glass and lid top agree within 0.001 mm. The enclosure and reference bodies passed the assembly interference check. The upper assembly clears all 173 solids in the populated trainer export. The three nut envelopes and the module-side plug approach also clear the assembly. With RESET displaced 0.55 mm, its foot contacts SW4 and clears the other modeled board components. Both print meshes are watertight. Actual glass alignment, cable routing and button feel still need a physical fit check.
