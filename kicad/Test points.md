# Trainer test points

Fit 16 Harwin S1751-46 surface-mount probe loops on the default build at TP1, TP2, TP4–TP12 and TP21–TP25. TP3 is a plated clip hole. [Harwin drawing and specifications](https://www.harwin.com/products/S1751-46)

## Power on the PP3 schematic

TP1 (I SOURCE) and TP2 (I LOAD) are probe loops on opposite sides of the current-measurement link at the input of regulator U7. JP1 and the optional 10 Ω shunt R4 are in parallel.

| Rail point | Ground point | What to measure |
|---|---|---|
| TP5 | TP6 | USB VBUS at the connector and USB throw of SW1 |
| TP7 | TP8 | System VDD at the regulated output of U7 (3.447 V nominal) |
| TP9 | TP10 | Alkaline PP3 positive terminal and battery throw of SW1 |
| TP11 | TP12 | Selected input at SW1 common, before fuse F1 and protection diode D11 |
| TP1 | TP12 | Protected supply at the cathode of D11, on the source side of the shunt |
| TP2 | TP8 | U7 input voltage, on the regulator side of the shunt |

On USB or battery power, the link measures U7 input current, including the board load, the feedback divider and the regulator’s own current. For an ammeter reading, leave R4 unpopulated, remove the JP1 short and connect the meter across JP1.

For a scope measurement, fit R4 and remove the JP1 short. Probe TP1 with CH1 and TP2 with CH2, grounding the probes at TP12 and TP8 respectively. Use CH1 − CH2 for the shunt voltage and divide by 10 Ω to get input current. A 100 mV difference corresponds to 10 mA flowing toward U7. To measure at the battery terminals with an SMU, remove the PP3, select BAT and power J5 from the SMU.

## Waveform and trigger

| Test point | Signal |
|---|---|
| TP22 | BUF_B, the buffered DAC signal for physical B, at U2 pin 7 |
| TP23 | BUF_A, the buffered DAC signal for physical A, at U2 pin 1 |
| TP24 | B_SEL from PA0, the selector for physical B |
| TP25 | A_GPIO from PB13, the digital signal available to physical A |

| Physical output | Selector | Low selects GPIO | High selects buffered DAC |
|---|---|---|---|
| A, J2 BNC center | A_SEL from PB11 | PB13 (TIM21_CH1) via U3 pin 9 | PA5 (DAC channel 2) via U3 pin 2 |
| B, TP3 clip point | B_SEL from PA0 | PA1 (TIM2_CH2) via U3 pin 7 | PA4 (DAC channel 1) via U3 pin 4 |

Compare the A output with TP25 in GPIO mode or TP23 in DAC mode. For B, probe TP24 to see the selection timing and TP22 to see the analog level. Use the J2 shell or TP4 for ground. Both selectors and both GPIO signal inputs have 100 kΩ pull-downs, so both outputs default low while the MCU pins are high impedance.

## LED data and buttons

Probe TP21 (PB15, U1 pin 28) to observe LED_DATA at the first LED’s data input. PB15 supports SPI2_MOSI. The VALUE button (SW3) uses PB12, U1 pin 25. The MODE button (SW2) uses PB14, U1 pin 27. [STM32L072 alternate-function tables](https://www.st.com/resource/en/datasheet/stm32l072cb.pdf#page=54)
