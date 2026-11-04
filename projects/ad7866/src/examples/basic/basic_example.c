/***************************************************************************//**
 *   @file   basic_example.c
 *   @brief  Basic example for the ad7866 project.
 *   @author John Erasmus Mari Geronimo (johnerasmusmari.geronimo@analog.com)
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#include "common_data.h"
#include "no_os_delay.h"
#include "no_os_print_log.h"

#define AD7866_PRINT_PERIOD_MS	1000

int example_main()
{
	struct ad7866_desc *ad7866;
	int16_t val;
	int ret;
	int i;

	ret = ad7866_init(&ad7866, &ad7866_ip);
	if (ret) {
		pr_err("AD7866 init failed: %d\n", ret);
		return ret;
	}

	while (1) {
		for (i = 0; i < AD7866_NUM_CHANNELS; i++) {
			ret = ad7866_read_channel(ad7866, i, &val);
			if (ret) {
				pr_err("Channel %d read failed: %d\n", i, ret);
				goto remove;
			}

			pr_info("Channel %d: raw %d, %d mV\n", i, val,
				ad7866_raw_to_mv(ad7866, val));
		}

		no_os_mdelay(AD7866_PRINT_PERIOD_MS);
	}

remove:
	ad7866_remove(ad7866);

	return ret;
}
