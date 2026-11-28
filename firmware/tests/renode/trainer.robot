*** Settings ***
Documentation    Waveform Workout Rev 1 front panel, EEPROM, and J6 integration.

*** Keywords ***
Create Trainer
    [Arguments]    ${panel_address}=${EMPTY}
    Execute Command    mach create
    Execute Command    machine LoadPlatformDescription @platforms/cpus/stm32l072.repl
    IF    '${panel_address}' != ''
        Execute Command    machine LoadPlatformDescriptionFromString "panel: Mocks.DummyI2CSlave @ i2c1 ${panel_address}"
        Create Log Tester    1
        Execute Command    logLevel -1 sysbus.i2c1.panel
    END
    Execute Command    sysbus LoadELF "${ELF}"
    Create Terminal Tester    sysbus.usart1    defaultPauseEmulation=True

Boot Trainer
    [Arguments]    ${expected}
    Execute Command    sysbus.gpioPortB OnGPIO 12 true
    Execute Command    sysbus.gpioPortB OnGPIO 14 true
    Start Emulation
    Wait For Line On Uart    Playback request: ${expected}
    Execute Command    emulation RunFor "0.3"

Reset Trainer
    [Arguments]    ${expected}
    Execute Command    pause
    Execute Command    machine Reset
    Execute Command    sysbus LoadELF "${ELF}"
    Boot Trainer    ${expected}

Press Key
    [Arguments]    ${pin}    ${expected}
    Execute Command    sysbus.gpioPortB OnGPIO ${pin} false
    Wait For Line On Uart    Playback request: ${expected}
    Execute Command    sysbus.gpioPortB OnGPIO ${pin} true
    Execute Command    emulation RunFor "0.1"

Check J6 Power
    [Arguments]    ${powered}
    # STM32 GPIOB ODR bit 5 is active low.
    ${odr}=    Execute Command    sysbus ReadDoubleWord 0x50000414
    ${on}=    Evaluate    (int($odr.strip(), 0) & 0x20) == 0
    Should Be Equal    ${on}    ${powered}

*** Test Cases ***
VALUE Press Advances Once And Survives Reset
    Create Trainer
    Boot Trainer    shape sine
    Press Key    12    shape square
    Reset Trainer    shape square
    Write Line To Uart    trainer settings show
    Wait For Line On Uart    shape square

Uncommitted Newest Preset Falls Back To Previous Save
    Create Trainer
    Boot Trainer    shape sine
    Press Key    12    shape square
    Press Key    12    shape triangle
    # Slot 1 contains the second save. Simulate losing its commit marker.
    Execute Command    sysbus WriteByte 0x0808001C 0
    Reset Trainer    shape square

Bad Checksum Falls Back Even With A Valid Commit Marker
    Create Trainer
    Boot Trainer    shape sine
    Press Key    12    shape square
    Press Key    12    shape triangle
    # Change a valid shape index without updating the CRC.
    Execute Command    sysbus WriteByte 0x08080013 0
    Reset Trainer    shape square

Legacy Preset Upgrades And Sequence Wraps
    Create Trainer
    # Version 1, sequence 255, square wave; CRC-8/CCITT is 0x59.
    Execute Command    sysbus WriteDoubleWord 0x08080000 0x0100FF01
    Execute Command    sysbus WriteDoubleWord 0x08080004 0x00000102
    Execute Command    sysbus WriteDoubleWord 0x08080008 0x00005900
    Execute Command    sysbus WriteByte 0x0808000C 0xA5
    Boot Trainer    shape square
    Write Line To Uart    trainer settings show
    Wait For Line On Uart    oled_addr 0x3D
    Press Key    12    shape triangle
    ${header}=    Execute Command    sysbus ReadWord 0x08080010
    Should Be Equal As Integers    ${header}    0x0002
    Reset Trainer    shape triangle

MODE Selects Amplitude And Shell Edits Persist
    Create Trainer
    Boot Trainer    shape sine
    Execute Command    sysbus.gpioPortB OnGPIO 14 false
    Execute Command    emulation RunFor "0.1"
    Execute Command    sysbus.gpioPortB OnGPIO 14 true
    Execute Command    emulation RunFor "0.1"
    Write Line To Uart    trainer settings show
    Wait For Line On Uart    mode amp
    Write Line To Uart    trainer settings set amp 500mVpp
    Wait For Line On Uart    Saved amp 500mVpp
    Reset Trainer    amp 500mVpp

Missing OLED Releases J6 And Allows Panel Edits
    Create Trainer
    Boot Trainer    shape sine
    Check J6 Power    ${False}
    Write Line To Uart    i2c_scan
    Wait For Line On Uart    0 device(s) found on J6
    Check J6 Power    ${False}
    Press Key    12    shape square

Default OLED Address Appears On J6
    Create Trainer    0x3D
    Boot Trainer    shape sine
    Check J6 Power    ${True}
    Wait For Log Entry    i2c1.panel: Received 2 bytes: [0x0, 0xAF]
    Write Line To Uart    i2c_scan
    Wait For Line On Uart    0x3D
    Wait For Line On Uart    1 device(s) found on J6
    Check J6 Power    ${True}

Saved Alternate OLED Address Selects Display On Next Boot
    Create Trainer    0x3C
    Boot Trainer    shape sine
    Check J6 Power    ${False}
    Write Line To Uart    trainer settings set oled_addr 0x3C
    Wait For Line On Uart    Saved oled_addr 0x3C for next boot
    Reset Trainer    shape sine
    Check J6 Power    ${True}
    Wait For Log Entry    i2c1.panel: Received 2 bytes: [0x0, 0xAF]
    Write Line To Uart    i2c_scan
    Wait For Line On Uart    0x3C
    Wait For Line On Uart    1 device(s) found on J6
