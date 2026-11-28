# Waveform Workout Renode test plan

Run the front panel, settings store, shell, optional OLED, and USB power monitor against Renode's STM32L072 model before Rev 1 hardware arrives. Each case starts with a fresh machine. Within a case, reset restarts the image with the existing EEPROM contents.

| Case | Stimulus | Acceptance |
| --- | --- | --- |
| VALUE and restart | Boot from empty EEPROM, press and release PB12, then reset. | Playback starts at sine, advances to square once, and restores square. The shell reads square from EEPROM. |
| Incomplete preset | Save square and triangle, then clear the second slot's commit marker. | Reset restores square. This represents the EEPROM contents before the final commit byte is written. |
| Bad checksum | Change the newest record's shape byte while retaining its commit marker. | Reset restores the older square preset. |
| Legacy format and sequence wrap | Seed a version 1 square preset at sequence 255 and press VALUE. | The old preset loads with OLED address 0x3D. The new slot uses version 2 and sequence 0. Reset selects triangle. |
| MODE and shell | Press and release PB14, read settings, then set amplitude through the shell. | MODE selects amplitude. After reset the playback request specifies 500 mVpp. |
| Empty J6 | Boot and run `i2c_scan` with no I²C target, then press VALUE. | The scan reports zero devices, PB5 releases the active-low rail enable, and the panel advances to square. |
| Default OLED | Connect an I²C target at 0x3D. | The target receives the SSD1306 display-on command, the scan finds only 0x3D, and J6 stays enabled. |
| Alternate OLED | Connect the target at 0x3C, save its address through the shell, then reset. | The first boot releases J6. The next boot sends display-on to 0x3C, the scan finds only 0x3C, and J6 stays enabled. |
| USB supply thresholds | With cable present, drive VDD through 3099, 3100, 3000, 2999, 3099, and 3100 mV. | The stack enables, disables, and enables once each at the expected crossings. It holds its prior state inside the hysteresis band. |
| Cable reconnects | Boot with a cable, unplug and replug three times, and press VALUE during the first disconnect. Reconnect at 3050 mV before restoring nominal VDD. | Each unplug disables USB. Each reconnect waits for adequate VDD. Only boot and the button request playback, and no event requests idle. |
| Failed VDD reading | Inject a failed sample fetch and a failed channel read separately while attached. | Either failure disables USB. A valid reading restores it. |
| Failed USB transition | Return an I/O error on the first enable and first disable request. | The power worker retries both, reaches the requested stack state, and makes no duplicate requests while stable. |

## Running on Windows

Use the project's Zephyr 4.5.0 workspace, its SDK, and Renode 1.17.1. Activate the Zephyr build environment. Start in a space-free checkout's `firmware` directory or point `$fw` at a space-free junction to the current `firmware` directory. Zephyr splits overlay paths containing spaces. Build from the west workspace that contains the fetched dependencies.

```powershell
$fw = (Get-Location).Path
$build = Join-Path $env:TEMP 'ww-renode-build'
Set-Location C:\src\ww-zephyr
west build -b waveform_workout -d $build "$fw/tests/renode"
```

With Renode's installation directory on PATH, set up the runner once in a separate Python environment.

```powershell
$renodeDir = Split-Path (Get-Command renode.exe).Source
$venv = Join-Path $env:LOCALAPPDATA 'ww-renode-test-venv'
python -m venv $venv
& "$venv/Scripts/python.exe" -m pip install -r "$renodeDir/tests/requirements.txt"
```

Run the suite and open the generated `report.html` in the results directory. Give each run its own directory because Renode 1.17.1 can fail when reopening existing Robot log paths on Windows. The upstream platform fetches its register description on first use.

```powershell
$env:Path = "$venv/Scripts;$env:Path"
$elf = (Resolve-Path "$build/zephyr/zephyr.elf").Path.Replace('\', '/')
$results = Join-Path $env:TEMP ("ww-renode-" + (Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
& "$renodeDir/renode-test.bat" --variable "ELF:$elf" -r $results "$fw/tests/renode/trainer.robot"
```

## Coverage and bench follow-up

The test image compiles the production event loop, gpio-keys input path, lesson code, EEPROM driver and record format, shell commands, I²C scanner, and OLED code. Test instrumentation reports each playback request on USART1. The test overlay moves the shell to USART1 and uses SPI2 without DMA. The I²C target acknowledges commands. The J6 power assertion reads the GPIO output latch.

USB tests drive the board's active-low PA15 VBUS input and supply millivolts through a test voltage sensor. They run the production 100 ms power worker, USB initialization, CDC ACM registration, and Zephyr USB stack with a software device controller. They check the stack's enabled flag and count enable/disable calls after each input change. The retry case injects a single error before forwarding subsequent requests to the stack. No USB host participates.

On the boards, measure TIM6/DMA dual-DAC playback, TIM2/TIM21 pulse alignment, filter response under load, and pixel timing. Check native USB enumeration and reconnects, visible OLED output, and J6 rail voltage. The EEPROM fault cases exercise recovery from injected record contents. Physical power-loss timing needs a bench test.

Verified on Windows with Zephyr 4.5.0 and Renode 1.17.1 on 2026-11-28. All twelve cases passed.
