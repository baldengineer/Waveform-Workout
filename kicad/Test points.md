# Trainer test points

TP1–TP4 are bare PCB features. Fit a Harwin S1751-46 surface-mount probe loop at each of TP5–TP25 on the default build (21 loops total). [Harwin drawing and specifications](https://www.harwin.com/products/S1751-46)

## Power

Each rail measurement has a dedicated board-ground point. TP1 and TP2 are the original sense pads on opposite sides of the current-measurement link. To measure a populated shunt with two grounded scope probes, put the tips on TP1 and TP2, connect the probe grounds to TP13 and TP14, and subtract the channels.

| Rail point | Ground point | What to measure |
|---|---|---|
| TP5 | TP6 | USB VBUS at the connector, before the 3.3 V regulator |
| TP7 | TP8 | Regulated USB 3.3 V, before SW1 |
| TP9 | TP10 | Two-cell AAA NiMH pack positive terminal, before SW1 |
| TP11 | TP12 | Selected source at the SW1 common and MAX40200 input |
| TP1 | TP13 | MAX40200 output, source side of JP1 |
| TP2 | TP14 | Boost converter input, load side of JP1 |

TP11 and TP1 show the voltage across U5 during a source change. U5 blocks reverse current into the selected source. With BAT selected, JP1 measures the current drawn from the pack by the boost converter and its loads. Probe the boost output capacitor to measure regulated system VDD, nominally 3.45 V. When using J5 to emulate the pack with an SMU, remove both cells. [MAX40200 datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/MAX40200.pdf)

## Waveform and trigger

| Test point | Signal |
|---|---|
| TP22 | Buffered DAC A at U2 pin 1, before the analog switch |
| TP23 | Buffered DAC B at U2 pin 7, before the analog switch |
| TP24 | PULSE_SEL, the control signal for the channel A pulse switch |
| TP25 | MARKER, the digital marker fed to the channel B switch |

Compare TP22 with J4 pin 1 (output A), or TP23 and TP25 with TP3 (output B). J4 pin 2 and TP4 are ground.

## STM32 breakout

These pins are available for firmware experiments. Alternate functions that share a pin are selected in firmware. An external I²C bus needs pull-ups to the trainer's I/O voltage. [STM32L072 pin and alternate-function tables](https://www.st.com/resource/en/datasheet/stm32l072cb.pdf#page=46)

| Test point | STM32 pin | LQFP48 pin | Available functions |
|---|---|---:|---|
| TP15 | PA6 | 16 | ADC_IN6, TIM3_CH1, COMP1_OUT |
| TP16 | PA9 | 30 | USART1_TX or I2C1_SCL |
| TP17 | PA10 | 31 | USART1_RX or I2C1_SDA |
| TP18 | PB12 | 25 | SPI2_NSS |
| TP19 | PB13 | 26 | SPI2_SCK, TIM21_CH1, I2C2_SCL |
| TP20 | PB14 | 27 | SPI2_MISO, TIM21_CH2, I2C2_SDA |
| TP21 | PB15 | 28 | SPI2_MOSI |
