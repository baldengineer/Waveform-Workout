# Trainer test points

TP1–TP4 are bare PCB features. Fit a Harwin S1751-46 surface-mount probe loop at TP5–TP12 and TP15–TP26 on the default build (20 loops total). [Harwin drawing and specifications](https://www.harwin.com/products/S1751-46)

## Power on the PP3 schematic

TP1 and TP2 are the sense pads on opposite sides of the current-measurement link at the input of regulator U7. JP1 and the optional 10 Ω shunt R4 are in parallel.

| Rail point | Ground point | What to measure |
|---|---|---|
| TP5 | TP6 | USB VBUS at the connector and USB throw of SW1 |
| TP7 or TP26 | TP8 | System VDD at the regulated output of U7 (3.447 V nominal) |
| TP9 | TP10 | Alkaline PP3 positive terminal and battery throw of SW1 |
| TP11 | TP12 | Selected input at SW1 common, before fuse F1 and protection diode D11 |
| TP1 | TP12 | Protected supply at the cathode of D11, on the source side of the shunt |
| TP2 | TP8 | U7 input voltage, on the regulator side of the shunt |

On USB or battery power, the link measures U7 input current, including the board load, the feedback divider and the regulator’s own current. For an ammeter reading, leave R4 unpopulated, remove the JP1 short and connect the meter across JP1.

For a scope measurement, fit R4 and remove the JP1 short. Probe TP1 with CH1 and TP2 with CH2, grounding the probes at TP12 and TP8 respectively. Use CH1 − CH2 for the shunt voltage and divide by 10 Ω to get input current. A 100 mV difference corresponds to 10 mA flowing toward U7. To measure at the battery terminals with an SMU, remove the PP3, select BAT and power J5 from the SMU.

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
