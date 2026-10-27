/***************************************************************************//**
 *   @file   iio_admv1355.c
 *   @brief  Implementation of admv1355 IIO Driver.
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
#include <string.h>
#include <inttypes.h>
#include "no_os_error.h"
#include "no_os_util.h"
#include "no_os_alloc.h"
#include "iio_admv1355.h"

static const char* const admv1355_rf_band_items[] = {
	"low_band",
	"high_band",
};

static const char* const admv1355_mixer_sideband_items[] = {
	"LSB",
	"USB",
};

static const char* const admv1355_if_mode_items[] = {
	"baseband",
	"complex_if_low",
	"complex_if_high",
	"if_hybrid",
};

static const char* const admv1355_dsa_4bit_items[] = {
	"0dB", "-1dB", "-2dB", "-3dB", "-4dB", "-5dB", "-6dB", "-7dB",
	"-8dB", "-9dB", "-10dB", "-11dB", "-12dB", "-13dB", "-14dB", "-15dB"
};

static const char* const admv1355_dsa_iq_items[] = {
	"0dB", "-100mdB", "-200mdB", "-300mdB",
	"-400mdB", "-500mdB", "-600mdB", "-700mdB",
	"-800mdB", "-900mdB", "-1000mdB", "-1100mdB",
	"-1200mdB", "-1300mdB", "-1400mdB", "-1500mdB"
};

static const char* const admv1355_lo_x4_filter_items[] = {
	"10GHz_14GHz",
	"14GHz_18GHz",
	"ge_18GHz",
	"lt_10GHz",
};
static const uint8_t admv1355_lo_x4_filter_reg_values[] = {
	0x00, 0x05, 0x07, 0x08,
};

static const char* const admv1355_lo_x3_filter_items[] = {
	"ge_24GHz",
	"21GHz_24GHz",
	"19GHz_21GHz",
	"17GHz_19GHz",
	"15GHz_17GHz",
	"12GHz_15GHz",
	"lt_12GHz",
};
static const uint8_t admv1355_lo_x3_filter_reg_values[] = {
	0x00, 0x02, 0x03, 0x04, 0x05, 0x07, 0x1C,
};

static const char* const admv1355_lo_x1_filter_items[] = {
	"ge_21GHz",
	"17GHz_21GHz",
	"15GHz_17GHz",
	"12GHz_15GHz",
	"lt_12GHz",
};
static const uint8_t admv1355_lo_x1_filter_reg_values[] = {
	0x00, 0x01, 0x05, 0x0A, 0x1F,
};

static int admv1355_iio_find_enum_item(const char* value,
				      const char* const* items,
				      uint32_t num_items)
{
	uint32_t i;
	size_t item_len;

	for (i = 0; i < num_items; i++) {
		item_len = strlen(items[i]);
		if (!strncmp(value, items[i], item_len) &&
		    (value[item_len] == '\0' || value[item_len] == '\n'))
			return i;
	}

	return -EINVAL;
}

static int admv1355_iio_find_filter_item(uint8_t value,
					 const uint8_t* reg_values,
					 uint32_t num_items)
{
	uint32_t i;

	for (i = 0; i < num_items; i++)
		if (reg_values[i] == value)
			return i;

	return -EINVAL;
}

static int admv1355_iio_read_available(char* buf, uint32_t len,
				       const char* const* items,
				       uint32_t num_items)
{
	uint32_t written = 0;
	uint32_t i;
	int ret;

	if (!len)
		return -EINVAL;

	for (i = 0; i < num_items; i++) {
		ret = snprintf(buf + written, len - written, "%s%s",
			       i ? " " : "", items[i]);
		if (ret < 0)
			return ret;
		if ((uint32_t)ret >= len - written)
			return len;
		written += ret;
	}

	return written;
}

/**
 * @brief Wrapper for reading an admv1355 register.
 */
static int admv1355_iio_read_reg(void* dev, uint32_t reg, uint32_t* readval)
{
	Admv1355IioDev* iio_dev = dev;
	uint8_t tmp;
	int ret;

	ret = admv1355_spi_read(iio_dev->admv1355_dev, (uint16_t)reg, &tmp);
	if (ret)
		return ret;

	*readval = tmp;

	return 0;
}

/**
 * @brief Wrapper for writing an admv1355 register.
 */
static int admv1355_iio_write_reg(void* dev, uint32_t reg, uint32_t writeval)
{
	Admv1355IioDev* iio_dev = dev;

	return admv1355_spi_write(iio_dev->admv1355_dev, (uint16_t)reg,
				  (uint8_t)writeval);
}

/**
 * @brief Handles the read request for the device attributes.
 */
static int admv1355_iio_read_device_attr(void*dev, char*buf, uint32_t len,
		const IioChInfo *channel, intptr_t priv)
{
	Admv1355IioDev* iio_admv1355 = (Admv1355IioDev*)dev;
	Admv1355Dev* admv1355;
	Admv1355RfBand rf_band;
	Admv1355MixerSideband sideband;
	Admv1355IfMode if_mode;
	uint8_t dsa1, dsa2, dsai, dsaq, phase_i, phase_q;
	uint8_t lpf_val, hpf_val;
	uint8_t raw_val;
	uint16_t lo_trap;
	bool en;
	int32_t val = 0;
	int temp_mdeg;
	int ret;

	if (!iio_admv1355)
		return -EINVAL;

	admv1355 = iio_admv1355->admv1355_dev;
	if (!admv1355)
		return -EINVAL;

	switch (priv) {
	case ADMV1355_IIO_DEV_ATTR_RF_BAND_AVAILABLE:
		return admv1355_iio_read_available(buf, len, admv1355_rf_band_items,
						   NO_OS_ARRAY_SIZE(admv1355_rf_band_items));
	case ADMV1355_IIO_DEV_ATTR_MIXER_SIDEBAND_AVAILABLE:
		return admv1355_iio_read_available(buf, len,
						   admv1355_mixer_sideband_items,
						   NO_OS_ARRAY_SIZE(admv1355_mixer_sideband_items));
	case ADMV1355_IIO_DEV_ATTR_IF_MODE_AVAILABLE:
		return admv1355_iio_read_available(buf, len, admv1355_if_mode_items,
						   NO_OS_ARRAY_SIZE(admv1355_if_mode_items));
	case ADMV1355_IIO_DEV_ATTR_RF_DSA1_AVAILABLE:
	case ADMV1355_IIO_DEV_ATTR_RF_DSA2_AVAILABLE:
	case ADMV1355_IIO_DEV_ATTR_RF_DSA1_BYPASS_AVAILABLE:
	case ADMV1355_IIO_DEV_ATTR_RF_DSA2_BYPASS_AVAILABLE:
		return admv1355_iio_read_available(buf, len, admv1355_dsa_4bit_items,
						   NO_OS_ARRAY_SIZE(admv1355_dsa_4bit_items));
	case ADMV1355_IIO_DEV_ATTR_IF_DSAI_AVAILABLE:
	case ADMV1355_IIO_DEV_ATTR_IF_DSAQ_AVAILABLE:
		return admv1355_iio_read_available(buf, len, admv1355_dsa_iq_items,
						   NO_OS_ARRAY_SIZE(admv1355_dsa_iq_items));
	case ADMV1355_IIO_DEV_ATTR_LO_X4_FILTER_AVAILABLE:
		return admv1355_iio_read_available(buf, len,
						   admv1355_lo_x4_filter_items,
						   NO_OS_ARRAY_SIZE(admv1355_lo_x4_filter_items));
	case ADMV1355_IIO_DEV_ATTR_LO_X3_FILTER_AVAILABLE:
		return admv1355_iio_read_available(buf, len,
						   admv1355_lo_x3_filter_items,
						   NO_OS_ARRAY_SIZE(admv1355_lo_x3_filter_items));
	case ADMV1355_IIO_DEV_ATTR_LO_X1_FILTER_AVAILABLE:
		return admv1355_iio_read_available(buf, len,
						   admv1355_lo_x1_filter_items,
						   NO_OS_ARRAY_SIZE(admv1355_lo_x1_filter_items));
	case ADMV1355_IIO_DEV_ATTR_RF_BAND:
		ret = admv1355_get_rf_band(admv1355, &rf_band);
		if (ret)
			return ret;
		return snprintf(buf, len, "%s", admv1355_rf_band_items[rf_band]);

	case ADMV1355_IIO_DEV_ATTR_MIXER_SIDEBAND:
		ret = admv1355_get_mixer_sideband(admv1355, &sideband);
		if (ret)
			return ret;
		return snprintf(buf, len, "%s", admv1355_mixer_sideband_items[sideband]);

	case ADMV1355_IIO_DEV_ATTR_IF_MODE:
		ret = admv1355_get_if_mode(admv1355, &if_mode);
		if (ret)
			return ret;
		return snprintf(buf, len, "%s", admv1355_if_mode_items[if_mode]);

	case ADMV1355_IIO_DEV_ATTR_RF_DSA1:
		ret = admv1355_get_rf_dsa(admv1355, &dsa1, &dsa2);
		if (ret)
			return ret;
		return snprintf(buf, len, "%s", admv1355_dsa_4bit_items[dsa1]);

	case ADMV1355_IIO_DEV_ATTR_RF_DSA2:
		ret = admv1355_get_rf_dsa(admv1355, &dsa1, &dsa2);
		if (ret)
			return ret;
		return snprintf(buf, len, "%s", admv1355_dsa_4bit_items[dsa2]);

	case ADMV1355_IIO_DEV_ATTR_IF_DSAI:
		ret = admv1355_get_if_dsa(admv1355, &dsai, &dsaq);
		if (ret)
			return ret;
		return snprintf(buf, len, "%s", admv1355_dsa_iq_items[dsai]);

	case ADMV1355_IIO_DEV_ATTR_IF_DSAQ:
		ret = admv1355_get_if_dsa(admv1355, &dsai, &dsaq);
		if (ret)
			return ret;
		return snprintf(buf, len, "%s", admv1355_dsa_iq_items[dsaq]);

	case ADMV1355_IIO_DEV_ATTR_LO_PHASE_I:
		ret = admv1355_get_lo_phase(admv1355, &phase_i, &phase_q);
		if (ret)
			return ret;
		val = phase_i;
		break;

	case ADMV1355_IIO_DEV_ATTR_LO_PHASE_Q:
		ret = admv1355_get_lo_phase(admv1355, &phase_i, &phase_q);
		if (ret)
			return ret;
		val = phase_q;
		break;

	case ADMV1355_IIO_DEV_ATTR_LO_IN:
		return snprintf(buf, len, "%llu", admv1355->lo_in);

	case ADMV1355_IIO_DEV_ATTR_TEMP:
		ret = admv1355_read_temperature(admv1355, &temp_mdeg);
		if (ret)
			return ret;
		val = temp_mdeg;
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_DSA1_BYPASS:
		ret = admv1355_get_rf_dsa_bypass(admv1355, &dsa1, &dsa2);
		if (ret)
			return ret;
		val = dsa1;
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_DSA2_BYPASS:
		ret = admv1355_get_rf_dsa_bypass(admv1355, &dsa1, &dsa2);
		if (ret)
			return ret;
		val = dsa2;
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_BYPASS_LPF_EN:
		ret = admv1355_get_rf_lpf_bypass(admv1355, &en, &lpf_val);
		if (ret)
			return ret;
		return snprintf(buf, len, "%s", en ? "true" : "false");

	case ADMV1355_IIO_DEV_ATTR_RF_BYPASS_LPF_VAL:
		ret = admv1355_get_rf_lpf_bypass(admv1355, &en, &lpf_val);
		if (ret)
			return ret;
		val = lpf_val;
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_BYPASS_HPF_EN:
		ret = admv1355_get_rf_hpf_bypass(admv1355, &en, &hpf_val);
		if (ret)
			return ret;
		return snprintf(buf, len, "%s", en ? "true" : "false");

	case ADMV1355_IIO_DEV_ATTR_RF_BYPASS_HPF_VAL:
		ret = admv1355_get_rf_hpf_bypass(admv1355, &en, &hpf_val);
		if (ret)
			return ret;
		val = hpf_val;
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_DIRECT_LPF_VAL:
		ret = admv1355_get_rf_direct_filter(admv1355, &lpf_val, &hpf_val);
		if (ret)
			return ret;
		val = lpf_val;
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_DIRECT_HPF_VAL:
		ret = admv1355_get_rf_direct_filter(admv1355, &lpf_val, &hpf_val);
		if (ret)
			return ret;
		val = hpf_val;
		break;

	case ADMV1355_IIO_DEV_ATTR_LO_X4_FILTER:
		ret = admv1355_get_lo_x4_filter(admv1355, &raw_val);
		if (ret)
			return ret;
		ret = admv1355_iio_find_filter_item(raw_val,
						    admv1355_lo_x4_filter_reg_values,
						    NO_OS_ARRAY_SIZE(admv1355_lo_x4_filter_items));
		return ret < 0 ? ret : snprintf(buf, len, "%s",
						 admv1355_lo_x4_filter_items[ret]);

	case ADMV1355_IIO_DEV_ATTR_LO_X3_FILTER:
		ret = admv1355_get_lo_x3_filter(admv1355, &raw_val);
		if (ret)
			return ret;
		ret = admv1355_iio_find_filter_item(raw_val,
						    admv1355_lo_x3_filter_reg_values,
						    NO_OS_ARRAY_SIZE(admv1355_lo_x3_filter_items));
		return ret < 0 ? ret : snprintf(buf, len, "%s",
						 admv1355_lo_x3_filter_items[ret]);

	case ADMV1355_IIO_DEV_ATTR_LO_X1_FILTER:
		ret = admv1355_get_lo_x1_filter(admv1355, &raw_val);
		if (ret)
			return ret;
		ret = admv1355_iio_find_filter_item(raw_val,
						    admv1355_lo_x1_filter_reg_values,
						    NO_OS_ARRAY_SIZE(admv1355_lo_x1_filter_items));
		return ret < 0 ? ret : snprintf(buf, len, "%s",
						 admv1355_lo_x1_filter_items[ret]);

	case ADMV1355_IIO_DEV_ATTR_LO_TRAP:
		ret = admv1355_get_lo_trap(admv1355, &lo_trap);
		if (ret)
			return ret;
		val = lo_trap;
		break;

	case ADMV1355_IIO_DEV_ATTR_LO_DBL_BAND:
		ret = admv1355_get_lo_dbl_band(admv1355, &raw_val);
		if (ret)
			return ret;
		val = raw_val;
		break;

	case ADMV1355_IIO_DEV_ATTR_LON_OFFSET_I:
		ret = admv1355_get_lon_offset(admv1355, &dsa1, &dsa2);
		if (ret)
			return ret;
		val = dsa1;
		break;

	case ADMV1355_IIO_DEV_ATTR_LON_OFFSET_Q:
		ret = admv1355_get_lon_offset(admv1355, &dsa1, &dsa2);
		if (ret)
			return ret;
		val = dsa2;
		break;

	case ADMV1355_IIO_DEV_ATTR_FILTER_LUT_EN:
		ret = admv1355_get_filter_lut_en(admv1355, &en);
		if (ret)
			return ret;
		return snprintf(buf, len, "%s", en ? "true" : "false");

	case ADMV1355_IIO_DEV_ATTR_FILTER_LOAD_EN:
		ret = admv1355_get_filter_load_en(admv1355, &en);
		if (ret)
			return ret;
		return snprintf(buf, len, "%s", en ? "true" : "false");

	case ADMV1355_IIO_DEV_ATTR_FILTER_TABLE_SEL:
		ret = admv1355_get_filter_table_sel(admv1355, &raw_val);
		if (ret)
			return ret;
		return snprintf(buf, len, "%c", raw_val ? 'B' : 'A');

	case ADMV1355_IIO_DEV_ATTR_GAIN_LUT_EN:
		ret = admv1355_get_gain_lut_en(admv1355, &en);
		if (ret)
			return ret;
		return snprintf(buf, len, "%s", en ? "true" : "false");

	case ADMV1355_IIO_DEV_ATTR_GAIN_LUT_BYPASS_EN:
		ret = admv1355_get_gain_lut_bypass_en(admv1355, &en);
		if (ret)
			return ret;
		return snprintf(buf, len, "%s", en ? "true" : "false");

	case ADMV1355_IIO_DEV_ATTR_GAIN_LOAD_EN:
		ret = admv1355_get_gain_load_en(admv1355, &en);
		if (ret)
			return ret;
		return snprintf(buf, len, "%s", en ? "true" : "false");

	case ADMV1355_IIO_DEV_ATTR_DIRECT_GPO_G:
		ret = admv1355_get_gpo_g_direct(admv1355, &raw_val);
		if (ret)
			return ret;
		val = raw_val;
		break;

	case ADMV1355_IIO_DEV_ATTR_BYPASS_GPO_G:
		ret = admv1355_get_gpo_g_bypass(admv1355, &raw_val);
		if (ret)
			return ret;
		val = raw_val;
		break;

	case ADMV1355_IIO_DEV_ATTR_DIRECT_GPO_F:
		ret = admv1355_get_gpo_f_direct(admv1355, &raw_val);
		if (ret)
			return ret;
		val = raw_val;
		break;

	case ADMV1355_IIO_DEV_ATTR_GPO_F_OE:
		ret = admv1355_get_gpo_f_oe(admv1355, &raw_val);
		if (ret)
			return ret;
		val = raw_val;
		break;

	case ADMV1355_IIO_DEV_ATTR_GPO_G_OE:
		ret = admv1355_get_gpo_g_oe(admv1355, &raw_val);
		if (ret)
			return ret;
		val = raw_val;
		break;

	case ADMV1355_IIO_DEV_ATTR_FILTER_SM_STEP:
		ret = admv1355_get_filter_sm_step(admv1355, &raw_val);
		if (ret)
			return ret;
		val = raw_val;
		break;

	default:
		return -EINVAL;
	}

	return iio_format_value(buf, len, IIO_VAL_INT, 1, &val);
}

/**
 * @brief Handles the write request for the device attributes.
 */
static int admv1355_iio_write_device_attr(void* dev, char* buf, uint32_t len,
		const IioChInfo* channel, intptr_t priv)
{
	Admv1355IioDev* iio_admv1355 = (Admv1355IioDev*)dev;
	Admv1355Dev* admv1355;
	uint8_t dsa1, dsa2, dsai, dsaq, phase_i, phase_q;
	uint8_t lpf_val, hpf_val;
	uint64_t lo_in;
	int32_t val;
	bool en;
	bool dummy_en;
	int ret;

	if (!iio_admv1355)
		return -EINVAL;

	admv1355 = iio_admv1355->admv1355_dev;
	if (!admv1355)
		return -EINVAL;

	switch (priv) {
	case ADMV1355_IIO_DEV_ATTR_RF_BAND:
		val = admv1355_iio_find_enum_item(buf, admv1355_rf_band_items,
						  NO_OS_ARRAY_SIZE(admv1355_rf_band_items));
		if (val < 0)
			return val;
		ret = admv1355_set_rf_band(admv1355, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_MIXER_SIDEBAND:
		val = admv1355_iio_find_enum_item(buf,
						  admv1355_mixer_sideband_items,
						  NO_OS_ARRAY_SIZE(admv1355_mixer_sideband_items));
		if (val < 0)
			return val;
		ret = admv1355_set_mixer_sideband(admv1355, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_IF_MODE:
		val = admv1355_iio_find_enum_item(buf, admv1355_if_mode_items,
						  NO_OS_ARRAY_SIZE(admv1355_if_mode_items));
		if (val < 0)
			return val;
		ret = admv1355_set_if_mode(admv1355, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_DSA1:
		val = admv1355_iio_find_enum_item(buf, admv1355_dsa_4bit_items,
						  NO_OS_ARRAY_SIZE(admv1355_dsa_4bit_items));
		if (val < 0)
			return val;
		ret = admv1355_get_rf_dsa(admv1355, &dsa1, &dsa2);
		if (ret)
			return ret;
		ret = admv1355_set_rf_dsa(admv1355, val, dsa2);
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_DSA2:
		val = admv1355_iio_find_enum_item(buf, admv1355_dsa_4bit_items,
						  NO_OS_ARRAY_SIZE(admv1355_dsa_4bit_items));
		if (val < 0)
			return val;
		ret = admv1355_get_rf_dsa(admv1355, &dsa1, &dsa2);
		if (ret)
			return ret;
		ret = admv1355_set_rf_dsa(admv1355, dsa1, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_IF_DSAI:
		val = admv1355_iio_find_enum_item(buf, admv1355_dsa_iq_items,
						  NO_OS_ARRAY_SIZE(admv1355_dsa_iq_items));
		if (val < 0)
			return val;
		ret = admv1355_get_if_dsa(admv1355, &dsai, &dsaq);
		if (ret)
			return ret;
		ret = admv1355_set_if_dsa(admv1355, val, dsaq);
		break;

	case ADMV1355_IIO_DEV_ATTR_IF_DSAQ:
		val = admv1355_iio_find_enum_item(buf, admv1355_dsa_iq_items,
						  NO_OS_ARRAY_SIZE(admv1355_dsa_iq_items));
		if (val < 0)
			return val;
		ret = admv1355_get_if_dsa(admv1355, &dsai, &dsaq);
		if (ret)
			return ret;
		ret = admv1355_set_if_dsa(admv1355, dsai, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_LO_PHASE_I:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_get_lo_phase(admv1355, &phase_i, &phase_q);
		if (ret)
			return ret;
		ret = admv1355_set_lo_phase(admv1355, val, phase_q);
		break;

	case ADMV1355_IIO_DEV_ATTR_LO_PHASE_Q:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_get_lo_phase(admv1355, &phase_i, &phase_q);
		if (ret)
			return ret;
		ret = admv1355_set_lo_phase(admv1355, phase_i, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_LO_IN:
		sscanf(buf, "%"PRIu64, &lo_in);
		ret = admv1355_set_lo_filters(admv1355, lo_in);
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_DSA1_BYPASS:
		val = admv1355_iio_find_enum_item(buf, admv1355_dsa_4bit_items,
						  NO_OS_ARRAY_SIZE(admv1355_dsa_4bit_items));
		if (val < 0)
			return val;
		ret = admv1355_get_rf_dsa_bypass(admv1355, &dsa1, &dsa2);
		if (ret)
			return ret;
		ret = admv1355_set_rf_dsa_bypass(admv1355, val, dsa2);
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_DSA2_BYPASS:
		val = admv1355_iio_find_enum_item(buf, admv1355_dsa_4bit_items,
						  NO_OS_ARRAY_SIZE(admv1355_dsa_4bit_items));
		if (val < 0)
			return val;
		ret = admv1355_get_rf_dsa_bypass(admv1355, &dsa1, &dsa2);
		if (ret)
			return ret;
		ret = admv1355_set_rf_dsa_bypass(admv1355, dsa1, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_BYPASS_LPF_EN:
		en = !strncmp(buf, "true", 4) || !strncmp(buf, "1", 1);
		ret = admv1355_get_rf_lpf_bypass(admv1355, &dummy_en, &lpf_val);
		if (ret)
			return ret;
		ret = admv1355_set_rf_lpf_bypass(admv1355, en, lpf_val);
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_BYPASS_LPF_VAL:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_get_rf_lpf_bypass(admv1355, &en, &lpf_val);
		if (ret)
			return ret;
		ret = admv1355_set_rf_lpf_bypass(admv1355, en, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_BYPASS_HPF_EN:
		en = !strncmp(buf, "true", 4) || !strncmp(buf, "1", 1);
		ret = admv1355_get_rf_hpf_bypass(admv1355, &dummy_en, &hpf_val);
		if (ret)
			return ret;
		ret = admv1355_set_rf_hpf_bypass(admv1355, en, hpf_val);
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_BYPASS_HPF_VAL:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_get_rf_hpf_bypass(admv1355, &en, &hpf_val);
		if (ret)
			return ret;
		ret = admv1355_set_rf_hpf_bypass(admv1355, en, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_DIRECT_LPF_VAL:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_get_rf_direct_filter(admv1355, &lpf_val, &hpf_val);
		if (ret)
			return ret;
		ret = admv1355_set_rf_direct_filter(admv1355, val, hpf_val);
		break;

	case ADMV1355_IIO_DEV_ATTR_RF_DIRECT_HPF_VAL:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_get_rf_direct_filter(admv1355, &lpf_val, &hpf_val);
		if (ret)
			return ret;
		ret = admv1355_set_rf_direct_filter(admv1355, lpf_val, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_LO_X4_FILTER:
		val = admv1355_iio_find_enum_item(buf, admv1355_lo_x4_filter_items,
						  NO_OS_ARRAY_SIZE(admv1355_lo_x4_filter_items));
		if (val < 0)
			return val;
		val = admv1355_lo_x4_filter_reg_values[val];
		ret = admv1355_set_lo_x4_filter(admv1355, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_LO_X3_FILTER:
		val = admv1355_iio_find_enum_item(buf, admv1355_lo_x3_filter_items,
						  NO_OS_ARRAY_SIZE(admv1355_lo_x3_filter_items));
		if (val < 0)
			return val;
		val = admv1355_lo_x3_filter_reg_values[val];
		ret = admv1355_set_lo_x3_filter(admv1355, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_LO_X1_FILTER:
		val = admv1355_iio_find_enum_item(buf, admv1355_lo_x1_filter_items,
						  NO_OS_ARRAY_SIZE(admv1355_lo_x1_filter_items));
		if (val < 0)
			return val;
		val = admv1355_lo_x1_filter_reg_values[val];
		ret = admv1355_set_lo_x1_filter(admv1355, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_LO_TRAP:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_set_lo_trap(admv1355, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_LO_DBL_BAND:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_set_lo_dbl_band(admv1355, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_LON_OFFSET_I:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_get_lon_offset(admv1355, &dsa1, &dsa2);
		if (ret)
			return ret;
		ret = admv1355_set_lon_offset(admv1355, val, dsa2);
		break;

	case ADMV1355_IIO_DEV_ATTR_LON_OFFSET_Q:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_get_lon_offset(admv1355, &dsa1, &dsa2);
		if (ret)
			return ret;
		ret = admv1355_set_lon_offset(admv1355, dsa1, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_FILTER_LUT_EN:
		ret = admv1355_set_filter_lut_en(admv1355, !strncmp(buf, "true", 4));
		break;

	case ADMV1355_IIO_DEV_ATTR_FILTER_LOAD_EN:
		ret = admv1355_set_filter_load_en(admv1355, !strncmp(buf, "true", 4));
		break;

	case ADMV1355_IIO_DEV_ATTR_FILTER_TABLE_SEL:
		ret = admv1355_set_filter_table_sel(admv1355, (buf[0] == 'B') ? 1 : 0);
		break;

	case ADMV1355_IIO_DEV_ATTR_GAIN_LUT_EN:
		ret = admv1355_set_gain_lut_en(admv1355, !strncmp(buf, "true", 4));
		break;

	case ADMV1355_IIO_DEV_ATTR_GAIN_LUT_BYPASS_EN:
		ret = admv1355_set_gain_lut_bypass_en(admv1355, !strncmp(buf, "true", 4));
		break;

	case ADMV1355_IIO_DEV_ATTR_GAIN_LOAD_EN:
		ret = admv1355_set_gain_load_en(admv1355, !strncmp(buf, "true", 4));
		break;

	case ADMV1355_IIO_DEV_ATTR_DIRECT_GPO_G:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_set_gpo_g_direct(admv1355, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_BYPASS_GPO_G:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_set_gpo_g_bypass(admv1355, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_DIRECT_GPO_F:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_set_gpo_f_direct(admv1355, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_GPO_F_OE:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_set_gpo_f_oe(admv1355, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_GPO_G_OE:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_set_gpo_g_oe(admv1355, val);
		break;

	case ADMV1355_IIO_DEV_ATTR_FILTER_SM_STEP:
		ret = iio_parse_value(buf, IIO_VAL_INT, &val, NULL);
		if (ret)
			return ret;
		ret = admv1355_set_filter_sm_step(admv1355, val);
		break;

	default:
		return -EINVAL;
	}

	if (ret)
		return ret;

	return len;
}

#define ADMV1355_IIO_ATTR(_name, _id) { \
	.name = _name, \
	.priv = _id, \
	.show = admv1355_iio_read_device_attr, \
	.store = admv1355_iio_write_device_attr, \
}

#define ADMV1355_IIO_ATTR_AVAILABLE(_name, _id) { \
	.name = _name, \
	.priv = _id, \
	.show = admv1355_iio_read_device_attr, \
}

static IioAttribute admv1355_if_attrs[] = {
	ADMV1355_IIO_ATTR("mode", ADMV1355_IIO_DEV_ATTR_IF_MODE),
	ADMV1355_IIO_ATTR_AVAILABLE("mode_available",
				    ADMV1355_IIO_DEV_ATTR_IF_MODE_AVAILABLE),
	END_ATTRIBUTES_ARRAY
};

static IioAttribute admv1355_rf_attrs[] = {
	ADMV1355_IIO_ATTR("band", ADMV1355_IIO_DEV_ATTR_RF_BAND),
	ADMV1355_IIO_ATTR_AVAILABLE("band_available",
				    ADMV1355_IIO_DEV_ATTR_RF_BAND_AVAILABLE),
	ADMV1355_IIO_ATTR("direct_dsa1_gain", ADMV1355_IIO_DEV_ATTR_RF_DSA1),
	ADMV1355_IIO_ATTR_AVAILABLE("direct_dsa1_gain_available",
				    ADMV1355_IIO_DEV_ATTR_RF_DSA1_AVAILABLE),
	ADMV1355_IIO_ATTR("direct_dsa2_gain", ADMV1355_IIO_DEV_ATTR_RF_DSA2),
	ADMV1355_IIO_ATTR_AVAILABLE("direct_dsa2_gain_available",
				    ADMV1355_IIO_DEV_ATTR_RF_DSA2_AVAILABLE),
	ADMV1355_IIO_ATTR("bypass_dsa1_gain", ADMV1355_IIO_DEV_ATTR_RF_DSA1_BYPASS),
	ADMV1355_IIO_ATTR_AVAILABLE("bypass_dsa1_gain_available",
				    ADMV1355_IIO_DEV_ATTR_RF_DSA1_BYPASS_AVAILABLE),
	ADMV1355_IIO_ATTR("bypass_dsa2_gain", ADMV1355_IIO_DEV_ATTR_RF_DSA2_BYPASS),
	ADMV1355_IIO_ATTR_AVAILABLE("bypass_dsa2_gain_available",
				    ADMV1355_IIO_DEV_ATTR_RF_DSA2_BYPASS_AVAILABLE),
	ADMV1355_IIO_ATTR("direct_lpf_val", ADMV1355_IIO_DEV_ATTR_RF_DIRECT_LPF_VAL),
	ADMV1355_IIO_ATTR("direct_hpf_val", ADMV1355_IIO_DEV_ATTR_RF_DIRECT_HPF_VAL),
	ADMV1355_IIO_ATTR("bypass_lpf_en", ADMV1355_IIO_DEV_ATTR_RF_BYPASS_LPF_EN),
	ADMV1355_IIO_ATTR("bypass_lpf_val", ADMV1355_IIO_DEV_ATTR_RF_BYPASS_LPF_VAL),
	ADMV1355_IIO_ATTR("bypass_hpf_en", ADMV1355_IIO_DEV_ATTR_RF_BYPASS_HPF_EN),
	ADMV1355_IIO_ATTR("bypass_hpf_val", ADMV1355_IIO_DEV_ATTR_RF_BYPASS_HPF_VAL),
	END_ATTRIBUTES_ARRAY
};

static IioAttribute admv1355_lo_attrs[] = {
	ADMV1355_IIO_ATTR("sideband", ADMV1355_IIO_DEV_ATTR_MIXER_SIDEBAND),
	ADMV1355_IIO_ATTR_AVAILABLE("sideband_available",
				    ADMV1355_IIO_DEV_ATTR_MIXER_SIDEBAND_AVAILABLE),
	ADMV1355_IIO_ATTR("dsai_0p1db", ADMV1355_IIO_DEV_ATTR_IF_DSAI),
	ADMV1355_IIO_ATTR_AVAILABLE("dsai_0p1db_available",
				    ADMV1355_IIO_DEV_ATTR_IF_DSAI_AVAILABLE),
	ADMV1355_IIO_ATTR("dsaq_0p1db", ADMV1355_IIO_DEV_ATTR_IF_DSAQ),
	ADMV1355_IIO_ATTR_AVAILABLE("dsaq_0p1db_available",
				    ADMV1355_IIO_DEV_ATTR_IF_DSAQ_AVAILABLE),
	ADMV1355_IIO_ATTR("direct_i_phase_val", ADMV1355_IIO_DEV_ATTR_LO_PHASE_I),
	ADMV1355_IIO_ATTR("direct_q_phase_val", ADMV1355_IIO_DEV_ATTR_LO_PHASE_Q),
	ADMV1355_IIO_ATTR("direct_lon_offset_i", ADMV1355_IIO_DEV_ATTR_LON_OFFSET_I),
	ADMV1355_IIO_ATTR("direct_lon_offset_q", ADMV1355_IIO_DEV_ATTR_LON_OFFSET_Q),
	ADMV1355_IIO_ATTR("lo_x4_filter", ADMV1355_IIO_DEV_ATTR_LO_X4_FILTER),
	ADMV1355_IIO_ATTR_AVAILABLE("lo_x4_filter_available",
				    ADMV1355_IIO_DEV_ATTR_LO_X4_FILTER_AVAILABLE),
	ADMV1355_IIO_ATTR("lo_x3_filter", ADMV1355_IIO_DEV_ATTR_LO_X3_FILTER),
	ADMV1355_IIO_ATTR_AVAILABLE("lo_x3_filter_available",
				    ADMV1355_IIO_DEV_ATTR_LO_X3_FILTER_AVAILABLE),
	ADMV1355_IIO_ATTR("lo_x1_filter", ADMV1355_IIO_DEV_ATTR_LO_X1_FILTER),
	ADMV1355_IIO_ATTR_AVAILABLE("lo_x1_filter_available",
				    ADMV1355_IIO_DEV_ATTR_LO_X1_FILTER_AVAILABLE),
	ADMV1355_IIO_ATTR("lo_trap", ADMV1355_IIO_DEV_ATTR_LO_TRAP),
	ADMV1355_IIO_ATTR("lo_dbl_band", ADMV1355_IIO_DEV_ATTR_LO_DBL_BAND),
	END_ATTRIBUTES_ARRAY
};

static IioAttribute admv1355_temp_attrs[] = {
	{
		.name = "input",
		.priv = ADMV1355_IIO_DEV_ATTR_TEMP,
		.show = admv1355_iio_read_device_attr,
	},
	END_ATTRIBUTES_ARRAY
};

static IioAttribute admv1355_iio_attrs[] = {
	{
		.name = "filter_lut_en",
		.priv = ADMV1355_IIO_DEV_ATTR_FILTER_LUT_EN,
		.show = admv1355_iio_read_device_attr,
		.store = admv1355_iio_write_device_attr,
	},
	{
		.name = "filter_load_en",
		.priv = ADMV1355_IIO_DEV_ATTR_FILTER_LOAD_EN,
		.show = admv1355_iio_read_device_attr,
		.store = admv1355_iio_write_device_attr,
	},
	{
		.name = "filter_table_sel",
		.priv = ADMV1355_IIO_DEV_ATTR_FILTER_TABLE_SEL,
		.show = admv1355_iio_read_device_attr,
		.store = admv1355_iio_write_device_attr,
	},
	{
		.name = "gain_lut_en",
		.priv = ADMV1355_IIO_DEV_ATTR_GAIN_LUT_EN,
		.show = admv1355_iio_read_device_attr,
		.store = admv1355_iio_write_device_attr,
	},
	{
		.name = "gain_lut_bypass_en",
		.priv = ADMV1355_IIO_DEV_ATTR_GAIN_LUT_BYPASS_EN,
		.show = admv1355_iio_read_device_attr,
		.store = admv1355_iio_write_device_attr,
	},
	{
		.name = "gain_load_en",
		.priv = ADMV1355_IIO_DEV_ATTR_GAIN_LOAD_EN,
		.show = admv1355_iio_read_device_attr,
		.store = admv1355_iio_write_device_attr,
	},
	{
		.name = "direct_gpo_g",
		.priv = ADMV1355_IIO_DEV_ATTR_DIRECT_GPO_G,
		.show = admv1355_iio_read_device_attr,
		.store = admv1355_iio_write_device_attr,
	},
	{
		.name = "bypass_gpo_g",
		.priv = ADMV1355_IIO_DEV_ATTR_BYPASS_GPO_G,
		.show = admv1355_iio_read_device_attr,
		.store = admv1355_iio_write_device_attr,
	},
	{
		.name = "direct_gpo_f",
		.priv = ADMV1355_IIO_DEV_ATTR_DIRECT_GPO_F,
		.show = admv1355_iio_read_device_attr,
		.store = admv1355_iio_write_device_attr,
	},
	{
		.name = "gpo_f_oe",
		.priv = ADMV1355_IIO_DEV_ATTR_GPO_F_OE,
		.show = admv1355_iio_read_device_attr,
		.store = admv1355_iio_write_device_attr,
	},
	{
		.name = "gpo_g_oe",
		.priv = ADMV1355_IIO_DEV_ATTR_GPO_G_OE,
		.show = admv1355_iio_read_device_attr,
		.store = admv1355_iio_write_device_attr,
	},
	{
		.name = "filter_sm_step",
		.priv = ADMV1355_IIO_DEV_ATTR_FILTER_SM_STEP,
		.show = admv1355_iio_read_device_attr,
		.store = admv1355_iio_write_device_attr,
	},
	END_ATTRIBUTES_ARRAY
};

static IioChannel admv1355_channels[] = {
	{
		.name = "if",
		.ch_type = IIO_ALTVOLTAGE,
		.channel = 0,
		.indexed = true,
		.attributes = admv1355_if_attrs,
	},
	{
		.name = "rf",
		.ch_type = IIO_ALTVOLTAGE,
		.channel = 1,
		.ch_out = true,
		.indexed = true,
		.attributes = admv1355_rf_attrs,
	},
	{
		.name = "lo",
		.ch_type = IIO_ALTVOLTAGE,
		.channel = 2,
		.indexed = true,
		.attributes = admv1355_lo_attrs,
	},
	{
		.name = "temperature",
		.ch_type = IIO_TEMP,
		.channel = 0,
		.indexed = true,
		.attributes = admv1355_temp_attrs,
	},
};

static IioDevice admv1355_iio_dev = {
	.num_ch = NO_OS_ARRAY_SIZE(admv1355_channels),
	.channels = admv1355_channels,
	.attributes = admv1355_iio_attrs,
	.debug_reg_read = admv1355_iio_read_reg,
	.debug_reg_write = admv1355_iio_write_reg,
};

/**
 * @brief Initializes the ADMV1355 IIO driver.
 * @param iio_dev - The iio device structure.
 * @param init_param - The iio device initialization structure.
 * @return 0 in case of success, negative error code otherwise.
 */
int admv1355_iio_init(Admv1355IioDev** iio_dev,
		       Admv1355IioDevInitParam* init_param)
{
	Admv1355IioDev* dev;
	int ret;

	dev = (Admv1355IioDev*)no_os_calloc(1, sizeof(*dev));
	if (!dev)
		return -ENOMEM;

	dev->iio_dev = &admv1355_iio_dev;

	ret = admv1355_init(&dev->admv1355_dev, init_param->admv1355_dev_init);
	if (ret)
		goto error;

	*iio_dev = dev;

	return 0;

error:
	no_os_free(dev);

	return ret;
}

/**
 * @brief Free the resources allocated by admv1355_iio_init().
 * @param desc - The iio device structure.
 * @return 0 in case of success, negative error code otherwise.
 */
int admv1355_iio_remove(Admv1355IioDev* desc)
{
	int ret;

	ret = admv1355_remove(desc->admv1355_dev);
	if (ret)
		return ret;

	no_os_free(desc);

	return 0;
}
