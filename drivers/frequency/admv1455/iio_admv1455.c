/***************************************************************************//**
 *   @file   iio_admv1455.c
 *   @brief  Implementation of admv1455 IIO Driver.
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iio_admv1455.h"
#include "no_os_alloc.h"
#include "no_os_error.h"

static const char* const admv1455_rf_band_items[] = {
    "17p7GHz_34GHz", "30GHz_55GHz",
};

static const char* const admv1455_lo_sideband_items[] = {
    "USB", "LSB",
};

static const char* const admv1455_dsa_4bit_items[] = {
    "0dB",  "-1dB", "-2dB",  "-3dB",  "-4dB",  "-5dB",	"-6dB",	 "-7dB",
    "-8dB", "-9dB", "-10dB", "-11dB", "-12dB", "-13dB", "-14dB", "-15dB"
};

static const char* const admv1455_dsa_1bit_items[] = {
	"0dB", "-6dB"
};

static const char* const admv1455_dsa_iq_items[] = {
    "0dB",		"-100mdB",  "-200mdB",	"-300mdB", "-400mdB",  "-500mdB",
    "-600mdB",	"-700mdB",  "-800mdB",	"-900mdB", "-1000mdB", "-1100mdB",
    "-1200mdB", "-1300mdB", "-1400mdB", "-1500mdB"
};

static const char* const admv1455_if_subband_items[] = {
    "4GHz", "6GHz", "8GHz", "10GHz",
};

static const char* const admv1455_lo_band_items[] = {
    "8GHz_12GHz", "12GHz_28GHz",
};

static const char* const admv1455_x1_filter_items[] = {
    "21GHz_28GHz", "18GHz_21GHz", "14GHz_18GHz", "12GHz_14GHz", "8GHz_12GHz",
};

static const unsigned int admv1455_x1_filter_values[] = {
    0, 15, 23, 27, 31,
};

static const char* const admv1455_x3_filter_items[] = {
    "24GHz_28GHz", "21GHz_24GHz", "19GHz_21GHz", "17GHz_19GHz",
    "15GHz_17GHz", "12GHz_15GHz", "8GHz_12GHz",
};

static const unsigned int admv1455_x3_filter_values[] = {
    0, 2, 3, 4, 5, 7, 28,
};

static const char* const admv1455_x4_filter_items[] = {
    "10GHz_14GHz", "14GHz_18GHz", "18GHz_28GHz", "8GHz_10GHz",
};

static const unsigned int admv1455_x4_filter_values[] = {
    0, 5, 7, 8,
};

static const char* const admv1455_lo_trap_items[] = {
    "21GHz_28GHz",   "18p5GHz_21GHz", "14p5GHz_18p5GHz",
    "12GHz_14p5GHz", "8GHz_12GHz",
};

static const unsigned int admv1455_lo_trap_values[] = {
    0x1E7, 0x1A7, 0x1A3, 0x3AA, 0x318,
};

static const char* const admv1455_lo_doubler_items[] = {
    "24GHz_28GHz", "18GHz_24GHz", "14GHz_18GHz",
    "12GHz_14GHz", "10GHz_12GHz", "8GHz_10GHz",
};

static const unsigned int admv1455_lo_doubler_values[] = {
    0xC0, 0x80, 0x08, 0x04, 0x1D, 0x1F,
};

typedef struct admv1455_enum {
	const char* const* items;
	const unsigned int* values;
	uint32_t count;
} Admv1455Enum;

#define ADMV1455_ENUM(_items, _values)                                         \
	{_items, _values, NO_OS_ARRAY_SIZE(_items)}

static const Admv1455Enum admv1455_enums[ADMV1455_CONTROL_COUNT] = {
    [ADMV1455_RF_BAND] = ADMV1455_ENUM(admv1455_rf_band_items, NULL),
    [ADMV1455_IF_SUBBAND] = ADMV1455_ENUM(admv1455_if_subband_items, NULL),
    [ADMV1455_LO_TRAP] =
	ADMV1455_ENUM(admv1455_lo_trap_items, admv1455_lo_trap_values),
    [ADMV1455_DIRECT_DSA1] = ADMV1455_ENUM(admv1455_dsa_4bit_items, NULL),
    [ADMV1455_DIRECT_DSA2] = ADMV1455_ENUM(admv1455_dsa_1bit_items, NULL),
    [ADMV1455_DIRECT_DSA3] = ADMV1455_ENUM(admv1455_dsa_4bit_items, NULL),
    [ADMV1455_DIRECT_DSA4] = ADMV1455_ENUM(admv1455_dsa_4bit_items, NULL),
    [ADMV1455_DIRECT_DSA5] = ADMV1455_ENUM(admv1455_dsa_4bit_items, NULL),
    [ADMV1455_BYPASS_DSA1] = ADMV1455_ENUM(admv1455_dsa_4bit_items, NULL),
    [ADMV1455_BYPASS_DSA2] = ADMV1455_ENUM(admv1455_dsa_1bit_items, NULL),
    [ADMV1455_BYPASS_DSA3] = ADMV1455_ENUM(admv1455_dsa_4bit_items, NULL),
    [ADMV1455_BYPASS_DSA4] = ADMV1455_ENUM(admv1455_dsa_4bit_items, NULL),
    [ADMV1455_BYPASS_DSA5] = ADMV1455_ENUM(admv1455_dsa_4bit_items, NULL),
    [ADMV1455_LO_SIDEBAND] = ADMV1455_ENUM(admv1455_lo_sideband_items, NULL),
    [ADMV1455_LO_BAND] = ADMV1455_ENUM(admv1455_lo_band_items, NULL),
    [ADMV1455_LO_X1_FILTER] =
	ADMV1455_ENUM(admv1455_x1_filter_items, admv1455_x1_filter_values),
    [ADMV1455_LO_X3_FILTER] =
	ADMV1455_ENUM(admv1455_x3_filter_items, admv1455_x3_filter_values),
    [ADMV1455_LO_X4_FILTER] =
	ADMV1455_ENUM(admv1455_x4_filter_items, admv1455_x4_filter_values),
    [ADMV1455_LO_DOUBLER] =
	ADMV1455_ENUM(admv1455_lo_doubler_items, admv1455_lo_doubler_values),
    [ADMV1455_DSAI] = ADMV1455_ENUM(admv1455_dsa_iq_items, NULL),
    [ADMV1455_DSAQ] = ADMV1455_ENUM(admv1455_dsa_iq_items, NULL),
};

#define ADMV1455_AVAILABLE NO_OS_BIT(8)
#define ADMV1455_TEMP_RAW 512
#define ADMV1455_POWER_RAW 513

static int admv1455_enum_parse(Admv1455IioDevAttrId control, const char* text)
{
	const Admv1455Enum* options = &admv1455_enums[control];
	uint32_t index;

	for (index = 0; index < options->count; index++)
		if (!strcmp(text, options->items[index]))
			return options->values ? (int)options->values[index]
					       : (int)index;
	return -EINVAL;
}

static const char* admv1455_enum_name(Admv1455IioDevAttrId control,
				      uint16_t value)
{
	const Admv1455Enum* options = &admv1455_enums[control];
	uint32_t index;

	for (index = 0; index < options->count; index++)
		if ((options->values ? options->values[index] : index) == value)
			return options->items[index];
	return NULL;
}

static bool admv1455_is_bool(Admv1455IioDevAttrId control)
{
	return control == ADMV1455_FILTER_LOAD_EN ||
	       control == ADMV1455_BYPASS_GAIN_TABLE_EN ||
	       control == ADMV1455_GAIN_TABLE_EN ||
	       control == ADMV1455_GAIN_LOAD_EN;
}

static int admv1455_iio_read(void* device, char* buf, uint32_t len,
			     const IioChInfo* channel, intptr_t priv)
{
	Admv1455IioDev* dev = device;
	Admv1455IioDevAttrId control = priv & ~ADMV1455_AVAILABLE;
	const Admv1455Enum* options;
	const char* name;
	uint16_t value;
	uint8_t raw = 0;
	uint8_t dsa1, dsa2, dsa3, dsa4, dsa5;
	uint8_t dsai, dsaq, lpf, hpf, phase_i, phase_q;
	bool enable;
	uint32_t index, used = 0;
	int ret;

	(void)channel;
	if (!dev || !dev->admv1455_dev || !buf || !len)
		return -EINVAL;
	if (priv == ADMV1455_TEMP_RAW || priv == ADMV1455_POWER_RAW) {
		ret = priv == ADMV1455_TEMP_RAW
			  ? admv1455_read_temperature(dev->admv1455_dev, &raw)
			  : admv1455_read_detector(dev->admv1455_dev, &raw);
		return ret ? ret : snprintf(buf, len, "%u", raw);
	}
	if ((unsigned int)control >= ADMV1455_CONTROL_COUNT)
		return -EINVAL;
	options = &admv1455_enums[control];
	if (priv & ADMV1455_AVAILABLE) {
		if (!options->count)
			return -EINVAL;
		for (index = 0; index < options->count; index++) {
			ret = snprintf(buf + used, len - used, "%s%s",
				       index ? " " : "", options->items[index]);
			if (ret < 0)
				return ret;
			if ((uint32_t)ret >= len - used)
				return -ENOSPC;
			used += ret;
		}
		return used;
	}
	switch (control) {
	case ADMV1455_RF_BAND:
		ret = admv1455_get_rf_band(dev->admv1455_dev, &raw);
		break;
	case ADMV1455_IF_SUBBAND:
		ret = admv1455_get_if_subband(dev->admv1455_dev, &raw);
		break;
	case ADMV1455_DIRECT_DSA1:
	case ADMV1455_DIRECT_DSA2:
	case ADMV1455_DIRECT_DSA3:
		ret =
		    admv1455_get_rf_dsa(dev->admv1455_dev, &dsa1, &dsa2, &dsa3);
		if (ret)
			return ret;
		raw = control == ADMV1455_DIRECT_DSA1	? dsa1
		      : control == ADMV1455_DIRECT_DSA2 ? dsa2
							: dsa3;
		break;
	case ADMV1455_DIRECT_DSA4:
	case ADMV1455_DIRECT_DSA5:
		ret = admv1455_get_if_dsa(dev->admv1455_dev, &dsa4, &dsa5);
		if (ret)
			return ret;
		raw = control == ADMV1455_DIRECT_DSA4 ? dsa4 : dsa5;
		break;
	case ADMV1455_BYPASS_DSA1:
	case ADMV1455_BYPASS_DSA2:
	case ADMV1455_BYPASS_DSA3:
		ret = admv1455_get_rf_dsa_bypass(dev->admv1455_dev, &dsa1,
						 &dsa2, &dsa3);
		if (ret)
			return ret;
		raw = control == ADMV1455_BYPASS_DSA1	? dsa1
		      : control == ADMV1455_BYPASS_DSA2 ? dsa2
							: dsa3;
		break;
	case ADMV1455_BYPASS_DSA4:
	case ADMV1455_BYPASS_DSA5:
		ret =
		    admv1455_get_if_dsa_bypass(dev->admv1455_dev, &dsa4, &dsa5);
		if (ret)
			return ret;
		raw = control == ADMV1455_BYPASS_DSA4 ? dsa4 : dsa5;
		break;
	case ADMV1455_DSAI:
	case ADMV1455_DSAQ:
		ret = admv1455_get_if_iq_dsa(dev->admv1455_dev, &dsai, &dsaq);
		if (ret)
			return ret;
		raw = control == ADMV1455_DSAI ? dsai : dsaq;
		break;
	case ADMV1455_DSA1_OFFSET:
	case ADMV1455_DSA2_OFFSET:
	case ADMV1455_DSA3_OFFSET:
		ret = admv1455_get_rf_dsa_offset(dev->admv1455_dev, &dsa1,
						 &dsa2, &dsa3);
		if (ret)
			return ret;
		raw = control == ADMV1455_DSA1_OFFSET	? dsa1
		      : control == ADMV1455_DSA2_OFFSET ? dsa2
							: dsa3;
		break;
	case ADMV1455_DSA4_OFFSET:
	case ADMV1455_DSA5_OFFSET:
		ret =
		    admv1455_get_if_dsa_offset(dev->admv1455_dev, &dsa4, &dsa5);
		if (ret)
			return ret;
		raw = control == ADMV1455_DSA4_OFFSET ? dsa4 : dsa5;
		break;
	case ADMV1455_LPF_VALUE:
	case ADMV1455_HPF_VALUE:
		ret = admv1455_get_rf_direct_filter(dev->admv1455_dev, &lpf,
						    &hpf);
		if (ret)
			return ret;
		raw = control == ADMV1455_LPF_VALUE ? lpf : hpf;
		break;
	case ADMV1455_LO_PHASE_I:
	case ADMV1455_LO_PHASE_Q:
		ret = admv1455_get_lo_phase(dev->admv1455_dev, &phase_i,
					    &phase_q);
		if (ret)
			return ret;
		raw = control == ADMV1455_LO_PHASE_I ? phase_i : phase_q;
		break;
	case ADMV1455_LO_SIDEBAND:
		ret = admv1455_get_mixer_sideband(dev->admv1455_dev, &raw);
		break;
	case ADMV1455_LO_BAND:
		ret = admv1455_get_lo_band(dev->admv1455_dev, &raw);
		break;
	case ADMV1455_LO_X1_FILTER:
		ret = admv1455_get_lo_x1_filter(dev->admv1455_dev, &raw);
		break;
	case ADMV1455_LO_X3_FILTER:
		ret = admv1455_get_lo_x3_filter(dev->admv1455_dev, &raw);
		break;
	case ADMV1455_LO_X4_FILTER:
		ret = admv1455_get_lo_x4_filter(dev->admv1455_dev, &raw);
		break;
	case ADMV1455_LO_TRAP:
		ret = admv1455_get_lo_trap(dev->admv1455_dev, &value);
		break;
	case ADMV1455_LO_DOUBLER:
		ret = admv1455_get_lo_dbl_band(dev->admv1455_dev, &raw);
		break;
	case ADMV1455_FILTER_LOAD_EN:
		ret = admv1455_get_filter_load_en(dev->admv1455_dev, &enable);
		break;
	case ADMV1455_BYPASS_GAIN_TABLE_EN:
		ret =
		    admv1455_get_gain_lut_bypass_en(dev->admv1455_dev, &enable);
		break;
	case ADMV1455_GAIN_TABLE_EN:
		ret = admv1455_get_gain_lut_en(dev->admv1455_dev, &enable);
		break;
	case ADMV1455_GAIN_LOAD_EN:
		ret = admv1455_get_gain_load_en(dev->admv1455_dev, &enable);
		break;
	case ADMV1455_FILTER_TABLE_SEL:
		ret = admv1455_get_filter_table_sel(dev->admv1455_dev, &raw);
		break;
	case ADMV1455_DIRECT_GPO_F:
		ret = admv1455_get_gpo_f_direct(dev->admv1455_dev, &raw);
		break;
	case ADMV1455_DIRECT_GPO_G:
		ret = admv1455_get_gpo_g_direct(dev->admv1455_dev, &raw);
		break;
	case ADMV1455_BYPASS_GPO_G:
		ret = admv1455_get_gpo_g_bypass(dev->admv1455_dev, &raw);
		break;
	case ADMV1455_GAIN_LUT_MODE:
		ret = admv1455_get_gain_lut_mode(dev->admv1455_dev, &raw);
		break;
	case ADMV1455_GAIN_SPI_POINTER:
		ret = admv1455_get_gain_spi_pointer(dev->admv1455_dev, &raw);
		break;
	case ADMV1455_GPO_F_OE:
		ret = admv1455_get_gpo_f_oe(dev->admv1455_dev, &raw);
		break;
	case ADMV1455_GPO_G_OE:
		ret = admv1455_get_gpo_g_oe(dev->admv1455_dev, &raw);
		break;
	default:
		return -EINVAL;
	}
	if (ret)
		return ret;
	if (admv1455_is_bool(control))
		return snprintf(buf, len, "%s", enable ? "true" : "false");
	if (control != ADMV1455_LO_TRAP)
		value = raw;
	if (options->count) {
		name = admv1455_enum_name(control, value);
		return name ? snprintf(buf, len, "%s", name) : -EINVAL;
	}
	if (control == ADMV1455_FILTER_TABLE_SEL)
		return snprintf(buf, len, "%c", value ? 'B' : 'A');
	return snprintf(buf, len, "%u", value);
}

static int admv1455_iio_write(void* device, char* buf, uint32_t len,
			      const IioChInfo* channel, intptr_t priv)
{
	Admv1455IioDev* dev = device;
	Admv1455IioDevAttrId control = priv;
	char text[64];
	char* end;
	unsigned long value;
	uint8_t dsa1, dsa2, dsa3, dsa4, dsa5;
	uint8_t dsai, dsaq, lpf, hpf, phase_i, phase_q;
	uint32_t size;
	int ret;

	(void)channel;
	if (!dev || !dev->admv1455_dev || !buf || !len || len >= sizeof(text) ||
	    (unsigned int)control >= ADMV1455_CONTROL_COUNT)
		return -EINVAL;
	memcpy(text, buf, len);
	text[len] = '\0';
	size = strlen(text);
	if (size && text[size - 1] == '\n')
		text[--size] = '\0';
	if (!size)
		return -EINVAL;
	if (admv1455_enums[control].count) {
		ret = admv1455_enum_parse(control, text);
		if (ret < 0)
			return ret;
		value = ret;
	} else if (admv1455_is_bool(control)) {
		if (!strcmp(text, "true") || !strcmp(text, "1") ||
		    !strcmp(text, "y") || !strcmp(text, "Y") ||
		    !strcmp(text, "on") || !strcmp(text, "ON"))
			value = 1;
		else if (!strcmp(text, "false") || !strcmp(text, "0") ||
			 !strcmp(text, "n") || !strcmp(text, "N") ||
			 !strcmp(text, "off") || !strcmp(text, "OFF"))
			value = 0;
		else
			return -EINVAL;
	} else if (control == ADMV1455_FILTER_TABLE_SEL) {
		if (strcmp(text, "A") && strcmp(text, "B"))
			return -EINVAL;
		value = text[0] == 'B';
	} else {
		if (text[0] == '-')
			return -EINVAL;
		errno = 0;
		value = strtoul(text, &end, 0);
		if (errno || *end || end == text || value > UINT16_MAX)
			return -EINVAL;
	}
	if (value > UINT8_MAX && control != ADMV1455_LO_TRAP)
		return -EINVAL;
	switch (control) {
	case ADMV1455_RF_BAND:
		ret = admv1455_set_rf_band(dev->admv1455_dev, value);
		break;
	case ADMV1455_IF_SUBBAND:
		ret = admv1455_set_if_subband(dev->admv1455_dev, value);
		break;
	case ADMV1455_DIRECT_DSA1:
	case ADMV1455_DIRECT_DSA2:
	case ADMV1455_DIRECT_DSA3:
		ret =
		    admv1455_get_rf_dsa(dev->admv1455_dev, &dsa1, &dsa2, &dsa3);
		if (ret)
			return ret;
		ret = admv1455_set_rf_dsa(
		    dev->admv1455_dev,
		    control == ADMV1455_DIRECT_DSA1 ? value : dsa1,
		    control == ADMV1455_DIRECT_DSA2 ? value : dsa2,
		    control == ADMV1455_DIRECT_DSA3 ? value : dsa3);
		break;
	case ADMV1455_DIRECT_DSA4:
	case ADMV1455_DIRECT_DSA5:
		ret = admv1455_get_if_dsa(dev->admv1455_dev, &dsa4, &dsa5);
		if (ret)
			return ret;
		ret = admv1455_set_if_dsa(
		    dev->admv1455_dev,
		    control == ADMV1455_DIRECT_DSA4 ? value : dsa4,
		    control == ADMV1455_DIRECT_DSA5 ? value : dsa5);
		break;
	case ADMV1455_BYPASS_DSA1:
	case ADMV1455_BYPASS_DSA2:
	case ADMV1455_BYPASS_DSA3:
		ret = admv1455_get_rf_dsa_bypass(dev->admv1455_dev, &dsa1,
						 &dsa2, &dsa3);
		if (ret)
			return ret;
		ret = admv1455_set_rf_dsa_bypass(
		    dev->admv1455_dev,
		    control == ADMV1455_BYPASS_DSA1 ? value : dsa1,
		    control == ADMV1455_BYPASS_DSA2 ? value : dsa2,
		    control == ADMV1455_BYPASS_DSA3 ? value : dsa3);
		break;
	case ADMV1455_BYPASS_DSA4:
	case ADMV1455_BYPASS_DSA5:
		ret =
		    admv1455_get_if_dsa_bypass(dev->admv1455_dev, &dsa4, &dsa5);
		if (ret)
			return ret;
		ret = admv1455_set_if_dsa_bypass(
		    dev->admv1455_dev,
		    control == ADMV1455_BYPASS_DSA4 ? value : dsa4,
		    control == ADMV1455_BYPASS_DSA5 ? value : dsa5);
		break;
	case ADMV1455_DSAI:
	case ADMV1455_DSAQ:
		ret = admv1455_get_if_iq_dsa(dev->admv1455_dev, &dsai, &dsaq);
		if (ret)
			return ret;
		ret = admv1455_set_if_iq_dsa(
		    dev->admv1455_dev, control == ADMV1455_DSAI ? value : dsai,
		    control == ADMV1455_DSAQ ? value : dsaq);
		break;
	case ADMV1455_DSA1_OFFSET:
	case ADMV1455_DSA2_OFFSET:
	case ADMV1455_DSA3_OFFSET:
		ret = admv1455_get_rf_dsa_offset(dev->admv1455_dev, &dsa1,
						 &dsa2, &dsa3);
		if (ret)
			return ret;
		ret = admv1455_set_rf_dsa_offset(
		    dev->admv1455_dev,
		    control == ADMV1455_DSA1_OFFSET ? value : dsa1,
		    control == ADMV1455_DSA2_OFFSET ? value : dsa2,
		    control == ADMV1455_DSA3_OFFSET ? value : dsa3);
		break;
	case ADMV1455_DSA4_OFFSET:
	case ADMV1455_DSA5_OFFSET:
		ret =
		    admv1455_get_if_dsa_offset(dev->admv1455_dev, &dsa4, &dsa5);
		if (ret)
			return ret;
		ret = admv1455_set_if_dsa_offset(
		    dev->admv1455_dev,
		    control == ADMV1455_DSA4_OFFSET ? value : dsa4,
		    control == ADMV1455_DSA5_OFFSET ? value : dsa5);
		break;
	case ADMV1455_LPF_VALUE:
	case ADMV1455_HPF_VALUE:
		ret = admv1455_get_rf_direct_filter(dev->admv1455_dev, &lpf,
						    &hpf);
		if (ret)
			return ret;
		ret = admv1455_set_rf_direct_filter(
		    dev->admv1455_dev,
		    control == ADMV1455_LPF_VALUE ? value : lpf,
		    control == ADMV1455_HPF_VALUE ? value : hpf);
		break;
	case ADMV1455_LO_PHASE_I:
	case ADMV1455_LO_PHASE_Q:
		ret = admv1455_get_lo_phase(dev->admv1455_dev, &phase_i,
					    &phase_q);
		if (ret)
			return ret;
		ret = admv1455_set_lo_phase(
		    dev->admv1455_dev,
		    control == ADMV1455_LO_PHASE_I ? value : phase_i,
		    control == ADMV1455_LO_PHASE_Q ? value : phase_q);
		break;
	case ADMV1455_LO_SIDEBAND:
		ret = admv1455_set_mixer_sideband(dev->admv1455_dev, value);
		break;
	case ADMV1455_LO_BAND:
		ret = admv1455_set_lo_band(dev->admv1455_dev, value);
		break;
	case ADMV1455_LO_X1_FILTER:
		ret = admv1455_set_lo_x1_filter(dev->admv1455_dev, value);
		break;
	case ADMV1455_LO_X3_FILTER:
		ret = admv1455_set_lo_x3_filter(dev->admv1455_dev, value);
		break;
	case ADMV1455_LO_X4_FILTER:
		ret = admv1455_set_lo_x4_filter(dev->admv1455_dev, value);
		break;
	case ADMV1455_LO_TRAP:
		ret = admv1455_set_lo_trap(dev->admv1455_dev, value);
		break;
	case ADMV1455_LO_DOUBLER:
		ret = admv1455_set_lo_dbl_band(dev->admv1455_dev, value);
		break;
	case ADMV1455_FILTER_LOAD_EN:
		ret = admv1455_set_filter_load_en(dev->admv1455_dev, value);
		break;
	case ADMV1455_BYPASS_GAIN_TABLE_EN:
		ret = admv1455_set_gain_lut_bypass_en(dev->admv1455_dev, value);
		break;
	case ADMV1455_GAIN_TABLE_EN:
		ret = admv1455_set_gain_lut_en(dev->admv1455_dev, value);
		break;
	case ADMV1455_GAIN_LOAD_EN:
		ret = admv1455_set_gain_load_en(dev->admv1455_dev, value);
		break;
	case ADMV1455_FILTER_TABLE_SEL:
		ret = admv1455_set_filter_table_sel(dev->admv1455_dev, value);
		break;
	case ADMV1455_DIRECT_GPO_F:
		ret = admv1455_set_gpo_f_direct(dev->admv1455_dev, value);
		break;
	case ADMV1455_DIRECT_GPO_G:
		ret = admv1455_set_gpo_g_direct(dev->admv1455_dev, value);
		break;
	case ADMV1455_BYPASS_GPO_G:
		ret = admv1455_set_gpo_g_bypass(dev->admv1455_dev, value);
		break;
	case ADMV1455_GAIN_LUT_MODE:
		ret = admv1455_set_gain_lut_mode(dev->admv1455_dev, value);
		break;
	case ADMV1455_GAIN_SPI_POINTER:
		ret = admv1455_set_gain_spi_pointer(dev->admv1455_dev, value);
		break;
	case ADMV1455_GPO_F_OE:
		ret = admv1455_set_gpo_f_oe(dev->admv1455_dev, value);
		break;
	case ADMV1455_GPO_G_OE:
		ret = admv1455_set_gpo_g_oe(dev->admv1455_dev, value);
		break;
	default:
		return -EINVAL;
	}
	return ret ? ret : (int)len;
}

static int admv1455_iio_read_reg(void* device, uint32_t reg, uint32_t* value)
{
	Admv1455IioDev* dev = device;
	uint8_t raw;
	int ret;

	if (!dev || !value || reg > 0xFFF)
		return -EINVAL;
	ret = admv1455_spi_read(dev->admv1455_dev, reg, &raw);
	if (!ret)
		*value = raw;
	return ret;
}

static int admv1455_iio_write_reg(void* device, uint32_t reg, uint32_t value)
{
	Admv1455IioDev* dev = device;

	if (!dev || reg > 0xFFF || value > UINT8_MAX)
		return -EINVAL;
	return admv1455_spi_write(dev->admv1455_dev, reg, value);
}

static int admv1455_filter_lut_parse(Admv1455Dev* dev, Admv1455Lut table,
				     const char* line)
{
	static const unsigned int limits[] = {
	    1,	15, 15, 15, 3, 15, 63, 3,  15, 15, 15,
	    15, 1,  4,	6,  3, 4,  31, 31, 1,  5,
	};
	static const uint8_t subbands[] = {3, 1, 0, 0};
	unsigned int fields[21];
	unsigned int index, field;
	uint8_t data[ADMV1455_FILTER_LUT_ENTRY_SIZE];
	unsigned int trap;
	char label;
	int consumed = 0;
	int ret;

	ret = sscanf(line,
		     "LUT%c_%u RF (%u,%u,%u,%u,%u,%u,%u) "
		     "IF (%u,%u,%u,%u,%u) LO (%u,%u,%u,%u,%u,%u,%u,%u,%u) %n",
		     &label, &index, &fields[0], &fields[1], &fields[2],
		     &fields[3], &fields[4], &fields[5], &fields[6], &fields[7],
		     &fields[8], &fields[9], &fields[10], &fields[11],
		     &fields[12], &fields[13], &fields[14], &fields[15],
		     &fields[16], &fields[17], &fields[18], &fields[19],
		     &fields[20], &consumed);
	if (ret != 23 || !consumed || line[consumed] ||
	    label != (table == ADMV1455_FILTER_LUTA ? 'A' : 'B') ||
	    index >= ADMV1455_FILTER_LUT_NUM_ENTRIES)
		return -EINVAL;
	for (field = 0; field < NO_OS_ARRAY_SIZE(fields); field++)
		if (fields[field] > limits[field])
			return -EINVAL;

	trap = admv1455_lo_trap_values[fields[16]];
	data[0] = (fields[0] << 7) | (subbands[fields[7]] << 5) |
		  admv1455_x1_filter_values[fields[13]];
	data[1] = (fields[12] << 7) | (fields[0] ? 0x60 : 0) |
		  admv1455_x3_filter_values[fields[14]];
	data[2] = (fields[1] << 4) | admv1455_x4_filter_values[fields[15]];
	data[3] = trap;
	data[4] = (fields[2] << 4) | (fields[7] ? 0 : 0x0C) | (trap >> 8);
	data[5] = fields[6];
	data[6] = (fields[4] << 4) | fields[3];
	data[7] = (fields[8] << 4) | fields[5];
	data[8] = fields[9];
	data[9] = fields[18];
	data[10] = (fields[19] << 5) | fields[17];
	data[11] = (fields[11] << 4) | fields[10];
	data[12] = admv1455_lo_doubler_values[fields[20]];
	return admv1455_write_lut_entry(dev, table, index, data);
}

static int admv1455_gain_lut_parse(Admv1455Dev* dev, const char* line)
{
	static const Admv1455IioDevAttrId controls[] = {
	    ADMV1455_DIRECT_DSA1, ADMV1455_DIRECT_DSA2, ADMV1455_DIRECT_DSA3,
	    ADMV1455_DIRECT_DSA4, ADMV1455_DIRECT_DSA5,
	};
	char names[5][32];
	int gains[5];
	uint8_t data[ADMV1455_GAIN_LUT_ENTRY_SIZE];
	unsigned int index, gpo, field;
	int consumed = 0;
	int ret;

	ret = sscanf(line,
		     "GAIN_%u RF (%31[^,],%31[^,],%31[^)]) "
		     "IF (%31[^,],%31[^)]) %u %n",
		     &index, names[0], names[1], names[2], names[3], names[4],
		     &gpo, &consumed);
	if (ret != 7 || !consumed || line[consumed] ||
	    index >= ADMV1455_GAIN_LUT_NUM_ENTRIES || gpo > 31)
		return -EINVAL;
	for (field = 0; field < NO_OS_ARRAY_SIZE(gains); field++) {
		gains[field] =
		    admv1455_enum_parse(controls[field], names[field]);
		if (gains[field] < 0)
			return gains[field];
	}
	data[0] = gains[0] | (gains[1] << 4);
	data[1] = gains[2] | (gains[3] << 4);
	data[2] = gains[4];
	data[3] = gpo;
	return admv1455_write_lut_entry(dev, ADMV1455_GAIN_LUT, index, data);
}

static int admv1455_iio_write_lut(void* device, char* buf, uint32_t len,
				  const IioChInfo* channel, intptr_t priv)
{
	Admv1455IioDev* dev = device;
	char* copy;
	char* line;
	char* next;
	int ret = 0;

	(void)channel;
	if (!dev || !dev->admv1455_dev || !buf || !len ||
	    len > ADMV1455_FILTER_LUT_CONFIG_SIZE ||
	    (unsigned int)priv > ADMV1455_GAIN_LUT || memchr(buf, '\0', len))
		return -EINVAL;
	copy = no_os_calloc(len + 1, 1);
	if (!copy)
		return -ENOMEM;
	memcpy(copy, buf, len);
	for (line = copy; line; line = next) {
		next = strchr(line, '\n');
		if (next)
			*next++ = '\0';
		if (!*line || *line == '#')
			continue;
		ret = priv == ADMV1455_GAIN_LUT
			  ? admv1455_gain_lut_parse(dev->admv1455_dev, line)
			  : admv1455_filter_lut_parse(dev->admv1455_dev, priv,
						      line);
		if (ret)
			break;
	}
	no_os_free(copy);
	return ret ? ret : (int)len;
}

static int admv1455_enum_index(Admv1455IioDevAttrId control, uint16_t value)
{
	const Admv1455Enum* options = &admv1455_enums[control];
	uint32_t index;

	for (index = 0; index < options->count; index++)
		if ((options->values ? options->values[index] : index) == value)
			return index;
	return 0;
}

static int admv1455_filter_lut_format(Admv1455Dev* dev, Admv1455Lut table,
				      uint8_t index, char* buf, uint32_t len)
{
	uint8_t data[ADMV1455_FILTER_LUT_ENTRY_SIZE];
	uint8_t fine;
	unsigned int subband = 2;
	int ret;

	ret = admv1455_read_lut_entry(dev, table, index, data);
	if (ret)
		return ret;
	if ((data[0] & 0x60) == 0x60 && (data[4] & 0x0C) == 0x0C)
		subband = 0;
	else if ((data[0] & 0x60) == 0x20 && !(data[4] & 0x0C))
		subband = 1;
	else if (!(data[0] & 0x60) && !(data[4] & 0x0C)) {
		ret = admv1455_spi_read(dev, ADMV1455_REG_IF_BPF_FINE, &fine);
		if (ret)
			return ret;
		if ((fine & ADMV1455_IF_BPF_FINE_MSK) == 0x10)
			subband = 3;
	}
	return snprintf(
	    buf, len,
	    "LUT%c_%u RF (%u,%u,%u,%u,%u,%u,%u) "
	    "IF (%u,%u,%u,%u,%u) LO (%u,%d,%d,%d,%d,%u,%u,%u,%d)\n",
	    table == ADMV1455_FILTER_LUTA ? 'A' : 'B', index, data[0] >> 7,
	    data[2] >> 4, data[4] >> 4, data[6] & 15, (data[6] >> 4) & 3,
	    data[7] & 15, data[5] & 63, subband, data[7] >> 4, data[8] & 15,
	    data[11] & 15, data[11] >> 4, data[1] >> 7,
	    admv1455_enum_index(ADMV1455_LO_X1_FILTER, data[0] & 31),
	    admv1455_enum_index(ADMV1455_LO_X3_FILTER, data[1] & 31),
	    admv1455_enum_index(ADMV1455_LO_X4_FILTER, data[2] & 15),
	    admv1455_enum_index(ADMV1455_LO_TRAP,
				data[3] | ((data[4] & 3) << 8)),
	    data[10] & 31, data[9] & 31, (data[10] >> 5) & 1,
	    admv1455_enum_index(ADMV1455_LO_DOUBLER, data[12]));
}

static int admv1455_iio_read_lut(void* device, char* buf, uint32_t len,
				 const IioChInfo* channel, intptr_t priv)
{
	Admv1455IioDev* dev = device;
	uint8_t data[ADMV1455_GAIN_LUT_ENTRY_SIZE];
	uint32_t index, count, used = 0;
	int ret;

	(void)channel;
	if (!dev || !dev->admv1455_dev || !buf || !len ||
	    (unsigned int)priv > ADMV1455_GAIN_LUT)
		return -EINVAL;
	count = priv == ADMV1455_GAIN_LUT ? ADMV1455_GAIN_LUT_NUM_ENTRIES
					  : ADMV1455_FILTER_LUT_NUM_ENTRIES;
	for (index = 0; index < count; index++) {
		if (priv == ADMV1455_GAIN_LUT) {
			ret = admv1455_read_lut_entry(dev->admv1455_dev, priv,
						      index, data);
			if (ret)
				return ret;
			ret = snprintf(
			    buf + used, len - used,
			    "GAIN_%u RF (%s,%s,%s) IF (%s,%s) %u\n", index,
			    admv1455_dsa_4bit_items[data[0] & 15],
			    admv1455_dsa_1bit_items[(data[0] >> 4) & 1],
			    admv1455_dsa_4bit_items[data[1] & 15],
			    admv1455_dsa_4bit_items[data[1] >> 4],
			    admv1455_dsa_4bit_items[data[2] & 15],
			    data[3] & 31);
		} else {
			ret = admv1455_filter_lut_format(
			    dev->admv1455_dev, priv, index, buf + used,
			    len - used);
		}
		if (ret < 0)
			return ret;
		if ((uint32_t)ret >= len - used)
			return -ENOSPC;
		used += ret;
	}
	return used;
}

#define ADMV1455_ATTR(_name, _control)                                         \
	{.name = _name,                                                        \
	 .priv = _control,                                                     \
	 .show = admv1455_iio_read,                                            \
	 .store = admv1455_iio_write}
#define ADMV1455_ENUM_ATTR(_name, _control)                                    \
	ADMV1455_ATTR(_name, _control),                                        \
	{                                                                      \
		.name = _name "_available",                                    \
		.priv = _control | ADMV1455_AVAILABLE,                         \
		.show = admv1455_iio_read                                      \
	}

static IioAttribute admv1455_rf_attrs[] = {
    ADMV1455_ENUM_ATTR("band", ADMV1455_RF_BAND),
    ADMV1455_ATTR("direct_lpf_val", ADMV1455_LPF_VALUE),
    ADMV1455_ATTR("direct_hpf_val", ADMV1455_HPF_VALUE),
    ADMV1455_ENUM_ATTR("direct_dsa1_gain", ADMV1455_DIRECT_DSA1),
    ADMV1455_ENUM_ATTR("direct_dsa2_gain", ADMV1455_DIRECT_DSA2),
    ADMV1455_ENUM_ATTR("direct_dsa3_gain", ADMV1455_DIRECT_DSA3),
    ADMV1455_ATTR("direct_dsa1_offset", ADMV1455_DSA1_OFFSET),
    ADMV1455_ATTR("direct_dsa2_offset", ADMV1455_DSA2_OFFSET),
    ADMV1455_ATTR("direct_dsa3_offset", ADMV1455_DSA3_OFFSET),
    ADMV1455_ENUM_ATTR("bypass_dsa1_gain", ADMV1455_BYPASS_DSA1),
    ADMV1455_ENUM_ATTR("bypass_dsa2_gain", ADMV1455_BYPASS_DSA2),
    ADMV1455_ENUM_ATTR("bypass_dsa3_gain", ADMV1455_BYPASS_DSA3),
    END_ATTRIBUTES_ARRAY};

static IioAttribute admv1455_if_attrs[] = {
    ADMV1455_ENUM_ATTR("direct_dsa4_gain", ADMV1455_DIRECT_DSA4),
    ADMV1455_ENUM_ATTR("direct_dsa5_gain", ADMV1455_DIRECT_DSA5),
    ADMV1455_ATTR("direct_dsa4_offset", ADMV1455_DSA4_OFFSET),
    ADMV1455_ATTR("direct_dsa5_offset", ADMV1455_DSA5_OFFSET),
    ADMV1455_ENUM_ATTR("bypass_dsa4_gain", ADMV1455_BYPASS_DSA4),
    ADMV1455_ENUM_ATTR("bypass_dsa5_gain", ADMV1455_BYPASS_DSA5),
    ADMV1455_ENUM_ATTR("dsai_0p1db", ADMV1455_DSAI),
    ADMV1455_ENUM_ATTR("dsaq_0p1db", ADMV1455_DSAQ),
    ADMV1455_ENUM_ATTR("subband", ADMV1455_IF_SUBBAND),
    END_ATTRIBUTES_ARRAY};

static IioAttribute admv1455_lo_attrs[] = {
    ADMV1455_ENUM_ATTR("sideband", ADMV1455_LO_SIDEBAND),
    ADMV1455_ENUM_ATTR("x1_filter", ADMV1455_LO_X1_FILTER),
    ADMV1455_ENUM_ATTR("x3_filter", ADMV1455_LO_X3_FILTER),
    ADMV1455_ENUM_ATTR("x4_filter", ADMV1455_LO_X4_FILTER),
    ADMV1455_ENUM_ATTR("trap_filter", ADMV1455_LO_TRAP),
    ADMV1455_ATTR("direct_i_phase_val", ADMV1455_LO_PHASE_I),
    ADMV1455_ATTR("direct_q_phase_val", ADMV1455_LO_PHASE_Q),
    ADMV1455_ENUM_ATTR("band", ADMV1455_LO_BAND),
    ADMV1455_ENUM_ATTR("doubler_band", ADMV1455_LO_DOUBLER),
    END_ATTRIBUTES_ARRAY};

static IioAttribute admv1455_temp_attrs[] = {
    {.name = "raw", .priv = ADMV1455_TEMP_RAW, .show = admv1455_iio_read},
    END_ATTRIBUTES_ARRAY};

static IioAttribute admv1455_power_attrs[] = {
    {.name = "raw", .priv = ADMV1455_POWER_RAW, .show = admv1455_iio_read},
    END_ATTRIBUTES_ARRAY};

static IioAttribute admv1455_attrs[] = {
    ADMV1455_ATTR("filter_load_en", ADMV1455_FILTER_LOAD_EN),
    ADMV1455_ATTR("filter_table_sel", ADMV1455_FILTER_TABLE_SEL),
    ADMV1455_ATTR("direct_gpo_f", ADMV1455_DIRECT_GPO_F),
    ADMV1455_ATTR("bypass_gain_table_en", ADMV1455_BYPASS_GAIN_TABLE_EN),
    ADMV1455_ATTR("bypass_gpo_g", ADMV1455_BYPASS_GPO_G),
    ADMV1455_ATTR("gain_table_en", ADMV1455_GAIN_TABLE_EN),
    ADMV1455_ATTR("gain_load_en", ADMV1455_GAIN_LOAD_EN),
    ADMV1455_ATTR("direct_gpo_g", ADMV1455_DIRECT_GPO_G),
    ADMV1455_ATTR("gain_lut_mode", ADMV1455_GAIN_LUT_MODE),
    ADMV1455_ATTR("gain_spi_pointer", ADMV1455_GAIN_SPI_POINTER),
    ADMV1455_ATTR("gpo_f_oe", ADMV1455_GPO_F_OE),
    ADMV1455_ATTR("gpo_g_oe", ADMV1455_GPO_G_OE),
    {.name = "filter_table_config_A",
     .priv = ADMV1455_FILTER_LUTA,
     .show = admv1455_iio_read_lut,
     .store = admv1455_iio_write_lut},
    {.name = "filter_table_config_B",
     .priv = ADMV1455_FILTER_LUTB,
     .show = admv1455_iio_read_lut,
     .store = admv1455_iio_write_lut},
    {.name = "gain_table_config",
     .priv = ADMV1455_GAIN_LUT,
     .show = admv1455_iio_read_lut,
     .store = admv1455_iio_write_lut},
    END_ATTRIBUTES_ARRAY};

static IioChannel admv1455_channels[] = {
    {.name = "rf",
     .ch_type = IIO_ALTVOLTAGE,
     .channel = 0,
     .indexed = true,
     .attributes = admv1455_rf_attrs},
    {.name = "if",
     .ch_type = IIO_ALTVOLTAGE,
     .channel = 1,
     .indexed = true,
     .ch_out = true,
     .attributes = admv1455_if_attrs},
    {.name = "lo",
     .ch_type = IIO_ALTVOLTAGE,
     .channel = 2,
     .indexed = true,
     .attributes = admv1455_lo_attrs},
    {.ch_type = IIO_TEMP,
     .channel = 0,
     .indexed = true,
     .attributes = admv1455_temp_attrs},
    {.ch_type = IIO_POWER,
     .channel = 0,
     .indexed = true,
     .attributes = admv1455_power_attrs},
};

static IioDevice admv1455_iio_dev = {
    .num_ch = NO_OS_ARRAY_SIZE(admv1455_channels),
    .channels = admv1455_channels,
    .attributes = admv1455_attrs,
    .debug_reg_read = admv1455_iio_read_reg,
    .debug_reg_write = admv1455_iio_write_reg,
};

int admv1455_iio_init(Admv1455IioDev** device,
		      const Admv1455IioDevInitParam* param)
{
	Admv1455IioDev* dev;
	int ret;

	if (!device || !param || !param->admv1455_dev_init)
		return -EINVAL;
	dev = no_os_calloc(1, sizeof(*dev));
	if (!dev)
		return -ENOMEM;
	ret = admv1455_init(&dev->admv1455_dev, param->admv1455_dev_init);
	if (ret) {
		no_os_free(dev);
		return ret;
	}
	dev->iio_dev = &admv1455_iio_dev;
	*device = dev;
	return 0;
}

int admv1455_iio_remove(Admv1455IioDev* dev)
{
	int ret;

	if (!dev)
		return -EINVAL;
	ret = admv1455_remove(dev->admv1455_dev);
	no_os_free(dev);
	return ret;
}