/***************************************************************************//**
 *   @file   iio_lt8491.c
 *   @brief  Implementation of IIO LT8491 Driver.
 *   @author John Erasmus Mari Geronimo (johnerasmusmari.geronimo@analog.com)
********************************************************************************
 * Copyright 2024(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include <errno.h>
#include "iio_lt8491.h"
#include "lt8491.h"
#include "no_os_alloc.h"
#include "no_os_error.h"

#define LT8491_IIO_CH_ATTR_RW(_name, _priv)	\
	{					\
		.name = _name,			\
		.priv = _priv,			\
		.show = lt8491_ch_attr_show,	\
		.store = lt8491_ch_attr_store,	\
	}

#define LT8491_IIO_CH_ATTR_RO(_name, _priv)	\
	{					\
		.name = _name,			\
		.priv = _priv,			\
		.show = lt8491_ch_attr_show,	\
		.store = NULL,			\
	}

#define LT8491_IIO_CH_ATTR_WO(_name, _priv)	\
	{					\
		.name = _name,			\
		.priv = _priv,			\
		.show = NULL,			\
		.store = lt8491_ch_attr_store,	\
	}

enum lt8491_attr_priv {
	LT8491_TBAT,
	LT8491_POUT,
	LT8491_PIN,
	LT8491_EFF,
	LT8491_IOUT,
	LT8491_IIN,
	LT8491_VBAT,
	LT8491_VIN,
	LT8491_CHARGING_STAGE,
	LT8491_CHARGING_STATUS,
	LT8491_ENABLE,
	LT8491_RESTART,
	LT8491_SCRATCH,
	LT8491_SERIAL_ID,
};

static int32_t lt8491_iio_reg_read(struct lt8491_iio_device *dev,
				   uint32_t reg, uint32_t *readval)
{
	return lt8491_reg_read(dev->dev, (uint8_t)reg, (uint16_t*)readval,
			       false);
}

static int32_t lt8491_iio_reg_write(struct lt8491_iio_device *dev,
				    uint32_t reg, uint32_t writeval)
{
	return lt8491_reg_write(dev->dev, (uint8_t)reg, (uint16_t)writeval,
				false);
}

static int lt8491_ch_attr_show(void *ddev, char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	struct lt8491_iio_device *dev = ddev;
	int ret;
	int32_t val;
	uint32_t uval, uvals[3];

	switch (priv) {
	case LT8491_TBAT:
		ret = lt8491_read_tbat(dev->dev, &val);
		if (ret)
			return ret;

		return iio_format_value(buf, len, IIO_VAL_INT, 1, &val);
	case LT8491_POUT:
		ret = lt8491_read_pout(dev->dev, &uval);
		if (ret)
			return ret;

		val = uval;

		return iio_format_value(buf, len, IIO_VAL_INT, 1, &val);
	case LT8491_PIN:
		ret = lt8491_read_pin(dev->dev, &uval);
		if (ret)
			return ret;

		val = uval;

		return iio_format_value(buf, len, IIO_VAL_INT, 1, &val);
	case LT8491_EFF:
		ret = lt8491_read_efficiency(dev->dev, &uval);
		if (ret)
			return ret;

		val = uval;

		return iio_format_value(buf, len, IIO_VAL_INT, 1, &val);
	case LT8491_IOUT:
		ret = lt8491_read_iout(dev->dev, &uval);
		if (ret)
			return ret;

		val = uval;

		return iio_format_value(buf, len, IIO_VAL_INT, 1, &val);
	case LT8491_IIN:
		ret = lt8491_read_iin(dev->dev, &uval);
		if (ret)
			return ret;

		val = uval;

		return iio_format_value(buf, len, IIO_VAL_INT, 1, &val);
	case LT8491_VBAT:
		ret = lt8491_read_vbat(dev->dev, &uval);
		if (ret)
			return ret;

		val = uval;

		return iio_format_value(buf, len, IIO_VAL_INT, 1, &val);
	case LT8491_VIN:
		ret = lt8491_read_vin(dev->dev, &uval);
		if (ret)
			return ret;

		val = uval;

		return iio_format_value(buf, len, IIO_VAL_INT, 1, &val);
	case LT8491_CHARGING_STAGE:
		ret = lt8491_read_charging_stage(dev->dev, &val);
		if (ret)
			return ret;

		return iio_format_value(buf, len, IIO_VAL_CHAR, 1, &val);
	case LT8491_CHARGING_STATUS:
		ret = lt8491_read_charging_status(dev->dev, &uval);
		if (ret)
			return ret;

		val = uval;

		return iio_format_value(buf, len, IIO_VAL_INT, 1, &val);
	case LT8491_ENABLE:
		ret = lt8491_read_enable(dev->dev, &uval);
		if (ret)
			return ret;

		val = uval;

		return iio_format_value(buf, len, IIO_VAL_INT, 1, &val);
	case LT8491_SCRATCH:
		ret = lt8491_read_scratch(dev->dev, &uval);
		if (ret)
			return ret;

		val = uval;

		return iio_format_value(buf, len, IIO_VAL_INT, 1, &val);
	case LT8491_SERIAL_ID:
		ret = lt8491_read_serial_id(dev->dev, uvals);
		if (ret)
			return ret;

		return iio_format_value(buf, len, IIO_VAL_INT_MULTIPLE,
					NO_OS_ARRAY_SIZE(uvals),
					(int32_t *)uvals);
	default:
		return -EOPNOTSUPP;
	}
}

static int lt8491_ch_attr_store(void *ddev, char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	struct lt8491_iio_device *dev = ddev;
	int32_t val;
	int ret;

	switch (priv) {
	case LT8491_ENABLE:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;

		if (val >> 1)
			return -EINVAL;

		ret = lt8491_write_enable(dev->dev, val);
		if (ret)
			return ret;

		return len;
	case LT8491_RESTART:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;

		if (val >> 1)
			return -EINVAL;

		if (val) {
			ret = lt8491_restart_chip(dev->dev);
			if (ret)
				return ret;
		}

		return len;
	case LT8491_SCRATCH:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;

		ret = lt8491_write_scratch(dev->dev, val);
		if (ret)
			return ret;

		return len;
	default:
		return -EOPNOTSUPP;
	}

	if (ret)
		return ret;

	return len;
}

static struct iio_attribute lt8491_iio_attrs[] = {
	LT8491_IIO_CH_ATTR_RO("tbat", LT8491_TBAT),
	LT8491_IIO_CH_ATTR_RO("pout", LT8491_POUT),
	LT8491_IIO_CH_ATTR_RO("pin", LT8491_PIN),
	LT8491_IIO_CH_ATTR_RO("efficiency", LT8491_EFF),
	LT8491_IIO_CH_ATTR_RO("iout", LT8491_IOUT),
	LT8491_IIO_CH_ATTR_RO("iin", LT8491_IIN),
	LT8491_IIO_CH_ATTR_RO("vbat", LT8491_VBAT),
	LT8491_IIO_CH_ATTR_RO("vin", LT8491_VIN),
	LT8491_IIO_CH_ATTR_RO("charging_stage", LT8491_CHARGING_STAGE),
	LT8491_IIO_CH_ATTR_RO("charging_status", LT8491_CHARGING_STATUS),
	LT8491_IIO_CH_ATTR_RW("enable", LT8491_ENABLE),
	LT8491_IIO_CH_ATTR_WO("reset", LT8491_RESTART),
	LT8491_IIO_CH_ATTR_RW("scratch", LT8491_SCRATCH),
	LT8491_IIO_CH_ATTR_RO("serial_id", LT8491_SERIAL_ID),
	END_ATTRIBUTES_ARRAY
};

static struct iio_device lt8491_iio_dev = {
	.debug_reg_read = (int32_t (*)()) lt8491_iio_reg_read,
	.debug_reg_write = (int32_t (*)()) lt8491_iio_reg_write,
	.attributes = lt8491_iio_attrs,
};

/**
 * @brief Initializes the LT8491 IIO driver
 * @param iio_device - The iio device structure.
 * @param iio_init_param - Parameters for the initialization of iio_dev
 * @return 0 in case of success, errno errors otherwise
 */
int lt8491_iio_init(struct lt8491_iio_device **iio_device,
		    struct lt8491_iio_init_param *iio_init_param)
{
	struct lt8491_iio_device *iio_device_temp;
	int ret;

	if (!iio_init_param || !iio_init_param->init_param)
		return -EINVAL;

	iio_device_temp = no_os_calloc(1, sizeof(*iio_device_temp));
	if (!iio_device_temp)
		return -ENOMEM;

	ret = lt8491_init(&iio_device_temp->dev, iio_init_param->init_param);
	if (ret)
		goto free_dev;

	ret = lt8491_restart_chip(iio_device_temp->dev);
	if (ret)
		goto remove_dev;

	ret = lt8491_write_enable(iio_device_temp->dev, false);
	if (ret)
		goto remove_dev;

	ret = lt8491_configure_telemetry(iio_device_temp->dev,
					 iio_init_param->init_param);
	if (ret)
		goto remove_dev;

	iio_device_temp->iio_dev = &lt8491_iio_dev;

	*iio_device = iio_device_temp;

	return 0;

remove_dev:
	lt8491_remove(iio_device_temp->dev);
free_dev:
	no_os_free(iio_device_temp);

	return ret;
}

/**
 * @brief Free resources allocated by the init function
 * @param iio_device - The iio device structure.
 * @return 0 in case of success, errno errors otherwise
 */
int lt8491_iio_remove(struct lt8491_iio_device *iio_device)
{
	lt8491_remove(iio_device->dev);

	no_os_free(iio_device);

	return 0;
}
