EVAL-ADMV1455 no-OS Example Project
=====================================

.. no-os-doxygen::

.. contents:: Table of Contents
    :depth: 3

Supported Evaluation Boards
----------------------------

* `EVAL-ADMV1455 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-admv1455.html>`_

Overview
--------

The ADMV1455 is a microwave downconverter covering an RF input frequency
range of 17.7 GHz to 55 GHz in two bands, with an LO input range of 8 GHz to
28 GHz and a tunable IF band-pass filter (4 GHz to 10 GHz). The EVAL-ADMV1455
evaluation board is programmed over SPI.

Applications
------------

* Point to point microwave radios
* Radar and electronic warfare
* Instrumentation and automatic test equipment (ATE)

Hardware Specifications
-------------------------

Power Supply Requirements
~~~~~~~~~~~~~~~~~~~~~~~~~~

The EVAL-ADMV1455 requires an external power supply; refer to the
`EVAL-ADMV1455 User Guide <https://www.analog.com/media/en/technical-documentation/user-guides/eval-admv1455.pdf>`_
for supply voltage and sequencing requirements.

No-OS Supported Examples
--------------------------

Shared initialization data is defined in
`src/common <https://github.com/analogdevicesinc/no-OS/tree/main/projects/eval-admv1455/src/common>`__,
and platform-specific macros and extra init parameters are in
`src/platform <https://github.com/analogdevicesinc/no-OS/tree/main/projects/eval-admv1455/src/platform>`__.

Basic Example
~~~~~~~~~~~~~~

The basic example drives DUT_LOAD and the LUT address pins low, initializes
the ADMV1455 (reset/CEN sequencing, SPI soft reset, default register
sequence, scratch-pad link check), dumps every named register, and exercises
the driver APIs (RF band, sideband, IF subband, DSAs, LO phase/band/trap,
raw temperature and detector readback) over UART.

This example is built by selecting the ``basic`` variant (see the Build
Command section below).

IIO Example
~~~~~~~~~~~~

The IIO example initializes the ADMV1455 and starts an IIOD server on the
console UART. Disconnect the serial terminal after the startup messages and
connect a libiio client, for example::

   iio_info -u serial:/dev/ttyUSB0,115200,8n1

This example is built by selecting the ``iio`` variant.

Xilinx (ZCU102)
~~~~~~~~~~~~~~~~

Used Hardware
^^^^^^^^^^^^^^

* `EVAL-ADMV1455 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-admv1455.html>`_
* `ZCU102 <https://www.xilinx.com/ZCU102>`_ (Zynq UltraScale+ MPSoC)

Connections
^^^^^^^^^^^^

Connect the EVAL-ADMV1455 to the ZCU102 FMC HPC0 connector. The
``admv_fmc_bob`` HDL design routes the ADMV1455 to ``axi_spi_fmc`` chip
select 1 and to ``axi_gpio_fmc`` bits 14-27 (RSTB, CEN, LOAD, ADD_G[5:0],
ADD_F[4:0]). The UART console appears on the ZCU102 USB-UART adapter at
**115200 baud, 8N1**.

.. note::
   The verified hardware description is generated at
   ``/opt/hdl/projects/admv_fmc_bob/zcu102/admv_fmc_bob_zcu102.sdk/system_top.xsa``.
   Confirm that the SPI chip select and AXI GPIO bit numbers in
   ``src/platform/xilinx/parameters.h`` match the generated design when
   using a different XSA.

Build Command
^^^^^^^^^^^^^^

The Xilinx platform uses the CMake/Ninja build system via the
``no_os_build.py`` helper script. Available variants: ``basic``, ``iio``.
Available boards: ``zcu102``.

For toolchain setup and prerequisites, see the
:doc:`Xilinx CMake build guide </build_guides/build_xilinx_cmake>`.

.. code-block:: bash

   # source the Vitis environment (adjust path to your Vitis install)
   source ~/.xilinx/2025.1/Vitis/settings64.sh

   cd no-OS

   # build the basic example on the ZCU102 (requires a .xsa hardware file)
   python tools/scripts/no_os_build.py build \
      --project eval-admv1455 --variant basic --board zcu102 \
      --hardware /opt/hdl/projects/admv_fmc_bob/zcu102/admv_fmc_bob_zcu102.sdk/system_top.xsa

   # build and flash (requires a connected debug probe)
   python tools/scripts/no_os_build.py build \
      --project eval-admv1455 --variant basic --board zcu102 \
      --hardware /opt/hdl/projects/admv_fmc_bob/zcu102/admv_fmc_bob_zcu102.sdk/system_top.xsa \
      --probe openocd --flash
