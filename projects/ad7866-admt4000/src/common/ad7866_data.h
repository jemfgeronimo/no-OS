/***************************************************************************//**
 *   @file   ad7866_data.h
 *   @brief  AD7866 data for the ad7866-admt4000 project.
 *   @author John Erasmus Mari Geronimo (johnerasmusmari.geronimo@analog.com)
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#ifndef AD7866_DATA_H_
#define AD7866_DATA_H_

#include "parameters.h"
#include "ad7866.h"
#include "iio_ad7866.h"

/*
 * The EVAL-ADMT4000ARD1Z uses almost all the Arduino header pins, so the
 * AD7866 chip select, A0 and RANGE pins below are PLACEHOLDERS. They share
 * SPI1 with the ADMT4000 and the TMC5240 but have not been checked against
 * the SDP-K1 pinout or the wiring of the AD7866 board. Define
 * AD7866_PINS_CONFIRMED once they are final.
 */
#ifndef AD7866_PINS_CONFIRMED
#warning "AD7866 CS, A0 and RANGE pins are placeholders, check them before use"
#endif

#define AD7866_CS_PORT			2
#define AD7866_CS			8
#define AD7866_GPIO_A0_PORT		2
#define AD7866_GPIO_A0			9
#define AD7866_GPIO_RANGE_PORT		2
#define AD7866_GPIO_RANGE		10

/* Set to match the supply and the reference of the AD7866 board. */
#define AD7866_VDD_MV			3300

#define AD7866_SPI_EXTRA		&spi_ad7866_extra_ip

extern struct stm32_spi_init_param spi_ad7866_extra_ip;
extern struct ad7866_init_param ad7866_ip;

#endif /* AD7866_DATA_H_ */
