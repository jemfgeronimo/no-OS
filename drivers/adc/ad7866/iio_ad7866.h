/***************************************************************************//**
 *   @file   iio_ad7866.h
 *   @brief  Header file of the AD7866 IIO Driver.
 *   @author John Erasmus Mari Geronimo (johnerasmusmari.geronimo@analog.com)
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#ifndef __IIO_AD7866_H__
#define __IIO_AD7866_H__

#include "iio.h"
#include "iio_types.h"
#include "ad7866.h"

/**
 * @struct ad7866_iio_desc
 * @brief AD7866 IIO device descriptor.
 */
struct ad7866_iio_desc {
	struct ad7866_desc *ad7866_desc;
	struct iio_device *iio_dev;
	/** Channels enabled in the buffer, one bit per enum ad7866_channel */
	uint32_t active_channels;
	uint8_t nb_active_channels;
};

/**
 * @struct ad7866_iio_init_param
 * @brief AD7866 IIO initialization parameters.
 */
struct ad7866_iio_init_param {
	struct ad7866_init_param *ad7866_init;
};

/* Initialize the AD7866 IIO device. */
int ad7866_iio_init(struct ad7866_iio_desc **iio_desc,
		    struct ad7866_iio_init_param *init_param);

/* Remove the resources allocated by ad7866_iio_init(). */
int ad7866_iio_remove(struct ad7866_iio_desc *iio_desc);

#endif /* __IIO_AD7866_H__ */
