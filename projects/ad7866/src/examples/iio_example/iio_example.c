/***************************************************************************//**
 *   @file   iio_example.c
 *   @brief  IIO example for the ad7866 project.
 *   @author John Erasmus Mari Geronimo (johnerasmusmari.geronimo@analog.com)
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#include "common_data.h"
#include "iio_ad7866.h"
#include "iio_app.h"
#include "no_os_print_log.h"

#ifndef DATA_BUFFER_SIZE
#define DATA_BUFFER_SIZE	400
#endif

#define AD7866_BUFFER_BYTES	(DATA_BUFFER_SIZE * AD7866_NUM_CHANNELS * \
				 sizeof(uint16_t))

static uint8_t iio_data_buffer[AD7866_BUFFER_BYTES];

int example_main()
{
	struct ad7866_iio_desc *ad7866_iio_desc;
	struct ad7866_iio_init_param ad7866_iio_ip = {
		.ad7866_init = &ad7866_ip,
	};
	struct iio_data_buffer voltage_buff = {
		.buff = (void *)iio_data_buffer,
		.size = AD7866_BUFFER_BYTES,
	};
	struct iio_app_init_param app_init_param = {0};
	struct iio_app_desc *app;
	int ret;

	ret = ad7866_iio_init(&ad7866_iio_desc, &ad7866_iio_ip);
	if (ret) {
		pr_err("AD7866 IIO init failed: %d\n", ret);
		return ret;
	}

	struct iio_app_device iio_devices[] = {
		{
			.name = "ad7866",
			.dev = ad7866_iio_desc,
			.dev_descriptor = ad7866_iio_desc->iio_dev,
			.read_buff = &voltage_buff,
		},
	};

	app_init_param.devices = iio_devices;
	app_init_param.nb_devices = NO_OS_ARRAY_SIZE(iio_devices);
	app_init_param.uart_init_params = ad7866_uart_ip;

	ret = iio_app_init(&app, app_init_param);
	if (ret)
		goto remove;

	return iio_app_run(app);

remove:
	ad7866_iio_remove(ad7866_iio_desc);

	return ret;
}
