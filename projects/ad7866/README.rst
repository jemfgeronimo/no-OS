ad7866 no-OS Example Project
============================

.. no-os-doxygen::

.. contents:: Table of Contents
    :depth: 3

Supported Evaluation Boards
---------------------------

* `EVAL-AD7866CB <https://www.analog.com/EVAL-AD7866CB>`_

Overview
--------

The :adi:`AD7866` is a dual 12-bit, 1 MSPS successive approximation ADC with
two simultaneously sampled ADCs, each preceded by a 2-channel input
multiplexer, for a total of four single-ended inputs. It operates from a
single 2.7 V to 5.25 V supply, has an on-chip 2.5 V reference that can be
overdriven, and a serial interface compatible with SPI, QSPI and DSP
interfaces.

Hardware Specifications
-----------------------

Power Supply Requirements
~~~~~~~~~~~~~~~~~~~~~~~~~

AVDD and DVDD are 2.7 V to 5.25 V and must be within 0.3 V of each other.
VDRIVE (2.7 V to 5.25 V) sets the voltage of the serial interface and must
match the logic level of the host. The 0 V to 2 x VREF input range requires a
supply that is at least 2 x VREF, which is 5 V with the 2.5 V reference.

The ``vdd_mv`` and ``vref_mv`` fields of ``ad7866_ip`` in
`src/common/common_data.c <https://github.com/analogdevicesinc/no-OS/tree/main/projects/ad7866/src/common/common_data.c>`__
must match the supply and the reference of the board.

No-OS Supported Examples
------------------------

This project is organized around the no-OS variant based build flow.
Selecting a variant at build time (``--variant <name>``) chooses which
application is compiled. The platform ``main()`` is a thin dispatcher that
calls ``example_main()``, provided by the selected example. Shared
initialization data is defined in
`src/common <https://github.com/analogdevicesinc/no-OS/tree/main/projects/ad7866/src/common>`__,
and platform-specific macros and extra init parameters are in
`src/platform <https://github.com/analogdevicesinc/no-OS/tree/main/projects/ad7866/src/platform>`__.

Basic Example
~~~~~~~~~~~~~

The basic example reads the four channels once per second and prints the raw
code and the voltage in mV on the UART console, 230400 baud.

IIO Example
~~~~~~~~~~~

The IIO example runs an IIOD server on the board, which can be accessed from a
host computer with IIO clients through libiio. The four channels are exposed
as ``voltage0`` to ``voltage3`` and can be read with tools like ``iio_info``,
``iio_attr`` and ``iio_readdev``. Details of the IIO implementation are
documented in the
`AD7866 driver <https://github.com/analogdevicesinc/no-OS/tree/main/drivers/adc/ad7866/README.rst>`__.

If you are not familiar with ADI IIO Application, please take a look at:
:dokuwiki:`IIO No-OS </resources/tools-software/no-os-software/iio>`

If you are not familiar with ADI IIO-Oscilloscope Client, please take a look
at:
:dokuwiki:`IIO Oscilloscope </resources/tools-software/linux-software/iio_oscilloscope>`

No-OS Supported Platforms
-------------------------

STM32
~~~~~

Used Hardware
^^^^^^^^^^^^^

* `SDP-K1 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/sdp-k1.html>`_
* `EVAL-AD7866CB <https://www.analog.com/EVAL-AD7866CB>`_

Connections
^^^^^^^^^^^

The AD7866 is connected to the SDP-K1 Arduino header with jumper wires. The
SPI mode is 2 and the console is UART5 at 230400 baud.

==============  ==============  ============================================
SDP-K1 pin      AD7866 pin      Description
==============  ==============  ============================================
D10             CS              Chip select
D12             DOUTA           SPI MISO, both results are read on this line
D13             SCLK            SPI clock
D2              A0              Channel pair select
D3              RANGE           Input range and coding select
GND             AGND, DGND      Ground
==============  ==============  ============================================

Build Command
^^^^^^^^^^^^^

The STM32 platform uses the CMake/Ninja build system via the
``no_os_build.py`` helper script. Available variants: ``basic``,
``iio_example``. Available boards: ``sdp-ck1z``.

For toolchain setup and prerequisites, see the
:doc:`STM32 CMake build guide </build_guides/build_stm32_cmake>`.

.. code-block:: bash

	# set the path to STM32CubeMX and STM32CubeIDE (only if they are not
	# in a default install location)
	export STM32CUBEMX=</path/to/stm32cubemx>
	export STM32CUBEIDE=</path/to/stm32cubeide>
	# Windows (PowerShell):
	#   $env:STM32CUBEMX = "C:\ST\STM32CubeMX"
	#   $env:STM32CUBEIDE = "C:\ST\STM32CubeIDE"

	cd no-OS

	# build the example on the SDP-K1 board
	python tools/scripts/no_os_build.py build \
		--project ad7866 --variant basic --board sdp-ck1z

	# build and flash (requires a connected debug probe)
	python tools/scripts/no_os_build.py build \
		--project ad7866 --variant basic --board sdp-ck1z \
		--probe openocd --flash
