/***************************************************************************//**
 *   @file   basic_example.c
 *   @brief  Basic example code for eval-admv1355 project.
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
#include "admv1355.h"
#include "no_os_gpio.h"
#include "no_os_delay.h"
#include "no_os_print_log.h"
#include "no_os_error.h"
#include "no_os_util.h"

/* Every named register in admv1355.h, for a full register dump. */
static const struct {
	const char *name;
	uint16_t addr;
} admv1355_regs[] = {
	{ "SDO_CTRL",        ADMV1355_REG_SDO_CTRL },
	{ "PRODUCT_ID_LSB",  ADMV1355_REG_PRODUCT_ID_LSB },
	{ "SCRATCH_PAD",     ADMV1355_REG_SCRATCH_PAD },
	{ "NVM_CTRL",        ADMV1355_REG_NVM_CTRL },
	{ "NVM_ADDR",        ADMV1355_REG_NVM_ADDR },
	{ "NVM_DATA",        ADMV1355_REG_NVM_DATA },
	{ "NVM_LOAD",        ADMV1355_REG_NVM_LOAD },
	{ "RF_HS_PD",        ADMV1355_REG_RF_HS_PD },
	{ "FILTER_SM_STEP",  ADMV1355_REG_FILTER_SM_STEP },
	{ "FILTER_LUT_EN",   ADMV1355_REG_FILTER_LUT_EN },
	{ "FILTER_TBL_SEL",  ADMV1355_REG_FILTER_TBL_SEL },
	{ "FILTER_LOAD_EN",  ADMV1355_REG_FILTER_LOAD_EN },
	{ "GAIN_LOAD_EN",    ADMV1355_REG_GAIN_LOAD_EN },
	{ "GAIN_LUT_EN",     ADMV1355_REG_GAIN_LUT_EN },
	{ "GAIN_TBL_BYP",    ADMV1355_REG_GAIN_TBL_BYP },
	{ "DSA_BYPASS",      ADMV1355_REG_DSA_BYPASS },
	{ "GPO_G_BYPASS",    ADMV1355_REG_GPO_G_BYPASS },
	{ "RF_LPF",          ADMV1355_REG_RF_LPF },
	{ "RF_HPF",          ADMV1355_REG_RF_HPF },
	{ "ADC_MUX",         ADMV1355_REG_ADC_MUX },
	{ "ADC_CTRL",        ADMV1355_REG_ADC_CTRL },
	{ "ADC_RESET",       ADMV1355_REG_ADC_RESET },
	{ "LON_CTRL",        ADMV1355_REG_LON_CTRL },
	{ "ADC_OUT",         ADMV1355_REG_ADC_OUT },
	{ "DSA_DIRECT",      ADMV1355_REG_DSA_DIRECT },
	{ "GPO_G_DIRECT",    ADMV1355_REG_GPO_G_DIRECT },
	{ "IF_ENABLE",       ADMV1355_REG_IF_ENABLE },
	{ "GPO_F_OE",        ADMV1355_REG_GPO_F_OE },
	{ "GPO_GF_OE",       ADMV1355_REG_GPO_GF_OE },
	{ "LO_X4_FILTER",    ADMV1355_REG_LO_X4_FILTER },
	{ "LO_X3_FILTER",    ADMV1355_REG_LO_X3_FILTER },
	{ "LO_X1_FILTER",    ADMV1355_REG_LO_X1_FILTER },
	{ "GPO_F_DIRECT",    ADMV1355_REG_GPO_F_DIRECT },
	{ "LO_TRAP_7_0",     ADMV1355_REG_LO_TRAP_7_0 },
	{ "LO_TRAP_9_8",     ADMV1355_REG_LO_TRAP_9_8 },
	{ "LO_BAND",         ADMV1355_REG_LO_BAND },
	{ "LON_OFFSET_I",    ADMV1355_REG_LON_OFFSET_I },
	{ "LON_OFFSET_Q",    ADMV1355_REG_LON_OFFSET_Q },
	{ "IF_GAIN",         ADMV1355_REG_IF_GAIN },
	{ "RF_FILTER",       ADMV1355_REG_RF_FILTER },
	{ "LO_DBL_BAND",     ADMV1355_REG_LO_DBL_BAND },
};

/**
 * @brief Read and print every named register in the ADMV1355 map.
 * @param dev - The device structure.
 * @return 0 in case of success, negative error code otherwise.
 */
static int admv1355_dump_registers(struct admv1355_dev *dev)
{
	uint8_t val;
	size_t i;
	int ret;

	pr_info("---- register dump ----\n");
	for (i = 0; i < NO_OS_ARRAY_SIZE(admv1355_regs); i++) {
		ret = admv1355_spi_read(dev, admv1355_regs[i].addr, &val);
		if (ret) {
			pr_err("read 0x%03X (%s) failed with code: %d\n",
			       admv1355_regs[i].addr, admv1355_regs[i].name, ret);
			return ret;
		}

		pr_info("0x%03X %-16s = 0x%.2X\n", admv1355_regs[i].addr,
			admv1355_regs[i].name, val);
	}
	pr_info("------------------------\n");

	return 0;
}

/**
 * @brief Exercise every set/get API pair exposed by the ADMV1355 driver.
 * @param dev - The device structure.
 * @return 0 in case of success, negative error code otherwise.
 */
static int admv1355_exercise_api(struct admv1355_dev *dev)
{
	enum admv1355_rf_band rf_band;
	enum admv1355_mixer_sideband sideband;
	enum admv1355_if_mode if_mode;
	uint8_t dsa1, dsa2, dsai, dsaq, phase_i, phase_q;
	int temp_mdeg;
	int ret;

	ret = admv1355_set_rf_band(dev, ADMV1355_RF_BAND_HIGH);
	if (ret)
		return ret;
	ret = admv1355_get_rf_band(dev, &rf_band);
	if (ret)
		return ret;
	pr_info("RF band: %s\n", rf_band == ADMV1355_RF_BAND_HIGH ? "high" : "low");

	ret = admv1355_set_mixer_sideband(dev, ADMV1355_SIDEBAND_USB);
	if (ret)
		return ret;
	ret = admv1355_get_mixer_sideband(dev, &sideband);
	if (ret)
		return ret;
	pr_info("Mixer sideband: %s\n",
		sideband == ADMV1355_SIDEBAND_USB ? "USB" : "LSB");

	ret = admv1355_set_if_mode(dev, ADMV1355_IF_MODE_BASEBAND);
	if (ret)
		return ret;
	ret = admv1355_get_if_mode(dev, &if_mode);
	if (ret)
		return ret;
	pr_info("IF mode: %d\n", if_mode);

	ret = admv1355_set_rf_dsa(dev, 3, 5);
	if (ret)
		return ret;
	ret = admv1355_get_rf_dsa(dev, &dsa1, &dsa2);
	if (ret)
		return ret;
	pr_info("RF DSA1: -%ddB, DSA2: -%ddB\n", dsa1, dsa2);

	ret = admv1355_set_if_dsa(dev, 2, 4);
	if (ret)
		return ret;
	ret = admv1355_get_if_dsa(dev, &dsai, &dsaq);
	if (ret)
		return ret;
	pr_info("IF DSAI: -%d00mdB, DSAQ: -%d00mdB\n", dsai, dsaq);

	ret = admv1355_set_lo_phase(dev, 7, 11);
	if (ret)
		return ret;
	ret = admv1355_get_lo_phase(dev, &phase_i, &phase_q);
	if (ret)
		return ret;
	pr_info("LO phase I: %d, Q: %d\n", phase_i, phase_q);

	ret = admv1355_set_lo_filters(dev, admv1355_ip.lo_in);
	if (ret)
		return ret;
	pr_info("LO filters reselected for %llu Hz\n", admv1355_ip.lo_in);

	ret = admv1355_read_temperature(dev, &temp_mdeg);
	if (ret)
		return ret;
	pr_info("Die temperature: %d m\xc2\xb0""C\n", temp_mdeg);

	return 0;
}

/***************************************************************************//**
 * @brief Basic example main execution.
 *
 * Drives the ADMV1355 DUT_LOAD and hardware LUT address pins (unused while
 * the on-chip filter/gain LUTs are disabled) to a known state, initializes
 * the ADMV1355 device (RSTB/CEN bring-up is handled by admv1355_init()),
 * dumps every named register, and exercises every set/get API pair exposed
 * by the driver.
 *
 * @return 0 in case of success, negative error code otherwise.
*******************************************************************************/
int basic_example_main(void)
{
	struct admv1355_dev *admv1355_dev;
	int ret;

	pr_info("eval-admv1355 basic example starting\n");

	ret = admv1355_board_gpio_init();
	if (ret)
		return ret;

	pr_info("board GPIO bring-up done, initializing admv1355\n");

	ret = admv1355_init(&admv1355_dev, &admv1355_ip);
	if (ret) {
		pr_err("admv1355_init failed with code: %d\n", ret);
		return ret;
	}

	pr_info("admv1355 successfully initialized\n");

	ret = admv1355_dump_registers(admv1355_dev);
	if (ret)
		goto error;

	ret = admv1355_exercise_api(admv1355_dev);
	if (ret)
		goto error;

	return admv1355_remove(admv1355_dev);

error:
	admv1355_remove(admv1355_dev);

	return ret;
}
