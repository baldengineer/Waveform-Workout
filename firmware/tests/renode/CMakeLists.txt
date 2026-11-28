# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.20.0)
set(firmware "${CMAKE_CURRENT_SOURCE_DIR}/../..")
list(APPEND BOARD_ROOT "${firmware}")
set(KCONFIG_ROOT "${firmware}/app/Kconfig")
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(trainer_renode)
target_include_directories(app PRIVATE "${firmware}/app/src")
target_sources(app PRIVATE
  "${firmware}/app/src/main.c"
  "${firmware}/app/src/lesson.c"
  "${firmware}/app/src/preset_store.c"
  "${firmware}/app/src/trainer_shell.c"
  "${firmware}/app/src/i2c_scan.c"
  "${firmware}/app/src/usb_console.c"
  "${firmware}/app/src/oled.c"
  test_usb.c
  test_outputs.c)
target_link_options(app PUBLIC -Wl,--wrap=usbd_enable -Wl,--wrap=usbd_disable)
