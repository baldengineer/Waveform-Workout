# Waveform Workout Rev 1 firmware handoff

Waveform Workout generates two known signals for a learner to measure on an oscilloscope. **A is the center of the J2 BNC. B is the TP3 probe clip, with ground at TP4.** Use DC coupling and a 1 MΩ scope input or a compensated high-impedance probe. Both outputs share ground and each has a 330 Ω series resistor. A 50 Ω termination will substantially attenuate the signal.

This document targets the Rev 1 PP3 board with U1 **STM32L072CBT6**, LQFP48, 128 KiB flash. Pin assignments were checked against the schematic at `0b94bae` and the Rev 1 production pad netlist. Waveform values below are firmware targets at the connectors. The board test plan defines the measurements for the first article. [Rev 1 acceptance plan](kicad/Rev%201%20board%20test%20plan.md)

## What MODE, VALUE and the lights mean

On power-up, generate the same **1 kHz sine, 1.00 Vpp, centered on +0.75 V** on A and B. A DC-coupled scope should show a 1 ms period and peaks near +0.25 V and +1.25 V.

**MODE** advances through the six rows below. **VALUE** advances through positions 1, 2 and 3 of the selected row, then wraps. Count one action per debounced press. Show one mode light and one numbered value light together for about two seconds after a change. For example, **FREQ + VAL 2** means 1 kHz, while **DC + VAL 2** means +0.50 V. The printed positions carry the meaning. Rev 1 specifies no color code.

| Mode light | VAL 1 | VAL 2 | VAL 3 | At the scope |
| --- | --- | --- | --- | --- |
| SHAPE | Sine | Square | Triangle | A and B carry the same shape. Change shape while keeping amplitude and frequency. |
| AMP | 0.20 Vpp | 0.50 Vpp | 1.00 Vpp | Change peak-to-peak swing on both channels while keeping the +0.75 V center, shape and frequency. |
| FREQ | 100 Hz | 1 kHz | 10 kHz | Change both channels to a 10 ms, 1 ms or 100 µs period. Keep shape and amplitude. |
| DC | +0.25 V | +0.50 V | +1.00 V | Both outputs hold the selected DC voltage relative to ground. |
| XY | 0° | 90° | 180° | A and B carry equal-amplitude, 1 kHz sines centered on +0.75 V. B is phase-shifted relative to A. With matched scales and centered traces, expect a rising line, circle or falling line. |
| TRIG | Narrow pulse | Variable width | Missing pulse | A carries the exercise. B marks the event or the start of the sequence. |

SHAPE, AMP and FREQ edit **one** basic waveform. Keep three independent settings. Entering DC, XY or TRIG switches both outputs to that exercise. Returning to any basic row uses the saved shape, amplitude and frequency. Keep output generation running when the display timer expires. Send a dark frame to clear the lights.

### Trigger exercises

Use the digital paths on both channels. A uses a 10 kHz time base (100 µs per period). Time the edges in hardware and align B's rising edge with the specified A period. The two pins belong to different timers. Check their alignment at the connectors, including while updating LEDs.

| TRIG value | A at J2 | B at TP3 | What the learner can trigger on |
| --- | --- | --- | --- |
| 1 · Narrow pulse | 999 pulses at 50 µs high, then one at 2 µs high. Repeat every 100 ms. | Rise with the 2 µs pulse. The proposed marker width is 10 µs. | Trigger on B to find the event, then on A positive pulse width below 5 µs. |
| 2 · Variable width | Repeat 10, 40 and 80 µs high in successive periods. | Rise at the start of the 10 µs pulse, once per 300 µs group. | An A positive-width window of 30–50 µs picks out the middle pulse. |
| 3 · Missing pulse | Normally 50 µs high. Suppress one whole pulse every 1,000 periods. | Rise when the missing A edge would have occurred. | Expect 200 µs between A rising edges and 150 µs continuously low. Trigger on A low time above 100 µs. |

### Indicator driver

With the BNC on the left, the front panel reads **SHAPE, AMP, FREQ, DC, XY, TRIG, VAL 1, VAL 2, VAL 3**. The nine Worldsemi WS2812B-2020-V6 pixels share one data pin, PB15. Index 0 consumes the first 24 bits sent. For example, a FREQ + VAL 2 indication lights wire indices 6 and 1.

| Wire index | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Part | D9 | D8 | D7 | D6 | D5 | D4 | D3 | D1 | D2 |
| Board label | VAL 3 | VAL 2 | VAL 1 | TRIG | XY | DC | FREQ | SHAPE | AMP |

Send nine 24-bit words in GRB byte order, most significant bit first, with the 800 kbit/s single-wire encoding and a low latch interval longer than 280 µs. The pixels hold their last frame while the wire is idle. Send all zeros on initialization and timeout. Keep the two lit pixels dim within the board's current budget. If using SPI2 to encode the pulses, configure only MOSI on PB15 and manage NSS in software. PB12, PB13 and PB14 serve the VALUE button, A waveform and MODE button. [Worldsemi WS2812B-2020-V6 timing and frame format](https://www.tme.com/Document/f57342b0dc3e7d573aa1f8b6b5942fcf/WS2812B-2020-V6_V1.2_EN.pdf)

Two separate green LEDs have different meanings. **D10 (USB PWR)** is wired to VBUS and can light even when SW1 selects BAT and the MCU is off. **D12** is a software-controlled diagnostic LED on PA2, active high. An open battery-mode enhancement calls for a brief blink of one settings LED per second, advancing in order after the normal indication expires. Rev 1 exposes VBUS presence to firmware. With both sources attached, the MCU cannot read SW1's position. [Battery indication request](https://github.com/baldengineer/Waveform-Workout/issues/2)

## STM32 pin map

Pin numbers are physical U1 leads. Net names match the schematic. GPIO levels refer to the regulated 3.45 V domain. AF is the STM32 alternate-function number. [Rev 1 schematic](kicad/oscilloscope%20trainer%20hw.kicad_sch) · [ST pinout and alternate-function tables](https://www.st.com/resource/en/datasheet/stm32l072cb.pdf#page=43)

| U1 pin | STM32 | Schematic net | Firmware configuration and board connection |
| ---: | --- | --- | --- |
| 10 | PA0 | `B_SEL` | Push-pull output to U3. Low selects B digital, high selects B analog. R7 pulls low. Probe TP24. |
| 11 | PA1 | `B_GPIO` | Digital B waveform or marker, TIM2_CH2 AF2 or GPIO output. R35 pulls low. |
| 12 | PA2 | `LED` | GPIO output. High sources current through R26 and lights green D12. Start low. |
| 14 | PA4 | `B_DAC` | Analog mode, buffered DAC_OUT1. Drives the B filter through R29. |
| 15 | PA5 | `A_DAC` | Analog mode, buffered DAC_OUT2. Drives the A filter through R30. |
| 22 | PB11 | `A_SEL` | Push-pull output to U3. Low selects A digital, high selects A analog. R8 pulls low. |
| 25 | PB12 | `BUTTON_V` | VALUE/SW3. Input with internal pull-up. Released high, pressed low. |
| 26 | PB13 | `A_GPIO` | Digital A waveform, TIM21_CH1 AF6 or GPIO output. R34 pulls low. Probe TP25. |
| 27 | PB14 | `BUTTON_M` | MODE/SW2. Input with internal pull-up. Released high, pressed low. |
| 28 | PB15 | `LED_DATA` | WS2812 data output. Idle low. SPI2_MOSI AF0 is available. TP21 taps the line to D9 DIN. |
| 32 | PA11 | `USB_DM` | Native USB full-speed D− via U6 ESD protection to USB-C J1. |
| 33 | PA12 | `USB_DP` | Native USB full-speed D+ via U6 to J1. The device controls attachment with the internal D+ pull-up. |
| 34 | PA13 | `SWDIO` | SWD debug data, AF0, J3 pin 2. Preserve debugger access. |
| 37 | PA14 | `SWCLK` | SWD debug clock, AF0, J3 pin 4. |
| 38 | PA15 | `USB_VBUS_N` | GPIO input. Q1 pulls low when J1 has VBUS. R10 pulls high when VBUS is absent. |
| 41 | PB5 | `QT_PWR_EN_N` | Controls Q2. Low powers J6, high disables it. R33 pulls high during reset. |
| 42 | PB6 | `I2C1_SCL` | I²C1 clock, AF1 open-drain, J6 pin 4. Pull-up to switched QT_VDD. |
| 43 | PB7 | `I2C1_SDA` | I²C1 data, AF1 open-drain, J6 pin 3. Pull-up to switched QT_VDD. |

| Other U1 pins | Connection |
| --- | --- |
| 7 NRST | SW4 and J3 pin 10 can pull reset low. |
| 44 BOOT0 | R1, 10 kΩ to GND, selects normal flash boot with the default boot options. |
| 1, 24, 48 VDD · 9 VDDA · 36 VDD_USB | All on system VDD. VDDA supplies the DAC reference on this package. |
| 8 VSSA · 23, 35, 47 VSS | GND. |
| 2 PC13 · 3 PC14 · 4 PC15 · 5 PH0 · 6 PH1 | Unconnected. There is no fitted high-speed or 32.768 kHz crystal. Use internal clocks. |
| 13 PA3 · 16 PA6 · 17 PA7 · 18 PB0 · 19 PB1 · 20 PB2 · 21 PB10 | Unconnected. Leave unused GPIO in analog mode with pulls off. |
| 29 PA8 · 30 PA9 · 31 PA10 · 39 PB3 · 40 PB4 · 45 PB8 · 46 PB9 | Unconnected. PA15 provides the board's USB sense input. |

## How the board generates the outputs

U3 is a dual TS5A23157 analog switch. Each channel has its own source selector. The four 100 kΩ pull-downs on the selectors and digital inputs make both connectors default low while U1 is in reset.

| Physical output | Selector low · digital | Selector high · analog | Connector |
| --- | --- | --- | --- |
| A | PB13 → U3 pin 9 | PA5 / DAC channel 2 → filter L2 → U2 pin 1 (TP23) → U3 pin 2 | U3 pin 10 → R5 → J2 center |
| B | PA1 → U3 pin 7 | PA4 / DAC channel 1 → filter L1 → U2 pin 7 (TP22) → U3 pin 4 | U3 pin 6 → R6 → TP3 |

**Basic waveforms, DC and XY use the analog paths.** Each buffered 12-bit DAC drives a 5.6 kΩ resistor, a 22 pF shunt, a parallel 1 mH / 47 pF network in series with the signal, and a 68 pF shunt. The network passes DC and suppresses DAC sampling images, especially near 750 kHz. A TLV9002 voltage follower drives the switch and output resistor. Generate the basic square wave with the DAC too, so its high and low voltages follow AMP and the +0.75 V center. The digital path produces logic levels near GND and VDD and bypasses the reconstruction filter for the trigger exercises. [Outputs schematic](kicad/Outputs.kicad_sch) · [Filter design and measurements to make](kicad/DAC%20reconstruction%20filter%20test%20plan.md)

For DC, write and latch a fixed pair of codes. From the STM32's ideal transfer function, the starting code is `round(4096 × target_volts / VDDA)`, clamped to 0–4095. With 12-bit values in the right-aligned dual holding register `DAC_DHR12RD`, pack **physical B** in bits 11–0 and **physical A** in bits 27–16, e.g. `((uint32_t)a_code << 16) | b_code`. [ST RM0376, DAC transfer function and dual data registers](https://www.st.com/resource/en/reference_manual/rm0376-ultralowpower-stm32l0x2-advanced-armbased-32bit-mcus-stmicroelectronics.pdf#page=353)

For playback, a shared TIM6 trigger latches both DACs on the same sample edge. A circular DMA transfer can write the next packed pair with one 32-bit transfer per trigger. Enable one DAC channel's DMA request for that transfer. For XY, offset B's samples by zero, a quarter or half a cycle. At 1 kHz these offsets correspond to 0, 250 and 500 µs. [ST RM0376, dual-mode DMA requests](https://www.st.com/resource/en/reference_manual/rm0376-ultralowpower-stm32l0x2-advanced-armbased-32bit-mcus-stmicroelectronics.pdf#page=360)

The **250 kHz sine** is a separate filter diagnostic. At 1 MSample/s per DAC, the model repeats `1457, 1457, 324, 324` to target a 1.00 Vpp fundamental centered on +0.75 V into 1 MΩ in parallel with 100 pF. The model uses a different inductor from the fitted Bourns part. Record the as-built amplitude and spectrum before treating these codes as calibrated. [Simulation and reference traces](report/README.md)

## Power, startup and external interfaces

- **Power path.** SW1 selects 5 V USB-C or a 9 V PP3 (BT1, or J5 for battery emulation). The selected input passes through resettable fuse F1, Schottky D11 and the JP1 current link into U7, a TPS7A2401 linear regulator. Its nominal output is 3.447 V. JP1 is shorted for normal operation. With JP1 open and the optional 10 Ω R4 fitted, regulator input current is `(TP1 − TP2) / 10 Ω`. For example, 100 mV means 10 mA. F1 has a 50 mA hold rating at 20 °C. Budget for the MCU, pixels and any J6 load together. [Power schematic](kicad/Power.kicad_sch) · [Rail and probe map](kicad/Test%20points.md)
- **Startup sequence.** Keep both selectors and digital sources low, QT power off, LED data low and USB detached. Configure button pull-ups and send a dark pixel frame. Load the first sine sample pair and trigger a conversion with both selectors still low. After the analog paths settle, select them and start periodic playback. Keep pixel updates and button debounce independent of waveform timing. When changing routes, establish the new source level before selecting it.
- **USB.** Use the STM32's crystal-less full-speed device peripheral with HSI48 and clock recovery from USB frames. Attach only when PA15 is low and VDD_USB is at least 3.1 V. Detach on VBUS loss or before the falling rail crosses 3.0 V. The regulated rail can be estimated using the ADC's internal VREFINT and its factory calibration. USB data can operate while BAT powers the board. [Power schematic](kicad/Power.kicad_sch) · [ST clock recovery and USB](https://www.st.com/resource/en/datasheet/stm32l072cb.pdf#page=36)
- **STEMMA QT.** J6 pins 1–4 are GND, switched QT_VDD, SDA and SCL. PB5 low supplies the accessory and its two 5.1 kΩ bus pull-ups. Use open-drain I²C after the supply is up. Release both lines and disable internal pull-ups before turning the port off so the MCU cannot power the accessory through a signal pin.
- **SWD.** J3 is a keyed 10-position, 1.27 mm header. Pin 1 senses target VDD, 2 is SWDIO, 4 is SWCLK, 10 is NRST, and 3/5/9 are GND. Pin 7 is the key and 6/8 are unused. Power the target through SW1 and JP1.

For a quick pin-map check, drive A's DAC to 0.50 V and B's to 1.00 V while their digital sources run at 1 kHz and 2 kHz respectively. Toggle each selector independently and watch J2 and TP3. Then walk one dim pixel through all nine positions. These checks expose a swapped DAC, reversed selector polarity or incorrect LED lookup before debugging a lesson. [Rev 1 board test plan, sections 4–6](kicad/Rev%201%20board%20test%20plan.md)
