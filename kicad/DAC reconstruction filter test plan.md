# DAC reconstruction filter test plan

The prototype target is a 250 kHz sine with a 1.00 Vpp fundamental, centered at 0.75 V, into a 1 MΩ scope input. Run both DAC channels at 1 MSample/s. Record the actual waveform Vpp as well as the fundamental because a sampling image can change the peak reading. This is a bench proposal. All predictions below are calculated.

## Circuit under test

Build the same network for PA4 and PA5. Enable the STM32 DAC output buffers. Connect the TLV9002 as a voltage follower and take the output to the existing analog switch.

```text
                         +------- 47 pF -------+
                         |                     |
DAC pin -- 5.6 kΩ --o----+------- 1 mH --------+----o---- TLV9002 (+)
                   |                               |
                  22 pF                           68 pF
                   |                               |
                  GND                             GND
```

Use a 1% resistor and C0G/NP0 capacitors. The candidate coil is the Coilcraft 1812LS-105XJRC, nominally 1 mH at 790 kHz, with typical Q of 30 at that frequency. The parallel tank sits **in series** with the signal. It passes DC through the winding and presents a high impedance around 750 kHz. The two grounded capacitors attenuate additional images. Place the resistor at the DAC pin so that it isolates the DAC from the filter capacitance. Include probe and trace capacitance when checking the DAC’s 50 pF direct-load limit. [Coilcraft 1812LS specifications](https://www.coilcraft.com/en-us/products/rf/ferrite-core-chip-inductors/1812-%284532%29/1812ls/) · [ST DAC load, output swing and settling, Table 67](https://www.st.com/resource/en/datasheet/stm32l072cb.pdf#page=107)

## Predictions for the first build

| Quantity | Nominal estimate |
| --- | ---: |
| Passive filter gain at 250 kHz | −1.4 dB |
| DAC hold loss at 250 kHz | −0.91 dB |
| Follower gain at 250 kHz | −0.26 dB |
| 750 kHz component at follower output, relative to 250 kHz | −41 dBc |
| 1.25 MHz component at follower output, relative to 250 kHz | −33 dBc |

These estimates use an ideal DAC hold, a low-impedance source, 75 pF total at the follower input, and a single-pole 1 MHz follower. The approximate coil model uses 820 µH with 9.65 pF winding capacitance from Coilcraft’s model and an effective series loss of about 129 Ω near the notch (Q ≈ 30). With 47 pF fitted, this model puts the notch near 738 kHz. The exact center and depth depend on the part and layout. The switch, external load and the DAC’s large-step settling are additional bench variables. TI specifies typical bandwidth and slew rate at 5 V. [Coilcraft model, 1812LS-105](https://www.coilcraft.com/getmedia/f7d0afe6-7e6d-4f7c-8183-7c22379d6d80/spice_1812ls.pdf) · [TLV9002 electrical characteristics](https://www.ti.com/lit/ds/symlink/tlv9002.pdf#page=17)

## Bench procedure

1. **Measure the analog filter first.** On a fixture or with the DAC pin physically disconnected from the resistor, drive the network from a low-impedance generator. Power the TLV9002 and reference the generator amplitude to the measured voltage at the resistor input. Sweep 10 kHz to 2 MHz at 0.2 Vpp centered on 0.75 V. Record gain at 250 kHz, notch frequency and depth, and gain at 750 kHz and 1.25 MHz. Start with 47 pF across the coil. Reduce it if the notch is low and increase it if high. Record the fitted value and repeat for both channels. Probe at the follower output for tuning. A probe on the tank can move the notch.
2. **Calibrate at the connector.** Reconnect the DAC, select the analog paths on both switches, and start with low amplitude. Use a repeating four-sample sine table at 45°, 135°, 225° and 315°. At this frequency it programs two high samples followed by two low samples. Start around 0.27 V and 1.23 V for the final levels, then adjust their separation and midpoint from the measured fundamental and DC level. This table phase allows gain correction around 0.75 V within the buffered DAC swing. Observe the DAC pin with a low-capacitance probe and check both voltage levels, step settling, clipping and DMA underruns. ST’s 1 MSample/s specification uses a one-LSB step. [ST DAC timing, Table 67](https://www.st.com/resource/en/datasheet/stm32l072cb.pdf#page=109)
3. **Capture the spectrum and trace.** Connect A to a 1 MΩ input and B to a compensated high-impedance probe. Record probe capacitance and scope bandwidth. Acquire at least 1,000 cycles at 50 MSample/s or faster. Use a coherent record or a Hann window with amplitude correction and integrated tone power. Report DC, fundamental Vpp, time-domain Vpp, and 500 kHz, 750 kHz, 1.00 MHz, 1.25 MHz and 1.75 MHz components in dBc. Calculate THD through the seventh harmonic. At exactly 250 kHz, the 750 kHz image coincides with the third harmonic. For diagnosis, repeat at 240 kHz with a 25-sample, six-cycle sine table and distinguish the 720 kHz harmonic from the 760 kHz image.
4. **Locate any loss or distortion.** Compare the DAC pin, TLV9002 input, TP22/TP23 and the connectors at the same code amplitude. Use a low-capacitance active probe for the input measurement and repeat the output capture with it removed. Record the A-to-B amplitude and phase difference with both channels playing the same table. Inspect both outputs for ringing during amplitude changes. Compare against a 10 kHz sine and check DC presets after the filter has settled. [Board test points](Test%20points.md)
5. **Repeat at operating voltage.** Test on USB power and at the 3.0 V and 2.0 V loaded battery-input test points. Record battery-terminal operating current with LEDs off and the debugger detached. Measure the DAC supply at the MCU's VDDA bypass capacitor. On the boost-equipped board, TP2 measures the converter input. Repeat the 250 kHz measurement on both channels at each supply condition. [Board rail measurements](Test%20points.md)

### Compare the 250 kHz simulation with oscilloscope captures

1. Before trimming the tank capacitor or calibrating the output amplitude, fit the nominal 47 pF tank capacitor and run the model’s repeating DAC codes **1457, 1457, 324, 324** at **1 MSample/s**. Measure VDDA and record the total resistance and capacitance at the connector, including the probe and cable. The reference run uses 3.45 V and a 1 MΩ load in parallel with 100 pF. Record any differences so the comparison can be repeated under the measured conditions. [Reference waveform and spectrum](../report/250khz-connector.png) · [Simulation model](../report/simulate_250khz.py)
2. Capture the DAC pin and its loaded connector simultaneously for each channel using the acquisition settings in step 3. Export the DC-coupled sample records. Set time zero at the rising DAC transition, using the midpoint between the low and high levels. Overlay three periods (12 µs) of the measured and simulated traces on the same voltage and time scales. Keep the connector trace aligned to the DAC trigger so the overlay preserves filter delay, gain error and DC offset.
3. Use the full record to calculate the mean, trace Vpp, fundamental Vpp and image levels. Reference each spectrum to its own 250 kHz fundamental. Enter measured minus simulated for each row below, in mV for voltages and dB for spectral levels. If an image falls below the scope’s noise floor, record an upper bound. Save the raw records, waveform overlay and spectrum with the board revision, channel and load. Record any post-calibration run separately. Apply the first-build acceptance limits in **Decision and results**.

| Connector measurement | Nominal simulation | A measured / difference | B measured / difference |
| --- | ---: | --- | --- |
| DC mean | 0.7500 V | | |
| Time-domain peak-to-peak | 0.9785 Vpp | | |
| 250 kHz fundamental | 0.9999 Vpp | | |
| 750 kHz image | −41.2 dBc | | |
| 1.25 MHz image | −33.5 dBc | | |

### Voltage window during startup and LED brightness changes

Use the calibrated low and high DAC levels from step 2 (nominally 0.27 V and 1.23 V) as the reference for each channel. Capture VDDA, the DAC pin and its loaded connector simultaneously with DC coupling and pretrigger history. Repeat for A and B. Check the buffered DAC’s operating headroom during playback against the instantaneous limits of 0.20 V and VDDA − 0.20 V. [STM32 DAC output swing, Table 67](https://www.st.com/resource/en/datasheet/stm32l072cb.pdf#page=107)

- **Cold startup.** Trigger on the rising supply from fully off. Capture the rail ramp, DAC enable, first 250 kHz samples and settled output. If normal boot selects another preset, capture normal boot first and repeat with the test firmware starting directly in the 250 kHz preset. Report any connector pulse before playback separately. From the first intended sample pair, measure DAC undershoot and overshoot relative to the calibrated levels, minimum rail headroom, and elapsed time until the connector meets the amplitude and mean-voltage criteria below consistently.
- **LED brightness transitions.** Keep both channels playing the same 250 kHz tone. Trigger on an LED drive edge or current step. Capture off to maximum, maximum to off (including indicator timeout), and intermediate brightness changes in both directions. Include the largest simultaneous LED load change on the lowest battery input. For PWM dimming, capture individual PWM edges and the brightness transition. Compare complete sine cycles before, during and after each event so that averaging does not hide a short disturbance.
- **Record each event.** Save the trace with channel, supply condition, LED levels, minimum VDDA, DAC minimum and maximum, and largest departure from each calibrated level. Record any clipping or headroom violation, worst connector amplitude and mean-voltage error, duration outside the existing limits, and time to recover. Use cycle-by-cycle amplitude and mean during the event and confirm the settled fundamental using step 3.

## Decision and results

Proposed first-build criteria are 1.00 Vpp fundamental within ±5%, 0.75 V mean within ±25 mV, and both the 750 kHz and 1.25 MHz components at or below −30 dBc at each connector. Record THD and the largest spur separately. Report the existing 1 mA battery target alongside signal quality. A miss at one rail or on one channel should identify the first node where the error appears.

| Supply / channel | Fitted tank C / notch frequency | Fundamental Vpp / mean V | Trace Vpp | 750 kHz / 1.25 MHz dBc | THD / largest spur | VDD / current | Capture |
| --- | --- | --- | --- | --- | --- | --- | --- |
| USB / A, B | | | | | | | |
| 3.0 V battery / A, B | | | | | | | |
| 2.0 V loaded battery / A, B | | | | | | | |
