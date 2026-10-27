/***************************************************************************//**
 *   @file   admv1455.c
 *   @brief  Implementation of admv1455 Driver.
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
#include "admv1455.h"
#include "no_os_alloc.h"
#include "no_os_delay.h"
#include "no_os_error.h"
#include "no_os_print_log.h"

static const uint8_t admv1455_if_subband_values[] = {3, 1, 0, 0};
static const uint8_t admv1455_if_fine_values[] = {0x1F, 0x1D, 0x12, 0x10};

int admv1455_spi_read(Admv1455Dev* dev, uint16_t reg, uint8_t* value)
{
	uint8_t frame[3];
	int ret;

	if (!dev || !value || reg > 0xFFF)
		return -EINVAL;

	frame[0] = 0x80 | (dev->chip_addr << 4) | (reg >> 8);
	frame[1] = reg;
	frame[2] = 0;
	ret = no_os_spi_write_and_read(dev->spi_desc, frame, sizeof(frame));
	if (ret)
		return ret;
	*value = frame[2];
	return 0;
}

int admv1455_spi_write(Admv1455Dev* dev, uint16_t reg, uint8_t value)
{
	uint8_t frame[3];

	if (!dev || reg > 0xFFF)
		return -EINVAL;

	frame[0] = (dev->chip_addr << 4) | (reg >> 8);
	frame[1] = reg;
	frame[2] = value;
	return no_os_spi_write_and_read(dev->spi_desc, frame, sizeof(frame));
}

int admv1455_spi_update_bits(Admv1455Dev* dev, uint16_t reg, uint8_t mask,
			     uint8_t value)
{
	uint8_t current;
	int ret;

	ret = admv1455_spi_read(dev, reg, &current);
	if (ret)
		return ret;
	return admv1455_spi_write(dev, reg, (current & ~mask) | (value & mask));
}

int admv1455_get_rf_band(Admv1455Dev* dev, uint8_t* band)
{
	uint8_t data;
	int ret;

	if (!band)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_0x800, &data);
	if (ret)
		return ret;
	*band = no_os_field_get(ADMV1455_DIRECT_RF_BAND_MSK, data);
	return 0;
}

int admv1455_set_rf_band(Admv1455Dev* dev, uint8_t band)
{
	int ret;

	if (band > 1)
		return -EINVAL;
	ret = admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_0x800, ADMV1455_DIRECT_RF_BAND_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_RF_BAND_MSK, band));
	if (ret)
		return ret;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_0x801, ADMV1455_DIRECT_RF_SUBBAND_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_RF_SUBBAND_MSK, band ? 3 : 0));
}

int admv1455_get_mixer_sideband(Admv1455Dev* dev, uint8_t* sideband)
{
	uint8_t data;
	int ret;

	if (!sideband)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_0x801, &data);
	if (ret)
		return ret;
	*sideband = no_os_field_get(ADMV1455_DIRECT_SIDEBAND_MSK, data);
	return 0;
}

int admv1455_set_mixer_sideband(Admv1455Dev* dev, uint8_t sideband)
{
	if (sideband > 1)
		return -EINVAL;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_0x801, ADMV1455_DIRECT_SIDEBAND_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_SIDEBAND_MSK, sideband));
}

int admv1455_get_if_subband(Admv1455Dev* dev, uint8_t* subband)
{
	uint8_t data, fine;
	uint32_t index;
	int ret;

	if (!subband)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_0x800, &data);
	if (ret)
		return ret;
	ret = admv1455_spi_read(dev, ADMV1455_REG_IF_BPF_FINE, &fine);
	if (ret)
		return ret;
	for (index = 0; index < NO_OS_ARRAY_SIZE(admv1455_if_fine_values);
	     index++) {
		if (no_os_field_get(ADMV1455_DIRECT_IF_SUBBAND_MSK, data) ==
			admv1455_if_subband_values[index] &&
		    no_os_field_get(ADMV1455_IF_BPF_FINE_MSK, fine) ==
			admv1455_if_fine_values[index]) {
			*subband = index;
			return 0;
		}
	}
	return -EINVAL;
}

int admv1455_set_if_subband(Admv1455Dev* dev, uint8_t subband)
{
	int ret;

	if (subband >= NO_OS_ARRAY_SIZE(admv1455_if_subband_values))
		return -EINVAL;
	ret = admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_0x800, ADMV1455_DIRECT_IF_SUBBAND_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_IF_SUBBAND_MSK,
			     admv1455_if_subband_values[subband]));
	if (ret)
		return ret;
	ret = admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_0x804, ADMV1455_DIRECT_BPF_COARSE_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_BPF_COARSE_MSK, subband ? 0 : 3));
	if (ret)
		return ret;
	ret = admv1455_spi_update_bits(dev, ADMV1455_REG_IF_RES_CAL,
				       ADMV1455_IF_RES_CAL_MSK,
				       subband ? 0xE : 0xC);
	if (ret)
		return ret;
	return admv1455_spi_update_bits(dev, ADMV1455_REG_IF_BPF_FINE,
					ADMV1455_IF_BPF_FINE_MSK,
					admv1455_if_fine_values[subband]);
}

int admv1455_get_rf_dsa(Admv1455Dev* dev, uint8_t* dsa1, uint8_t* dsa2,
			uint8_t* dsa3)
{
	uint8_t data;
	int ret;

	if (!dsa1 || !dsa2 || !dsa3)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_DSA1_DSA2, &data);
	if (ret)
		return ret;
	*dsa1 = no_os_field_get(ADMV1455_DSA1_MSK, data);
	*dsa2 = no_os_field_get(ADMV1455_DSA2_MSK, data);
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_DSA3_DSA4, &data);
	if (ret)
		return ret;
	*dsa3 = no_os_field_get(ADMV1455_DSA3_MSK, data);
	return 0;
}

int admv1455_set_rf_dsa(Admv1455Dev* dev, uint8_t dsa1, uint8_t dsa2,
			uint8_t dsa3)
{
	int ret;

	if (dsa1 > 15 || dsa2 > 1 || dsa3 > 15)
		return -EINVAL;
	ret = admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_DSA1_DSA2,
	    ADMV1455_DSA1_MSK | ADMV1455_DSA2_MSK,
	    no_os_field_prep(ADMV1455_DSA1_MSK, dsa1) |
		no_os_field_prep(ADMV1455_DSA2_MSK, dsa2));
	if (ret)
		return ret;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_DSA3_DSA4, ADMV1455_DSA3_MSK,
	    no_os_field_prep(ADMV1455_DSA3_MSK, dsa3));
}

int admv1455_get_if_dsa(Admv1455Dev* dev, uint8_t* dsa4, uint8_t* dsa5)
{
	uint8_t data;
	int ret;

	if (!dsa4 || !dsa5)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_DSA3_DSA4, &data);
	if (ret)
		return ret;
	*dsa4 = no_os_field_get(ADMV1455_DSA4_MSK, data);
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_DSA5, &data);
	if (ret)
		return ret;
	*dsa5 = no_os_field_get(ADMV1455_DSA5_MSK, data);
	return 0;
}

int admv1455_set_if_dsa(Admv1455Dev* dev, uint8_t dsa4, uint8_t dsa5)
{
	int ret;

	if (dsa4 > 15 || dsa5 > 15)
		return -EINVAL;
	ret = admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_DSA3_DSA4, ADMV1455_DSA4_MSK,
	    no_os_field_prep(ADMV1455_DSA4_MSK, dsa4));
	if (ret)
		return ret;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_DSA5, ADMV1455_DSA5_MSK,
	    no_os_field_prep(ADMV1455_DSA5_MSK, dsa5));
}

int admv1455_get_rf_dsa_bypass(Admv1455Dev* dev, uint8_t* dsa1, uint8_t* dsa2,
			       uint8_t* dsa3)
{
	uint8_t data;
	int ret;

	if (!dsa1 || !dsa2 || !dsa3)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_BYPASS_DSA1_DSA2, &data);
	if (ret)
		return ret;
	*dsa1 = no_os_field_get(ADMV1455_DSA1_MSK, data);
	*dsa2 = no_os_field_get(ADMV1455_DSA2_MSK, data);
	ret = admv1455_spi_read(dev, ADMV1455_REG_BYPASS_DSA3_DSA4, &data);
	if (ret)
		return ret;
	*dsa3 = no_os_field_get(ADMV1455_DSA3_MSK, data);
	return 0;
}

int admv1455_set_rf_dsa_bypass(Admv1455Dev* dev, uint8_t dsa1, uint8_t dsa2,
			       uint8_t dsa3)
{
	int ret;

	if (dsa1 > 15 || dsa2 > 1 || dsa3 > 15)
		return -EINVAL;
	ret = admv1455_spi_update_bits(
	    dev, ADMV1455_REG_BYPASS_DSA1_DSA2,
	    ADMV1455_DSA1_MSK | ADMV1455_DSA2_MSK,
	    no_os_field_prep(ADMV1455_DSA1_MSK, dsa1) |
		no_os_field_prep(ADMV1455_DSA2_MSK, dsa2));
	if (ret)
		return ret;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_BYPASS_DSA3_DSA4, ADMV1455_DSA3_MSK,
	    no_os_field_prep(ADMV1455_DSA3_MSK, dsa3));
}

int admv1455_get_if_dsa_bypass(Admv1455Dev* dev, uint8_t* dsa4, uint8_t* dsa5)
{
	uint8_t data;
	int ret;

	if (!dsa4 || !dsa5)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_BYPASS_DSA3_DSA4, &data);
	if (ret)
		return ret;
	*dsa4 = no_os_field_get(ADMV1455_DSA4_MSK, data);
	ret = admv1455_spi_read(dev, ADMV1455_REG_BYPASS_DSA5, &data);
	if (ret)
		return ret;
	*dsa5 = no_os_field_get(ADMV1455_DSA5_MSK, data);
	return 0;
}

int admv1455_set_if_dsa_bypass(Admv1455Dev* dev, uint8_t dsa4, uint8_t dsa5)
{
	int ret;

	if (dsa4 > 15 || dsa5 > 15)
		return -EINVAL;
	ret = admv1455_spi_update_bits(
	    dev, ADMV1455_REG_BYPASS_DSA3_DSA4, ADMV1455_DSA4_MSK,
	    no_os_field_prep(ADMV1455_DSA4_MSK, dsa4));
	if (ret)
		return ret;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_BYPASS_DSA5, ADMV1455_DSA5_MSK,
	    no_os_field_prep(ADMV1455_DSA5_MSK, dsa5));
}

int admv1455_get_if_iq_dsa(Admv1455Dev* dev, uint8_t* dsai, uint8_t* dsaq)
{
	uint8_t data;
	int ret;

	if (!dsai || !dsaq)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_DSA_IQ, &data);
	if (ret)
		return ret;
	*dsai = no_os_field_get(ADMV1455_DIRECT_DSAI_MSK, data);
	*dsaq = no_os_field_get(ADMV1455_DIRECT_DSAQ_MSK, data);
	return 0;
}

int admv1455_set_if_iq_dsa(Admv1455Dev* dev, uint8_t dsai, uint8_t dsaq)
{
	if (dsai > 15 || dsaq > 15)
		return -EINVAL;
	return admv1455_spi_write(
	    dev, ADMV1455_REG_DIRECT_DSA_IQ,
	    no_os_field_prep(ADMV1455_DIRECT_DSAI_MSK, dsai) |
		no_os_field_prep(ADMV1455_DIRECT_DSAQ_MSK, dsaq));
}

int admv1455_get_rf_dsa_offset(Admv1455Dev* dev, uint8_t* dsa1, uint8_t* dsa2,
			       uint8_t* dsa3)
{
	uint8_t data;
	int ret;

	if (!dsa1 || !dsa2 || !dsa3)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_DSA_OFF_12, &data);
	if (ret)
		return ret;
	*dsa1 = no_os_field_get(ADMV1455_DIRECT_DSA1_OFF_MSK, data);
	*dsa2 = no_os_field_get(ADMV1455_DIRECT_DSA2_OFF_MSK, data);
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_DSA_OFF_34, &data);
	if (ret)
		return ret;
	*dsa3 = no_os_field_get(ADMV1455_DIRECT_DSA3_OFF_MSK, data);
	return 0;
}

int admv1455_set_rf_dsa_offset(Admv1455Dev* dev, uint8_t dsa1, uint8_t dsa2,
			       uint8_t dsa3)
{
	int ret;

	if (dsa1 > 15 || dsa2 > 3 || dsa3 > 15)
		return -EINVAL;
	ret = admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_DSA_OFF_12,
	    ADMV1455_DIRECT_DSA1_OFF_MSK | ADMV1455_DIRECT_DSA2_OFF_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_DSA1_OFF_MSK, dsa1) |
		no_os_field_prep(ADMV1455_DIRECT_DSA2_OFF_MSK, dsa2));
	if (ret)
		return ret;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_DSA_OFF_34, ADMV1455_DIRECT_DSA3_OFF_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_DSA3_OFF_MSK, dsa3));
}

int admv1455_get_if_dsa_offset(Admv1455Dev* dev, uint8_t* dsa4, uint8_t* dsa5)
{
	uint8_t data;
	int ret;

	if (!dsa4 || !dsa5)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_DSA_OFF_34, &data);
	if (ret)
		return ret;
	*dsa4 = no_os_field_get(ADMV1455_DIRECT_DSA4_OFF_MSK, data);
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_DSA_OFF_5, &data);
	if (ret)
		return ret;
	*dsa5 = no_os_field_get(ADMV1455_DIRECT_DSA5_OFF_MSK, data);
	return 0;
}

int admv1455_set_if_dsa_offset(Admv1455Dev* dev, uint8_t dsa4, uint8_t dsa5)
{
	int ret;

	if (dsa4 > 15 || dsa5 > 15)
		return -EINVAL;
	ret = admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_DSA_OFF_34, ADMV1455_DIRECT_DSA4_OFF_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_DSA4_OFF_MSK, dsa4));
	if (ret)
		return ret;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_DSA_OFF_5, ADMV1455_DIRECT_DSA5_OFF_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_DSA5_OFF_MSK, dsa5));
}

int admv1455_get_rf_direct_filter(Admv1455Dev* dev, uint8_t* lpf, uint8_t* hpf)
{
	uint8_t data;
	int ret;

	if (!lpf || !hpf)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_0x802, &data);
	if (ret)
		return ret;
	*lpf = no_os_field_get(ADMV1455_DIRECT_RF_LPF_MSK, data);
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_0x804, &data);
	if (ret)
		return ret;
	*hpf = no_os_field_get(ADMV1455_DIRECT_RF_HPF_MSK, data);
	return 0;
}

int admv1455_set_rf_direct_filter(Admv1455Dev* dev, uint8_t lpf, uint8_t hpf)
{
	int ret;

	if (lpf > 15 || hpf > 15)
		return -EINVAL;
	ret = admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_0x802, ADMV1455_DIRECT_RF_LPF_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_RF_LPF_MSK, lpf));
	if (ret)
		return ret;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_0x804, ADMV1455_DIRECT_RF_HPF_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_RF_HPF_MSK, hpf));
}

int admv1455_get_lo_phase(Admv1455Dev* dev, uint8_t* phase_i, uint8_t* phase_q)
{
	uint8_t data;
	int ret;

	if (!phase_i || !phase_q)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_LO_PHASE_I, &data);
	if (ret)
		return ret;
	*phase_i = no_os_field_get(ADMV1455_DIRECT_LO_PHASE_I_MSK, data);
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_LO_PHASE_Q, &data);
	if (ret)
		return ret;
	*phase_q = no_os_field_get(ADMV1455_DIRECT_LO_PHASE_Q_MSK, data);
	return 0;
}

int admv1455_set_lo_phase(Admv1455Dev* dev, uint8_t phase_i, uint8_t phase_q)
{
	int ret;

	if (phase_i > 31 || phase_q > 31)
		return -EINVAL;
	ret = admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_LO_PHASE_I, ADMV1455_DIRECT_LO_PHASE_I_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_LO_PHASE_I_MSK, phase_i));
	if (ret)
		return ret;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_LO_PHASE_Q, ADMV1455_DIRECT_LO_PHASE_Q_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_LO_PHASE_Q_MSK, phase_q));
}

int admv1455_get_lo_band(Admv1455Dev* dev, uint8_t* band)
{
	uint8_t data;
	int ret;

	if (!band)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_LO_PHASE_I, &data);
	if (ret)
		return ret;
	*band = no_os_field_get(ADMV1455_DIRECT_LO_BAND_MSK, data);
	return 0;
}

int admv1455_set_lo_band(Admv1455Dev* dev, uint8_t band)
{
	if (band > 1)
		return -EINVAL;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_LO_PHASE_I, ADMV1455_DIRECT_LO_BAND_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_LO_BAND_MSK, band));
}

int admv1455_get_lo_x1_filter(Admv1455Dev* dev, uint8_t* value)
{
	uint8_t data;
	int ret;

	if (!value)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_0x800, &data);
	if (ret)
		return ret;
	*value = no_os_field_get(ADMV1455_DIRECT_LO_X1_FILTER_MSK, data);
	return 0;
}

int admv1455_set_lo_x1_filter(Admv1455Dev* dev, uint8_t value)
{
	if (value > 31)
		return -EINVAL;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_0x800, ADMV1455_DIRECT_LO_X1_FILTER_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_LO_X1_FILTER_MSK, value));
}

int admv1455_get_lo_x3_filter(Admv1455Dev* dev, uint8_t* value)
{
	uint8_t data;
	int ret;

	if (!value)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_0x801, &data);
	if (ret)
		return ret;
	*value = no_os_field_get(ADMV1455_DIRECT_LO_X3_FILTER_MSK, data);
	return 0;
}

int admv1455_set_lo_x3_filter(Admv1455Dev* dev, uint8_t value)
{
	if (value > 31)
		return -EINVAL;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_0x801, ADMV1455_DIRECT_LO_X3_FILTER_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_LO_X3_FILTER_MSK, value));
}

int admv1455_get_lo_x4_filter(Admv1455Dev* dev, uint8_t* value)
{
	uint8_t data;
	int ret;

	if (!value)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_0x802, &data);
	if (ret)
		return ret;
	*value = no_os_field_get(ADMV1455_DIRECT_LO_X4_FILTER_MSK, data);
	return 0;
}

int admv1455_set_lo_x4_filter(Admv1455Dev* dev, uint8_t value)
{
	if (value > 15)
		return -EINVAL;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_0x802, ADMV1455_DIRECT_LO_X4_FILTER_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_LO_X4_FILTER_MSK, value));
}

int admv1455_get_lo_trap(Admv1455Dev* dev, uint16_t* value)
{
	uint8_t low, high;
	int ret;

	if (!value)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_0x803, &low);
	if (ret)
		return ret;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_0x804, &high);
	if (ret)
		return ret;
	*value =
	    low | (no_os_field_get(ADMV1455_DIRECT_LO_TRAP_MSB_MSK, high) << 8);
	return 0;
}

int admv1455_set_lo_trap(Admv1455Dev* dev, uint16_t value)
{
	int ret;

	if (value > 1023)
		return -EINVAL;
	ret = admv1455_spi_write(dev, ADMV1455_REG_DIRECT_0x803, value);
	if (ret)
		return ret;
	return admv1455_spi_update_bits(dev, ADMV1455_REG_DIRECT_0x804,
					ADMV1455_DIRECT_LO_TRAP_MSB_MSK,
					value >> 8);
}

int admv1455_get_lo_dbl_band(Admv1455Dev* dev, uint8_t* value)
{
	return admv1455_spi_read(dev, ADMV1455_REG_DIRECT_LO_DBLR, value);
}

int admv1455_set_lo_dbl_band(Admv1455Dev* dev, uint8_t value)
{
	return admv1455_spi_write(dev, ADMV1455_REG_DIRECT_LO_DBLR, value);
}

int admv1455_get_filter_load_en(Admv1455Dev* dev, bool* enable)
{
	uint8_t data;
	int ret;

	if (!enable)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_FILTER_LOAD_EN, &data);
	if (ret)
		return ret;
	*enable = !!(data & NO_OS_BIT(0));
	return 0;
}

int admv1455_set_filter_load_en(Admv1455Dev* dev, bool enable)
{
	return admv1455_spi_update_bits(dev, ADMV1455_REG_FILTER_LOAD_EN,
					NO_OS_BIT(0),
					enable ? NO_OS_BIT(0) : 0);
}

int admv1455_get_filter_table_sel(Admv1455Dev* dev, uint8_t* table)
{
	uint8_t data;
	int ret;

	if (!table)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_FILTER_TABLE_SEL, &data);
	if (ret)
		return ret;
	*table = data & NO_OS_BIT(0);
	return 0;
}

int admv1455_set_filter_table_sel(Admv1455Dev* dev, uint8_t table)
{
	if (table > 1)
		return -EINVAL;
	return admv1455_spi_update_bits(dev, ADMV1455_REG_FILTER_TABLE_SEL,
					NO_OS_BIT(0), table);
}

int admv1455_get_gain_lut_en(Admv1455Dev* dev, bool* enable)
{
	uint8_t data;
	int ret;

	if (!enable)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_GAIN_TABLE_EN, &data);
	if (ret)
		return ret;
	*enable = !!(data & NO_OS_BIT(0));
	return 0;
}

int admv1455_set_gain_lut_en(Admv1455Dev* dev, bool enable)
{
	return admv1455_spi_update_bits(dev, ADMV1455_REG_GAIN_TABLE_EN,
					NO_OS_BIT(0),
					enable ? NO_OS_BIT(0) : 0);
}

int admv1455_get_gain_lut_bypass_en(Admv1455Dev* dev, bool* enable)
{
	uint8_t data;
	int ret;

	if (!enable)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_BYPASS_GAIN_EN, &data);
	if (ret)
		return ret;
	*enable = !!(data & NO_OS_BIT(0));
	return 0;
}

int admv1455_set_gain_lut_bypass_en(Admv1455Dev* dev, bool enable)
{
	return admv1455_spi_update_bits(dev, ADMV1455_REG_BYPASS_GAIN_EN,
					NO_OS_BIT(0),
					enable ? NO_OS_BIT(0) : 0);
}

int admv1455_get_gain_load_en(Admv1455Dev* dev, bool* enable)
{
	uint8_t data;
	int ret;

	if (!enable)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_EN_LOAD_G, &data);
	if (ret)
		return ret;
	*enable = !!(data & NO_OS_BIT(0));
	return 0;
}

int admv1455_set_gain_load_en(Admv1455Dev* dev, bool enable)
{
	return admv1455_spi_update_bits(dev, ADMV1455_REG_EN_LOAD_G,
					NO_OS_BIT(0),
					enable ? NO_OS_BIT(0) : 0);
}

int admv1455_get_gain_lut_mode(Admv1455Dev* dev, uint8_t* mode)
{
	uint8_t data;
	int ret;

	if (!mode)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_GAIN_LUT_MODE, &data);
	if (ret)
		return ret;
	*mode = no_os_field_get(ADMV1455_GAIN_LUT_MODE_MSK, data);
	return 0;
}

int admv1455_set_gain_lut_mode(Admv1455Dev* dev, uint8_t mode)
{
	if (mode > 1)
		return -EINVAL;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_GAIN_LUT_MODE, ADMV1455_GAIN_LUT_MODE_MSK,
	    no_os_field_prep(ADMV1455_GAIN_LUT_MODE_MSK, mode));
}

int admv1455_get_gain_spi_pointer(Admv1455Dev* dev, uint8_t* pointer)
{
	uint8_t data;
	int ret;

	if (!pointer)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_GAIN_SPI_POINTER, &data);
	if (ret)
		return ret;
	*pointer = no_os_field_get(ADMV1455_GAIN_SPI_POINTER_MSK, data);
	return 0;
}

int admv1455_set_gain_spi_pointer(Admv1455Dev* dev, uint8_t pointer)
{
	if (pointer > 127)
		return -EINVAL;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_GAIN_SPI_POINTER, ADMV1455_GAIN_SPI_POINTER_MSK,
	    no_os_field_prep(ADMV1455_GAIN_SPI_POINTER_MSK, pointer));
}

int admv1455_get_gpo_f_direct(Admv1455Dev* dev, uint8_t* value)
{
	uint8_t data;
	int ret;

	if (!value)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_GPO_F, &data);
	if (ret)
		return ret;
	*value = no_os_field_get(ADMV1455_DIRECT_GPO_F_MSK, data);
	return 0;
}

int admv1455_set_gpo_f_direct(Admv1455Dev* dev, uint8_t value)
{
	if (value > 63)
		return -EINVAL;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_GPO_F, ADMV1455_DIRECT_GPO_F_MSK,
	    no_os_field_prep(ADMV1455_DIRECT_GPO_F_MSK, value));
}

int admv1455_get_gpo_g_direct(Admv1455Dev* dev, uint8_t* value)
{
	uint8_t data;
	int ret;

	if (!value)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_DIRECT_GPO_G, &data);
	if (ret)
		return ret;
	*value = no_os_field_get(ADMV1455_GPO_G_MSK, data);
	return 0;
}

int admv1455_set_gpo_g_direct(Admv1455Dev* dev, uint8_t value)
{
	if (value > 31)
		return -EINVAL;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_DIRECT_GPO_G, ADMV1455_GPO_G_MSK,
	    no_os_field_prep(ADMV1455_GPO_G_MSK, value));
}

int admv1455_get_gpo_g_bypass(Admv1455Dev* dev, uint8_t* value)
{
	uint8_t data;
	int ret;

	if (!value)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_BYPASS_GPO_G, &data);
	if (ret)
		return ret;
	*value = no_os_field_get(ADMV1455_GPO_G_MSK, data);
	return 0;
}

int admv1455_set_gpo_g_bypass(Admv1455Dev* dev, uint8_t value)
{
	if (value > 31)
		return -EINVAL;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_BYPASS_GPO_G, ADMV1455_GPO_G_MSK,
	    no_os_field_prep(ADMV1455_GPO_G_MSK, value));
}

int admv1455_get_gpo_f_oe(Admv1455Dev* dev, uint8_t* value)
{
	uint8_t data;
	int ret;

	if (!value)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_GPO_FG_OE_LSB, &data);
	if (ret)
		return ret;
	*value = no_os_field_get(ADMV1455_GPO_F_OE_MSK, data);
	return 0;
}

int admv1455_set_gpo_f_oe(Admv1455Dev* dev, uint8_t value)
{
	if (value > 63)
		return -EINVAL;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_GPO_FG_OE_LSB, ADMV1455_GPO_F_OE_MSK,
	    no_os_field_prep(ADMV1455_GPO_F_OE_MSK, value));
}

int admv1455_get_gpo_g_oe(Admv1455Dev* dev, uint8_t* value)
{
	uint8_t low, high;
	int ret;

	if (!value)
		return -EINVAL;
	ret = admv1455_spi_read(dev, ADMV1455_REG_GPO_FG_OE_LSB, &low);
	if (ret)
		return ret;
	ret = admv1455_spi_read(dev, ADMV1455_REG_GPO_FG_OE_MSB, &high);
	if (ret)
		return ret;
	*value = no_os_field_get(ADMV1455_GPO_G_OE_LSB_MSK, low) |
		 (no_os_field_get(ADMV1455_GPO_G_OE_MSB_MSK, high) << 2);
	return 0;
}

int admv1455_set_gpo_g_oe(Admv1455Dev* dev, uint8_t value)
{
	int ret;

	if (value > 31)
		return -EINVAL;
	ret = admv1455_spi_update_bits(
	    dev, ADMV1455_REG_GPO_FG_OE_LSB, ADMV1455_GPO_G_OE_LSB_MSK,
	    no_os_field_prep(ADMV1455_GPO_G_OE_LSB_MSK, value & 3));
	if (ret)
		return ret;
	return admv1455_spi_update_bits(
	    dev, ADMV1455_REG_GPO_FG_OE_MSB, ADMV1455_GPO_G_OE_MSB_MSK,
	    no_os_field_prep(ADMV1455_GPO_G_OE_MSB_MSK, value >> 2));
}

int admv1455_read_temperature(Admv1455Dev* dev, uint8_t* value)
{
	return admv1455_spi_read(dev, ADMV1455_REG_ADC_TEMP_READBACK, value);
}

int admv1455_read_detector(Admv1455Dev* dev, uint8_t* value)
{
	return admv1455_spi_read(dev, ADMV1455_REG_ADC_DETECTOR_READBACK,
				 value);
}

static int admv1455_lut_address(Admv1455Lut table, uint8_t index,
				uint16_t* address)
{
	switch (table) {
	case ADMV1455_FILTER_LUTA:
	case ADMV1455_FILTER_LUTB:
		if (index >= ADMV1455_FILTER_LUT_NUM_ENTRIES)
			return -EINVAL;
		*address = (table == ADMV1455_FILTER_LUTA
				? ADMV1455_REG_FILTER_LUTA_BASE
				: ADMV1455_REG_FILTER_LUTB_BASE) +
			   index * ADMV1455_FILTER_LUT_ENTRY_SIZE;
		return ADMV1455_FILTER_LUT_ENTRY_SIZE;
	case ADMV1455_GAIN_LUT:
		if (index >= ADMV1455_GAIN_LUT_NUM_ENTRIES)
			return -EINVAL;
		*address = ADMV1455_REG_GAIN_LUT_BASE +
			   index * ADMV1455_GAIN_LUT_ENTRY_SIZE;
		return ADMV1455_GAIN_LUT_ENTRY_SIZE;
	default:
		return -EINVAL;
	}
}

int admv1455_read_lut_entry(Admv1455Dev* dev, Admv1455Lut table, uint8_t index,
			    uint8_t* data)
{
	uint16_t address;
	int size, offset, ret;

	if (!dev || !data)
		return -EINVAL;
	size = admv1455_lut_address(table, index, &address);
	if (size < 0)
		return size;
	for (offset = 0; offset < size; offset++) {
		ret = admv1455_spi_read(dev, address + offset, &data[offset]);
		if (ret)
			return ret;
	}
	return 0;
}

int admv1455_write_lut_entry(Admv1455Dev* dev, Admv1455Lut table, uint8_t index,
			     const uint8_t* data)
{
	uint16_t address;
	int size, offset, ret;

	if (!dev || !data)
		return -EINVAL;
	size = admv1455_lut_address(table, index, &address);
	if (size < 0)
		return size;
	for (offset = 0; offset < size; offset++) {
		ret = admv1455_spi_write(dev, address + offset, data[offset]);
		if (ret)
			return ret;
	}
	return 0;
}

int admv1455_remove(Admv1455Dev* dev)
{
	int ret, status = 0;

	if (!dev)
		return -EINVAL;
	if (dev->gpio_cen) {
		ret = no_os_gpio_set_value(dev->gpio_cen, NO_OS_GPIO_LOW);
		if (ret && !status)
			status = ret;
		ret = no_os_gpio_remove(dev->gpio_cen);
		if (ret && !status)
			status = ret;
	}
	if (dev->gpio_reset) {
		ret = no_os_gpio_set_value(dev->gpio_reset, NO_OS_GPIO_LOW);
		if (ret && !status)
			status = ret;
		ret = no_os_gpio_remove(dev->gpio_reset);
		if (ret && !status)
			status = ret;
	}
	if (dev->spi_desc) {
		ret = no_os_spi_remove(dev->spi_desc);
		if (ret && !status)
			status = ret;
	}
	no_os_free(dev);
	return status;
}

int admv1455_init(Admv1455Dev** device, const Admv1455InitParam* param)
{
	static const struct {
		uint16_t reg;
		uint8_t value;
	} init_seq[] = {
	    {0x104, 0x00}, {0x105, 0x00}, {0x107, 0xAF}, {0x108, 0x02},
	    {0x172, 0x00}, {0x173, 0x00}, {0x17D, 0x01}, {0x184, 0x01},
	    {0x188, 0x0E}, {0x202, 0x00}, {0x285, 0x00}, {0x2A2, 0x12},
	    {0x600, 0x00}, {0x601, 0x00}, {0x602, 0x00}, {0x800, 0x1F},
	    {0x801, 0x80}, {0x802, 0x05}, {0x803, 0xA7}, {0x804, 0xF1},
	    {0x809, 0x0F}, {0x80A, 0x2F}, {0x80C, 0x80}, {0x780, 0xFF},
	    {0x781, 0x07},
	};
	Admv1455Dev* dev;
	uint32_t index;
	uint8_t product_id;
	uint8_t value;
	int ret;

	if (!device || !param || !param->spi_init || param->chip_addr > 3 ||
	    param->spi3wire)
		return -EINVAL;
	dev = no_os_calloc(1, sizeof(*dev));
	if (!dev)
		return -ENOMEM;
	dev->chip_addr = param->chip_addr;
	ret = no_os_spi_init(&dev->spi_desc, param->spi_init);
	if (ret)
		goto error;
	ret = no_os_gpio_get_optional(&dev->gpio_cen, param->gpio_cen);
	if (ret)
		goto error;
	if (dev->gpio_cen) {
		ret =
		    no_os_gpio_direction_output(dev->gpio_cen, NO_OS_GPIO_LOW);
		if (ret)
			goto error;
	}
	ret = no_os_gpio_get_optional(&dev->gpio_reset, param->gpio_reset);
	if (ret)
		goto error;
	if (dev->gpio_reset) {
		ret = no_os_gpio_direction_output(dev->gpio_reset,
						  NO_OS_GPIO_LOW);
		if (ret)
			goto error;
		no_os_udelay(10);
		ret = no_os_gpio_set_value(dev->gpio_reset, NO_OS_GPIO_HIGH);
		if (ret)
			goto error;
		no_os_udelay(10);
	}
	ret = admv1455_spi_write(dev, ADMV1455_REG_SPI_CONTROL,
				 ADMV1455_SPI_SOFT_RESET);
	if (ret)
		goto error;
	ret = admv1455_spi_write(dev, ADMV1455_REG_SPI_CONTROL,
				 ADMV1455_SPI_4WIRE);
	if (ret)
		goto error;
	for (index = 0; index < NO_OS_ARRAY_SIZE(init_seq); index++) {
		ret = admv1455_spi_write(dev, init_seq[index].reg,
					 init_seq[index].value);
		if (ret)
			goto error;
	}
	if (dev->gpio_cen) {
		ret = no_os_gpio_set_value(dev->gpio_cen, NO_OS_GPIO_HIGH);
		if (ret)
			goto error;
	}
	ret = admv1455_spi_read(dev, ADMV1455_REG_PRODUCT_ID_LSB, &product_id);
	if (ret)
		goto error;
	ret = admv1455_spi_write(dev, ADMV1455_REG_SCRATCH_PAD,
				 ADMV1455_SCRATCH_TEST_VAL);
	if (ret)
		goto error;
	ret = admv1455_spi_read(dev, ADMV1455_REG_SCRATCH_PAD, &value);
	if (ret)
		goto error;
	if (value != ADMV1455_SCRATCH_TEST_VAL) {
		pr_err("admv1455: scratch pad read 0x%02X (expected 0x%02X), "
		       "product ID 0x%02X (expected 0x%02X)\n", value,
		       ADMV1455_SCRATCH_TEST_VAL, product_id, ADMV1455_PRODUCT_ID);
		ret = -EIO;
		goto error;
	}
	*device = dev;
	return 0;
error:
	admv1455_remove(dev);
	return ret;
}