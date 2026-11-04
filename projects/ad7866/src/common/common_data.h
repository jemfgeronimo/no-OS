/***************************************************************************//**
 *   @file   common_data.h
 *   @brief  Defines common data to be used by ad7866 examples.
 *   @author John Erasmus Mari Geronimo (johnerasmusmari.geronimo@analog.com)
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#ifndef COMMON_DATA_H_
#define COMMON_DATA_H_

#include "parameters.h"
#include "no_os_gpio.h"
#include "no_os_spi.h"
#include "no_os_uart.h"
#include "ad7866.h"

extern struct no_os_uart_init_param ad7866_uart_ip;
extern struct ad7866_init_param ad7866_ip;

#endif /* COMMON_DATA_H_ */
