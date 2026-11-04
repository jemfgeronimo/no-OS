AD7866 no-OS driver
===================

.. no-os-doxygen::

Supported Devices
-----------------

- :adi:`AD7866`

Overview
--------

The AD7866 is a dual 12-bit, 1 MSPS successive approximation ADC with a
serial interface. Each of the two ADCs (A and B) has a 2-channel input
multiplexer and a track-and-hold amplifier, so the part has four single-ended
inputs (VA1, VA2, VB1, VB2) and converts one channel of each ADC
simultaneously. It operates from a single 2.7 V to 5.25 V supply, and the
serial interface voltage is set separately through VDRIVE. The reference is
an on-chip 2.5 V reference that can be overdriven, or an external reference
in the 2 V to 3 V range.

The device has no registers. The conversion is started and framed by CS, and
the configuration is made with pins that are sampled on the CS falling edge:

* **A0** selects the channel pair converted in the next conversion: VA1 and
  VB1 when low, VA2 and VB2 when high.
* **RANGE** selects the input range and the output coding of the next
  conversion: 0 V to VREF with straight binary coding when low, and
  VREF +/- VREF (0 V to 2 x VREF) with twos complement coding when high.

Each 16-bit word that the device shifts out is made of a leading zero, three
STATUS bits (RANGE, A0 and A/B) and the 12-bit conversion result, MSB first.
Keeping CS low for 32 SCLK cycles returns the result of the other ADC on the
same data line, so both results are read through one SPI MISO line. The A/B
STATUS bit identifies the ADC, so either DOUTA or DOUTB can be connected to
MISO.

AD7866 Device Configuration
---------------------------

Driver Initialization
~~~~~~~~~~~~~~~~~~~~~

In order to be able to use the device, you will have to provide the support
for the communication protocol (SPI) and for two GPIOs, A0 and RANGE.

The first API to be called is **ad7866_init**. Make sure that it returns 0,
which means that the driver was initialized correctly. It validates the
parameters, drives the pins to their initial levels, wakes the device up from
any power mode it powered up in and runs a conversion that checks the STATUS
bits, so it returns an error if the device does not respond or if the pins
are not wired as configured.

The SPI interface must use mode 2 (CPOL = 1, CPHA = 0) with a clock between
10 kHz and 20 MHz. A conversion takes 16 SCLK cycles, and reading both ADCs
through one MISO line takes 32, which limits the throughput to 625 kSPS at
20 MHz (the 1 MSPS rate needs both data lines read in parallel).

If a pin is hardwired on the board, set the corresponding GPIO init parameter
to NULL and set the ``pair`` or ``range`` field to the level it is tied to.
Changing a hardwired setting returns ``-ENOTSUP``.

Reference and supply
~~~~~~~~~~~~~~~~~~~~

``vref_mv`` is the voltage on VREF/DCAPA/DCAPB, 2500 for the internal
reference (``AD7866_VREF_INTERNAL_MV``), and ``vdd_mv`` is the AVDD supply.
They are used to validate the configuration, as the selected input range must
not exceed VDD, and to convert the results to millivolts.

Channel pair and range selection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The A0 and RANGE pins are sampled at the start of a conversion and apply to
the next one. **ad7866_set_pair** and **ad7866_set_range** change the pins
and run a conversion that is discarded so that the change is in effect when
they return. The conversion results are returned as ``int16_t`` raw codes:
unsigned for the 0 V to VREF range and sign extended for the VREF +/- VREF
range.

Conversions
~~~~~~~~~~~

* **ad7866_read_channel** reads a single channel (``enum ad7866_channel``).
* **ad7866_convert** reads the simultaneous result of ADC A and ADC B for the
  currently selected pair, and selects the pair of the following conversion.
  Passing alternating pairs reads both pairs with one conversion each.
* **ad7866_raw_to_mv** converts a raw code to millivolts for the current
  range.

Power-down modes are not supported by the driver.

AD7866 Driver Initialization Example
------------------------------------

.. code-block:: bash

	struct ad7866_desc *ad7866;
	int16_t val;

	struct no_os_spi_init_param spi_ip = {
		.device_id = SPI_DEVICE_ID,
		.max_speed_hz = 2000000,
		.chip_select = SPI_CS,
		.mode = NO_OS_SPI_MODE_2,
		.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
		.platform_ops = SPI_OPS,
		.extra = SPI_EXTRA,
	};

	struct no_os_gpio_init_param a0_ip = {
		.port = GPIO_A0_PORT,
		.number = GPIO_A0,
		.platform_ops = GPIO_OPS,
		.extra = GPIO_EXTRA,
	};

	struct no_os_gpio_init_param range_ip = {
		.port = GPIO_RANGE_PORT,
		.number = GPIO_RANGE,
		.platform_ops = GPIO_OPS,
		.extra = GPIO_EXTRA,
	};

	struct ad7866_init_param ad7866_ip = {
		.spi_init = &spi_ip,
		.gpio_a0_init = &a0_ip,
		.gpio_range_init = &range_ip,
		.vref_mv = AD7866_VREF_INTERNAL_MV,
		.vdd_mv = 3300,
		.pair = AD7866_PAIR_1,
		.range = AD7866_RANGE_0_TO_VREF,
	};

	ret = ad7866_init(&ad7866, &ad7866_ip);
	if (ret)
		goto error;

	ret = ad7866_read_channel(ad7866, AD7866_CH_VB2, &val);
	if (ret)
		goto remove;

	pr_info("VB2: %d mV\n", ad7866_raw_to_mv(ad7866, val));

AD7866 no-OS IIO support
------------------------

The AD7866 IIO driver comes on top of the AD7866 driver and offers support
for interfacing IIO clients through libiio.

AD7866 IIO Device Configuration
-------------------------------

Channel Attributes
~~~~~~~~~~~~~~~~~~

There are four voltage channels, ``voltage0`` to ``voltage3``, which are VA1,
VA2, VB1 and VB2. The measured voltage in mV is
``(raw + offset) * scale``.

* ``raw - single conversion of the channel``
* ``scale - mV per LSB, VREF / 4096 or 2 x VREF / 4096 depending on range``
* ``offset - 0, or 2048 for the twos complement range``

Global Attributes
~~~~~~~~~~~~~~~~~

* ``range - input range: 0_TO_VREF or VREF_PM_VREF``
* ``range_available - list of the supported input ranges``

Buffered capture
~~~~~~~~~~~~~~~~

Any combination of the four channels can be captured in the buffer. The
channels are converted in pairs, so enabling channels from both pairs makes
the driver alternate A0 between conversions, and results are returned in
ascending channel order. The sample rate is not paced by a hardware
trigger, it is determined by the SPI speed and the host.

AD7866 IIO Driver Initialization Example
----------------------------------------

.. code-block:: bash

	int ret;

	struct ad7866_iio_desc *ad7866_iio_desc;
	struct ad7866_iio_init_param ad7866_iio_ip = {
		.ad7866_init = &ad7866_ip,
	};

	struct iio_app_desc *app;
	struct iio_app_init_param app_init_param = {0};

	ret = ad7866_iio_init(&ad7866_iio_desc, &ad7866_iio_ip);
	if (ret)
		goto exit;

	struct iio_app_device iio_devices[] = {
		{
			.name = "ad7866",
			.dev = ad7866_iio_desc,
			.dev_descriptor = ad7866_iio_desc->iio_dev,
		},
	};

	app_init_param.devices = iio_devices;
	app_init_param.nb_devices = NO_OS_ARRAY_SIZE(iio_devices);
	app_init_param.uart_init_params = uip;

	ret = iio_app_init(&app, app_init_param);
	if (ret)
		goto remove_iio_ad7866;

	return iio_app_run(app);
