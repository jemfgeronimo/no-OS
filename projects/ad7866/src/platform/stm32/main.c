/***************************************************************************//**
 *   @file   main.c
 *   @brief  Main file for STM32 platform of ad7866 project
 *   @author John Erasmus Mari Geronimo (johnerasmusmari.geronimo@analog.com)
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#include "parameters.h"
#include "common_data.h"
#include "no_os_error.h"
#include "no_os_print_log.h"
#include "no_os_uart.h"

extern int example_main();

int main()
{
	struct no_os_uart_desc *uart_desc;
	int ret;

	spi_extra_ip.get_input_clock = HAL_RCC_GetPCLK1Freq;

	stm32_init();

	ret = no_os_uart_init(&uart_desc, &ad7866_uart_ip);
	if (ret)
		return ret;

	no_os_uart_stdio(uart_desc);

	ret = example_main();
	pr_info("Example main returned %d\n", ret);

	return ret;
}
