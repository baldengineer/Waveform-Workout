# Waveform Workout Rev 1 board support

Zephyr target `waveform_workout` uses the STM32L072CBT6 with 128 KiB of flash and 20 KiB of SRAM. The west manifest pins Zephyr 4.5.0. Run the following commands from the `firmware` directory in a configured Zephyr environment.

## Build and flash

For a fresh checkout, initialize the workspace once and fetch its dependencies.

```sh
west init -l manifest
west update
```

In a checkout with a space-free path, build Blinky with the firmware directory as the board root.

```sh
west build -b waveform_workout -d build/blinky deps/zephyr/samples/basic/blinky -- -DBOARD_ROOT="$PWD"
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
west build -b waveform_workout -d build/blinky deps/zephyr/samples/basic/blinky -- "-DBOARD_ROOT=$boardRoot"
```

On subsequent builds, run the final command from the same workspace with `$boardRoot` pointing to the project's `firmware` directory.

Blinky toggles the green diagnostic LED D12 on PA2. To program it, power the trainer, connect an external ST-LINK to J3 including NRST, and run `west flash -d build/blinky`. OpenOCD is the default runner. The target also provides `stm32cubeprogrammer` and `jlink` runners. The board has no wired UART console.

To exercise the nine addressable LEDs instead, build `deps/zephyr/samples/drivers/led/led_strip` with the same board root and a separate build directory such as `build/pixels`.

## Application interfaces

| Hardware | Zephyr interface |
| --- | --- |
| Diagnostic LED and keys | `led0` drives D12. `sw0` reads MODE and `sw1` reads VALUE. A logical one means pressed. |
| Nine front-panel pixels | `led-strip` uses SPI2 MOSI on PB15. A 24 MHz internal system clock supplies the 3 MHz SPI bit clock. DMA1 channels 5 and 6 feed SPI2 while leaving channels 2 and 4 available for the DAC. |
| Analog outputs | `DAC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), dac_a)` selects A on PA5, DAC channel 2. Use `dac_b` for B on PA4, DAC channel 1. For synchronized playback, use TIM6 and the DAC's `dual` DMA entry (DMA1 channel 2, request 9) to transfer packed 32-bit A/B pairs. |
| Source switches and digital outputs | On `/zephyr,user`, `a-select-gpios` and `b-select-gpios` select analog when asserted. `a-digital-gpios` and `b-digital-gpios` drive the digital paths. For timer output, enable `&pwm_a` (TIM21 CH1) or `&pwm_b` (TIM2 CH2) in an application overlay. |
| STEMMA QT | `zephyr_i2c` is J6. Enable the `qt_vdd` regulator before the first transaction. The rail defaults off and the bus uses its switched external pull-ups. |
| Native USB | `zephyr_udc0` uses HSI48 with clock recovery from USB frames. Use `vbus-present-gpios` on `/zephyr,user` and the `volt-sensor0` supply-voltage reading to implement the attach and detach thresholds. |

The [firmware handoff](../FIRMWARE.md) specifies the pixel order, startup sequence, waveform timing and USB voltage thresholds. The 3 MHz pixel encoding gives a nominal 1.00 µs zero-bit low interval, at the upper timing limit for the fitted LEDs. Check the waveform at TP21 during bring-up.
