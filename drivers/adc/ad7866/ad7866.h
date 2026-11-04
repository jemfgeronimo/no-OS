/***************************************************************************//**
 *   @file   ad7866.h
 *   @brief  Header file of AD7866 Driver.
 *   @author John Erasmus Mari Geronimo (johnerasmusmari.geronimo@analog.com)
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#ifndef __AD7866_H__
#define __AD7866_H__

#include <stdint.h>
#include "no_os_gpio.h"
#include "no_os_spi.h"
#include "no_os_util.h"

#define AD7866_RESOLUTION		12
#define AD7866_NUM_CHANNELS		4

/* Each 16-bit word is: 1 leading zero, 3 STATUS bits, 12 data bits. */
#define AD7866_ZERO_MASK		NO_OS_BIT(15)
#define AD7866_STATUS_RANGE_MASK	NO_OS_BIT(14)
#define AD7866_STATUS_A0_MASK		NO_OS_BIT(13)
#define AD7866_STATUS_AB_MASK		NO_OS_BIT(12)
#define AD7866_DATA_MASK		NO_OS_GENMASK(11, 0)

#define AD7866_SPI_MIN_HZ		10000
#define AD7866_SPI_MAX_HZ		20000000

#define AD7866_VREF_INTERNAL_MV		2500
#define AD7866_VREF_MIN_MV		2000
#define AD7866_VREF_MAX_MV		3000
#define AD7866_VDD_MIN_MV		2700
#define AD7866_VDD_MAX_MV		5250

/**
 * @enum ad7866_channel
 * @brief Analog input channels. VA1/VA2 feed ADC A and VB1/VB2 feed ADC B.
 */
enum ad7866_channel {
	AD7866_CH_VA1,
	AD7866_CH_VA2,
	AD7866_CH_VB1,
	AD7866_CH_VB2,
};

/**
 * @enum ad7866_pair
 * @brief Pair of channels converted simultaneously, selected by the A0 pin.
 */
enum ad7866_pair {
	/** A0 low: VA1 and VB1 */
	AD7866_PAIR_1,
	/** A0 high: VA2 and VB2 */
	AD7866_PAIR_2,
};

/**
 * @enum ad7866_range
 * @brief Input range and output coding, selected by the RANGE pin.
 */
enum ad7866_range {
	/** RANGE low: 0 V to VREF, straight binary coding */
	AD7866_RANGE_0_TO_VREF,
	/** RANGE high: VREF +/- VREF (0 to 2 x VREF), twos complement */
	AD7866_RANGE_VREF_PM_VREF,
};

/**
 * @struct ad7866_init_param
 * @brief Initialization parameters of the AD7866.
 */
struct ad7866_init_param {
	/** SPI parameters, mode 2 (CPOL = 1, CPHA = 0) */
	struct no_os_spi_init_param *spi_init;
	/** A0 pin. NULL if hardwired to the level given by pair */
	struct no_os_gpio_init_param *gpio_a0_init;
	/** RANGE pin. NULL if hardwired to the level given by range */
	struct no_os_gpio_init_param *gpio_range_init;
	/** Reference voltage in mV (2500 when using the internal reference) */
	uint32_t vref_mv;
	/** AVDD supply voltage in mV */
	uint32_t vdd_mv;
	/** Initial channel pair */
	enum ad7866_pair pair;
	/** Initial input range */
	enum ad7866_range range;
};

/**
 * @struct ad7866_desc
 * @brief AD7866 device descriptor.
 */
struct ad7866_desc {
	struct no_os_spi_desc *spi_desc;
	struct no_os_gpio_desc *gpio_a0;
	struct no_os_gpio_desc *gpio_range;
	uint32_t vref_mv;
	uint32_t vdd_mv;
	/* The pins are sampled on the CS falling edge and apply to the next
	 * conversion: pair/range are what the next frame converts, while
	 * pair_sel/range_sel are the levels currently driven on the pins.
	 */
	enum ad7866_pair pair;
	enum ad7866_range range;
	enum ad7866_pair pair_sel;
	enum ad7866_range range_sel;
};

/* Select the input range and output coding. */
int ad7866_set_range(struct ad7866_desc *desc, enum ad7866_range range);

/* Select the channel pair of the following conversions. */
int ad7866_set_pair(struct ad7866_desc *desc, enum ad7866_pair pair);

/* Read one simultaneous conversion of ADC A and ADC B. */
int ad7866_convert(struct ad7866_desc *desc, enum ad7866_pair next_pair,
		   int16_t *val_a, int16_t *val_b);

/* Read the raw code of a single channel. */
int ad7866_read_channel(struct ad7866_desc *desc, enum ad7866_channel channel,
			int16_t *val);

/* Convert a raw code to millivolts using the current range. */
int32_t ad7866_raw_to_mv(const struct ad7866_desc *desc, int16_t raw);

/* Initialize the AD7866. */
int ad7866_init(struct ad7866_desc **desc,
		struct ad7866_init_param *init_param);

/* Remove the resources allocated by ad7866_init(). */
int ad7866_remove(struct ad7866_desc *desc);

#endif /* __AD7866_H__ */
