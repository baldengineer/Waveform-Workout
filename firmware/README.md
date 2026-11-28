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

## USB serial shell

The trainer and filter diagnostic include Zephyr's interactive shell over native USB CDC ACM. After flashing, power the board through SW1 and JP1 and connect a USB data cable. Open the new serial port in a terminal such as PuTTY, with 115200 baud, 8 data bits, no parity, one stop bit, and hardware flow control off. CDC ACM does not use the baud rate to time USB transfers. The terminal must assert DTR; press Enter to display the `trainer:~$` prompt.

Useful commands are:

```text
help
kernel version
kernel uptime
kernel thread list
kernel thread stacks
device list
```

Tab completes commands and the arrow keys recall command history. Waveform playback and the front-panel buttons work with USB disconnected while the board remains powered.

To fit the 20 KiB SRAM, the single full-speed CDC interface uses a 512-byte USB request pool and the interrupt stack is 1 KiB; the shell retains its 2 KiB stack. During board bring-up, run `kernel thread stacks` after exercising the shell and USB reconnects to inspect runtime stack usage, and check that waveform playback continues with USB disconnected while independently powered.

The application checks the active-low VBUS detector and VREFINT supply measurement every 100 ms. USB attaches with cable power present and VDD at or above 3.1 V, and detaches below 3.0 V, on cable removal, or on a failed power reading. This adds hysteresis around the STM32's [3.0 V USB minimum supply](https://www.st.com/resource/en/datasheet/stm32l072kz.pdf). Reconnect the terminal if the serial device disappears and re-enumerates. D12 also indicates a USB initialization error; waveform initialization still proceeds.

USB descriptors identify the device as `Waveform Workout`, with its MCU hardware ID as the serial number. `CONFIG_WORKOUT_USB_VID` and `CONFIG_WORKOUT_USB_PID` default to Zephyr's development console IDs (`2fe3:0004`); use assigned IDs before distributing devices outside development. The implementation uses Zephyr's [USB device stack and CDC ACM class](https://docs.zephyrproject.org/latest/services/connectivity/usb/device_next/cdc_acm.html).

## First scope check

Use DC coupling and a 1 MΩ input. Connect CH1 to J2 and CH2 to TP3 with ground at TP4. With fresh EEPROM, both channels should show matching 1 kHz sines from about +0.25 to +1.25 V. MODE selects a row. VALUE cycles its three positions. The two dim green indicators show the selection for two seconds. The outputs continue when the indicators go dark. A steady D12 indicates an application setup or update error.

For a quick two-channel check, select XY and set its value to 90 degrees. In XY display with equal scales and centered traces, expect a circle. In normal time display, B leads A by 250 µs. XY uses the saved amplitude. TRIG runs at logic level and uses B to mark the exercise on A.

## Saved settings

The trainer restores the active mode, the value of each row, and the OLED address from the STM32's internal data EEPROM. Button presses and USB shell edits save immediately. Two versioned, checksummed records occupy the first 32 bytes of EEPROM. An interrupted update leaves the previous record available. Older firmware presets retain their waveform settings and start with OLED address `0x3D`. The next save upgrades the record. With no valid record, the trainer uses the factory settings below.

```text
trainer settings show
trainer settings set shape square
trainer settings set freq 10kHz
trainer settings set mode freq
```

`show` reads EEPROM. `set` changes one field and saves the full selection. Editing an inactive row preserves the active mode. Field names and values are case-sensitive.

| Field | Accepted values in button order | Factory setting |
| --- | --- | --- |
| `mode` | `shape`, `amp`, `freq`, `dc`, `xy`, `trig` | `shape` |
| `shape` | `sine`, `square`, `triangle` | `sine` |
| `amp` | `200mVpp`, `500mVpp`, `1Vpp` | `1Vpp` |
| `freq` | `100Hz`, `1kHz`, `10kHz` | `1kHz` |
| `dc` | `250mV`, `500mV`, `1V` | `250mV` |
| `xy` | `0deg`, `90deg`, `180deg` | `0deg` |
| `trig` | `rare-pulse`, `pulse-width`, `missing` | `rare-pulse` |
| `oled_addr` (shell only) | `0x3C`, `0x3D` | `0x3D` |

To check persistence, run the example commands, press reset, and run `trainer settings show` again. The scope should still show a 10 kHz square wave. Press VALUE in the frequency row, then power-cycle the board to check that the buttons also save the new 100 Hz selection. The separate filter diagnostic ignores trainer settings.

## Optional SSD1306 OLED

Connect an I²C SSD1306 to J6 before power-up. The default trainer image enables the QT rail, waits for the module to reset, and initializes a **128×64** panel at 7-bit address **0x3D** for the Adafruit 326 enclosure. It displays the active row and value after boot and after edits. An absent or unresponsive module releases the QT rail and the trainer continues. The drawing code in `app/src/oled.c` uses Zephyr's native character framebuffer, which provides fonts, text, points, lines, rectangles and circles. [Zephyr framebuffer API](https://docs.zephyrproject.org/latest/doxygen/html/group__monochrome__character__framebuffer.html) · [Adafruit OLED addressing](https://learn.adafruit.com/monochrome-oled-breakouts?view=all)

For a 128×32 panel, add the supplied overlay at build time. Use an absolute path to the overlay if building outside the firmware directory.

```sh
west build -b waveform_workout -d build/oled32 app -- -DEXTRA_DTC_OVERLAY_FILE=oled-128x32.overlay
```

The overlay sets the geometry of both SSD1306 address variants. For different glass, set `width`, `height`, `multiplex-ratio` and the COM configuration on both nodes. An OLED-disabled build (`-DCONFIG_WORKOUT_OLED=n` after `--`) and the filter diagnostic leave J6 off during boot.

### Find the panel address on J6

Run `i2c_scan` in the USB shell. It temporarily powers J6, probes the 7-bit addresses `0x08` through `0x77` without sending data bytes, and prints each address that acknowledges. It then releases its power request. An active OLED stays powered. The scanner is also available in OLED-disabled and filter builds.

For a panel that responds at `0x3C`, run:

```text
i2c_scan
trainer settings set oled_addr 0x3C
trainer settings show
```

Press RESET to initialize the display at the saved address. Use `trainer settings set oled_addr 0x3D` to restore the default. Match the address to the panel's hardware strap.

## Filter diagnostic and board samples

To run the separate 250 kHz filter test, build the same application with a separate output directory. Substitute the full path to `app` when using an external workspace.

```sh
west build -b waveform_workout -d build/filter app -- -DCONFIG_WORKOUT_FILTER_TEST=y
west flash -d build/filter
```

This image continuously plays the raw codes 1457, 1457, 324, 324 on both DACs at 1 MSample/s. Measure the connector amplitude and spectrum under the specified load. Flash `build/trainer` again to restore the front-panel exercises.

For a board-only check, build the Zephyr sample `deps/zephyr/samples/basic/blinky` for D12 or `deps/zephyr/samples/drivers/led/led_strip` for the pixel chain. Pass `-DBOARD_ROOT=<absolute path to firmware>` after `--` when building either sample.

## Firmware structure and host check

`app/src/lesson.c` defines the presets, physical pixel order, packed DAC samples and trigger sequences. `usb_console.c` initializes USB CDC ACM and monitors cable power and VDD independently of the trainer loop; Zephyr's serial shell handles terminal input and output. `outputs.c` drives both DACs from TIM6 using circular DMA. In TRIG, TIM2 starts TIM21 over ITR0. Two internal TIM2 compares load the next A and B pulse widths through DMA halfway through each 100 µs period. The trigger tables store one byte per width. DMA expands each byte into a 16-bit timer compare value, which the timers latch at rollover. SPI2 uses separate DMA channels for the front-panel pixels. [STM32L0x2 reference manual, tables 51 and 95](https://www.st.com/resource/en/reference_manual/rm0376-ultralowpower-stm32l0x2-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

From `firmware`, a host C compiler can check the state transitions, LED positions, waveform samples and trigger sequences.

```sh
mkdir -p build
cc -std=c11 -Wall -Wextra -Werror -Iapp/src app/src/lesson.c tests/lesson_test.c -lm -o build/lesson-test
./build/lesson-test
```

The [Renode test plan](tests/renode/README.md) runs the Rev 1 front panel, EEPROM recovery, shell, and optional OLED against an emulated STM32L072. It includes Windows build and run commands.

## Application interfaces

| Hardware | Zephyr interface |
| --- | --- |
| Diagnostic LED and keys | `led0` drives D12. `sw0` reads MODE and `sw1` reads VALUE. A logical one means pressed. |
| Nine front-panel pixels | `led-strip` uses SPI2 MOSI on PB15. A 24 MHz internal system clock supplies the 3 MHz SPI bit clock. DMA1 channels 5 and 6 feed SPI2 while leaving channels 2 and 4 available for the DAC. |
| Analog outputs | `DAC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), dac_a)` selects A on PA5, DAC channel 2. Use `dac_b` for B on PA4, DAC channel 1. For synchronized playback, use TIM6 and the DAC's `dual` DMA entry (DMA1 channel 2, request 9) to transfer packed 32-bit A/B pairs. |
| Source switches and digital outputs | On `/zephyr,user`, `a-select-gpios` and `b-select-gpios` select analog when asserted. `a-digital-gpios` and `b-digital-gpios` drive the digital paths. For timer output, enable `&pwm_a` (TIM21 CH1) or `&pwm_b` (TIM2 CH2) in an application overlay. |
| STEMMA QT | `zephyr_i2c` is J6. Enable the `qt_vdd` regulator before the first transaction. The bus uses switched external pull-ups. The OLED build powers J6 for discovery and keeps it on while the panel is in use. |
| Native USB | `zephyr_udc0` uses HSI48 with clock recovery from USB frames. The application connects CDC ACM to `zephyr,console` and `zephyr,shell-uart`. `vbus-present-gpios` on `/zephyr,user` and the `volt-sensor0` supply-voltage reading gate USB attachment. |

The 3 MHz pixel encoding gives a nominal 1.00 µs zero-bit low interval, at the upper timing limit for the fitted LEDs. Check the waveform at TP21 during bring-up.
