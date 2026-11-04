/***************************************************************************//**
 *   @file   common_data.c
 *   @brief  Defines common data to be used by ad7866 examples.
 *   @author John Erasmus Mari Geronimo (johnerasmusmari.geronimo@analog.com)
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#include "common_data.h"

struct no_os_uart_init_param ad7866_uart_ip = {
	.device_id = UART_DEVICE_ID,
	.irq_id = UART_IRQ_ID,
	.asynchronous_rx = true,
	.baud_rate = UART_BAUDRATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.extra = UART_EXTRA,
	.platform_ops = UART_OPS,
};

static struct no_os_spi_init_param ad7866_spi_ip = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = SPI_BAUDRATE,
	.chip_select = SPI_CS,
	.mode = NO_OS_SPI_MODE_2,
	.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
	.platform_ops = SPI_OPS,
	.extra = SPI_EXTRA,
};

static struct no_os_gpio_init_param ad7866_gpio_a0_ip = {
	.port = GPIO_A0_PORT,
	.number = GPIO_A0,
	.platform_ops = GPIO_OPS,
	.extra = GPIO_EXTRA,
};

static struct no_os_gpio_init_param ad7866_gpio_range_ip = {
	.port = GPIO_RANGE_PORT,
	.number = GPIO_RANGE,
	.platform_ops = GPIO_OPS,
	.extra = GPIO_EXTRA,
};

/* Set vref_mv and vdd_mv to match the board supply and reference setup. */
struct ad7866_init_param ad7866_ip = {
	.spi_init = &ad7866_spi_ip,
	.gpio_a0_init = &ad7866_gpio_a0_ip,
	.gpio_range_init = &ad7866_gpio_range_ip,
	.vref_mv = AD7866_VREF_INTERNAL_MV,
	.vdd_mv = 3300,
	.pair = AD7866_PAIR_1,
	.range = AD7866_RANGE_0_TO_VREF,
};
