/* SPDX-License-Identifier: MIT */

#include <zephyr/init.h>
#include <stm32_ll_bus.h>
#include <stm32_ll_crs.h>

static int usb_clock_recovery_init(void)
{
	/* HSI48 tracks the host's 1 kHz USB start-of-frame packets. */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_CRS);
	LL_CRS_SetSyncSignalSource(LL_CRS_SYNC_SOURCE_USB);
	LL_CRS_EnableAutoTrimming();
	LL_CRS_EnableFreqErrorCounter();
	return 0;
}

SYS_INIT(usb_clock_recovery_init, PRE_KERNEL_2, 0);
