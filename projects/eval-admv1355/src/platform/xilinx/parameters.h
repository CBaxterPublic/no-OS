/***************************************************************************//**
 *   @file   parameters.h
 *   @brief  Definitions specific to the xilinx platform used by the
 *           eval-admv1355 project.
 *   @author Analog Devices, Inc.
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. “AS IS” AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL ANALOG DEVICES, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#include <xparameters.h>
#include "xilinx_spi.h"
#include "xilinx_gpio.h"
#include "xil_cache.h"
#ifdef IIO_SUPPORT
#include "xilinx_uart.h"
#endif

/*
 * hdl/projects/admv_fmc_bob/zcu102 wires the ADMV1355 (shared with ADMV1455
 * and ADMV8909 on the same FMC board) to two AXI (PL) cores rather than the
 * Zynq PS SPI0/EMIO GPIO used by the older "zed" reference design:
 *
 *   - axi_spi_fmc (AXI Quad SPI, 1 MHz): chip select 0 is ADMV1355
 *     (1 -> ADMV1455, 2 -> ADMV8909), see
 *     hdl/projects/admv_fmc_bob/common/admv_fmc_bob_bd.tcl
 *   - axi_gpio_fmc (AXI GPIO, dual channel): ADMV1355's 14 control pins are
 *     on channel 1, bits [13:0] (admv1355_gpio_00..13 in
 *     hdl/projects/admv_fmc_bob/zcu102/system_top.v)
 *
 * Bit-to-signal assignment within [13:0], per the EVAL-ADMV1355 FMC
 * breakout pin map (DUT_RSTB is active-low; DUT_ADD_G[5:0]/DUT_ADD_F[4:0]
 * are the LUT address input pins, unused while hardware LUT addressing is
 * disabled, and DUT_LOAD's function is not yet characterized):
 *   0 = DUT_RSTB, 1 = DUT_CEN, 2 = DUT_LOAD, 3-8 = DUT_ADD_G[0:5],
 *   9-13 = DUT_ADD_F[0:4]
 */
#define UART_DEVICE_ID			XPAR_XUARTPS_0_DEVICE_ID
#define UART_IRQ_ID			XPAR_XUARTPS_0_INTR
#define UART_BAUDRATE			115200

#define SPI_BASEADDR			XPAR_AXI_SPI_FMC_BASEADDR
#define GPIO_BASEADDR			XPAR_AXI_GPIO_FMC_BASEADDR

/* AXI GPIO channel 1, bits [13:0] -- see admv_fmc_bob_bd.tcl / system_top.v */
#define GPIO_ADMV1355_RESET_N		0
#define GPIO_ADMV1355_CEN		1
#define GPIO_ADMV1355_LOAD		2
#define GPIO_ADMV1355_ADD_G0		3
#define GPIO_ADMV1355_ADD_G1		4
#define GPIO_ADMV1355_ADD_G2		5
#define GPIO_ADMV1355_ADD_G3		6
#define GPIO_ADMV1355_ADD_G4		7
#define GPIO_ADMV1355_ADD_G5		8
#define GPIO_ADMV1355_ADD_F0		9
#define GPIO_ADMV1355_ADD_F1		10
#define GPIO_ADMV1355_ADD_F2		11
#define GPIO_ADMV1355_ADD_F3		12
#define GPIO_ADMV1355_ADD_F4		13

#define SPI_CS				0
#define SPI_BAUDRATE			1000000

#define SPI_OPS				&xil_spi_ops
#define SPI_EXTRA			&admv1355_spi_extra_ip
#define GPIO_OPS			&xil_gpio_ops
#define GPIO_EXTRA			&admv1355_gpio_extra_ip
#ifdef IIO_SUPPORT
#define UART_OPS			&xil_uart_ops
#define UART_EXTRA			&admv1355_uart_extra_ip
#endif

extern struct xil_spi_init_param admv1355_spi_extra_ip;
extern struct xil_gpio_init_param admv1355_gpio_extra_ip;
#ifdef IIO_SUPPORT
extern struct xil_uart_init_param admv1355_uart_extra_ip;
#endif

#endif /* __PARAMETERS_H__ */
