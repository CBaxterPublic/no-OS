ADMV1455 no-OS Driver
====================

The core and IIO drivers follow the Linux ADMV1455 driver on
``origin/staging/zcu102_saco_admv_tests`` in the local
``linux-innersource`` repository. The Linux checkout is not modified.

Initialization
--------------

Enable ``CONFIG_FREQUENCY_ADMV1455`` with SPI and GPIO support. Provide an
``Admv1455InitParam`` and call ``admv1455_init``. The driver applies the
Linux default IF-mode register sequence, enables CEN after configuration,
reads the product ID, and verifies the SPI link with a scratchpad test.
As in Linux, a different product-ID value alone does not reject the device.

``chip_addr`` is the two-bit address encoded in each SPI command; it is
independent of the SPI controller's chip select. Four-wire SPI is supported.
``spi3wire = true`` is rejected because the portable no-OS API does not
provide a three-wire turnaround configuration.

RESET and CEN GPIOs are optional. RESET uses physical active-low levels,
and CEN is active high. Configure external supplies and board-level GPIOs
before initialization. On FPGA platforms, load the design containing the
SPI/GPIO peripherals before accessing them.

Controls
--------

The core uses dedicated getters and setters, following the ADMV1355 driver.
``admv1455_get_rf_dsa`` and ``admv1455_set_rf_dsa`` operate on DSA1/2/3;
the IF DSA functions operate on DSA4/5. Separate functions handle bypass
gains, offsets, fine I/Q attenuation, paired RF filters, and paired LO phase.
Writes preserve unrelated fields, including RF/IF fields sharing a register.

Values are unshifted register-field values. ``admv1455_set_if_subband``
takes an index: 0=4GHz, 1=6GHz, 2=8GHz, 3=10GHz. Selecting it programs
IF_SUBBAND, BPF_COARSE, IF_RES_CAL, and IF_BPF_FINE together.
``admv1455_set_rf_band`` updates both RF_BAND and RF_SUBBAND.
``admv1455_set_lo_trap`` accepts a ten-bit value, not a string-table index.

The DSA2 value is 0 or 1 (0 or -6dB); other gain DSAs are 0 through 15.
I/Q attenuation is 0 through 15 in 0.1dB steps. Set gain-LUT mode to 0 or 1.
Temperature and detector reads return raw ADC codes, matching Linux.

Calls are synchronous. Applications sharing a device across threads must
serialize access, including multi-register updates. Use ``admv1455_remove``
to release resources and drive CEN low and RESET active.

IIO
---

Enable ``CONFIG_FREQUENCY_IIO_ADMV1455`` with ``CONFIG_IIO`` and SPI.
Call ``admv1455_iio_init`` with ``Admv1455IioDevInitParam`` and register the
resulting ``iio_dev`` descriptor and wrapper pointer with ``iio_app``.

The descriptor exposes RF input altvoltage0, IF output altvoltage1, LO input
altvoltage2, temperature0, and power0. Channel attributes, enum strings,
``*_available`` lists, and device control names follow Linux. The no-OS
framework represents RF/IF/LO as channel names rather than Linux's
``extend_name`` filename suffixes. Debug register access is also available.

LUTs
----

The core provides raw-entry reads/writes: filter tables A and B each have
32 entries of 13 bytes; the gain table has 67 entries of 4 bytes. Callers
must supply a buffer of the corresponding entry size.

IIO exposes ``filter_table_config_A``, ``filter_table_config_B``, and
``gain_table_config`` as ordinary attributes, since no-OS does not have
Linux binary-attribute callbacks. Each write can contain one or more lines;
blank lines and lines beginning with ``#`` are ignored. Formats match Linux::

   LUTA_0 RF (0,0,15,0,0,0,0) IF (2,0,0,0,0) LO (1,0,0,0,0,15,15,0,0)
   GAIN_0 RF (0dB,-6dB,-1dB) IF (-2dB,-3dB) 0

Filter fields are decimal indices/numbers in this order::

   RF (band,lpf,hpf,dsa1_off,dsa2_off,dsa3_off,gpo_f)
   IF (subband,dsa4_off,dsa5_off,dsai,dsaq)
   LO (sideband,x1,x3,x4,trap,i_phase,q_phase,band,doubler)

Use ``LUTB`` for table B. Enum indices follow the corresponding channel's
``*_available`` order. Writes validate each entry before programming it;
SPI errors can leave an entry partially written. Reading returns all
entries without Linux's explanatory comment header, and requires a
sufficiently large callback buffer (otherwise ``-ENOSPC`` is returned).
8GHz and 10GHz LUT entries share coarse bits; readback uses the current
global IF fine setting to distinguish them, as in Linux. Configure that
global setting separately with ``admv1455_set_if_subband``.

This driver does not configure board routing or create an evaluation
project. Hardware startup and IIO connectivity must be verified on a board.