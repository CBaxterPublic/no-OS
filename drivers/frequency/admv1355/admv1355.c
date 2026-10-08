/***************************************************************************//**
 *   @file   admv1355.c
 *   @brief  Implementation of admv1355 Driver.
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
#include <malloc.h>
#include "admv1355.h"
#include "no_os_error.h"
#include "no_os_alloc.h"
#include "no_os_delay.h"
#include "no_os_print_log.h"

/**
 * @brief Writes data to the ADMV1355 over SPI.
 * @param dev - The device structure.
 * @param reg_addr - The (12-bit) register address.
 * @param data - Data byte to write.
 * @return 0 in case of success, negative error code otherwise.
 */
int admv1355_spi_write(Admv1355Dev*dev, uint16_t reg_addr,
			uint8_t data)
{
	uint8_t buff[ADMV1355_BUFF_SIZE_BYTES];

	/*
	 * Frame format (3 bytes):
	 *   Byte 0: [R/Wn][0][0][0][A11:A8]
	 *   Byte 1: [A7:A0]
	 *   Byte 2: [D7:D0]
	 */
	buff[0] = ADMV1355_SPI_WRITE_CMD | ((reg_addr >> 8) & 0x0F);
	buff[1] = reg_addr & 0xFF;
	buff[2] = data;

	return no_os_spi_write_and_read(dev->spi_desc, buff,
					 ADMV1355_BUFF_SIZE_BYTES);
}

/**
 * @brief Reads data from the ADMV1355 over SPI.
 * @param dev - The device structure.
 * @param reg_addr - The (12-bit) register address.
 * @param data - Data byte read from the device.
 * @return 0 in case of success, negative error code otherwise.
 */
int admv1355_spi_read(Admv1355Dev*dev, uint16_t reg_addr,
		       uint8_t*data)
{
	uint8_t buff[ADMV1355_BUFF_SIZE_BYTES];
	int ret;

	buff[0] = ADMV1355_SPI_READ_CMD | ((reg_addr >> 8) & 0x0F);
	buff[1] = reg_addr & 0xFF;
	buff[2] = 0x00;

	ret = no_os_spi_write_and_read(dev->spi_desc, buff,
					ADMV1355_BUFF_SIZE_BYTES);
	if (ret)
		return ret;

	*data = buff[2];

	return 0;
}

/**
 * @brief Update ADMV1355 register bits.
 * @param dev - The device structure.
 * @param reg_addr - The register address.
 * @param mask - Mask for the bits to be updated.
 * @param data - Data written to the device (requires prior bit shifting).
 * @return 0 in case of success, negative error code otherwise.
 */
int admv1355_spi_update_bits(Admv1355Dev* dev, uint16_t reg_addr,
			      uint8_t mask, uint8_t data)
{
	uint8_t read_val;
	int ret;

	ret = admv1355_spi_read(dev, reg_addr, &read_val);
	if (ret)
		return ret;

	read_val &= ~mask;
	read_val |= data;

	return admv1355_spi_write(dev, reg_addr, read_val);
}

/**
 * @brief Set the RF band (0x805[7] and 0x806[6]).
 */
int admv1355_set_rf_band(Admv1355Dev* dev,
			  Admv1355RfBand rf_band)
{
	int ret;

	ret = admv1355_spi_update_bits(dev, ADMV1355_REG_LO_TRAP_9_8,
					ADMV1355_RF_BAND_805_MSK,
					no_os_field_prep(ADMV1355_RF_BAND_805_MSK, rf_band));
	if (ret)
		return ret;

	ret = admv1355_spi_update_bits(dev, ADMV1355_REG_LO_BAND,
					ADMV1355_MIXER_BAND_806_MSK,
					no_os_field_prep(ADMV1355_MIXER_BAND_806_MSK, rf_band));
	if (ret)
		return ret;

	dev->rf_band = rf_band;

	return 0;
}

/**
 * @brief Get the RF band.
 */
int admv1355_get_rf_band(Admv1355Dev* dev,
			  Admv1355RfBand* rf_band)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_LO_TRAP_9_8, &data);
	if (ret)
		return ret;

	*rf_band = no_os_field_get(ADMV1355_RF_BAND_805_MSK, data);

	return 0;
}

/**
 * @brief Set the mixer sideband (0x806[5]).
 */
int admv1355_set_mixer_sideband(Admv1355Dev* dev,
				 Admv1355MixerSideband sideband)
{
	int ret;

	ret = admv1355_spi_update_bits(dev, ADMV1355_REG_LO_BAND,
					ADMV1355_LO_SIDEBAND_MSK,
					no_os_field_prep(ADMV1355_LO_SIDEBAND_MSK, sideband));
	if (ret)
		return ret;

	dev->mixer_sideband = sideband;

	return 0;
}

/**
 * @brief Get the mixer sideband.
 */
int admv1355_get_mixer_sideband(Admv1355Dev* dev,
				 Admv1355MixerSideband* sideband)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_LO_BAND, &data);
	if (ret)
		return ret;

	*sideband = no_os_field_get(ADMV1355_LO_SIDEBAND_MSK, data);

	return 0;
}

/**
 * @brief Set the frequency translation / IF mode (0x602[3:0]).
 */
int admv1355_set_if_mode(Admv1355Dev*dev,
			  Admv1355IfMode if_mode)
{
	static const uint8_t if_mode_reg_values[] = {
		[ADMV1355_IF_MODE_BASEBAND] = ADMV1355_BB_EN_MSK,
		[ADMV1355_IF_MODE_COMPLEX_IF_LOW] = ADMV1355_IF_LOW_FREQ_EN_MSK,
		[ADMV1355_IF_MODE_COMPLEX_IF_HIGH] = ADMV1355_IF_HIGH_FREQ_EN_MSK,
		[ADMV1355_IF_MODE_HYBRID] = ADMV1355_IF_HYBRID_EN_MSK,
	};
	int ret;

	if (if_mode > ADMV1355_IF_MODE_HYBRID)
		return -EINVAL;

	ret = admv1355_spi_update_bits(dev, ADMV1355_REG_IF_ENABLE,
					ADMV1355_IF_MODE_MSK,
					if_mode_reg_values[if_mode]);
	if (ret)
		return ret;

	dev->if_mode = if_mode;

	return 0;
}

/**
 * @brief Get the frequency translation / IF mode.
 */
int admv1355_get_if_mode(Admv1355Dev*dev,
			  Admv1355IfMode*if_mode)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_IF_ENABLE, &data);
	if (ret)
		return ret;

	switch (data & ADMV1355_IF_MODE_MSK) {
	case ADMV1355_BB_EN_MSK:
		*if_mode = ADMV1355_IF_MODE_BASEBAND;
		break;
	case ADMV1355_IF_LOW_FREQ_EN_MSK:
		*if_mode = ADMV1355_IF_MODE_COMPLEX_IF_LOW;
		break;
	case ADMV1355_IF_HIGH_FREQ_EN_MSK:
		*if_mode = ADMV1355_IF_MODE_COMPLEX_IF_HIGH;
		break;
	case ADMV1355_IF_HYBRID_EN_MSK:
		*if_mode = ADMV1355_IF_MODE_HYBRID;
		break;
	default:
		return -EIO;
	}

	return 0;
}

/**
 * @brief Set the RF chain DSA1/DSA2 direct attenuation (0x600).
 */
int admv1355_set_rf_dsa(Admv1355Dev*dev, uint8_t dsa1, uint8_t dsa2)
{
	if (dsa1 > 15 || dsa2 > 15)
		return -EINVAL;

	return admv1355_spi_write(dev, ADMV1355_REG_DSA_DIRECT,
				   no_os_field_prep(ADMV1355_DSA1_DIRECT_MSK, dsa1) |
				   no_os_field_prep(ADMV1355_DSA2_DIRECT_MSK, dsa2));
}

/**
 * @brief Get the RF chain DSA1/DSA2 direct attenuation.
 */
int admv1355_get_rf_dsa(Admv1355Dev*dev, uint8_t*dsa1,
			 uint8_t*dsa2)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_DSA_DIRECT, &data);
	if (ret)
		return ret;

	*dsa1 = no_os_field_get(ADMV1355_DSA1_DIRECT_MSK, data);
	*dsa2 = no_os_field_get(ADMV1355_DSA2_DIRECT_MSK, data);

	return 0;
}

/**
 * @brief Set the IF chain fine I/Q attenuation, 0.1dB/step (0x80A).
 */
int admv1355_set_if_dsa(Admv1355Dev*dev, uint8_t dsai, uint8_t dsaq)
{
	if (dsai > 15 || dsaq > 15)
		return -EINVAL;

	return admv1355_spi_write(dev, ADMV1355_REG_IF_GAIN,
				   no_os_field_prep(ADMV1355_DSAI_MSK, dsai) |
				   no_os_field_prep(ADMV1355_DSAQ_MSK, dsaq));
}

/**
 * @brief Get the IF chain fine I/Q attenuation.
 */
int admv1355_get_if_dsa(Admv1355Dev*dev, uint8_t*dsai,
			 uint8_t*dsaq)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_IF_GAIN, &data);
	if (ret)
		return ret;

	*dsai = no_os_field_get(ADMV1355_DSAI_MSK, data);
	*dsaq = no_os_field_get(ADMV1355_DSAQ_MSK, data);

	return 0;
}

/**
 * @brief Set the LO I/Q phase adjust (0x805[6:2] / 0x806[4:0]).
 */
int admv1355_set_lo_phase(Admv1355Dev*dev, uint8_t phase_i,
			   uint8_t phase_q)
{
	int ret;

	if (phase_i > 31 || phase_q > 31)
		return -EINVAL;

	ret = admv1355_spi_update_bits(dev, ADMV1355_REG_LO_BAND,
					ADMV1355_LO_PHASE_I_MSK,
					no_os_field_prep(ADMV1355_LO_PHASE_I_MSK, phase_i));
	if (ret)
		return ret;

	return admv1355_spi_update_bits(dev, ADMV1355_REG_LO_TRAP_9_8,
					 ADMV1355_LO_PHASE_Q_MSK,
					 no_os_field_prep(ADMV1355_LO_PHASE_Q_MSK, phase_q));
}

/**
 * @brief Get the LO I/Q phase adjust.
 */
int admv1355_get_lo_phase(Admv1355Dev*dev, uint8_t*phase_i,
			   uint8_t*phase_q)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_LO_BAND, &data);
	if (ret)
		return ret;
	*phase_i = no_os_field_get(ADMV1355_LO_PHASE_I_MSK, data);

	ret = admv1355_spi_read(dev, ADMV1355_REG_LO_TRAP_9_8, &data);
	if (ret)
		return ret;
	*phase_q = no_os_field_get(ADMV1355_LO_PHASE_Q_MSK, data);

	return 0;
}

/**
 * @brief Select the LO x1/x3/x4 multiplier filters and trap filter/doubler
 *        band for a given LO input frequency (datasheet filter selection
 *        table).
 * @param dev - The device structure.
 * @param lo_in - LO input frequency, in Hz.
 * @return 0 in case of success, negative error code otherwise.
 */
int admv1355_set_lo_filters(Admv1355Dev*dev,
			     unsigned long long lo_in)
{
	unsigned long long lo_khz = lo_in / 1000;
	uint8_t lo_x4_filter, lo_x3_filter, lo_x1_filter;
	uint8_t lo_doubler_band, lo_band;
	uint16_t lo_trap_filter;
	int ret;

	if (lo_khz < 10000000ULL) {
		lo_x4_filter = 0x08;
		lo_x3_filter = 0x1C;
		lo_x1_filter = 0x1F;
		lo_trap_filter = 0x318;
		lo_doubler_band = 0x1F;
	} else if (lo_khz < 12000000ULL) {
		lo_x4_filter = 0x00;
		lo_x3_filter = 0x1C;
		lo_x1_filter = 0x1F;
		lo_trap_filter = 0x318;
		lo_doubler_band = 0x1D;
	} else if (lo_khz < 14000000ULL) {
		lo_x4_filter = 0x00;
		lo_x3_filter = 0x07;
		lo_x1_filter = 0x0A;
		lo_trap_filter = 0x3AA;
		lo_doubler_band = 0x04;
	} else if (lo_khz < 14500000ULL) {
		lo_x4_filter = 0x05;
		lo_x3_filter = 0x07;
		lo_x1_filter = 0x0A;
		lo_trap_filter = 0x3AA;
		lo_doubler_band = 0x08;
	} else if (lo_khz < 15000000ULL) {
		lo_x4_filter = 0x05;
		lo_x3_filter = 0x07;
		lo_x1_filter = 0x0A;
		lo_trap_filter = 0x1A3;
		lo_doubler_band = 0x08;
	} else if (lo_khz < 17000000ULL) {
		lo_x4_filter = 0x05;
		lo_x3_filter = 0x05;
		lo_x1_filter = 0x05;
		lo_trap_filter = 0x1A3;
		lo_doubler_band = 0x08;
	} else if (lo_khz < 18000000ULL) {
		lo_x4_filter = 0x05;
		lo_x3_filter = 0x04;
		lo_x1_filter = 0x01;
		lo_trap_filter = 0x1A3;
		lo_doubler_band = 0x08;
	} else if (lo_khz < 18500000ULL) {
		lo_x4_filter = 0x07;
		lo_x3_filter = 0x04;
		lo_x1_filter = 0x01;
		lo_trap_filter = 0x1A3;
		lo_doubler_band = 0x80;
	} else if (lo_khz < 19000000ULL) {
		lo_x4_filter = 0x07;
		lo_x3_filter = 0x04;
		lo_x1_filter = 0x01;
		lo_trap_filter = 0x1A7;
		lo_doubler_band = 0x80;
	} else if (lo_khz < 21000000ULL) {
		lo_x4_filter = 0x07;
		lo_x3_filter = 0x03;
		lo_x1_filter = 0x01;
		lo_trap_filter = 0x1A7;
		lo_doubler_band = 0x80;
	} else if (lo_khz < 24000000ULL) {
		lo_x4_filter = 0x07;
		lo_x3_filter = 0x02;
		lo_x1_filter = 0x00;
		lo_trap_filter = 0x1E7;
		lo_doubler_band = 0x80;
	} else {
		lo_x4_filter = 0x07;
		lo_x3_filter = 0x00;
		lo_x1_filter = 0x00;
		lo_trap_filter = 0x1E7;
		lo_doubler_band = 0xC0;
	}

	lo_band = (lo_khz >= 18000000ULL) ? 1 : 0;

	ret = admv1355_spi_write(dev, ADMV1355_REG_LO_X4_FILTER, lo_x4_filter);
	if (ret)
		return ret;

	ret = admv1355_spi_write(dev, ADMV1355_REG_LO_X3_FILTER, lo_x3_filter);
	if (ret)
		return ret;

	ret = admv1355_spi_write(dev, ADMV1355_REG_LO_X1_FILTER, lo_x1_filter);
	if (ret)
		return ret;

	ret = admv1355_spi_write(dev, ADMV1355_REG_LO_TRAP_7_0,
				  lo_trap_filter & 0xFF);
	if (ret)
		return ret;

	ret = admv1355_spi_update_bits(dev, ADMV1355_REG_LO_TRAP_9_8,
					ADMV1355_LO_TRAP_9_8_MSK,
					no_os_field_prep(ADMV1355_LO_TRAP_9_8_MSK,
							  (lo_trap_filter >> 8) & 0x03));
	if (ret)
		return ret;

	ret = admv1355_spi_write(dev, ADMV1355_REG_LO_DBL_BAND,
				  lo_doubler_band);
	if (ret)
		return ret;

	ret = admv1355_spi_update_bits(dev, ADMV1355_REG_LO_BAND,
					ADMV1355_LO_BAND_806_MSK,
					no_os_field_prep(ADMV1355_LO_BAND_806_MSK, lo_band));
	if (ret)
		return ret;

	dev->lo_in = lo_in;

	return 0;
}

/**
 * @brief Set the RF chain DSA1/DSA2 bypass-mode attenuation (0x28B).
 */
int admv1355_set_rf_dsa_bypass(Admv1355Dev*dev, uint8_t dsa1,
				uint8_t dsa2)
{
	if (dsa1 > 15 || dsa2 > 15)
		return -EINVAL;

	return admv1355_spi_write(dev, ADMV1355_REG_DSA_BYPASS,
				   no_os_field_prep(ADMV1355_DSA1_BYPASS_MSK, dsa1) |
				   no_os_field_prep(ADMV1355_DSA2_BYPASS_MSK, dsa2));
}

/**
 * @brief Get the RF chain DSA1/DSA2 bypass-mode attenuation.
 */
int admv1355_get_rf_dsa_bypass(Admv1355Dev*dev, uint8_t*dsa1,
				uint8_t*dsa2)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_DSA_BYPASS, &data);
	if (ret)
		return ret;

	*dsa1 = no_os_field_get(ADMV1355_DSA1_BYPASS_MSK, data);
	*dsa2 = no_os_field_get(ADMV1355_DSA2_BYPASS_MSK, data);

	return 0;
}

/**
 * @brief Set the RF LPF bypass enable and bypass-mode value (0x2A0).
 */
int admv1355_set_rf_lpf_bypass(Admv1355Dev*dev, bool enable,
				uint8_t value)
{
	if (value > 15)
		return -EINVAL;

	return admv1355_spi_write(dev, ADMV1355_REG_RF_LPF,
				   (enable ? ADMV1355_LPF_SELECT_MSK : 0) |
				   no_os_field_prep(ADMV1355_LPF_BYPASS_VAL_MSK, value));
}

/**
 * @brief Get the RF LPF bypass enable and bypass-mode value.
 */
int admv1355_get_rf_lpf_bypass(Admv1355Dev*dev, bool*enable,
				uint8_t*value)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_RF_LPF, &data);
	if (ret)
		return ret;

	*enable = !!(data & ADMV1355_LPF_SELECT_MSK);
	*value = no_os_field_get(ADMV1355_LPF_BYPASS_VAL_MSK, data);

	return 0;
}

/**
 * @brief Set the RF HPF bypass enable and bypass-mode value (0x2A1).
 */
int admv1355_set_rf_hpf_bypass(Admv1355Dev*dev, bool enable,
				uint8_t value)
{
	if (value > 63)
		return -EINVAL;

	return admv1355_spi_write(dev, ADMV1355_REG_RF_HPF,
				   (enable ? ADMV1355_HPF_SELECT_MSK : 0) |
				   no_os_field_prep(ADMV1355_HPF_BYPASS_VAL_MSK, value));
}

/**
 * @brief Get the RF HPF bypass enable and bypass-mode value.
 */
int admv1355_get_rf_hpf_bypass(Admv1355Dev*dev, bool*enable,
				uint8_t*value)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_RF_HPF, &data);
	if (ret)
		return ret;

	*enable = !!(data & ADMV1355_HPF_SELECT_MSK);
	*value = no_os_field_get(ADMV1355_HPF_BYPASS_VAL_MSK, data);

	return 0;
}

/**
 * @brief Set the RF direct LPF/HPF filter select values (0x80B).
 */
int admv1355_set_rf_direct_filter(Admv1355Dev*dev, uint8_t lpf_val,
				   uint8_t hpf_val)
{
	if (lpf_val > 15 || hpf_val > 15)
		return -EINVAL;

	return admv1355_spi_write(dev, ADMV1355_REG_RF_FILTER,
				   no_os_field_prep(ADMV1355_DIRECT_LPF_MSK, lpf_val) |
				   no_os_field_prep(ADMV1355_DIRECT_HPF_MSK, hpf_val));
}

/**
 * @brief Get the RF direct LPF/HPF filter select values.
 */
int admv1355_get_rf_direct_filter(Admv1355Dev*dev, uint8_t*lpf_val,
				   uint8_t*hpf_val)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_RF_FILTER, &data);
	if (ret)
		return ret;

	*lpf_val = no_os_field_get(ADMV1355_DIRECT_LPF_MSK, data);
	*hpf_val = no_os_field_get(ADMV1355_DIRECT_HPF_MSK, data);

	return 0;
}

/**
 * @brief Set the raw LO x4 multiplier filter register value (0x800).
 */
int admv1355_set_lo_x4_filter(Admv1355Dev*dev, uint8_t val)
{
	return admv1355_spi_write(dev, ADMV1355_REG_LO_X4_FILTER, val);
}

/**
 * @brief Get the raw LO x4 multiplier filter register value.
 */
int admv1355_get_lo_x4_filter(Admv1355Dev*dev, uint8_t*val)
{
	return admv1355_spi_read(dev, ADMV1355_REG_LO_X4_FILTER, val);
}

/**
 * @brief Set the raw LO x3 multiplier filter register value (0x801).
 */
int admv1355_set_lo_x3_filter(Admv1355Dev*dev, uint8_t val)
{
	return admv1355_spi_write(dev, ADMV1355_REG_LO_X3_FILTER, val);
}

/**
 * @brief Get the raw LO x3 multiplier filter register value.
 */
int admv1355_get_lo_x3_filter(Admv1355Dev*dev, uint8_t*val)
{
	return admv1355_spi_read(dev, ADMV1355_REG_LO_X3_FILTER, val);
}

/**
 * @brief Set the raw LO x1 multiplier filter register value (0x802).
 */
int admv1355_set_lo_x1_filter(Admv1355Dev*dev, uint8_t val)
{
	return admv1355_spi_write(dev, ADMV1355_REG_LO_X1_FILTER, val);
}

/**
 * @brief Get the raw LO x1 multiplier filter register value.
 */
int admv1355_get_lo_x1_filter(Admv1355Dev*dev, uint8_t*val)
{
	return admv1355_spi_read(dev, ADMV1355_REG_LO_X1_FILTER, val);
}

/**
 * @brief Set the LO trap filter (10-bit, spans 0x804/0x805).
 */
int admv1355_set_lo_trap(Admv1355Dev*dev, uint16_t val)
{
	int ret;

	if (val > 1023)
		return -EINVAL;

	ret = admv1355_spi_write(dev, ADMV1355_REG_LO_TRAP_7_0, val & 0xFF);
	if (ret)
		return ret;

	return admv1355_spi_update_bits(dev, ADMV1355_REG_LO_TRAP_9_8,
					 ADMV1355_LO_TRAP_9_8_MSK,
					 no_os_field_prep(ADMV1355_LO_TRAP_9_8_MSK,
							   (val >> 8) & 0x03));
}

/**
 * @brief Get the LO trap filter.
 */
int admv1355_get_lo_trap(Admv1355Dev*dev, uint16_t*val)
{
	uint8_t lo, hi;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_LO_TRAP_7_0, &lo);
	if (ret)
		return ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_LO_TRAP_9_8, &hi);
	if (ret)
		return ret;

	*val = lo | (no_os_field_get(ADMV1355_LO_TRAP_9_8_MSK, hi) << 8);

	return 0;
}

/**
 * @brief Set the LO doubler band (0x80C).
 */
int admv1355_set_lo_dbl_band(Admv1355Dev*dev, uint8_t val)
{
	return admv1355_spi_write(dev, ADMV1355_REG_LO_DBL_BAND, val);
}

/**
 * @brief Get the LO doubler band.
 */
int admv1355_get_lo_dbl_band(Admv1355Dev*dev, uint8_t*val)
{
	return admv1355_spi_read(dev, ADMV1355_REG_LO_DBL_BAND, val);
}

/**
 * @brief Set the LON offset I/Q (0x808/0x809).
 */
int admv1355_set_lon_offset(Admv1355Dev*dev, uint8_t offset_i,
			     uint8_t offset_q)
{
	int ret;

	ret = admv1355_spi_write(dev, ADMV1355_REG_LON_OFFSET_I, offset_i);
	if (ret)
		return ret;

	return admv1355_spi_write(dev, ADMV1355_REG_LON_OFFSET_Q, offset_q);
}

/**
 * @brief Get the LON offset I/Q.
 */
int admv1355_get_lon_offset(Admv1355Dev*dev, uint8_t*offset_i,
			     uint8_t*offset_q)
{
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_LON_OFFSET_I, offset_i);
	if (ret)
		return ret;

	return admv1355_spi_read(dev, ADMV1355_REG_LON_OFFSET_Q, offset_q);
}

/**
 * @brief Set the filter LUT enable (0x202).
 */
int admv1355_set_filter_lut_en(Admv1355Dev*dev, bool enable)
{
	return admv1355_spi_write(dev, ADMV1355_REG_FILTER_LUT_EN, enable ? 1 : 0);
}

/**
 * @brief Get the filter LUT enable.
 */
int admv1355_get_filter_lut_en(Admv1355Dev*dev, bool*enable)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_FILTER_LUT_EN, &data);
	if (ret)
		return ret;

	*enable = !!(data & NO_OS_BIT(0));

	return 0;
}

/**
 * @brief Set the filter LUT load enable (0x208).
 */
int admv1355_set_filter_load_en(Admv1355Dev*dev, bool enable)
{
	return admv1355_spi_write(dev, ADMV1355_REG_FILTER_LOAD_EN, enable ? 1 : 0);
}

/**
 * @brief Get the filter LUT load enable.
 */
int admv1355_get_filter_load_en(Admv1355Dev*dev, bool*enable)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_FILTER_LOAD_EN, &data);
	if (ret)
		return ret;

	*enable = !!(data & NO_OS_BIT(0));

	return 0;
}

/**
 * @brief Set the filter LUT table select (0x207), 0 = A, 1 = B.
 */
int admv1355_set_filter_table_sel(Admv1355Dev*dev, uint8_t table)
{
	if (table > 1)
		return -EINVAL;

	return admv1355_spi_write(dev, ADMV1355_REG_FILTER_TBL_SEL, table);
}

/**
 * @brief Get the filter LUT table select.
 */
int admv1355_get_filter_table_sel(Admv1355Dev*dev, uint8_t*table)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_FILTER_TBL_SEL, &data);
	if (ret)
		return ret;

	*table = data & NO_OS_BIT(0);

	return 0;
}

/**
 * @brief Set the gain LUT enable (0x285).
 */
int admv1355_set_gain_lut_en(Admv1355Dev*dev, bool enable)
{
	return admv1355_spi_write(dev, ADMV1355_REG_GAIN_LUT_EN, enable ? 1 : 0);
}

/**
 * @brief Get the gain LUT enable.
 */
int admv1355_get_gain_lut_en(Admv1355Dev*dev, bool*enable)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_GAIN_LUT_EN, &data);
	if (ret)
		return ret;

	*enable = !!(data & NO_OS_BIT(0));

	return 0;
}

/**
 * @brief Set the gain LUT bypass enable (0x28A).
 */
int admv1355_set_gain_lut_bypass_en(Admv1355Dev*dev, bool enable)
{
	return admv1355_spi_update_bits(dev, ADMV1355_REG_GAIN_TBL_BYP,
					 ADMV1355_GAIN_LUT_BYP_EN_MSK,
					 enable ? ADMV1355_GAIN_LUT_BYP_EN_MSK : 0);
}

/**
 * @brief Get the gain LUT bypass enable.
 */
int admv1355_get_gain_lut_bypass_en(Admv1355Dev*dev, bool*enable)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_GAIN_TBL_BYP, &data);
	if (ret)
		return ret;

	*enable = !!(data & ADMV1355_GAIN_LUT_BYP_EN_MSK);

	return 0;
}

/**
 * @brief Set the gain LUT load enable (0x281).
 */
int admv1355_set_gain_load_en(Admv1355Dev*dev, bool enable)
{
	return admv1355_spi_write(dev, ADMV1355_REG_GAIN_LOAD_EN, enable ? 1 : 0);
}

/**
 * @brief Get the gain LUT load enable.
 */
int admv1355_get_gain_load_en(Admv1355Dev*dev, bool*enable)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_GAIN_LOAD_EN, &data);
	if (ret)
		return ret;

	*enable = !!(data & NO_OS_BIT(0));

	return 0;
}

/**
 * @brief Set the GPO_G direct output value (0x601).
 */
int admv1355_set_gpo_g_direct(Admv1355Dev*dev, uint8_t val)
{
	if (val > 31)
		return -EINVAL;

	return admv1355_spi_update_bits(dev, ADMV1355_REG_GPO_G_DIRECT,
					 ADMV1355_GPO_G_DIRECT_MSK,
					 no_os_field_prep(ADMV1355_GPO_G_DIRECT_MSK, val));
}

/**
 * @brief Get the GPO_G direct output value.
 */
int admv1355_get_gpo_g_direct(Admv1355Dev*dev, uint8_t*val)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_GPO_G_DIRECT, &data);
	if (ret)
		return ret;

	*val = no_os_field_get(ADMV1355_GPO_G_DIRECT_MSK, data);

	return 0;
}

/**
 * @brief Set the GPO_G bypass-mode value (0x28C).
 */
int admv1355_set_gpo_g_bypass(Admv1355Dev*dev, uint8_t val)
{
	if (val > 31)
		return -EINVAL;

	return admv1355_spi_update_bits(dev, ADMV1355_REG_GPO_G_BYPASS,
					 ADMV1355_GPO_G_BYPASS_MSK,
					 no_os_field_prep(ADMV1355_GPO_G_BYPASS_MSK, val));
}

/**
 * @brief Get the GPO_G bypass-mode value.
 */
int admv1355_get_gpo_g_bypass(Admv1355Dev*dev, uint8_t*val)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_GPO_G_BYPASS, &data);
	if (ret)
		return ret;

	*val = no_os_field_get(ADMV1355_GPO_G_BYPASS_MSK, data);

	return 0;
}

/**
 * @brief Set the GPO_F direct output value (0x803).
 */
int admv1355_set_gpo_f_direct(Admv1355Dev*dev, uint8_t val)
{
	if (val > 63)
		return -EINVAL;

	return admv1355_spi_update_bits(dev, ADMV1355_REG_GPO_F_DIRECT,
					 ADMV1355_GPO_F_DIRECT_MSK,
					 no_os_field_prep(ADMV1355_GPO_F_DIRECT_MSK, val));
}

/**
 * @brief Get the GPO_F direct output value.
 */
int admv1355_get_gpo_f_direct(Admv1355Dev*dev, uint8_t*val)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_GPO_F_DIRECT, &data);
	if (ret)
		return ret;

	*val = no_os_field_get(ADMV1355_GPO_F_DIRECT_MSK, data);

	return 0;
}

/**
 * @brief Set the GPO_F output-enable mask (0x780).
 */
int admv1355_set_gpo_f_oe(Admv1355Dev*dev, uint8_t val)
{
	if (val > 63)
		return -EINVAL;

	return admv1355_spi_update_bits(dev, ADMV1355_REG_GPO_F_OE,
					 ADMV1355_GPO_F_OE_MSK,
					 no_os_field_prep(ADMV1355_GPO_F_OE_MSK, val));
}

/**
 * @brief Get the GPO_F output-enable mask.
 */
int admv1355_get_gpo_f_oe(Admv1355Dev*dev, uint8_t*val)
{
	uint8_t data;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_GPO_F_OE, &data);
	if (ret)
		return ret;

	*val = no_os_field_get(ADMV1355_GPO_F_OE_MSK, data);

	return 0;
}

/**
 * @brief Set the GPO_G output-enable mask (split across 0x780/0x781).
 */
int admv1355_set_gpo_g_oe(Admv1355Dev*dev, uint8_t val)
{
	int ret;

	if (val > 31)
		return -EINVAL;

	ret = admv1355_spi_update_bits(dev, ADMV1355_REG_GPO_F_OE,
					ADMV1355_GPO_G_OE_LO_MSK,
					no_os_field_prep(ADMV1355_GPO_G_OE_LO_MSK, val & 0x3));
	if (ret)
		return ret;

	return admv1355_spi_update_bits(dev, ADMV1355_REG_GPO_GF_OE,
					 ADMV1355_GPO_G_OE_HI_MSK,
					 no_os_field_prep(ADMV1355_GPO_G_OE_HI_MSK,
							   (val >> 2) & 0x7));
}

/**
 * @brief Get the GPO_G output-enable mask.
 */
int admv1355_get_gpo_g_oe(Admv1355Dev*dev, uint8_t*val)
{
	uint8_t lo, hi;
	int ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_GPO_F_OE, &lo);
	if (ret)
		return ret;

	ret = admv1355_spi_read(dev, ADMV1355_REG_GPO_GF_OE, &hi);
	if (ret)
		return ret;

	*val = no_os_field_get(ADMV1355_GPO_G_OE_LO_MSK, lo) |
	       (no_os_field_get(ADMV1355_GPO_G_OE_HI_MSK, hi) << 2);

	return 0;
}

/**
 * @brief Set the filter state-machine step (0x200).
 */
int admv1355_set_filter_sm_step(Admv1355Dev* dev, uint8_t val)
{
	return admv1355_spi_write(dev, ADMV1355_REG_FILTER_SM_STEP, val);
}

/**
 * @brief Get the filter state-machine step.
 */
int admv1355_get_filter_sm_step(Admv1355Dev* dev, uint8_t*val)
{
	return admv1355_spi_read(dev, ADMV1355_REG_FILTER_SM_STEP, val);
}

/**
 * @brief Read the on-chip temperature sensor.
 * @param dev - The device structure.
 * @param temp_mdeg - Temperature, in milli-degrees Celsius.
 * @return 0 in case of success, negative error code otherwise.
 */
int admv1355_read_temperature(Admv1355Dev* dev, int* temp_mdeg)
{
	uint8_t adc_code;
	int ret;

	ret = admv1355_spi_write(dev, ADMV1355_REG_ADC_RESET, 0x02);
	if (ret)
		return ret;

	ret = admv1355_spi_write(dev, ADMV1355_REG_ADC_CTRL, 0x09);
	if (ret)
		return ret;

	ret = admv1355_spi_write(dev, ADMV1355_REG_LON_CTRL, 0x02);
	if (ret)
		return ret;

	ret = admv1355_spi_write(dev, ADMV1355_REG_RF_HS_PD, 0x00);
	if (ret)
		return ret;

	ret = admv1355_spi_write(dev, ADMV1355_REG_ADC_MUX, 0x01);
	if (ret)
		return ret;

	ret = admv1355_spi_write(dev, ADMV1355_REG_ADC_RESET, 0x06);
	if (ret)
		return ret;

	no_os_udelay(2000);

	ret = admv1355_spi_read(dev, ADMV1355_REG_ADC_OUT, &adc_code);
	if (ret)
		return ret;

	/* TC (m°C) = -(5/6) * ADC_CODE + 173 (datasheet Eq. 3) */
	*temp_mdeg = -5 * (int)adc_code * 1000 / 6 + 173000;

	/* Restore power-down register to the init state */
	admv1355_spi_write(dev, ADMV1355_REG_RF_HS_PD, 0x02);
	admv1355_spi_write(dev, ADMV1355_REG_ADC_CTRL, 0x00);

	return 0;
}

/**
 * @brief Load and verify the on-chip NVM (calibration) shadow registers.
 */
static int admv1355_nvm_init(Admv1355Dev*dev)
{
	static const uint16_t shadow_regs[] = { 0x33, 0x34, 0x37, 0x38 };
	uint8_t addr_rd, data_rd;
	int ret, i;

	ret = admv1355_spi_write(dev, ADMV1355_REG_NVM_CTRL, 0x08);
	if (ret)
		return ret;

	no_os_udelay(2000);

	ret = admv1355_spi_write(dev, ADMV1355_REG_NVM_LOAD, 0x40);
	if (ret)
		return ret;

	no_os_udelay(2000);

	ret = admv1355_spi_write(dev, ADMV1355_REG_NVM_LOAD, 0x54);
	if (ret)
		return ret;

	no_os_udelay(2000);

	ret = admv1355_spi_write(dev, ADMV1355_REG_NVM_CTRL, 0x09);
	if (ret)
		return ret;

	for (i = 0; i < NO_OS_ARRAY_SIZE(shadow_regs); i++) {
		ret = admv1355_spi_write(dev, ADMV1355_REG_NVM_ADDR,
					  shadow_regs[i]);
		if (ret)
			return ret;

		ret = admv1355_spi_read(dev, ADMV1355_REG_NVM_ADDR, &addr_rd);
		if (ret)
			return ret;

		if (addr_rd != shadow_regs[i])
			return -EIO;

		ret = admv1355_spi_read(dev, ADMV1355_REG_NVM_DATA, &data_rd);
		if (ret)
			return ret;

		if (data_rd == 0)
			return -EIO;
	}

	return 0;
}

/**
 * @brief Initialize the ADMV1355 device.
 * @param device - The device structure.
 * @param init_param - The structure containing the device initial parameters.
 * @return 0 in case of success, negative error code otherwise.
 */
int admv1355_init(Admv1355Dev**device,
		   Admv1355InitParam*init_param)
{
	Admv1355Dev*dev;
	uint8_t product_id, scratch;
	int ret;

	dev = (Admv1355Dev*)no_os_calloc(1, sizeof(*dev));
	if (!dev)
		return -ENOMEM;

	ret = no_os_spi_init(&dev->spi_desc, init_param->spi_init);
	if (ret)
		goto error_dev;

	if (init_param->gpio_reset) {
		ret = no_os_gpio_get_optional(&dev->gpio_reset,
					       init_param->gpio_reset);
		if (ret)
			goto error_spi;

		if (dev->gpio_reset) {
			/* RSTB is active-high: pull high to bring the device
			 * out of reset. */
			ret = no_os_gpio_direction_output(dev->gpio_reset,
							   NO_OS_GPIO_LOW);
			if (ret)
				goto error_gpio_reset;

			no_os_udelay(2000);

			ret = no_os_gpio_set_value(dev->gpio_reset,
						    NO_OS_GPIO_HIGH);
			if (ret)
				goto error_gpio_reset;

			no_os_udelay(2000);
		}
	}

	if (init_param->gpio_cen) {
		ret = no_os_gpio_get_optional(&dev->gpio_cen,
					       init_param->gpio_cen);
		if (ret)
			goto error_gpio_reset;

		if (dev->gpio_cen) {
			/* CEN is active-high: pull high after RSTB to enable
			 * the chip before any SPI access. */
			ret = no_os_gpio_direction_output(dev->gpio_cen,
							   NO_OS_GPIO_HIGH);
			if (ret)
				goto error_gpio_cen;

			no_os_udelay(2000);
		}
	}

	/* Soft reset (self-clearing), then enable SDO for 4-wire SPI */
	ret = admv1355_spi_write(dev, ADMV1355_REG_SDO_CTRL, 0x81);
	if (ret)
		goto error_gpio_cen;

	no_os_udelay(2000);

	ret = admv1355_spi_write(dev, ADMV1355_REG_SDO_CTRL, 0x18);
	if (ret)
		goto error_gpio_cen;

	ret = admv1355_spi_read(dev, ADMV1355_REG_PRODUCT_ID_LSB, &product_id);
	if (ret)
		goto error_gpio_cen;

	pr_debug("admv1355: PRODUCT_ID 0x%.2x\n", product_id);

	/* SPI read/write sanity check via the scratch pad register */
	ret = admv1355_spi_write(dev, ADMV1355_REG_SCRATCH_PAD,
				  ADMV1355_SCRATCH_TEST_VAL);
	if (ret)
		goto error_gpio_cen;

	ret = admv1355_spi_read(dev, ADMV1355_REG_SCRATCH_PAD, &scratch);
	if (ret)
		goto error_gpio_cen;

	if (scratch != ADMV1355_SCRATCH_TEST_VAL) {
		ret = -EIO;
		goto error_gpio_cen;
	}

	ret = admv1355_nvm_init(dev);
	if (ret)
		goto error_gpio_cen;

	ret = admv1355_spi_write(dev, ADMV1355_REG_RF_HS_PD, 0x02);
	if (ret)
		goto error_gpio_cen;

	ret = admv1355_spi_write(dev, ADMV1355_REG_FILTER_LUT_EN, 0x00);
	if (ret)
		goto error_gpio_cen;

	ret = admv1355_spi_write(dev, ADMV1355_REG_FILTER_LOAD_EN, 0x00);
	if (ret)
		goto error_gpio_cen;

	ret = admv1355_spi_write(dev, ADMV1355_REG_GAIN_TBL_BYP, 0x01);
	if (ret)
		goto error_gpio_cen;

	ret = admv1355_spi_write(dev, ADMV1355_REG_DSA_BYPASS, 0x00);
	if (ret)
		goto error_gpio_cen;

	ret = admv1355_spi_write(dev, ADMV1355_REG_RF_LPF, 0x10);
	if (ret)
		goto error_gpio_cen;

	ret = admv1355_spi_write(dev, ADMV1355_REG_RF_HPF, 0xBF);
	if (ret)
		goto error_gpio_cen;

	ret = admv1355_spi_write(dev, ADMV1355_REG_GPO_G_DIRECT, 0x08);
	if (ret)
		goto error_gpio_cen;

	ret = admv1355_set_lo_filters(dev, init_param->lo_in);
	if (ret)
		goto error_gpio_cen;

	ret = admv1355_set_if_mode(dev, init_param->if_mode);
	if (ret)
		goto error_gpio_cen;

	ret = admv1355_set_rf_band(dev, init_param->rf_band);
	if (ret)
		goto error_gpio_cen;

	ret = admv1355_set_mixer_sideband(dev, init_param->mixer_sideband);
	if (ret)
		goto error_gpio_cen;

	*device = dev;

	return 0;

error_gpio_cen:
	no_os_gpio_remove(dev->gpio_cen);
error_gpio_reset:
	no_os_gpio_remove(dev->gpio_reset);
error_spi:
	no_os_spi_remove(dev->spi_desc);
error_dev:
	no_os_free(dev);

	return ret;
}

/**
 * @brief Free the resources allocated by admv1355_init().
 * @param dev - The device structure.
 * @return 0 in case of success, negative error code otherwise.
 */
int admv1355_remove(Admv1355Dev*dev)
{
	int ret;

	if (!dev)
		return -ENODEV;

	ret = no_os_gpio_remove(dev->gpio_cen);
	if (ret)
		return ret;

	ret = no_os_gpio_remove(dev->gpio_reset);
	if (ret)
		return ret;

	ret = no_os_spi_remove(dev->spi_desc);
	if (ret)
		return ret;

	no_os_free(dev);

	return 0;
}
