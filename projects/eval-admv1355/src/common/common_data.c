/***************************************************************************//**
 *   @file   common_data.c
 *   @brief  Defines common data to be used by eval-admv1355 examples.
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
#include "common_data.h"
#include "no_os_error.h"
#include "no_os_print_log.h"

typedef struct no_os_spi_init_param NoOsSpiInitParam;
NoOsSpiInitParam admv1355_spi_ip = {
	.max_speed_hz = SPI_BAUDRATE,
	.chip_select = SPI_CS,
	.mode = NO_OS_SPI_MODE_0,
	.platform_ops = SPI_OPS,
	.extra = SPI_EXTRA,
};

typedef struct no_os_gpio_init_param NoOsGpioInitParam;
NoOsGpioInitParam admv1355_gpio_reset_ip = {
	.port = 0,
	.number = GPIO_ADMV1355_RESET_N,
	.platform_ops = GPIO_OPS,
	.extra = GPIO_EXTRA,
};

NoOsGpioInitParam admv1355_gpio_cen_ip = {
	.port = 0,
	.number = GPIO_ADMV1355_CEN,
	.platform_ops = GPIO_OPS,
	.extra = GPIO_EXTRA,
};

NoOsGpioInitParam admv1355_gpio_load_ip = {
	.port = 0,
	.number = GPIO_ADMV1355_LOAD,
	.platform_ops = GPIO_OPS,
	.extra = GPIO_EXTRA,
};

/* DUT_ADD_G[5:0] / DUT_ADD_F[4:0]: hardware LUT address inputs, unused
 * while FILTER_LUT_EN/GAIN_LUT_EN are disabled (see admv1355_init()).
 */
NoOsGpioInitParam admv1355_gpio_add_g_ip[6] = {
	{ .port = 0, .number = GPIO_ADMV1355_ADD_G0, .platform_ops = GPIO_OPS, .extra = GPIO_EXTRA },
	{ .port = 0, .number = GPIO_ADMV1355_ADD_G1, .platform_ops = GPIO_OPS, .extra = GPIO_EXTRA },
	{ .port = 0, .number = GPIO_ADMV1355_ADD_G2, .platform_ops = GPIO_OPS, .extra = GPIO_EXTRA },
	{ .port = 0, .number = GPIO_ADMV1355_ADD_G3, .platform_ops = GPIO_OPS, .extra = GPIO_EXTRA },
	{ .port = 0, .number = GPIO_ADMV1355_ADD_G4, .platform_ops = GPIO_OPS, .extra = GPIO_EXTRA },
	{ .port = 0, .number = GPIO_ADMV1355_ADD_G5, .platform_ops = GPIO_OPS, .extra = GPIO_EXTRA },
};

NoOsGpioInitParam admv1355_gpio_add_f_ip[5] = {
	{ .port = 0, .number = GPIO_ADMV1355_ADD_F0, .platform_ops = GPIO_OPS, .extra = GPIO_EXTRA },
	{ .port = 0, .number = GPIO_ADMV1355_ADD_F1, .platform_ops = GPIO_OPS, .extra = GPIO_EXTRA },
	{ .port = 0, .number = GPIO_ADMV1355_ADD_F2, .platform_ops = GPIO_OPS, .extra = GPIO_EXTRA },
	{ .port = 0, .number = GPIO_ADMV1355_ADD_F3, .platform_ops = GPIO_OPS, .extra = GPIO_EXTRA },
	{ .port = 0, .number = GPIO_ADMV1355_ADD_F4, .platform_ops = GPIO_OPS, .extra = GPIO_EXTRA },
};

typedef struct admv1355_init_param NoOsAdmv1355InitParam;
NoOsAdmv1355InitParam admv1355_ip = {
	.spi_init = &admv1355_spi_ip,
	.gpio_reset = &admv1355_gpio_reset_ip,
	.gpio_cen = &admv1355_gpio_cen_ip,
	.lo_in = 20000000000ULL,
	.if_mode = ADMV1355_IF_MODE_BASEBAND,
	.rf_band = ADMV1355_RF_BAND_LOW,
	.mixer_sideband = ADMV1355_SIDEBAND_USB,
};

#ifdef IIO_SUPPORT
struct no_os_uart_init_param admv1355_uart_ip = {
	.device_id = UART_DEVICE_ID,
	.baud_rate = UART_BAUDRATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.platform_ops = UART_OPS,
	.extra = UART_EXTRA,
};
#endif

int admv1355_board_gpio_init(void)
{
	struct no_os_gpio_desc *gpio_load;
	struct no_os_gpio_desc *gpio_add_g[6];
	struct no_os_gpio_desc *gpio_add_f[5];
	int ret, i;

	ret = no_os_gpio_get(&gpio_load, &admv1355_gpio_load_ip);
	if (ret) {
		pr_err("gpio_get(load) failed with code: %d\n", ret);
		return ret;
	}

	ret = no_os_gpio_direction_output(gpio_load, NO_OS_GPIO_LOW);
	if (ret) {
		pr_err("gpio_direction_output(load) failed with code: %d\n", ret);
		return ret;
	}

	for (i = 0; i < 6; i++) {
		ret = no_os_gpio_get(&gpio_add_g[i], &admv1355_gpio_add_g_ip[i]);
		if (ret) {
			pr_err("gpio_get(add_g%d) failed with code: %d\n", i, ret);
			return ret;
		}

		ret = no_os_gpio_direction_output(gpio_add_g[i], NO_OS_GPIO_LOW);
		if (ret) {
			pr_err("gpio_direction_output(add_g%d) failed with code: %d\n",
			       i, ret);
			return ret;
		}
	}

	for (i = 0; i < 5; i++) {
		ret = no_os_gpio_get(&gpio_add_f[i], &admv1355_gpio_add_f_ip[i]);
		if (ret) {
			pr_err("gpio_get(add_f%d) failed with code: %d\n", i, ret);
			return ret;
		}

		ret = no_os_gpio_direction_output(gpio_add_f[i], NO_OS_GPIO_LOW);
		if (ret) {
			pr_err("gpio_direction_output(add_f%d) failed with code: %d\n",
			       i, ret);
			return ret;
		}
	}

	return 0;
}
