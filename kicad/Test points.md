# Trainer test points

TP1–TP4 are bare PCB features. Fit a Harwin S1751-46 surface-mount probe loop at each of TP5–TP25 on the default build (21 loops total). [Harwin drawing and specifications](https://www.harwin.com/products/S1751-46)

## Power on the PP3 schematic

Each rail measurement has a dedicated board-ground point. TP1 and TP2 are the original sense pads on opposite sides of the current-measurement link. To measure a populated shunt with two grounded scope probes, put the tips on TP1 and TP2, connect the probe grounds to TP13 and TP14, and subtract the channels.

| Rail point | Ground point | What to measure |
|---|---|---|
| TP5 | TP6 | USB VBUS at the connector and USB throw of SW1 |
| TP7 | TP8 | System VDD on the load side of JP1 |
| TP9 | TP10 | Alkaline PP3 positive terminal and battery throw of SW1 |
| TP11 | TP12 | Selected input at SW1 common, before protection diode D11 |
| TP1 | TP13 | U7 regulated output, source side of JP1 (3.447 V nominal) |
| TP2 | TP14 | Board load voltage after JP1 or the optional R4 shunt |

On either power source, an ammeter in place of JP1 reads the board load. The regulator and its feedback divider draw current upstream of JP1. To measure battery current including that overhead, remove the PP3 and supply J5 from an SMU with BAT selected. For a shunt measurement, fit R4 and remove the JP1 short. At 10 mA, the 10 Ω shunt drops 100 mV between TP1 and TP2. [TPS7A24 datasheet](https://www.ti.com/lit/ds/symlink/tps7a24.pdf)

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
