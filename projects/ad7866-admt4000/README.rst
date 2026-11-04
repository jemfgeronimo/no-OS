ad7866-admt4000 no-OS Example Project
=====================================

.. no-os-doxygen::

.. contents:: Table of Contents
    :depth: 3

Supported Evaluation Boards
---------------------------

* `EVAL-ADMT4000ARD1Z <https://www.analog.com/EVAL-ADMT4000ARD1Z>`_
* `EVAL-AD7866CB <https://www.analog.com/EVAL-AD7866CB>`_

Overview
--------

This project runs the :adi:`AD7866`, the :adi:`ADMT4000` and the
:adi:`TMC5240` together in one IIO application. The EVAL-ADMT4000ARD1Z
carries the ADMT4000 turn counter sensor and the TMC5240 stepper motor driver,
and the AD7866 dual 12-bit simultaneous sampling ADC is added next to them on
the same SPI bus of the SDP-K1.

The ADMT4000 and TMC5240 support, the board EEPROM handling and the
EVAL-ADMT4000ARD1Z peripherals are shared with the
`admt4000 project <https://github.com/analogdevicesinc/no-OS/tree/main/projects/admt4000>`__,
whose sources are compiled by this project. Only the AD7866 data and the
combined example are specific to it.

Hardware Specifications
-----------------------

Power Supply Requirements
~~~~~~~~~~~~~~~~~~~~~~~~~

The EVAL-ADMT4000ARD1Z is powered from the SDP-K1, and its motor assembly
needs at least 9 V DC on the DC barrel jack, see the
`admt4000 project <https://github.com/analogdevicesinc/no-OS/tree/main/projects/admt4000/README.rst>`__.
The AD7866 supply and reference must match the ``AD7866_VDD_MV`` macro and the
``vref_mv`` field of ``ad7866_ip`` in
`src/common <https://github.com/analogdevicesinc/no-OS/tree/main/projects/ad7866-admt4000/src/common>`__.

No-OS Supported Examples
------------------------

IIO Trigger Example
~~~~~~~~~~~~~~~~~~~

The example runs an IIOD server on the board, which can be accessed from a
host computer with IIO clients through libiio. Four IIO devices are exposed:

* ``admt_evb`` - the EVAL-ADMT4000ARD1Z supply, shutdown and coil reset
  controls.
* ``admt4000`` - the turn counter sensor, with buffered capture paced by its
  GPIO hardware trigger.
* ``tmc5240`` - the stepper motor driver.
* ``ad7866`` - the four voltage channels. The AD7866 has no data-ready pin, so
  its buffered capture is not hardware triggered, it is paced by the SPI
  speed and the host.

Details of each IIO implementation are documented in the driver pages:
`AD7866 <https://github.com/analogdevicesinc/no-OS/tree/main/drivers/adc/ad7866/README.rst>`__,
`ADMT4000 <https://github.com/analogdevicesinc/no-OS/tree/main/drivers/position/admt4000/README.rst>`__ and
`TMC5240 <https://github.com/analogdevicesinc/no-OS/tree/main/drivers/motor/tmc5240/README.rst>`__.

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
* `EVAL-ADMT4000ARD1Z <https://www.analog.com/EVAL-ADMT4000ARD1Z>`_
* `EVAL-AD7866CB <https://www.analog.com/EVAL-AD7866CB>`_

Connections
^^^^^^^^^^^

The EVAL-ADMT4000ARD1Z plugs into the Arduino header of the SDP-K1 and uses
almost all of its pins, see the
`admt4000 project <https://github.com/analogdevicesinc/no-OS/tree/main/projects/admt4000/README.rst>`__
for the pin description. The AD7866 shares SPI1 (SCLK, MISO, MOSI) with the
ADMT4000 and the TMC5240, and needs its own chip select and the A0 and RANGE
control pins. DOUTA of the AD7866 is connected to the SPI MISO line.

The chip select, A0 and RANGE pins in ``src/common/ad7866_data.h`` are
placeholders and have not been validated on hardware. The build prints a
warning until ``AD7866_PINS_CONFIRMED`` is defined.

Build Command
^^^^^^^^^^^^^

The STM32 platform uses the CMake/Ninja build system via the
``no_os_build.py`` helper script. Available variants: ``iio_trigger``.
Available boards: ``sdp-ck1z``.

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
		--project ad7866-admt4000 --variant iio_trigger --board sdp-ck1z

	# build and flash (requires a connected debug probe)
	python tools/scripts/no_os_build.py build \
		--project ad7866-admt4000 --variant iio_trigger --board sdp-ck1z \
		--probe openocd --flash
