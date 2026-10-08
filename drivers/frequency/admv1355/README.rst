ADMV1355 no-OS Driver
=====================

.. no-os-doxygen::

Supported Devices
------------------

* `ADMV1355 <https://www.analog.com/ADMV1355>`_

Overview
--------

The ADMV1355 is a wideband, microwave upconverter covering an RF output
frequency range of 17.7 GHz to 55 GHz. The device integrates an RF chain
with tunable sideband rejection filters and gain control digital step
attenuators (DSAs), and an LO chain with an internal 2x multiplier, an
LO input frequency range of 8.85 GHz to 27.5 GHz, a tunable LO harmonic
rejection filter, and I/Q phase correction. It supports three frequency
translation modes: baseband I/Q differential input (DC to 12 GHz), IF
hybrid input (8 GHz to 12 GHz), and complex IF input (1 GHz to 12 GHz).
The device is programmable through a simple 3-wire or 4-wire SPI
interface.

Applications
------------

* Point to point microwave radios
* Radar and electronic warfare
* Instrumentation and automatic test equipment (ATE)

ADMV1355 Device Configuration
------------------------------

Driver Initialization
~~~~~~~~~~~~~~~~~~~~~~

In order to be able to use the device, you have to provide the support for
the communication protocol (SPI), and, optionally, the RESET and
chip-enable (CEN) GPIOs.

The first API to be called is ``admv1355_init``. Make sure that it returns
0, which means that the driver was initialized correctly. During
initialization, the driver deasserts the optional RESET GPIO, performs a
SPI soft reset, enables the SDO pin for 4-wire operation, reads back the
product ID, verifies the SPI link through the scratch pad register, loads
and verifies the on-chip NVM calibration data, applies the default gain and
filter bypass configuration, selects the LO chain filters for the
requested ``lo_in`` frequency, and configures the requested frequency
translation mode, RF band, and mixer sideband. Finally, it asserts the
optional chip-enable GPIO. The SPI initialization parameters are passed
through the ``spi_init`` field of the initialization parameter structure.

LO Filter Selection
~~~~~~~~~~~~~~~~~~~~

The LO x1/x3/x4 multiplier filters, LO trap filter, and LO doubler band are
automatically selected during initialization (and can be reselected at
runtime with ``admv1355_set_lo_filters``) based on the ``lo_in`` frequency
parameter, matching the appropriate range from the datasheet filter
selection table.

Frequency Translation Mode
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The frequency translation mode is configured via the ``if_mode`` field in
the initialization parameter structure (or at runtime with
``admv1355_set_if_mode``): baseband I/Q (``ADMV1355_IF_MODE_BASEBAND``),
low complex IF (``ADMV1355_IF_MODE_COMPLEX_IF_LOW``), high complex IF
(``ADMV1355_IF_MODE_COMPLEX_IF_HIGH``), or IF hybrid
(``ADMV1355_IF_MODE_HYBRID``).

RF Band and Mixer Sideband
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The RF band (``rf_band``: ``ADMV1355_RF_BAND_LOW``/``ADMV1355_RF_BAND_HIGH``)
and the mixer sideband (``mixer_sideband``:
``ADMV1355_SIDEBAND_LSB``/``ADMV1355_SIDEBAND_USB``) can be set during
initialization or changed at runtime with ``admv1355_set_rf_band`` and
``admv1355_set_mixer_sideband``.

Gain Control
~~~~~~~~~~~~

The RF DSA1/DSA2 direct attenuation (0 dB to -15 dB, 1 dB/step) and the IF
fine I/Q attenuation (0 dB to -1.5 dB, 0.1 dB/step) can be read and set
using ``admv1355_get_rf_dsa``/``admv1355_set_rf_dsa`` and
``admv1355_get_if_dsa``/``admv1355_set_if_dsa``.

LO Phase Adjustment
~~~~~~~~~~~~~~~~~~~

The LO I/Q phase adjustment can be read and set using
``admv1355_get_lo_phase``/``admv1355_set_lo_phase``.

Temperature Sensor
~~~~~~~~~~~~~~~~~~~

The on-chip temperature sensor can be read with
``admv1355_read_temperature``, which returns the die temperature in
milli-degrees Celsius.

Direct Register Access
~~~~~~~~~~~~~~~~~~~~~~~

The ``admv1355_spi_read``, ``admv1355_spi_write``, and
``admv1355_spi_update_bits`` APIs allow direct access to any of the
device's registers.

ADMV1355 Driver Initialization Example
---------------------------------------

.. code-block:: C

	struct admv1355_dev *admv1355_dev;

	struct no_os_spi_init_param admv1355_spi_ip = {
		.device_id = 0,
		.max_speed_hz = 1000000,
		.mode = NO_OS_SPI_MODE_0,
		.chip_select = 0,
		.platform_ops = &xil_spi_ops,
		.extra = &admv1355_spi_extra_ip,
	};

	struct admv1355_init_param admv1355_ip = {
		.spi_init = &admv1355_spi_ip,
		.gpio_reset = &admv1355_gpio_reset_ip,
		.gpio_cen = &admv1355_gpio_cen_ip,
		.lo_in = 20000000000ULL,
		.if_mode = ADMV1355_IF_MODE_BASEBAND,
		.rf_band = ADMV1355_RF_BAND_LOW,
		.mixer_sideband = ADMV1355_SIDEBAND_USB,
	};

	ret = admv1355_init(&admv1355_dev, &admv1355_ip);
	if (ret)
		goto error;
