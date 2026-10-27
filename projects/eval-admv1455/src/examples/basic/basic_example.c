/***************************************************************************//**
 *   @file   basic_example.c
 *   @brief  Basic example code for eval-admv1455 project.
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
#include <stdio.h>
#include "basic_example.h"
#include "common_data.h"
#include "admv1455.h"
#include "no_os_gpio.h"
#include "no_os_delay.h"
#include "no_os_print_log.h"
#include "no_os_error.h"
#include "no_os_util.h"

/* Every named register in admv1455.h, for a full register dump. */
static const struct {
	const char* name;
	uint16_t addr;
} admv1455_regs[] = {
	{ "SPI_CONTROL",     ADMV1455_REG_SPI_CONTROL },
	{ "PRODUCT_ID_LSB",  ADMV1455_REG_PRODUCT_ID_LSB },
	{ "SCRATCH_PAD",     ADMV1455_REG_SCRATCH_PAD },
	{ "ADC_TEMP",        ADMV1455_REG_ADC_TEMP_READBACK },
	{ "ADC_DETECTOR",    ADMV1455_REG_ADC_DETECTOR_READBACK },
	{ "IF_MODE",         ADMV1455_REG_IF_MODE },
	{ "IF_RES_CAL",      ADMV1455_REG_IF_RES_CAL },
	{ "FILTER_TABLE_EN", ADMV1455_REG_FILTER_TABLE_EN },
	{ "FILTER_TBL_SEL",  ADMV1455_REG_FILTER_TABLE_SEL },
	{ "FILTER_LOAD_EN",  ADMV1455_REG_FILTER_LOAD_EN },
	{ "EN_LOAD_G",       ADMV1455_REG_EN_LOAD_G },
	{ "GAIN_SPI_PTR",    ADMV1455_REG_GAIN_SPI_POINTER },
	{ "GAIN_TABLE_EN",   ADMV1455_REG_GAIN_TABLE_EN },
	{ "GAIN_LUT_MODE",   ADMV1455_REG_GAIN_LUT_MODE },
	{ "BYPASS_GAIN_EN",  ADMV1455_REG_BYPASS_GAIN_EN },
	{ "BYPASS_DSA1_2",   ADMV1455_REG_BYPASS_DSA1_DSA2 },
	{ "BYPASS_DSA3_4",   ADMV1455_REG_BYPASS_DSA3_DSA4 },
	{ "BYPASS_DSA5",     ADMV1455_REG_BYPASS_DSA5 },
	{ "BYPASS_GPO_G",    ADMV1455_REG_BYPASS_GPO_G },
	{ "IF_BPF_FINE",     ADMV1455_REG_IF_BPF_FINE },
	{ "DIRECT_DSA1_2",   ADMV1455_REG_DIRECT_DSA1_DSA2 },
	{ "DIRECT_DSA3_4",   ADMV1455_REG_DIRECT_DSA3_DSA4 },
	{ "DIRECT_DSA5",     ADMV1455_REG_DIRECT_DSA5 },
	{ "DIRECT_GPO_G",    ADMV1455_REG_DIRECT_GPO_G },
	{ "GPO_FG_OE_LSB",   ADMV1455_REG_GPO_FG_OE_LSB },
	{ "GPO_FG_OE_MSB",   ADMV1455_REG_GPO_FG_OE_MSB },
	{ "DIRECT_0x800",    ADMV1455_REG_DIRECT_0x800 },
	{ "DIRECT_0x801",    ADMV1455_REG_DIRECT_0x801 },
	{ "DIRECT_0x802",    ADMV1455_REG_DIRECT_0x802 },
	{ "DIRECT_0x803",    ADMV1455_REG_DIRECT_0x803 },
	{ "DIRECT_0x804",    ADMV1455_REG_DIRECT_0x804 },
	{ "DIRECT_GPO_F",    ADMV1455_REG_DIRECT_GPO_F },
	{ "DSA_OFF_1_2",     ADMV1455_REG_DIRECT_DSA_OFF_12 },
	{ "DSA_OFF_3_4",     ADMV1455_REG_DIRECT_DSA_OFF_34 },
	{ "DSA_OFF_5",       ADMV1455_REG_DIRECT_DSA_OFF_5 },
	{ "LO_PHASE_Q",      ADMV1455_REG_DIRECT_LO_PHASE_Q },
	{ "LO_PHASE_I",      ADMV1455_REG_DIRECT_LO_PHASE_I },
	{ "DSA_IQ",          ADMV1455_REG_DIRECT_DSA_IQ },
	{ "LO_DBLR",         ADMV1455_REG_DIRECT_LO_DBLR },
};

/**
 * @brief Read and print every named register in the ADMV1455 map.
 * @param dev - The device structure.
 * @return 0 in case of success, negative error code otherwise.
 */
static int admv1455_dump_registers(Admv1455Dev* dev)
{
	uint8_t val;
	size_t i;
	int ret;

	pr_info("---- register dump ----\n");
	for (i = 0; i < NO_OS_ARRAY_SIZE(admv1455_regs); i++) {
		ret = admv1455_spi_read(dev, admv1455_regs[i].addr, &val);
		if (ret) {
			pr_err("read 0x%03X (%s) failed with code: %d\n",
			       admv1455_regs[i].addr, admv1455_regs[i].name, ret);
			return ret;
		}

		pr_info("0x%03X %-16s = 0x%.2X\n", admv1455_regs[i].addr,
			admv1455_regs[i].name, val);
	}
	pr_info("------------------------\n");

	return 0;
}

/**
 * @brief Exercise every set/get API pair exposed by the ADMV1455 driver.
 * @param dev - The device structure.
 * @return 0 in case of success, negative error code otherwise.
 */
static int admv1455_exercise_api(Admv1455Dev* dev)
{
	uint8_t band, sideband, subband, lo_band;
	uint8_t dsa1, dsa2, dsa3, dsa4, dsa5, dsai, dsaq, phase_i, phase_q;
	uint8_t temp, detector;
	uint16_t trap;
	int ret;

	ret = admv1455_set_rf_band(dev, 1);
	if (ret)
		return ret;
	ret = admv1455_get_rf_band(dev, &band);
	if (ret)
		return ret;
	pr_info("RF band: %s\n", band ? "30GHz_55GHz" : "17p7GHz_34GHz");

	ret = admv1455_set_mixer_sideband(dev, 1);
	if (ret)
		return ret;
	ret = admv1455_get_mixer_sideband(dev, &sideband);
	if (ret)
		return ret;
	pr_info("LO sideband: %s\n", sideband ? "LSB" : "USB");

	ret = admv1455_set_if_subband(dev, 2);
	if (ret)
		return ret;
	ret = admv1455_get_if_subband(dev, &subband);
	if (ret)
		return ret;
	pr_info("IF subband index: %d (0=4GHz 1=6GHz 2=8GHz 3=10GHz)\n",
		subband);

	ret = admv1455_set_rf_dsa(dev, 3, 1, 5);
	if (ret)
		return ret;
	ret = admv1455_get_rf_dsa(dev, &dsa1, &dsa2, &dsa3);
	if (ret)
		return ret;
	pr_info("RF DSA1: -%ddB, DSA2: -%ddB, DSA3: -%ddB\n", dsa1, dsa2 * 6,
		dsa3);

	ret = admv1455_set_if_dsa(dev, 2, 4);
	if (ret)
		return ret;
	ret = admv1455_get_if_dsa(dev, &dsa4, &dsa5);
	if (ret)
		return ret;
	pr_info("IF DSA4: -%ddB, DSA5: -%ddB\n", dsa4, dsa5);

	ret = admv1455_set_if_iq_dsa(dev, 1, 2);
	if (ret)
		return ret;
	ret = admv1455_get_if_iq_dsa(dev, &dsai, &dsaq);
	if (ret)
		return ret;
	pr_info("IF DSAI: -%d00mdB, DSAQ: -%d00mdB\n", dsai, dsaq);

	ret = admv1455_set_lo_phase(dev, 7, 11);
	if (ret)
		return ret;
	ret = admv1455_get_lo_phase(dev, &phase_i, &phase_q);
	if (ret)
		return ret;
	pr_info("LO phase I: %d, Q: %d\n", phase_i, phase_q);

	ret = admv1455_set_lo_band(dev, 1);
	if (ret)
		return ret;
	ret = admv1455_get_lo_band(dev, &lo_band);
	if (ret)
		return ret;
	pr_info("LO band: %s\n", lo_band ? "12GHz_28GHz" : "8GHz_12GHz");

	ret = admv1455_set_lo_trap(dev, 0x1A7);
	if (ret)
		return ret;
	ret = admv1455_get_lo_trap(dev, &trap);
	if (ret)
		return ret;
	pr_info("LO trap filter: 0x%03X\n", trap);

	ret = admv1455_read_temperature(dev, &temp);
	if (ret)
		return ret;
	ret = admv1455_read_detector(dev, &detector);
	if (ret)
		return ret;
	pr_info("Raw ADC temperature: %d, detector: %d\n", temp, detector);

	return 0;
}

/***************************************************************************//**
 * @brief Basic example main execution.
 *
 * Drives the ADMV1455 DUT_LOAD and hardware LUT address pins (unused while
 * the on-chip filter/gain LUTs are disabled) to a known state, initializes
 * the ADMV1455 device (RSTB/CEN bring-up is handled by admv1455_init()),
 * dumps every named register, and exercises every set/get API pair exposed
 * by the driver.
 *
 * @return 0 in case of success, negative error code otherwise.
*******************************************************************************/
int basic_example_main(void)
{
	Admv1455Dev* admv1455_dev;
	int ret;

	pr_info("eval-admv1455 basic example starting\n");

	ret = admv1455_board_gpio_init();
	if (ret)
		return ret;

	pr_info("board GPIO bring-up done, initializing admv1455\n");

	ret = admv1455_init(&admv1455_dev, &admv1455_ip);
	if (ret) {
		pr_err("admv1455_init failed with code: %d\n", ret);
		return ret;
	}

	pr_info("admv1455 successfully initialized\n");

	ret = admv1455_dump_registers(admv1455_dev);
	if (ret)
		goto error;

	ret = admv1455_exercise_api(admv1455_dev);
	if (ret)
		goto error;

	return admv1455_remove(admv1455_dev);

error:
	admv1455_remove(admv1455_dev);

	return ret;
}
