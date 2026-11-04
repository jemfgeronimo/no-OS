/***************************************************************************//**
 *   @file   iio_ad7866.c
 *   @brief  Source file of the AD7866 IIO Driver.
 *   @author John Erasmus Mari Geronimo (johnerasmusmari.geronimo@analog.com)
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "iio_ad7866.h"
#include "no_os_alloc.h"
#include "no_os_error.h"

#define AD7866_PAIR_1_MASK	(NO_OS_BIT(AD7866_CH_VA1) | \
				 NO_OS_BIT(AD7866_CH_VB1))
#define AD7866_PAIR_2_MASK	(NO_OS_BIT(AD7866_CH_VA2) | \
				 NO_OS_BIT(AD7866_CH_VB2))

#define AD7866_CHANNEL(_idx) {			\
	.ch_type = IIO_VOLTAGE,			\
	.channel = _idx,			\
	.indexed = true,			\
	.attributes = ad7866_iio_attrs,		\
	.address = _idx,			\
	.scan_type = &ad7866_iio_scan_type,	\
	.scan_index = _idx			\
}

static struct scan_type ad7866_iio_scan_type = {
	.sign = 'u',
	.realbits = AD7866_RESOLUTION,
	.storagebits = 16,
	.shift = 0,
	.is_big_endian = false,
};

static const char *const ad7866_range_avail[] = {
	[AD7866_RANGE_0_TO_VREF] = "0_TO_VREF",
	[AD7866_RANGE_VREF_PM_VREF] = "VREF_PM_VREF",
};

static int ad7866_iio_read_raw(void *device, char *buf, uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	struct ad7866_iio_desc *iio_desc = device;
	int32_t val32;
	int16_t val;
	int ret;

	ret = ad7866_read_channel(iio_desc->ad7866_desc, channel->address,
				  &val);
	if (ret)
		return ret;

	val32 = val;

	return iio_format_value(buf, len, IIO_VAL_INT, 1, &val32);
}

/* Scale in mV/LSB: VREF / 4096 for 0_TO_VREF, 2 * VREF / 4096 otherwise. */
static int ad7866_iio_read_scale(void *device, char *buf, uint32_t len,
				 const struct iio_ch_info *channel,
				 intptr_t priv)
{
	struct ad7866_iio_desc *iio_desc = device;
	struct ad7866_desc *desc = iio_desc->ad7866_desc;
	int32_t vals[2];

	vals[0] = desc->vref_mv;
	if (desc->range == AD7866_RANGE_VREF_PM_VREF)
		vals[0] *= 2;

	vals[1] = AD7866_RESOLUTION;

	return iio_format_value(buf, len, IIO_VAL_FRACTIONAL_LOG2, 2, vals);
}

/* The VREF_PM_VREF range is twos complement with 0 V at code -2048. */
static int ad7866_iio_read_offset(void *device, char *buf, uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t priv)
{
	struct ad7866_iio_desc *iio_desc = device;
	int32_t val32 = 0;

	if (iio_desc->ad7866_desc->range == AD7866_RANGE_VREF_PM_VREF)
		val32 = 1 << (AD7866_RESOLUTION - 1);

	return iio_format_value(buf, len, IIO_VAL_INT, 1, &val32);
}

static int ad7866_iio_read_range(void *device, char *buf, uint32_t len,
				 const struct iio_ch_info *channel,
				 intptr_t priv)
{
	struct ad7866_iio_desc *iio_desc = device;

	return sprintf(buf, "%s ",
		       ad7866_range_avail[iio_desc->ad7866_desc->range]);
}

static int ad7866_iio_write_range(void *device, char *buf, uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t priv)
{
	struct ad7866_iio_desc *iio_desc = device;
	uint32_t i;
	int ret;

	for (i = 0; i < NO_OS_ARRAY_SIZE(ad7866_range_avail); i++)
		if (!strcmp(buf, ad7866_range_avail[i]))
			break;

	if (i == NO_OS_ARRAY_SIZE(ad7866_range_avail))
		return -EINVAL;

	ret = ad7866_set_range(iio_desc->ad7866_desc, i);
	if (ret)
		return ret;

	iio_desc->iio_dev->channels->scan_type->sign =
		(i == AD7866_RANGE_VREF_PM_VREF) ? 's' : 'u';

	return 0;
}

static int ad7866_iio_read_range_avail(void *device, char *buf, uint32_t len,
				       const struct iio_ch_info *channel,
				       intptr_t priv)
{
	uint32_t length = 0;
	uint32_t i;

	for (i = 0; i < NO_OS_ARRAY_SIZE(ad7866_range_avail); i++)
		length += sprintf(buf + length, "%s ", ad7866_range_avail[i]);

	return length;
}

static int ad7866_iio_pre_enable(void *device, uint32_t mask)
{
	struct ad7866_iio_desc *iio_desc = device;
	enum ad7866_pair pair;

	if (!mask)
		return -EINVAL;

	iio_desc->active_channels = mask;
	iio_desc->nb_active_channels = no_os_hweight16(mask);

	/* Start the sequence on the first pair that has an active channel. */
	pair = (mask & AD7866_PAIR_1_MASK) ? AD7866_PAIR_1 : AD7866_PAIR_2;

	return ad7866_set_pair(iio_desc->ad7866_desc, pair);
}

static int ad7866_iio_post_disable(void *device)
{
	struct ad7866_iio_desc *iio_desc = device;

	iio_desc->active_channels = 0;
	iio_desc->nb_active_channels = 0;

	return 0;
}

/*
 * Read one scan of the active channels, in ascending channel order. Both
 * pairs are only converted when they hold an active channel, and each
 * conversion selects the pair of the next one so no conversion is wasted.
 */
static int ad7866_iio_read_scan(struct ad7866_iio_desc *iio_desc,
				uint16_t *scan)
{
	enum ad7866_pair pairs[2];
	int16_t vals[AD7866_NUM_CHANNELS];
	uint8_t nb_pairs = 0;
	int16_t val_a;
	int16_t val_b;
	uint8_t i;
	uint8_t j;
	int ret;

	if (iio_desc->active_channels & AD7866_PAIR_1_MASK)
		pairs[nb_pairs++] = AD7866_PAIR_1;
	if (iio_desc->active_channels & AD7866_PAIR_2_MASK)
		pairs[nb_pairs++] = AD7866_PAIR_2;

	/* A single channel read in between can leave another pair selected. */
	ret = ad7866_set_pair(iio_desc->ad7866_desc, pairs[0]);
	if (ret)
		return ret;

	for (i = 0; i < nb_pairs; i++) {
		ret = ad7866_convert(iio_desc->ad7866_desc,
				     pairs[(i + 1) % nb_pairs], &val_a, &val_b);
		if (ret)
			return ret;

		vals[AD7866_CH_VA1 + pairs[i]] = val_a;
		vals[AD7866_CH_VB1 + pairs[i]] = val_b;
	}

	for (i = 0, j = 0; i < AD7866_NUM_CHANNELS; i++)
		if (iio_desc->active_channels & NO_OS_BIT(i))
			scan[j++] = vals[i];

	return 0;
}

static int ad7866_iio_read_samples(void *device, void *buff, uint32_t samples)
{
	struct ad7866_iio_desc *iio_desc = device;
	uint16_t *buff_data = buff;
	uint32_t i;
	int ret;

	for (i = 0; i < samples; i++) {
		ret = ad7866_iio_read_scan(iio_desc, buff_data);
		if (ret)
			return ret;

		buff_data += iio_desc->nb_active_channels;
	}

	return samples;
}

static struct iio_attribute ad7866_iio_attrs[] = {
	{
		.name = "raw",
		.show = ad7866_iio_read_raw,
	},
	{
		.name = "scale",
		.show = ad7866_iio_read_scale,
	},
	{
		.name = "offset",
		.show = ad7866_iio_read_offset,
	},
	END_ATTRIBUTES_ARRAY
};

static struct iio_attribute ad7866_global_attrs[] = {
	{
		.name = "range",
		.show = ad7866_iio_read_range,
		.store = ad7866_iio_write_range,
	},
	{
		.name = "range_available",
		.show = ad7866_iio_read_range_avail,
	},
	END_ATTRIBUTES_ARRAY
};

static struct iio_channel ad7866_channels[] = {
	AD7866_CHANNEL(AD7866_CH_VA1),
	AD7866_CHANNEL(AD7866_CH_VA2),
	AD7866_CHANNEL(AD7866_CH_VB1),
	AD7866_CHANNEL(AD7866_CH_VB2),
};

static struct iio_device ad7866_iio_dev = {
	.num_ch = NO_OS_ARRAY_SIZE(ad7866_channels),
	.channels = ad7866_channels,
	.attributes = ad7866_global_attrs,
	.read_dev = ad7866_iio_read_samples,
	.pre_enable = ad7866_iio_pre_enable,
	.post_disable = ad7866_iio_post_disable,
};

/**
 * @brief Initialize the AD7866 IIO device.
 * @param iio_desc - Pointer to the IIO device descriptor to allocate.
 * @param init_param - Initialization parameters.
 * @return 0 in case of success, negative error code otherwise.
 */
int ad7866_iio_init(struct ad7866_iio_desc **iio_desc,
		    struct ad7866_iio_init_param *init_param)
{
	struct ad7866_iio_desc *iio_descriptor;
	int ret;

	if (!init_param || !init_param->ad7866_init)
		return -EINVAL;

	iio_descriptor = (struct ad7866_iio_desc *)no_os_calloc(1,
			 sizeof(*iio_descriptor));
	if (!iio_descriptor)
		return -ENOMEM;

	iio_descriptor->iio_dev = &ad7866_iio_dev;

	ret = ad7866_init(&iio_descriptor->ad7866_desc,
			  init_param->ad7866_init);
	if (ret)
		goto free_desc;

	iio_descriptor->iio_dev->channels->scan_type->sign =
		(init_param->ad7866_init->range == AD7866_RANGE_VREF_PM_VREF) ?
		's' : 'u';

	*iio_desc = iio_descriptor;

	return 0;

free_desc:
	no_os_free(iio_descriptor);

	return ret;
}

/**
 * @brief Free the resources allocated by ad7866_iio_init().
 * @param iio_desc - AD7866 IIO device descriptor.
 * @return 0 in case of success, negative error code otherwise.
 */
int ad7866_iio_remove(struct ad7866_iio_desc *iio_desc)
{
	if (!iio_desc)
		return -EINVAL;

	ad7866_remove(iio_desc->ad7866_desc);
	no_os_free(iio_desc);

	return 0;
}
