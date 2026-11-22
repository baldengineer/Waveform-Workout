# Development PCB

The board measures 110 × 56 mm. Output A is on the left-edge Kinghelm BNC, with its parallel 2.54 mm header 8.5 mm farther inboard. USB-C faces the right edge. Each of the six power measurement points has a ground point 7.5–8.0 mm away. The four waveform test pads, TP22–TP25, are on 12 mm centers above the indicator LEDs.

## Routing and stack

The fabrication target is JLC04161H-7628, 1.6 mm FR-4 with 1 oz outer and 0.5 oz inner copper. The top and bottom layers carry signals. In1.Cu is GND, and In2.Cu is system VDD downstream of JP1. The KiCad export uses generic dielectric thicknesses. Use the specified fabrication stack when checking impedance and ordering. [JLCPCB stackups](https://jlcpcb.com/impedance)

The general routing width is 0.30 mm and the power width is 0.50 mm. Through vias are 0.60 mm with 0.30 mm drills. Local neck-downs serve fine-pitch pads. The custom DRC rules cover copper clearance, drills, annular rings, routed edges and silkscreen. [JLCPCB rigid-board capabilities](https://jlcpcb.com/capabilities/pcb-capabilities)

All 77 surface-mount VDD and GND pads connect locally to their planes. Seventeen shared top-layer pours join adjacent pads to a via, including each of the five STM32 supply pins and its bypass capacitor. The other 42 pads have individual taps. Eight plated through-hole pads connect directly to the planes.

The USB pair stays on L1. Its main coupled section uses 0.3134 mm traces and a 0.250 mm edge gap over L2, targeting 90 Ω differential on the specified stack. Both MCU-to-ESD paths measure 16.09 mm. The connector escape connects both plug orientations. [JLCPCB impedance calculator](https://jlcpcb.com/pcb-impedance-calculator)

## Layout checks

KiCad found zero unrouted connections and zero shorts. The filled In1.Cu ground plane covers the full trace width of all four USB data nets. Across U1–U6 and Q1, non-ground tracks leave through their own pad ends, and non-ground vias clear the IC bodies and pin fields.

Four DRC errors remain inside J1. Each USB-C locating hole is 0.1944 mm from two overlapping ground pads, against the project's 0.25 mm hole-to-copper setting. Check the GCT land pattern and the fabricator's allowance before fabrication. [GCT USB4105 drawing](https://mm.digikey.com/Volume0/opasdata/d220001/medias/docus/5492/USB4105.pdf)

The 122 warnings match the 110 × 70 mm checkpoint. Eight concern silkscreen on BT1, TP1 and TP2, 30 concern footprint-library differences, and 84 concern the schematic `Group` field on PCB footprints. The USB-C receptacle, battery clip, mode button and Harwin loops lack their library 3D models on this board. Simultaneous probe fit still needs a physical check.

When editing the placement, move the silkscreen markings for the 33 relocated groups with their footprints. Those markings are now independent board graphics. The two inner-plane construction outlines extend past the lower board edge in the editor. Their filled copper stops 0.20 mm inside the 56 mm outline.
