/***************************************************************************//**
 *   @file   ad7866.c
 *   @brief  Source file of AD7866 Driver.
 *   @author John Erasmus Mari Geronimo (johnerasmusmari.geronimo@analog.com)
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#include <stdbool.h>
#include "ad7866.h"
#include "no_os_alloc.h"
#include "no_os_delay.h"
#include "no_os_error.h"

/* Both ADC results are read on one data line: 2 x 16 SCLK cycles. */
#define AD7866_FRAME_BYTES		4
#define AD7866_NUM_ADC			2
#define AD7866_SIGN_BIT			(AD7866_RESOLUTION - 1)
#define AD7866_CODES			(1 << AD7866_RESOLUTION)
/* Power-up time from full power-down is about 4 ms. */
#define AD7866_POWER_UP_MS		5

/**
 * @brief Read one frame of two 16-bit words, with CS held low throughout.
 * @param desc - AD7866 device descriptor.
 * @param words - Received words, in the order they were shifted out.
 * @return 0 in case of success, negative error code otherwise.
 */
static int ad7866_transfer(struct ad7866_desc *desc, uint16_t *words)
{
	uint8_t tx_buff[AD7866_FRAME_BYTES] = {0};
	uint8_t rx_buff[AD7866_FRAME_BYTES];
	struct no_os_spi_msg xfer = {
		.tx_buff = tx_buff,
		.rx_buff = rx_buff,
		.bytes_number = AD7866_FRAME_BYTES,
		.cs_change = 1,
	};
	int ret;

	ret = no_os_spi_transfer(desc->spi_desc, &xfer, 1);
	if (ret)
		return ret;

	words[0] = no_os_get_unaligned_be16(&rx_buff[0]);
	words[1] = no_os_get_unaligned_be16(&rx_buff[2]);

	return 0;
}

/**
 * @brief Decode the words of a frame.
 *
 * The STATUS bits tell which ADC a word belongs to, so the result does not
 * depend on whether DOUTA or DOUTB is wired to the SPI MISO line. The A0 and
 * RANGE status bits are checked against what the pins were set to before the
 * previous conversion, which flags a wrong wiring or a missing device.
 * @param desc - AD7866 device descriptor.
 * @param words - Words received in a frame.
 * @param val_a - Raw code of ADC A, can be NULL.
 * @param val_b - Raw code of ADC B, can be NULL.
 * @return 0 in case of success, negative error code otherwise.
 */
static int ad7866_decode(struct ad7866_desc *desc, const uint16_t *words,
			 int16_t *val_a, int16_t *val_b)
{
	int16_t val[AD7866_NUM_ADC];
	bool found[AD7866_NUM_ADC] = {false, false};
	uint16_t data;
	uint8_t adc;
	uint8_t i;

	for (i = 0; i < AD7866_NUM_ADC; i++) {
		if (words[i] & AD7866_ZERO_MASK)
			return -EIO;

		adc = no_os_field_get(AD7866_STATUS_AB_MASK, words[i]);
		if (found[adc])
			return -EIO;

		found[adc] = true;
		data = no_os_field_get(AD7866_DATA_MASK, words[i]);
		if (no_os_field_get(AD7866_STATUS_RANGE_MASK, words[i]))
			val[adc] = no_os_sign_extend16(data, AD7866_SIGN_BIT);
		else
			val[adc] = data;
	}

	if (no_os_field_get(AD7866_STATUS_A0_MASK, words[0]) != desc->pair)
		return -EIO;

	if (no_os_field_get(AD7866_STATUS_RANGE_MASK, words[0]) != desc->range)
		return -EIO;

	if (val_a)
		*val_a = val[0];
	if (val_b)
		*val_b = val[1];

	return 0;
}

/**
 * @brief Run one conversion frame with the pins already set.
 * @param desc - AD7866 device descriptor.
 * @param val_a - Raw code of ADC A, can be NULL.
 * @param val_b - Raw code of ADC B, can be NULL.
 * @return 0 in case of success, negative error code otherwise.
 */
static int ad7866_frame(struct ad7866_desc *desc, int16_t *val_a,
			int16_t *val_b)
{
	uint16_t words[AD7866_NUM_ADC];
	int ret;

	ret = ad7866_transfer(desc, words);
	if (ret)
		return ret;

	ret = ad7866_decode(desc, words, val_a, val_b);

	/* The pins were sampled on the CS falling edge of this frame. */
	desc->pair = desc->pair_sel;
	desc->range = desc->range_sel;

	return ret;
}

/**
 * @brief Drive the A0 and RANGE pins.
 * @param desc - AD7866 device descriptor.
 * @param pair - Channel pair to select.
 * @param range - Input range to select.
 * @return 0 in case of success, negative error code otherwise.
 */
static int ad7866_set_pins(struct ad7866_desc *desc, enum ad7866_pair pair,
			   enum ad7866_range range)
{
	int ret;

	if (pair != desc->pair_sel) {
		if (!desc->gpio_a0)
			return -ENOTSUP;

		ret = no_os_gpio_set_value(desc->gpio_a0,
					   pair == AD7866_PAIR_2 ?
					   NO_OS_GPIO_HIGH : NO_OS_GPIO_LOW);
		if (ret)
			return ret;

		desc->pair_sel = pair;
	}

	if (range != desc->range_sel) {
		if (!desc->gpio_range)
			return -ENOTSUP;

		ret = no_os_gpio_set_value(desc->gpio_range,
					   range == AD7866_RANGE_VREF_PM_VREF ?
					   NO_OS_GPIO_HIGH : NO_OS_GPIO_LOW);
		if (ret)
			return ret;

		desc->range_sel = range;
	}

	return 0;
}

/**
 * @brief Select the input range and output coding.
 *
 * The RANGE pin is sampled on the CS falling edge, so a dummy conversion is
 * run to apply the new range before returning.
 * @param desc - AD7866 device descriptor.
 * @param range - Input range to select.
 * @return 0 in case of success, negative error code otherwise.
 */
int ad7866_set_range(struct ad7866_desc *desc, enum ad7866_range range)
{
	int ret;

	if (range > AD7866_RANGE_VREF_PM_VREF)
		return -EINVAL;

	if (range == desc->range && range == desc->range_sel)
		return 0;

	/* The selected input range must not exceed VDD. */
	if (range == AD7866_RANGE_VREF_PM_VREF &&
	    2 * desc->vref_mv > desc->vdd_mv)
		return -EINVAL;

	ret = ad7866_set_pins(desc, desc->pair_sel, range);
	if (ret)
		return ret;

	return ad7866_frame(desc, NULL, NULL);
}

/**
 * @brief Select the channel pair of the following conversions.
 *
 * The A0 pin is sampled on the CS falling edge, so a dummy conversion is run
 * to apply the new pair before returning.
 * @param desc - AD7866 device descriptor.
 * @param pair - Channel pair to select.
 * @return 0 in case of success, negative error code otherwise.
 */
int ad7866_set_pair(struct ad7866_desc *desc, enum ad7866_pair pair)
{
	if (pair > AD7866_PAIR_2)
		return -EINVAL;

	if (pair == desc->pair && pair == desc->pair_sel)
		return 0;

	return ad7866_convert(desc, pair, NULL, NULL);
}

/**
 * @brief Read one simultaneous conversion of ADC A and ADC B.
 *
 * The conversion is done on the channel pair selected by the previous call.
 * The A0 pin is set to next_pair before the conversion starts, so reading
 * alternating pairs takes one conversion per pair. Raw codes are unsigned
 * for the 0_TO_VREF range and sign extended for the VREF_PM_VREF range.
 * @param desc - AD7866 device descriptor.
 * @param next_pair - Channel pair of the following conversion.
 * @param val_a - Raw code of ADC A, can be NULL.
 * @param val_b - Raw code of ADC B, can be NULL.
 * @return 0 in case of success, negative error code otherwise.
 */
int ad7866_convert(struct ad7866_desc *desc, enum ad7866_pair next_pair,
		   int16_t *val_a, int16_t *val_b)
{
	int ret;

	if (next_pair > AD7866_PAIR_2)
		return -EINVAL;

	ret = ad7866_set_pins(desc, next_pair, desc->range_sel);
	if (ret)
		return ret;

	return ad7866_frame(desc, val_a, val_b);
}

/**
 * @brief Read the raw code of a single channel.
 * @param desc - AD7866 device descriptor.
 * @param channel - Channel to read.
 * @param val - Raw code of the channel.
 * @return 0 in case of success, negative error code otherwise.
 */
int ad7866_read_channel(struct ad7866_desc *desc, enum ad7866_channel channel,
			int16_t *val)
{
	enum ad7866_pair pair;
	int16_t val_a;
	int16_t val_b;
	int ret;

	if (channel >= AD7866_NUM_CHANNELS)
		return -EINVAL;

	pair = channel & NO_OS_BIT(0);
	ret = ad7866_set_pair(desc, pair);
	if (ret)
		return ret;

	ret = ad7866_convert(desc, pair, &val_a, &val_b);
	if (ret)
		return ret;

	*val = (channel & NO_OS_BIT(1)) ? val_b : val_a;

	return 0;
}

/**
 * @brief Convert a raw code to millivolts using the current range.
 * @param desc - AD7866 device descriptor.
 * @param raw - Raw code, as returned by the read functions.
 * @return Voltage in millivolts.
 */
int32_t ad7866_raw_to_mv(const struct ad7866_desc *desc, int16_t raw)
{
	if (desc->range == AD7866_RANGE_VREF_PM_VREF)
		return ((int32_t)raw + AD7866_CODES / 2) * 2 *
		       (int32_t)desc->vref_mv / AD7866_CODES;

	return (int32_t)raw * (int32_t)desc->vref_mv / AD7866_CODES;
}

/**
 * @brief Initialize the AD7866.
 * @param desc - Pointer to the device descriptor to allocate.
 * @param init_param - Initialization parameters.
 * @return 0 in case of success, negative error code otherwise.
 */
int ad7866_init(struct ad7866_desc **desc,
		struct ad7866_init_param *init_param)
{
	struct ad7866_desc *descriptor;
	uint16_t words[AD7866_NUM_ADC];
	uint8_t level;
	int ret;

	if (!init_param || !init_param->spi_init)
		return -EINVAL;

	if (init_param->spi_init->max_speed_hz < AD7866_SPI_MIN_HZ ||
	    init_param->spi_init->max_speed_hz > AD7866_SPI_MAX_HZ)
		return -EINVAL;

	if (init_param->vref_mv < AD7866_VREF_MIN_MV ||
	    init_param->vref_mv > AD7866_VREF_MAX_MV)
		return -EINVAL;

	if (init_param->vdd_mv < AD7866_VDD_MIN_MV ||
	    init_param->vdd_mv > AD7866_VDD_MAX_MV)
		return -EINVAL;

	if (init_param->pair > AD7866_PAIR_2 ||
	    init_param->range > AD7866_RANGE_VREF_PM_VREF)
		return -EINVAL;

	if (init_param->range == AD7866_RANGE_VREF_PM_VREF &&
	    2 * init_param->vref_mv > init_param->vdd_mv)
		return -EINVAL;

	descriptor = (struct ad7866_desc *)no_os_calloc(1, sizeof(*descriptor));
	if (!descriptor)
		return -ENOMEM;

	descriptor->vref_mv = init_param->vref_mv;
	descriptor->vdd_mv = init_param->vdd_mv;
	descriptor->pair = init_param->pair;
	descriptor->pair_sel = init_param->pair;
	descriptor->range = init_param->range;
	descriptor->range_sel = init_param->range;

	ret = no_os_spi_init(&descriptor->spi_desc, init_param->spi_init);
	if (ret)
		goto free_desc;

	if (init_param->gpio_a0_init) {
		ret = no_os_gpio_get(&descriptor->gpio_a0,
				     init_param->gpio_a0_init);
		if (ret)
			goto free_spi;

		level = init_param->pair == AD7866_PAIR_2 ?
			NO_OS_GPIO_HIGH : NO_OS_GPIO_LOW;
		ret = no_os_gpio_direction_output(descriptor->gpio_a0, level);
		if (ret)
			goto free_gpio_a0;
	}

	if (init_param->gpio_range_init) {
		ret = no_os_gpio_get(&descriptor->gpio_range,
				     init_param->gpio_range_init);
		if (ret)
			goto free_gpio_a0;

		level = init_param->range == AD7866_RANGE_VREF_PM_VREF ?
			NO_OS_GPIO_HIGH : NO_OS_GPIO_LOW;
		ret = no_os_gpio_direction_output(descriptor->gpio_range,
						  level);
		if (ret)
			goto free_gpio_range;
	}

	/*
	 * After power-up the part may be in any power mode. A first dummy
	 * conversion wakes it up and samples the pins, then the full power-down
	 * wake-up time is waited and a checked conversion confirms the device.
	 */
	ret = ad7866_transfer(descriptor, words);
	if (ret)
		goto free_gpio_range;

	no_os_mdelay(AD7866_POWER_UP_MS);

	ret = ad7866_frame(descriptor, NULL, NULL);
	if (ret)
		goto free_gpio_range;

	*desc = descriptor;

	return 0;

free_gpio_range:
	no_os_gpio_remove(descriptor->gpio_range);
free_gpio_a0:
	no_os_gpio_remove(descriptor->gpio_a0);
free_spi:
	no_os_spi_remove(descriptor->spi_desc);
free_desc:
	no_os_free(descriptor);

	return ret;
}

/**
 * @brief Free the resources allocated by ad7866_init().
 * @param desc - AD7866 device descriptor.
 * @return 0 in case of success, negative error code otherwise.
 */
int ad7866_remove(struct ad7866_desc *desc)
{
	if (!desc)
		return -EINVAL;

	no_os_gpio_remove(desc->gpio_range);
	no_os_gpio_remove(desc->gpio_a0);
	no_os_spi_remove(desc->spi_desc);
	no_os_free(desc);

	return 0;
}
