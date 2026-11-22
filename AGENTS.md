# Project workflow

Commit completed, verified changes to this project automatically. Keep each commit scoped to the task.

## Schematic part numbers

Maintain each catalog link independently when assigning or updating schematic part numbers. Keep both link fields hidden on the drawing.

- When `LCSC` is assigned or changed, set `lcsc_link` to `https://www.lcsc.com/product-detail/{LCSC}.html`, even if `MPN` is empty.
- When `MPN` is assigned or changed, set `Octo_Link` to `https://octopart.com/search?q={encoded MPN}`, even if `LCSC` is empty. URL-encode the exact manufacturer part number, including any literal `+` as `%2B`.

## Schematic LEDs

- In a series circuit, prefer connecting the LED cathode directly to GND. Place any series resistor on the anode side so the marked cathode pad on the PCB provides an easy-to-find ground point for debugging.

## PCB silkscreen

- Make GND labels with editable KiCad silkscreen text and enable the native Knockout flag. Mirror back-side labels so they read from the solder side.
- Leave Knockout off for reference designators by default.

## PCB routing

- Fan IC signals outward from their own pads. Keep non-ground tracks and vias outside the IC body and pin field on every copper layer. A pad's own escape should leave through its outer end.
- Use In1.Cu for GND and In2.Cu for system VDD downstream of the current-measurement jumper.
- Connect surface-mount VDD and GND pads to the matching plane with a nearby via and a short pad-to-via connection. A plated through-hole pad can connect directly to the plane.
- When nearby same-net pins, bypass pads, or other features share a plane connection, join them with a local copper pour that overlaps the pads and via. Keep power distribution on the inner planes.
