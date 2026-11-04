/***************************************************************************//**
 *   @file   ad7866_data.c
 *   @brief  AD7866 data for the ad7866-admt4000 project.
 *   @author John Erasmus Mari Geronimo (johnerasmusmari.geronimo@analog.com)
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#include "ad7866_data.h"
#include "no_os_gpio.h"
#include "no_os_spi.h"

struct stm32_spi_init_param spi_ad7866_extra_ip = {
	.chip_select_port = AD7866_CS_PORT,
};

static struct no_os_spi_init_param ad7866_spi_ip = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = SPI_BAUDRATE,
	.chip_select = AD7866_CS,
	.mode = NO_OS_SPI_MODE_2,
	.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
	.platform_ops = SPI_OPS,
	.extra = AD7866_SPI_EXTRA,
};

static struct no_os_gpio_init_param ad7866_gpio_a0_ip = {
	.port = AD7866_GPIO_A0_PORT,
	.number = AD7866_GPIO_A0,
	.platform_ops = GPIO_OPS,
	.extra = GPIO_PP_EXTRA,
};

static struct no_os_gpio_init_param ad7866_gpio_range_ip = {
	.port = AD7866_GPIO_RANGE_PORT,
	.number = AD7866_GPIO_RANGE,
	.platform_ops = GPIO_OPS,
	.extra = GPIO_PP_EXTRA,
};

struct ad7866_init_param ad7866_ip = {
	.spi_init = &ad7866_spi_ip,
	.gpio_a0_init = &ad7866_gpio_a0_ip,
	.gpio_range_init = &ad7866_gpio_range_ip,
	.vref_mv = AD7866_VREF_INTERNAL_MV,
	.vdd_mv = AD7866_VDD_MV,
	.pair = AD7866_PAIR_1,
	.range = AD7866_RANGE_0_TO_VREF,
};
