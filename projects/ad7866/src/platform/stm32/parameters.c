/***************************************************************************//**
 *   @file   parameters.c
 *   @brief  Parameters for STM32 platform used by ad7866 project.
 *   @author John Erasmus Mari Geronimo (johnerasmusmari.geronimo@analog.com)
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#include "parameters.h"

struct stm32_uart_init_param uart_extra_ip = {
	.huart = &huart5,
};

struct stm32_spi_init_param spi_extra_ip = {
	.chip_select_port = SPI_CS_PORT,
};

struct stm32_gpio_init_param gpio_extra_ip = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};
