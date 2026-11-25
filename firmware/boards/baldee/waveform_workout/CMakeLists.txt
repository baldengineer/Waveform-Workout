# SPDX-License-Identifier: MIT

if(CONFIG_UDC_STM32 OR CONFIG_USB_DC_STM32)
  zephyr_sources(usb_clock.c)
endif()
