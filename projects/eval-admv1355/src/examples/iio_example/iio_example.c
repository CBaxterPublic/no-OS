/***************************************************************************//**
 *   @file   iio_example.c
 *   @brief  Implementation of IIO example for eval-admv1355 project.
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
#include "iio_admv1355.h"
#include "iio_app.h"
#include "iio_example.h"
#include "no_os_print_log.h"

/**
 * @brief IIO example main execution.
 *
 * @return ret - Result of the example execution. If working correctly, will
 *               execute continuously function iio_app_run and will not return.
 */
int example_main(void)
{
	Admv1355IioDev* admv1355_iio_dev;
	Admv1355IioDevInitParam admv1355_iio_ip;
	IioAppDesc* app;
	IioAppInitParam app_init_param = { 0 };
	int ret;

	pr_info("Starting IIO example\n");

	ret = admv1355_board_gpio_init();
	if (ret) {
		pr_err("admv1355_board_gpio_init failed with code: %d\n", ret);
		return ret;
	}

	admv1355_iio_ip.admv1355_dev_init = &admv1355_ip;
	ret = admv1355_iio_init(&admv1355_iio_dev, &admv1355_iio_ip);
	if (ret) {
		pr_err("admv1355_iio_init failed with code: %d\n", ret);
		return ret;
	}

	struct iio_app_device iio_devices[] = {
		{
			.name = "admv1355",
			.dev = admv1355_iio_dev,
			.dev_descriptor = admv1355_iio_dev->iio_dev,
		}
	};

	app_init_param.devices = iio_devices;
	app_init_param.nb_devices = NO_OS_ARRAY_SIZE(iio_devices);
	app_init_param.uart_init_params = admv1355_uart_ip;

	pr_info("Starting iio app\n");
	ret = iio_app_init(&app, app_init_param);
	if (ret) {
		pr_err("iio_app_init failed with code: %d\n", ret);
		goto exit;
	}

	ret = iio_app_run(app);

	iio_app_remove(app);
exit:
	admv1355_iio_remove(admv1355_iio_dev);

	if (ret)
		pr_info("Error!\n");

	return ret;
}
