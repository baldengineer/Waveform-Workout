# Development PCB

The board measures 110 × 70 mm. The left edge carries output A on the Kinghelm BNC. The parallel 2.54 mm header sits 8.5 mm behind its signal pin. The USB-C receptacle faces out of the right edge. Each power test point has a ground point 7.5–8 mm away. The four waveform test loops are on 12 mm centers.

## Routing and stack

Use JLC04161H-7628, 1.6 mm FR-4 with 1 oz outer and 0.5 oz inner copper. L1 and L4 carry signals. L2 is ground, and L3 is system VDD downstream of the current-measurement jumper. The fabrication note on the PCB identifies the intended stack. Select it explicitly when ordering. [JLCPCB stackups](https://jlcpcb.com/impedance)

The general routing width is 0.30 mm and the power width is 0.50 mm. Through vias are 0.60 mm with 0.30 mm drills. Local neck-downs serve fine-pitch pads. The custom DRC rules cover copper clearance, drills, annular rings, routed edges and silkscreen. [JLCPCB rigid-board capabilities](https://jlcpcb.com/capabilities/pcb-capabilities)

The USB pair stays on L1. Its main coupled section uses 0.3134 mm traces and a 0.250 mm edge gap over L2, targeting 90 Ω differential on the specified stack. Both MCU-to-ESD paths measure 16.09 mm. The connector escape connects both plug orientations. [JLCPCB impedance calculator](https://jlcpcb.com/pcb-impedance-calculator)

## First-pass checks

KiCad found zero unrouted connections and zero shorts. The filled L2 plane covers the centerline and both edges of all four USB data nets at 0.05 mm sampling intervals.

Four DRC errors remain inside J1. Each USB-C locating hole is 0.1944 mm from two overlapping ground pads, against the project's 0.25 mm hole-to-copper setting. Check the GCT land pattern and the fabricator's allowance before fabrication. [GCT USB4105 drawing](https://mm.digikey.com/Volume0/opasdata/d220001/medias/docus/5492/USB4105.pdf)

Of the 234 warnings, 120 concern silkscreen, 30 concern footprint-library differences and 84 concern the missing schematic `Group` field on PCB footprints. The PCB transfer also omitted the library 3D models for the USB-C receptacle, battery clip, mode button and Harwin loops. Check simultaneous probe fit on a physical mock-up before fixing the board dimensions.
