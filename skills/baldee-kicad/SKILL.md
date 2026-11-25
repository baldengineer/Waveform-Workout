---
name: baldee-kicad
description: Apply BaldEE's design and drafting preferences when creating, editing, or reviewing KiCad schematics, PCB layouts, BOMs, and component models for BaldEE's projects.
---

# BaldEE's KiCad preferences

Use the sections relevant to the requested change. Follow the current project's instructions and BaldEE's latest directions where they are more specific.

## Schematic drawing

- Prefer `_Small` passive symbols where a matching variant exists, such as `Device:R_Small` and `Device:C_Small`.
- Draw a connected wire under the full width of each horizontal net label. Align the wire ends of neighboring labels when practical.
- On each new sheet, put its title in the title block's `Title` field and a one-sentence description of its purpose in `Comment2`. Use bold 64 mil (1.6256 mm) text for circuit-function headings on the drawing.
- For a discrete LED with a series resistor, prefer the resistor on the anode side and the cathode directly on GND. The marked cathode then doubles as a ground point at the bench.

## Parts and sourcing

- For prototypes intended for rework, prefer 0603 (imperial) or larger passives and the matching KiCad `HandSolder` footprint when available.
- When sourcing from LCSC, record the catalog number on the symbol and flag parts without a stocked match. For JLCPCB assembly, look first for in-stock Basic parts that meet the electrical and footprint requirements. For Extended alternatives, compare the cost at the assembly quantity, including any fee or minimum per distinct part number. Check assembly stock and Economy or Standard eligibility separately from LCSC retail stock.
- For low-cost, single-wire addressable RGB indicators, consider the Worldsemi `WS2812B-2020-V6` (2 × 2 mm, LCSC `C52917434`) first. Check the board's supply range and the assembly service before choosing it. [LCSC part](https://www.lcsc.com/product-detail/C52917434.html) · [JLCPCB assembly listing](https://jlcpcb.com/partdetail/Worldsemi-LED/C52917434)
- Maintain the two schematic link fields independently. Keep both hidden on the drawing. An assignment or change to either identifier updates its own link even when the other identifier is blank.

| Identifier | Hidden field | Value |
| --- | --- | --- |
| `LCSC` | `lcsc_link` | `https://www.lcsc.com/product-detail/{LCSC}.html` |
| `MPN` | `Octo_Link` | `https://octopart.com/search?q={encoded MPN}` |

Encode the exact MPN as a URL query value, including all suffixes. For example, a literal `+` becomes `%2B`.

## Layout and probe access

- For ICs with perimeter pads, escape through the outer end of each signal pad. Keep non-ground tracks and vias outside the IC body and pin field on every copper layer after the escape. Use relatively wide traces where impedance and pitch permit.
- On four-layer boards with dedicated power distribution, use `In1.Cu` for GND and `In2.Cu` for system power. Put bypass capacitors close to their supply pins. Give each surface-mount GND or supply pad a short connection to a nearby plane via. When adjacent same-net pads can share a via, use a local copper pour that overlaps the pads and the via. Plated through-hole pads can connect directly to the plane.
- On development boards, leave room for the probe or clip and its silk label. Pair each power-rail test point with a nearby GND test point. Consider test access to interesting signal nodes and unused multifunction MCU pins. For a physical clip point, prefer the Harwin loop in BaldEE's personal library when available.

## Silkscreen for debugging

- Use one text size across front and back silkscreen. Use editable KiCad text with the native **Knockout** flag for GND labels. Mirror back-side labels so they read from the solder side. Leave Knockout off for reference designators by default.
- Mark a grounded passive terminal with a filled silkscreen rectangle beside the pad. Use a short ground bar beside grounded pins on ICs where space permits. Label through-hole pins on the solder side and outline ground pins there with a polygon.
- Hide test-point reference designators and leave a visible signal or GND label at each probe site. When moving footprints, move their board-level labels and markings with them. Check both sides for stale labels and overlap, including labels hidden under installed parts.

## Files and handoff

- Keep component STEP models in `step_models/` beside the KiCad project (`kicad/step_models/` in the trainer). Reference them as `${KIPRJMOD}/step_models/<filename>`. Commit agent-generated models with `_clanker` immediately before the extension. Git-ignore downloaded models and record their exact local filenames and source links in the model directory's tracked `README.md` so another checkout can restore them.
- Generate schematic and PCB PDFs only on explicit request. In projects using the trainer's Git workflow, commit completed, verified changes automatically and keep each commit scoped to the task. For schematic cleanup intended to preserve circuit behavior, compare pin-to-net connectivity as well as running ERC.
- When asked for a bench test plan, keep it concise and put signal names and key measurement values in `inline code`. For a printable worksheet, leave checkboxes and space for actual readings.

## Oscilloscope Trainer conventions

- Keep the `In2.Cu` system VDD distribution downstream of the current-measurement jumper. Preserve the measurement path when joining pours.
- Hide resistor, capacitor, and LED designators. Print the LED and pushbutton functions using native Knockout text. Keep each label beside the control or indicator it identifies. Omit ground bars on the trainer's TQFP MCU.
- For the trainer's Harwin GND loops, place at least two vias on each side of the footprint, preferably three per side where space permits.
