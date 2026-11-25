# Waveform Workout Rev 1 firmware

Zephyr target `waveform_workout` uses the STM32L072CBT6 with 128 KiB of flash and 20 KiB of SRAM. The west manifest pins Zephyr 4.5.0. Run the following commands from the `firmware` directory in a configured Zephyr environment.

## Build and flash

For a fresh checkout, initialize the workspace once and fetch its dependencies.

```sh
west init -l manifest
west update
```

In a space-free checkout, build the trainer and program it through J3 with an ST-LINK. Power the board through SW1 and JP1.

```sh
west build -b waveform_workout -d build/trainer app
west flash -d build/trainer
```

For a Windows project path with spaces, keep the board files in the project and put the Zephyr workspace and build output in a space-free directory. In PowerShell, start in this project's `firmware` directory. The example uses `C:\src\ww-zephyr` for the dependencies.

```powershell
$boardRoot = (Get-Location).Path
$workspace = 'C:\src\ww-zephyr'
New-Item -ItemType Directory -Force "$workspace\manifest" | Out-Null
Copy-Item manifest\west.yml "$workspace\manifest\west.yml"
Set-Location $workspace
$env:ZEPHYR_BASE = "$workspace\deps\zephyr"
west init -l manifest
west update
west build -b waveform_workout -d build/trainer "$boardRoot\app"
```

On subsequent builds, run the final command from the same workspace with `$boardRoot` pointing to the project's `firmware` directory.

Flash from that workspace with `west flash -d build/trainer`. OpenOCD is the default runner. The target also provides `stm32cubeprogrammer` and `jlink` runners. The board has no wired UART console.

## First scope check

Use DC coupling and a 1 MΩ input. Connect CH1 to J2 and CH2 to TP3 with ground at TP4. Reset should give two matching 1 kHz sines from about +0.25 to +1.25 V. MODE selects a row. VALUE cycles its three positions. The two dim green indicators show the selection for two seconds. The outputs continue when the panel goes dark. A steady D12 indicates an application setup or update error.

For a quick two-channel check, select XY and press VALUE once. In XY display with equal scales and centered traces, expect a circle. In normal time display, B leads A by 250 µs. XY uses the saved amplitude. TRIG runs at logic level and uses B to mark the exercise on A. The trainer keeps USB data detached and the QT rail off.

## Filter diagnostic and board samples

To run the separate 250 kHz filter test, build the same application with a separate output directory. Substitute the full path to `app` when using an external workspace.

```sh
west build -b waveform_workout -d build/filter app -- -DCONFIG_WORKOUT_FILTER_TEST=y
west flash -d build/filter
```

This image continuously plays the raw codes 1457, 1457, 324, 324 on both DACs at 1 MSample/s. Measure the connector amplitude and spectrum under the specified load. Flash `build/trainer` again to restore the front-panel exercises.

For a board-only check, build the Zephyr sample `deps/zephyr/samples/basic/blinky` for D12 or `deps/zephyr/samples/drivers/led/led_strip` for the pixel chain. Pass `-DBOARD_ROOT=<absolute path to firmware>` after `--` when building either sample.

## Firmware structure and host check

`app/src/lesson.c` defines the presets, physical pixel order, packed DAC samples and trigger sequences. `outputs.c` drives both DACs from TIM6 using circular DMA. In TRIG, TIM2 starts TIM21 over ITR0. Two internal TIM2 compares load the next A and B pulse widths through DMA halfway through each 100 µs period. The timers latch them at rollover. SPI2 uses separate DMA channels for the display. [STM32L0x2 reference manual, tables 51 and 95](https://www.st.com/resource/en/reference_manual/rm0376-ultralowpower-stm32l0x2-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

From `firmware`, a host C compiler can check the state transitions, LED positions, waveform samples and trigger sequences.

```sh
mkdir -p build
cc -std=c11 -Wall -Wextra -Werror -Iapp/src app/src/lesson.c tests/lesson_test.c -lm -o build/lesson-test
./build/lesson-test
```

## Application interfaces

| Hardware | Zephyr interface |
| --- | --- |
| Diagnostic LED and keys | `led0` drives D12. `sw0` reads MODE and `sw1` reads VALUE. A logical one means pressed. |
| Nine front-panel pixels | `led-strip` uses SPI2 MOSI on PB15. A 24 MHz internal system clock supplies the 3 MHz SPI bit clock. DMA1 channels 5 and 6 feed SPI2 while leaving channels 2 and 4 available for the DAC. |
| Analog outputs | `DAC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), dac_a)` selects A on PA5, DAC channel 2. Use `dac_b` for B on PA4, DAC channel 1. For synchronized playback, use TIM6 and the DAC's `dual` DMA entry (DMA1 channel 2, request 9) to transfer packed 32-bit A/B pairs. |
| Source switches and digital outputs | On `/zephyr,user`, `a-select-gpios` and `b-select-gpios` select analog when asserted. `a-digital-gpios` and `b-digital-gpios` drive the digital paths. For timer output, enable `&pwm_a` (TIM21 CH1) or `&pwm_b` (TIM2 CH2) in an application overlay. |
| STEMMA QT | `zephyr_i2c` is J6. Enable the `qt_vdd` regulator before the first transaction. The rail defaults off and the bus uses its switched external pull-ups. |
| Native USB | `zephyr_udc0` uses HSI48 with clock recovery from USB frames. Use `vbus-present-gpios` on `/zephyr,user` and the `volt-sensor0` supply-voltage reading to implement the attach and detach thresholds. |

The 3 MHz pixel encoding gives a nominal 1.00 µs zero-bit low interval, at the upper timing limit for the fitted LEDs. Check the waveform at TP21 during bring-up.
