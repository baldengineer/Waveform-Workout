# Development PCB

## PP3 routing trial

KiCad reports zero unconnected items, shorts, schematic mismatches, dangling tracks or dangling vias. The board has 666 track segments on F.Cu and B.Cu and 120 vias. All 92 footprints retain their first-pass positions.

In1.Cu carries GND and In2.Cu carries VDD downstream of JP1. All 90 surface-mount rail pads have a local connection to their plane, and all 10 plated through-hole rail pads contact their plane. The MCU-to-ESD USB data traces each measure 17.022 mm. The filled GND plane covers the full width of both traces and their connector branches. The IC escape check found no non-ground tracks or vias intruding into another pin’s escape or passing beneath an IC body.

Track and via copper clear the assumed 5 mm PP3 screw heads. The closest track edge is 2.559 mm from a mounting-hole center and the closest via edge is 2.661 mm away. A legacy front GND pour extends beneath the head at (135.745, 64.335) mm, reaching to within 1.901 mm of the hole center.

Four DRC errors remain inside USB-C connector J1. The locating holes are 0.1944 mm from the adjacent ground pads, against a 0.25 mm project rule. Review the land pattern and fabrication allowance before release. The 206 warnings comprise 167 silkscreen and text findings, 27 library-footprint differences, eight isolated-copper findings and four copper slivers. The board retains 25 legacy front-zone outlines alongside the 22 new local pours. Review those outlines and the mounting-hole copper before fabrication.

## PP3 placement checkpoint

The 92-footprint board places the Keystone 1294 on the back at (125.20, 76.25) mm with its terminals toward the BNC. Its four 3.3 mm mounting holes clear the existing via drills. Placement allows a 5 mm screw-head diameter. Actual fasteners and enclosure fit still need a mechanical check. J2 (BNC) and J1 (USB-C) share the board centerline at Y = 75.750 mm. The two DAC filter inductors sit on opposite sides of U2 with perpendicular axes.

At placement commit `7f638a6`, the board matched the schematic and passed courtyard-overlap and hole-to-hole checks. It retained the previous layout’s 677 track segments and 130 vias. DRC reported 100 unconnected items and 109 shorts before rerouting.

## Previous CR2032 layout

The dimensions, routing results, and checks below describe the earlier CR2032 revision.

The board measures 81.766681 × 37.50 mm. Shortening the 50.40 mm outline by 12.90 mm saved 1,054.79 mm², a 25.60% reduction to 3,066.25 mm². BT1 mounts on the back beneath the analog signal path and the two buffered DAC test points. The CR2032 slides out toward the top edge through a 20 mm-wide path clear of vias and through-hole leads. [Keystone 3002 retainer drawing, Fig. 2](https://www.keyelco.com/userAssets/file/K75p11.pdf)

The left-edge Kinghelm BNC (J2) and right-edge USB-C (J1) share a mating centerline at Y = 75.750 mm, 18.750 mm from each long board edge. The 2.54 mm header behind the BNC also carries output A. BT1 sits at (107.10, 68.00) mm on the back, clear of the BNC mounting feet and signal lead. The power selector is 7.33 mm from the USB connector in X and 16.00 mm in Y, measured between footprint origins. The six rail measurement points have ground points 6.17–7.97 mm away.

MODE and VALUE sit side by side on 8 mm centers. The six mode indicators and three value indicators form a single row, with 3.6 mm pitch within each group and a wider gap between groups. Together their 0805 courtyards occupy 32.70 × 3.36 mm. D10 indicates USB power separately.

### Routing and stack

The fabrication target is JLC04161H-7628, 1.6 mm FR-4 with 1 oz outer and 0.5 oz inner copper. The top and bottom layers carry signals. In1.Cu is GND, and In2.Cu is system VDD downstream of JP1. The KiCad export uses generic dielectric thicknesses. Use the specified fabrication stack when checking impedance and ordering. [JLCPCB stackups](https://jlcpcb.com/impedance)

The general routing width is 0.30 mm and the power width is 0.50 mm. Through vias are 0.60 mm with 0.30 mm drills. Local neck-downs serve fine-pitch pads. The custom DRC rules cover copper clearance, drills, annular rings, routed edges and silkscreen. [JLCPCB rigid-board capabilities](https://jlcpcb.com/capabilities/pcb-capabilities)

All 75 surface-mount VDD and GND pads connect locally to their planes. Each STM32 supply pin shares a top-layer pour and plane via with its bypass capacitor. BT1’s central negative contact reaches In1.Cu through a back-layer trace and a via outside the holder. Nine plated through-hole pads connect directly to the planes.

The USB pair stays on L1. Its main coupled section uses 0.3134 mm traces and a 0.250 mm edge gap over L2, targeting 90 Ω differential on the specified stack. A matched pair of 45° bends lines up the pair with the centered connector and ESD protector. Both MCU-to-ESD paths measure 17.02 mm. The connector escape connects both plug orientations. [JLCPCB impedance calculator](https://jlcpcb.com/pcb-impedance-calculator)

### Layout checks

KiCad found zero unrouted connections and zero shorts. The filled In1.Cu ground plane covers the full trace width of all four USB data nets. Across U1–U6 and Q1, non-ground tracks leave through their own pad ends, and non-ground vias clear the IC bodies and pin fields.

The eight front and six back GND labels use editable KiCad text with Knockout enabled. The back labels are mirrored for reading from the solder side.

Four DRC errors remain inside J1. Each USB-C locating hole is 0.1944 mm from two overlapping ground pads, against the project's 0.25 mm hole-to-copper setting. Check the GCT land pattern and the fabricator's allowance before fabrication. [GCT USB4105 drawing](https://mm.digikey.com/Volume0/opasdata/d220001/medias/docus/5492/USB4105.pdf)

The schematic and PCB agree on all 86 footprint assignments and all 248 connected physical pads. KiCad reports 31 silkscreen clipping or overlap warnings, 22 reference-text thickness warnings and 29 differences from library footprints. Another 86 warnings concern the schematic’s `Group` field, which is absent from the PCB footprints. Both pushbuttons have their library 3D models. Battery replacement in an enclosure and simultaneous probe fit still need physical checks.

When editing placement, move the associated board-level silkscreen markings with their footprints. The two inner-plane construction outlines extend beyond the finished board in the editor. KiCad clips their filled copper to the board-edge clearance.
