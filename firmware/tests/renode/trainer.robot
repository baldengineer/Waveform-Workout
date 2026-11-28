*** Settings ***
Documentation    Waveform Workout Rev 1 front panel, EEPROM, J6, and USB power monitoring.

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
    [Arguments]    ${expected}    ${vbus_pin}=true
    # PA15 is the active-low VBUS detector. High represents no cable.
    Execute Command    sysbus.gpioPortA OnGPIO 15 ${vbus_pin}
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

Set Test Input
    [Arguments]    ${symbol}    ${value}
    Execute Command    sysbus WriteDoubleWord `sysbus GetSymbolAddress "${symbol}"` ${value}

Check Test Counter
    [Arguments]    ${symbol}    ${expected}
    ${value}=    Execute Command    sysbus ReadDoubleWord `sysbus GetSymbolAddress "${symbol}"`
    Should Be Equal As Integers    ${value}    ${expected}    ${symbol}

Check USB After Two Polls
    [Arguments]    ${enabled}    ${enable_calls}    ${disable_calls}
    Execute Command    emulation RunFor "0.22"
    Check Test Counter    test_stack_enabled    ${enabled}
    Check Test Counter    test_enable_calls    ${enable_calls}
    Check Test Counter    test_disable_calls    ${disable_calls}

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

USB Attach And Detach Respect Both Millivolt Boundaries
    Create Trainer
    Boot Trainer    shape sine
    Set Test Input    test_vdd_mv    3099
    Execute Command    sysbus.gpioPortA OnGPIO 15 false
    Check USB After Two Polls    0    0    0
    Set Test Input    test_vdd_mv    3100
    Check USB After Two Polls    1    1    0
    Set Test Input    test_vdd_mv    3000
    Check USB After Two Polls    1    1    0
    Set Test Input    test_vdd_mv    2999
    Check USB After Two Polls    0    1    1
    Set Test Input    test_vdd_mv    3099
    Check USB After Two Polls    0    1    1
    Set Test Input    test_vdd_mv    3100
    Check USB After Two Polls    1    2    1

Repeated Cable Reconnects Preserve Standalone Playback
    Create Trainer
    Boot Trainer    shape sine    vbus_pin=false
    Check USB After Two Polls    1    1    0
    FOR    ${cycle}    IN RANGE    1    4
        Execute Command    sysbus.gpioPortA OnGPIO 15 true
        Check USB After Two Polls    0    ${cycle}    ${cycle}
        IF    ${cycle} == 1
            Press Key    12    shape square
        END
        Set Test Input    test_vdd_mv    3050
        Execute Command    sysbus.gpioPortA OnGPIO 15 false
        Check USB After Two Polls    0    ${cycle}    ${cycle}
        Set Test Input    test_vdd_mv    3447
        Check USB After Two Polls    1    ${cycle + 1}    ${cycle}
    END
    Check Test Counter    test_playback_requests    2
    Check Test Counter    test_idle_requests    0
    Write Line To Uart    trainer settings show
    Wait For Line On Uart    shape square

Failed Supply Read Detaches And Recovery Reattaches
    Create Trainer
    Boot Trainer    shape sine    vbus_pin=false
    Check USB After Two Polls    1    1    0
    FOR    ${fault}    IN RANGE    1    3
        Set Test Input    test_sensor_fault    ${fault}
        Check USB After Two Polls    0    ${fault}    ${fault}
        Set Test Input    test_sensor_fault    0
        Check USB After Two Polls    1    ${fault + 1}    ${fault}
    END

Failed Stack Transitions Are Retried
    Create Trainer
    Boot Trainer    shape sine
    Set Test Input    test_fail_next_transition    1
    Execute Command    sysbus.gpioPortA OnGPIO 15 false
    Check USB After Two Polls    1    2    0
    Set Test Input    test_fail_next_transition    1
    Execute Command    sysbus.gpioPortA OnGPIO 15 true
    Check USB After Two Polls    0    2    2
