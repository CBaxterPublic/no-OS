EVAL-ADMV1355 no-OS Example Project
=====================================

.. no-os-doxygen::

.. contents:: Table of Contents
    :depth: 3

Supported Evaluation Boards
----------------------------

* `EVAL-ADMV1355 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-admv1355.html>`_

Overview
--------

The ADMV1355 is a wideband, microwave upconverter covering an RF output
frequency range of 17.7 GHz to 55 GHz, with an LO input frequency range of
8.85 GHz to 27.5 GHz. The EVAL-ADMV1355 evaluation board provides a
simplified interface for evaluating the device, and is programmed over a
simple SPI interface.

Applications
------------

* Point to point microwave radios
* Radar and electronic warfare
* Instrumentation and automatic test equipment (ATE)

Hardware Specifications
-------------------------

Power Supply Requirements
~~~~~~~~~~~~~~~~~~~~~~~~~~

The EVAL-ADMV1355 requires an external power supply; refer to the
`EVAL-ADMV1355 User Guide <https://www.analog.com/media/en/technical-documentation/user-guides/eval-admv1355.pdf>`_
for supply voltage and sequencing requirements.

No-OS Supported Examples
--------------------------

Shared initialization data is defined in
`src/common <https://github.com/analogdevicesinc/no-OS/tree/main/projects/eval-admv1355/src/common>`__,
and platform-specific macros and extra init parameters are in
`src/platform <https://github.com/analogdevicesinc/no-OS/tree/main/projects/eval-admv1355/src/platform>`__.

Basic Example
~~~~~~~~~~~~~~

The basic example brings up the EVAL-ADMV1355 power and level-shifter
enables, initializes the ADMV1355 (SPI link verification, NVM calibration
load, LO filter selection, default gain/filter configuration), and prints
a few status values (RF band, RF DSA gain, die temperature) over UART.

This example is built by selecting the ``basic`` variant (see the Build
Command section below).

Xilinx (ZCU102)
~~~~~~~~~~~~~~~~

Used Hardware
^^^^^^^^^^^^^^

* `EVAL-ADMV1355 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-admv1355.html>`_
* `ZCU102 <https://www.xilinx.com/ZCU102>`_ (Zynq UltraScale+ MPSoC)

Connections
^^^^^^^^^^^^

Connect the EVAL-ADMV1355 to the ZCU102 FMC HPC0 connector. The ADMV1355
SPI and control signals (RESET, CEN, PWR_EN, DAC_RST_n, LOAD,
CHIP_ADD[1:0], LVL_EN_n) are routed from the ZynqMP PS SPI0 and PS GPIO
(EMIO) by the ``admv_fmc_bob`` HDL design. The UART console appears on the
ZCU102 USB-UART adapter at **115200 baud, 8N1**.

.. note::
   The verified hardware description is generated at
   ``/opt/hdl/projects/admv_fmc_bob/zcu102/admv_fmc_bob_zcu102.sdk/system_top.xsa``.
   Confirm that the GPIO/EMIO bit numbers in
   ``src/platform/xilinx/parameters.h`` match the generated design when
   using a different XSA.

Build Command
^^^^^^^^^^^^^^

The Xilinx platform uses the CMake/Ninja build system via the
``no_os_build.py`` helper script. Available variants: ``basic``. Available
boards: ``zcu102``.

For toolchain setup and prerequisites, see the
:doc:`Xilinx CMake build guide </build_guides/build_xilinx_cmake>`.

.. code-block:: bash

   # source the Vitis environment (adjust path to your Vitis install)
   source ~/.xilinx/2025.1/Vitis/settings64.sh

   cd no-OS

   # build the basic example on the ZCU102 (requires a .xsa hardware file)
   python tools/scripts/no_os_build.py build \
      --project eval-admv1355 --variant basic --board zcu102 \
      --hardware /opt/hdl/projects/admv_fmc_bob/zcu102/admv_fmc_bob_zcu102.sdk/system_top.xsa

   # build and flash (requires a connected debug probe)
   python tools/scripts/no_os_build.py build \
      --project eval-admv1355 --variant basic --board zcu102 \
      --hardware /opt/hdl/projects/admv_fmc_bob/zcu102/admv_fmc_bob_zcu102.sdk/system_top.xsa \
      --probe openocd --flash
