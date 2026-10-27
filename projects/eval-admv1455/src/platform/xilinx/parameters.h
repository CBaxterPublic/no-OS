/***************************************************************************//**
 *   @file   parameters.h
 *   @brief  Definitions specific to the xilinx platform used by the
 *           eval-admv1455 project.
 *   @author Charles Baxter (charles.baxter@analog.com)
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
 * hdl/projects/admv_fmc_bob/zcu102 shares one AXI Quad SPI (1 MHz) and one
 * AXI GPIO between ADMV1355, ADMV1455 and ADMV8909:
 *
 *   - axi_spi_fmc chip select 1 is ADMV1455 (0 -> ADMV1355, 2 -> ADMV8909)
 *   - axi_gpio_fmc channel 1 bits [27:14] are admv1455_gpio_00..13 in
 *     hdl/projects/admv_fmc_bob/zcu102/system_top.v
 *
 * Bit assignment matches zynqmp-zcu102-rev10-saco-admv.dts (DUT_RSTB is
 * active-low; DUT_ADD_G/F are LUT address inputs, unused while hardware
 * LUT addressing is disabled):
 *   14 = DUT_RSTB, 15 = DUT_CEN, 16 = DUT_LOAD, 17-22 = DUT_ADD_G[0:5],
 *   23-27 = DUT_ADD_F[0:4]
 */
#define UART_DEVICE_ID			XPAR_XUARTPS_0_DEVICE_ID
#define UART_IRQ_ID			XPAR_XUARTPS_0_INTR
#define UART_BAUDRATE			115200

#define SPI_BASEADDR			XPAR_AXI_SPI_FMC_BASEADDR
#define GPIO_BASEADDR			XPAR_AXI_GPIO_FMC_BASEADDR

#define GPIO_ADMV1455_RESET_N		14
#define GPIO_ADMV1455_CEN		15
#define GPIO_ADMV1455_LOAD		16
#define GPIO_ADMV1455_ADD_G0		17
#define GPIO_ADMV1455_ADD_G1		18
#define GPIO_ADMV1455_ADD_G2		19
#define GPIO_ADMV1455_ADD_G3		20
#define GPIO_ADMV1455_ADD_G4		21
#define GPIO_ADMV1455_ADD_G5		22
#define GPIO_ADMV1455_ADD_F0		23
#define GPIO_ADMV1455_ADD_F1		24
#define GPIO_ADMV1455_ADD_F2		25
#define GPIO_ADMV1455_ADD_F3		26
#define GPIO_ADMV1455_ADD_F4		27

#define SPI_CS				1
#define ADMV1455_CHIP_ADDR		0
#define SPI_BAUDRATE			1000000

#define SPI_OPS				&xil_spi_ops
#define SPI_EXTRA			&admv1455_spi_extra_ip
#define GPIO_OPS			&xil_gpio_ops
#define GPIO_EXTRA			&admv1455_gpio_extra_ip
#ifdef IIO_SUPPORT
#define UART_OPS			&xil_uart_ops
#define UART_EXTRA			&admv1455_uart_extra_ip
#endif

extern struct xil_spi_init_param admv1455_spi_extra_ip;
extern struct xil_gpio_init_param admv1455_gpio_extra_ip;
#ifdef IIO_SUPPORT
extern struct xil_uart_init_param admv1455_uart_extra_ip;
#endif

#endif /* __PARAMETERS_H__ */
