/***************************************************************************//**
 *   @file   parameters.h
 *   @brief  Definitions specific to STM32 platform used by ad7866 project.
 *   @author John Erasmus Mari Geronimo (johnerasmusmari.geronimo@analog.com)
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/

#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#include "stm32_hal.h"
#include "stm32_gpio.h"
#include "stm32_spi.h"
#include "stm32_uart.h"
#include "stm32_uart_stdio.h"

extern UART_HandleTypeDef huart5;

#define UART_DEVICE_ID			5
#define UART_BAUDRATE			230400
#define UART_EXTRA			&uart_extra_ip
#define UART_OPS			&stm32_uart_ops
#define UART_IRQ_ID			UART5_IRQn

/* SPI on the Arduino header: D10 (CS), D11 (MOSI), D12 (MISO), D13 (SCK) */
#define SPI_DEVICE_ID			1
#define SPI_BAUDRATE			2000000
#define SPI_OPS				&stm32_spi_ops
#define SPI_CS_PORT			0
#define SPI_CS				15
#define SPI_EXTRA			&spi_extra_ip

/* A0 and RANGE pins of the AD7866 on the Arduino header: D2 and D3 */
#define GPIO_OPS			&stm32_gpio_ops
#define GPIO_EXTRA			&gpio_extra_ip
#define GPIO_A0_PORT			6
#define GPIO_A0				7
#define GPIO_RANGE_PORT			3
#define GPIO_RANGE			12

#define HW_CARRIER_NAME			"SDP_K1"

extern struct stm32_uart_init_param uart_extra_ip;
extern struct stm32_spi_init_param spi_extra_ip;
extern struct stm32_gpio_init_param gpio_extra_ip;

#endif /* __PARAMETERS_H__ */
