# Waveform Workout Rev 1 board test plan

[Printable bench worksheet with fillable measurement fields](../output/pdf/Waveform%20Workout%20Rev%201%20bench%20worksheet.pdf)

Use one board for the full sequence. Screen the remaining boards with the lot checklist at the end. Match the assembly to the order paperwork and Rev 1 BOM (hardware commit `0b94bae`). [Assembly BOM](production/bom.csv) · [Hand-fit notes](LCSC%20BOM.csv)

**Bench setup**

- Current-limited supply or SMU, DMM, magnifier, `JP1` shorting cap and a `9 V` alkaline PP3.
- Two-channel scope with FFT, DC coupling and `1 MΩ` inputs. Connect `CH1` to BNC `J2` (A). Use a compensated `10×` probe on `TP3` (B), ground at `TP4`. Remove any `50 Ω` terminator.
- `ST-LINK` with keyed `1.27 mm` Cortex adapter, USB-C data cable and a low-current `3.45 V` STEMMA QT accessory with a known ID register.
- Record diagnostic and trainer image versions. Without firmware, complete inspection, rails, reset and SWD identification. Mark dependent tests `Blocked`.

![Rev 1 probe map, component side. BNC at left, USB-C at right.](../report/rev1-probe-map.svg)

## 1. Unpowered inspection

- [ ] Label the board and photograph both sides. Inspect IC leads and orientation, `D11` polarity, all nine pixels, `J1`, the `J3` key and probe loops. Check both filters for `1 mH` Bourns coils and `47 pF` tank capacitors.
- [ ] Hand-fit rear BNC `J2`. Check center → `R5-2` and shell → `TP4`. Mount `BT1` with insulating adhesive. Check positive socket → square `BT1-1/J5-1` and negative stud → `BT1-2/J5-2`. Leave the cell out.
- [ ] Unplug all cables. Check continuity among the GND loops, `J5-2` and BNC shell. Measure `TP5`, `TP9` and `TP7` to GND. **Hold** for a persistent near-short in both meter polarities after settling.
- [ ] Check `SW1` continuity. In `BAT`, `TP11` connects to `TP9`. In `USB`, it connects to `TP5`. The two inputs stay isolated.
- [ ] **Check `R4`.** Assembly BOM lists `10 Ω`, sourcing BOM says `DNP`. Remove the `JP1` cap and meter `TP1` to `TP2`. Expect `≈10 Ω` if fitted or high resistance if empty. Investigate a short. Record and refit the cap.

## 2. First power

1. Leave `BT1`, `J1`, `J3` and `J6` empty. Supply **off**, wire `J5-1` positive and `J5-2` return. Select `BAT`, hold `RESET` (`SW4`) and scope `TP7`.
2. Start at `5.0 V` with a `20 mA` limit. Record input current and the readings below. Release `RESET` and repeat. Once stable, repeat at `9.0 V`. Save the first board’s `TP7` startup trace.

| Probe / ground | Expected |
| --- | --- |
| `TP5 / TP6` · `TP9 / TP10` | USB VBUS at `TP5`, battery input at `TP9`. Check unused input for backfeed. |
| `TP11 / TP12` | Selected input at switch common. |
| `TP1 / TP12` | Selected input minus `F1` and `D11` drops. `D11-1` is the cathode. |
| `TP2 / TP8` | Equals `TP1` with `JP1` shorted. Feeds `U7` input and enable. |
| `TP7 / TP8` | `3.447 V` nominal. Proposed screen `3.35–3.55 V` at room temperature with `TP2 ≥3.95 V`. Confirm at `C4-1` (VDDA), `C5-1` (USB supply) and `J3-1`. |

**Cut power** for sustained current limiting, heating, oscillation, settled VDD outside the screen, or any VDD excursion to `≥3.6 V`.

- Trace the first bad node. If `TP2` is good, check `U7`, `R27/R28` (`178 kΩ/100 kΩ`) and VDD loading. [Regulator](https://www.ti.com/lit/ds/symlink/tps7a24.pdf#page=6) · [MCU](https://www.st.com/resource/en/datasheet/stm32l072cb.pdf)
- Keep LED and accessory loads below `F1`’s `50 mA` hold rating at `20 °C`, with margin. [Fuse](https://www.littelfuse.com/assetdocs/littelfuse-ptc-1206l-datasheet?assetguid=2b6a1515-d4ee-4c83-8bd4-152b4901b8f5)

| Single source connected | `SW1 = USB` | `SW1 = BAT` |
| --- | --- | --- |
| `J1`, current-limited `5 V` | VDD regulates, `D10` on | VDD decays, `D10` stays on |
| `J5` only | VDD decays, `D10` off | VDD regulates, `D10` off |

If the unused input floats, repeat the backfeed check with `10 kΩ` to GND.

## 3. Reset and programming

- [ ] Power off and attach `ST-LINK` to `J3` with probe power disabled. Use `1 VTref`, `2 SWDIO/PA13`, `4 SWCLK/PA14`, `10 NRST`, `3/5/9 GND` and `7 key`. Repower through `SW1`. Check `VTref = TP7`.
- [ ] `BOOT0` at `R1-1` is low. `J3-10` is near VDD when released, near `0 V` with `RESET` held. While held, both outputs are low and `J6` power is off.
- [ ] Read `STM32L072` family ID, `128 KiB` flash size and unique ID. Record existing image ID before flashing. Program and verify the diagnostic on the test board. If attach fails, check VTref/key/reset and retry slowly under reset.
- [ ] Power off, detach `J3` and cold-boot. Diagnostic starts with outputs and pixel data low, QT off. Green `D12` blinks and restarts on `RESET`. Probe `PA2/R26` for a missing blink.

## 4. Controls, USB and accessory port

| Check | Action and pass condition |
| --- | --- |
| `MODE` / `VALUE` | Enable pull-ups. Press, hold and release `SW2/PB14` then `SW3/PB12`. Each reads high → low → high and advances only its own counter once. |
| RGB chain | Send all-dark, then walk one dim red, green and blue pixel per position in the order below. Only the addressed pixel lights. Check data at `TP21/PB15` and the first failed link at upstream `DOUT-1` / downstream `DIN-3`. |
| USB-C data | Power from `J5` in `BAT`. Plug and unplug `J1` in both orientations. `USB_VBUS_N` (`R10-2/PA15`) goes low with VBUS, high without. Log expected enumeration and a successful data transfer in each orientation. |
| STEMMA QT power | At `J6`, use `1 GND` and `2 switched VDD out`. Fit `10 kΩ` across them. With data lines released, pin 2 discharges in reset. Drive `PB5` low to enable VDD. |
| STEMMA QT data | Use `3 SDA/PB7` and `4 SCL/PB6`. With power enabled, both idle high. Read the known register at `100 kHz` open-drain. Record address and matching ID. Release lines, drive `PB5` high and verify pin 2 discharges. |

**Pixel order**  `D9 VAL3` → `D8 VAL2` → `D7 VAL1` → `D6 TRIG` → `D5 XY` → `D4 DC` → `D3 FREQ` → `D1 SHAPE` → `D2 AMP`.

**USB threshold on the first board**

1. Keep PP3 out, `SW1` on `BAT` and host connected. Scope `C5-1` (USB supply) and `U6-4` (D+).
2. Slowly reduce `J5` from nominal until VDD crosses `3.0 V`, then restore it. Capture D+ pull-up release above `3.0 V`, reattach at `≥3.1 V` and successful enumeration. Unplug `J1`.

## 5. Output routing

Drive both GPIOs and buffered DACs simultaneously. Use `50%` duty on the GPIOs. Capture both connectors for each selector pair. Changing one selector should leave the other output unchanged.

| `A_SEL` | `B_SEL` | A · `J2` | B · `TP3` |
| --- | --- | --- | --- |
| `0` | `0` | `1 kHz` | `2 kHz` |
| `0` | `1` | `1 kHz` | `1.00 V` |
| `1` | `0` | `0.50 V` | `2 kHz` |
| `1` | `1` | `0.50 V` | `1.00 V` |

- **Pass** — DC within `±25 mV` (proposed). GPIO low `<0.1 × VDD`, high `>0.9 × VDD`.
- **Trace A** — GPIO `PB13/TP25`, DAC `PA5/DAC2 → R30-1 → L2 filter → TP23`, select `PB11/R8-1`, output `U3-10 → R5 → J2`.
- **Trace B** — GPIO `PA1/R35-1`, DAC `PA4/DAC1 → R29-1 → L1 filter → TP22`, select `PA0/TP24`, output `U3-6 → R6 → TP3`. [Probe points](Test%20points.md) · [Switch pinout](https://www.ti.com/lit/ds/symlink/ts5a23157.pdf#page=16)

## 6. Trainer exercises

Flash and verify the trainer image. Cold-boot with `J3` detached. Use `MODE` for the row and `VALUE` for the setting. Row/value indicators light for `≈2 s` and playback continues after timeout.

| Setting | Expected at the scope |
| --- | --- |
| Startup | Matching `1 kHz` sines on A/B, `1.00 Vpp` centered on `+0.75 V`. |
| `SHAPE`, `AMP`, `FREQ` | Cycle sine/square/triangle, `0.2/0.5/1.0 Vpp`, and `100 Hz/1 kHz/10 kHz` (`10 ms/1 ms/100 µs`). Change one setting at a time and verify the other two hold. |
| `DC` | DMM on A and B reads `0.25/0.50/1.00 V` after settling. Scope for ripple or oscillation. |
| `XY` | Equal `1 kHz` sines at `0°/90°/180°` give `0/250/500 µs` separation. Center equal scales and save the rising line, circle and falling line. |
| `TRIG` narrow | A normally `50 µs` high at `10 kHz`. Once in `1,000` periods (`100 ms`), B marks a `2 µs` A pulse. Target B marker `10 µs`. Confirm on B rising, then trigger on A positive width `<5 µs`. |
| `TRIG` variable | A high for `10/40/80 µs` in successive `100 µs` periods. B marks the group start. A `30–50 µs` width trigger selects the middle pulse. |
| `TRIG` missing | One A pulse missing per `1,000`. B marks its expected edge. A rising edges span `200 µs`, low time `150 µs`. Trigger on A low `>100 µs`. |

**Proposed limits** — amplitude `±5%`, DC/mean `±25 mV`, period `±2%`, phase `±5°`, pulse width `±10%`. For rare pulses, capture `≥200 ms` at a rate that resolves `2 µs`. Measure width at half-height and record pulse height and B-marker skew.

## 7. Fast sine, current and fit

**`250 kHz` acceptance on each output, on USB and `9 V` battery emulation**

1. Select the raw-table diagnostic and set `A_SEL = B_SEL = 1`. Run both buffered DACs at `1 MSample/s`, repeating `1457, 1457, 324, 324`. Record `VDDA` at `C4-1` and cable/probe load. Compare `TP23 → J2` and `TP22 → TP3`. Save pre-trim and calibrated captures.
2. Acquire `≥1,000 cycles` at `≥50 MSample/s`. Use coherent FFT or a corrected Hann window with integrated tone power. Record trace Vpp, fundamental, mean, THD, clipping and underruns. Report a noise-floor bound for unresolved images.

| Fundamental | DC mean | Images relative to fundamental |
| --- | --- | --- |
| `0.95–1.05 Vpp` | `0.725–0.775 V` | `750 kHz` and `1.25 MHz` each `≤−30 dBc` |

The simulation assumes `1 MΩ ∥ 100 pF` and a different coil model. [Reference traces](../report/README.md) · [Filter measurements](DAC%20reconstruction%20filter%20test%20plan.md)

**Current and mechanical checks**

For battery-current tests, remove PP3, USB, debugger, QT and bleed load. Keep scope loads connected, select `BAT` and short `JP1`.

| Measurement | What to record |
| --- | --- |
| Battery current at `J5` | On trainer firmware, log idle, DC, dual `1 kHz`, XY and trigger with LEDs dark, then the indication peak. Compare dark current with the original `1 mA` goal. Select `USB` with cable out for off current. Log dual `250 kHz` separately on its diagnostic. |
| Depleted PP3 trial | At `6.0 V` on `J5`, repeat the fast sine. Trigger on the brightest intended two-pixel indication and timeout. Record `TP2` headroom, minimum `VDDA`, output disturbance, recovery and resets. |
| Regulator current, `R4` fitted | Remove the `JP1` cap with power off. Use a differential probe or matched, zeroed channels with **both grounds on GND**. Calculate `(TP1 − TP2) / R4` and check `TP2` headroom. At `10 Ω`, `100 mV = 10 mA`. |
| Regulator current, `R4` empty | With power off, replace the `JP1` cap with a fused ammeter. Both regulator-current methods exclude direct VBUS loads such as `D10`. |

- [ ] **Fit.** With power off, restore the `JP1` cap. Restore trainer firmware, disconnect `J5` and install the PP3. Lock the BNC coupler, clip B and ground, and press both buttons. Confirm battery and adjacent-input access and uninterrupted outputs.

## Lot screen and per-board record

Keep the first board’s settings, loads and captures as the lot reference. Repeat the full plan after a relevant substitution or repeatable failure.

**Board / lot** ______  **Operator / date** ______  **Image IDs** ______  **`R4 / JP1`** ______  **Scope / probes / load / ambient** ______

| Check | Boards to test | Readings / capture | `Pass`, `Fail` or `Blocked` |
| --- | --- | --- | --- |
| Inspection, grounds, isolation | Every board | | |
| `TP11/TP1/TP2/TP7` and input mA at `J5 5 V`, `J5 9 V`, `USB 5 V` | Every board | | |
| SWD ID, flash verify, cold boot and reset | Every board | | |
| Buttons, RGB walk, USB both ways, QT ID | Every board | | |
| Four A/B routes, startup sine, `2 µs` pulse + B marker, PP3 operation | Every board | | |
| USB thresholds, full presets, XY and trigger timing | First board | | |
| Fast sine, current and transients | First board, record per supply/channel | | |

**Disposition** ______  **Fault node / rework / retest result** ______
