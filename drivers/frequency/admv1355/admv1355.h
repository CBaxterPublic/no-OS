/***************************************************************************//**
 *   @file   admv1355.h
 *   @brief  Header file for admv1355 Driver.
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
#ifndef ADMV1355_H_
#define ADMV1355_H_

#include <stdint.h>
#include <stdbool.h>
#include "no_os_spi.h"
#include "no_os_gpio.h"
#include "no_os_util.h"

/* ADMV1355 Register Map (12-bit addresses) */
#define ADMV1355_REG_SDO_CTRL		0x000
#define ADMV1355_REG_PRODUCT_ID_LSB	0x004
#define ADMV1355_REG_SCRATCH_PAD	0x00A
#define ADMV1355_REG_NVM_CTRL		0x078
#define ADMV1355_REG_NVM_ADDR		0x07B
#define ADMV1355_REG_NVM_DATA		0x07C
#define ADMV1355_REG_NVM_LOAD		0x07E
#define ADMV1355_REG_RF_HS_PD		0x13E
#define ADMV1355_REG_FILTER_SM_STEP	0x200
#define ADMV1355_REG_FILTER_LUT_EN	0x202
#define ADMV1355_REG_ADC_MUX		0x2A7
#define ADMV1355_REG_ADC_CTRL		0x300
#define ADMV1355_REG_ADC_RESET		0x302
#define ADMV1355_REG_LON_CTRL		0x306
#define ADMV1355_REG_ADC_OUT		0x30D
#define ADMV1355_REG_FILTER_TBL_SEL	0x207
#define ADMV1355_REG_FILTER_LOAD_EN	0x208
#define ADMV1355_REG_GAIN_LOAD_EN	0x281
#define ADMV1355_REG_GAIN_LUT_EN	0x285
#define ADMV1355_REG_GAIN_TBL_BYP	0x28A
#define ADMV1355_REG_DSA_BYPASS		0x28B
#define ADMV1355_REG_GPO_G_BYPASS	0x28C
#define ADMV1355_REG_RF_LPF			0x2A0
#define ADMV1355_REG_RF_HPF			0x2A1
#define ADMV1355_REG_DSA_DIRECT		0x600
#define ADMV1355_REG_GPO_G_DIRECT	0x601
#define ADMV1355_REG_IF_ENABLE		0x602
#define ADMV1355_REG_GPO_F_OE		0x780
#define ADMV1355_REG_GPO_GF_OE		0x781
#define ADMV1355_REG_GPO_F_DIRECT	0x803
#define ADMV1355_REG_LO_X4_FILTER	0x800
#define ADMV1355_REG_LO_X3_FILTER	0x801
#define ADMV1355_REG_LO_X1_FILTER	0x802
#define ADMV1355_REG_LO_TRAP_7_0	0x804
#define ADMV1355_REG_LO_TRAP_9_8	0x805
#define ADMV1355_REG_LO_BAND		0x806
#define ADMV1355_REG_LON_OFFSET_I	0x808
#define ADMV1355_REG_LON_OFFSET_Q	0x809
#define ADMV1355_REG_IF_GAIN		0x80A
#define ADMV1355_REG_RF_FILTER		0x80B
#define ADMV1355_REG_LO_DBL_BAND	0x80C

#define ADMV1355_REG_FILTER_LUTA_BASE	0x900
#define ADMV1355_REG_FILTER_LUTB_BASE	0xAA0
#define ADMV1355_REG_GAIN_LUT_BASE		0xE00

#define ADMV1355_FILTER_LUT_ENTRY_SIZE	13
#define ADMV1355_FILTER_LUT_NUM_ENTRIES	32
#define ADMV1355_FILTER_LUT_CONFIG_SIZE	4096

#define ADMV1355_GAIN_LUT_ENTRY_SIZE	2
#define ADMV1355_GAIN_LUT_NUM_ENTRIES	67
#define ADMV1355_GAIN_LUT_CONFIG_SIZE	2048

/* 0x28A: GAIN_LUT_BYPASS_EN */
#define ADMV1355_GAIN_LUT_BYP_EN_MSK	NO_OS_BIT(0)

/* 0x28B: DSA2_BYPASS_VALUE[7:4], DSA1_BYPASS_VALUE[3:0] */
#define ADMV1355_DSA1_BYPASS_MSK	NO_OS_GENMASK(3, 0)
#define ADMV1355_DSA2_BYPASS_MSK	NO_OS_GENMASK(7, 4)

/* 0x2A0: LPF_SELECT[4], LPF_BYPASS_VALUE[3:0] */
#define ADMV1355_LPF_SELECT_MSK	NO_OS_BIT(4)
#define ADMV1355_LPF_BYPASS_VAL_MSK	NO_OS_GENMASK(3, 0)

/* 0x2A1: HPF_SELECT[7], HPF_BYPASS_VALUE[5:0] */
#define ADMV1355_HPF_SELECT_MSK	NO_OS_BIT(7)
#define ADMV1355_HPF_BYPASS_VAL_MSK	NO_OS_GENMASK(5, 0)

/* 0x600: RF_DSA2_GAIN[7:4], RF_DSA1_GAIN[3:0] */
#define ADMV1355_DSA1_DIRECT_MSK	NO_OS_GENMASK(3, 0)
#define ADMV1355_DSA2_DIRECT_MSK	NO_OS_GENMASK(7, 4)

/* 0x602: IF_HYBRID_EN[3], IF_HIGH_FREQ_EN[2], IF_LOW_FREQ_EN[1], BB_EN[0] */
#define ADMV1355_BB_EN_MSK		NO_OS_BIT(0)
#define ADMV1355_IF_LOW_FREQ_EN_MSK	NO_OS_BIT(1)
#define ADMV1355_IF_HIGH_FREQ_EN_MSK	NO_OS_BIT(2)
#define ADMV1355_IF_HYBRID_EN_MSK	NO_OS_BIT(3)
#define ADMV1355_IF_MODE_MSK		NO_OS_GENMASK(3, 0)

/* 0x805: RF_BAND_SELECT[7], LO_PHASE_Q[6:2], LO_TRAP_FILTER[9:8] at [1:0] */
#define ADMV1355_RF_BAND_805_MSK	NO_OS_BIT(7)
#define ADMV1355_LO_PHASE_Q_MSK	NO_OS_GENMASK(6, 2)
#define ADMV1355_LO_TRAP_9_8_MSK	NO_OS_GENMASK(1, 0)

/* 0x806: LO_BAND[7], MIXER_BAND[6], LO_SIDEBAND[5], LO_PHASE_I[4:0] */
#define ADMV1355_LO_BAND_806_MSK	NO_OS_BIT(7)
#define ADMV1355_MIXER_BAND_806_MSK	NO_OS_BIT(6)
#define ADMV1355_LO_SIDEBAND_MSK	NO_OS_BIT(5)
#define ADMV1355_LO_PHASE_I_MSK	NO_OS_GENMASK(4, 0)

/* 0x80A: DSAQ[7:4], DSAI[3:0] */
#define ADMV1355_DSAI_MSK		NO_OS_GENMASK(3, 0)
#define ADMV1355_DSAQ_MSK		NO_OS_GENMASK(7, 4)

/* 0x80B: RF_FILTER: HPF_DIRECT[7:4], LPF_DIRECT[3:0] */
#define ADMV1355_DIRECT_LPF_MSK	NO_OS_GENMASK(3, 0)
#define ADMV1355_DIRECT_HPF_MSK	NO_OS_GENMASK(7, 4)

/* 0x601: GPO_G_DIRECT[4:0] */
#define ADMV1355_GPO_G_DIRECT_MSK	NO_OS_GENMASK(4, 0)

/* 0x28C: GPO_G_BYPASS[4:0] */
#define ADMV1355_GPO_G_BYPASS_MSK	NO_OS_GENMASK(4, 0)

/* 0x780: GPO_G_OE[7:6], GPO_F_OE[5:0] */
#define ADMV1355_GPO_F_OE_MSK		NO_OS_GENMASK(5, 0)
#define ADMV1355_GPO_G_OE_LO_MSK	NO_OS_GENMASK(7, 6)

/* 0x781: GPO_G_OE[2:0] (bits [4:2] of the full GPO_G_OE) */
#define ADMV1355_GPO_G_OE_HI_MSK	NO_OS_GENMASK(2, 0)

/* 0x803: GPO_F_DIRECT[5:0] */
#define ADMV1355_GPO_F_DIRECT_MSK	NO_OS_GENMASK(5, 0)

/* Miscellaneous Defines */
#define ADMV1355_BUFF_SIZE_BYTES	3
#define ADMV1355_SPI_READ_CMD		NO_OS_BIT(7)
#define ADMV1355_SPI_WRITE_CMD		0x00
#define ADMV1355_SCRATCH_TEST_VAL	0xA5

/**
 * @enum admv1355_rf_band
 * @brief RF band select (low/high, controls 0x805[7] and 0x806[6])
 */
typedef enum admv1355_rf_band {
	low_band,
	high_band,
	ADMV1355_RF_BAND_LOW = low_band,
	ADMV1355_RF_BAND_HIGH = high_band,
} Admv1355RfBand;

/**
 * @enum admv1355_mixer_sideband
 * @brief Mixer sideband selection (0x806[5])
 */
typedef enum admv1355_mixer_sideband {
	ADMV1355_SIDEBAND_LSB,
	ADMV1355_SIDEBAND_USB,
} Admv1355MixerSideband;

/**
 * @enum admv1355_if_mode
 * @brief Frequency translation / IF mode (0x602[3:0])
 */
typedef enum admv1355_if_mode {
	ADMV1355_IF_MODE_BASEBAND,
	ADMV1355_IF_MODE_COMPLEX_IF_LOW,
	ADMV1355_IF_MODE_COMPLEX_IF_HIGH,
	ADMV1355_IF_MODE_HYBRID,
} Admv1355IfMode;

/**
 * @struct admv1355_init_param
 * @brief ADMV1355 Initialization Parameters structure.
 */
typedef struct admv1355_init_param {
	/** SPI Initialization parameters */
	struct no_os_spi_init_param*spi_init;
	/** RESET GPIO Initialization parameters (optional, active low) */
	struct no_os_gpio_init_param*gpio_reset;
	/** Chip Enable GPIO Initialization parameters (optional, active high) */
	struct no_os_gpio_init_param*gpio_cen;
	/** LO input frequency, in Hz (used to select the LO chain filters) */
	unsigned long long		lo_in;
	/** Frequency translation / IF mode */
	Admv1355IfMode		if_mode;
	/** RF band select */
	Admv1355RfBand		rf_band;
	/** Mixer sideband select */
	Admv1355MixerSideband	mixer_sideband;
} Admv1355InitParam;

/**
 * @struct admv1355_dev
 * @brief ADMV1355 Device Descriptor.
 */
typedef struct admv1355_dev {
	/** SPI Descriptor */
	struct no_os_spi_desc*spi_desc;
	/** RESET GPIO Descriptor */
	struct no_os_gpio_desc*gpio_reset;
	/** Chip Enable GPIO Descriptor */
	struct no_os_gpio_desc*gpio_cen;
	/** LO input frequency, in Hz */
	unsigned long long		lo_in;
	/** Frequency translation / IF mode */
	Admv1355IfMode		if_mode;
	/** RF band select */
	Admv1355RfBand		rf_band;
	/** Mixer sideband select */
	Admv1355MixerSideband	mixer_sideband;
} Admv1355Dev;

int admv1355_spi_write(Admv1355Dev* dev, uint16_t reg_addr, uint8_t data);

int admv1355_spi_read(Admv1355Dev* dev, uint16_t reg_addr, uint8_t* data);

int admv1355_spi_update_bits(Admv1355Dev* dev, uint16_t reg_addr, uint8_t mask, uint8_t data);

int admv1355_set_rf_band(Admv1355Dev* dev, Admv1355RfBand rf_band);

int admv1355_get_rf_band(Admv1355Dev* dev, Admv1355RfBand* rf_band);

int admv1355_set_mixer_sideband(Admv1355Dev* dev, Admv1355MixerSideband sideband);

int admv1355_get_mixer_sideband(Admv1355Dev* dev, Admv1355MixerSideband* sideband);

int admv1355_set_if_mode(Admv1355Dev* dev, Admv1355IfMode if_mode);

int admv1355_get_if_mode(Admv1355Dev* dev, Admv1355IfMode* if_mode);

/** Set the RF chain DSA1/DSA2 direct attenuation (0 - 15, 1dB/step) */
int admv1355_set_rf_dsa(Admv1355Dev* dev, uint8_t dsa1, uint8_t dsa2);

int admv1355_get_rf_dsa(Admv1355Dev* dev, uint8_t* dsa1, uint8_t* dsa2);

/** Set the IF chain fine I/Q attenuation (0 - 15, 0.1dB/step) */
int admv1355_set_if_dsa(Admv1355Dev* dev, uint8_t dsai, uint8_t dsaq);

int admv1355_get_if_dsa(Admv1355Dev* dev, uint8_t* dsai, uint8_t* dsaq);

/** Set the LO I/Q phase adjust (0 - 31 each) */
int admv1355_set_lo_phase(Admv1355Dev* dev, uint8_t phase_i, uint8_t phase_q);

/** Get the LO I/Q phase adjust */
int admv1355_get_lo_phase(Admv1355Dev* dev, uint8_t* phase_i, uint8_t* phase_q);

/** Select the LO chain filters (LO x1/x3/x4 multiplier and trap filters) for
 *  a given LO input frequency, in Hz */
int admv1355_set_lo_filters(Admv1355Dev* dev, unsigned long long lo_in);

int admv1355_read_temperature(Admv1355Dev* dev, int*temp_mdegcel);

/** Set the RF chain DSA1/DSA2 bypass-mode attenuation (0 - 15, 1dB/step) */
int admv1355_set_rf_dsa_bypass(Admv1355Dev* dev, uint8_t dsa1, uint8_t dsa2);

/** Get the RF chain DSA1/DSA2 bypass-mode attenuation */
int admv1355_get_rf_dsa_bypass(Admv1355Dev* dev, uint8_t* dsa1, uint8_t* dsa2);

/** Set the RF LPF bypass enable and bypass-mode value (0 - 15) */
int admv1355_set_rf_lpf_bypass(Admv1355Dev* dev, bool enable, uint8_t value);

/** Get the RF LPF bypass enable and bypass-mode value */
int admv1355_get_rf_lpf_bypass(Admv1355Dev* dev, bool*enable, uint8_t* value);

/** Set the RF HPF bypass enable and bypass-mode value (0 - 63) */
int admv1355_set_rf_hpf_bypass(Admv1355Dev* dev, bool enable, uint8_t value);

/** Get the RF HPF bypass enable and bypass-mode value */
int admv1355_get_rf_hpf_bypass(Admv1355Dev* dev, bool* enable, uint8_t* value);

/** Set the RF direct LPF/HPF filter select values */
int admv1355_set_rf_direct_filter(Admv1355Dev* dev, uint8_t lpf_val, uint8_t hpf_val);

/** Get the RF direct LPF/HPF filter select values */
int admv1355_get_rf_direct_filter(Admv1355Dev* dev, uint8_t* lpf_val, uint8_t* hpf_val);

/** Set the raw LO x4/x3/x1 multiplier filter register values */
int admv1355_set_lo_x4_filter(Admv1355Dev* dev, uint8_t val);
int admv1355_get_lo_x4_filter(Admv1355Dev* dev, uint8_t* val);
int admv1355_set_lo_x3_filter(Admv1355Dev* dev, uint8_t val);
int admv1355_get_lo_x3_filter(Admv1355Dev* dev, uint8_t* val);
int admv1355_set_lo_x1_filter(Admv1355Dev* dev, uint8_t val);
int admv1355_get_lo_x1_filter(Admv1355Dev* dev, uint8_t* val);

/** Set/get the LO trap filter (10-bit) */
int admv1355_set_lo_trap(Admv1355Dev* dev, uint16_t val);
int admv1355_get_lo_trap(Admv1355Dev* dev, uint16_t* val);

/** Set/get the LO doubler band (8-bit) */
int admv1355_set_lo_dbl_band(Admv1355Dev* dev, uint8_t val);
int admv1355_get_lo_dbl_band(Admv1355Dev* dev, uint8_t* val);

/** Set/get the LON offset I/Q (8-bit each) */
int admv1355_set_lon_offset(Admv1355Dev* dev, uint8_t offset_i, uint8_t offset_q);
int admv1355_get_lon_offset(Admv1355Dev* dev, uint8_t* offset_i, uint8_t* offset_q);

/** Set/get the filter LUT enable */
int admv1355_set_filter_lut_en(Admv1355Dev* dev, bool enable);
int admv1355_get_filter_lut_en(Admv1355Dev* dev, bool* enable);

/** Set/get the filter LUT load enable */
int admv1355_set_filter_load_en(Admv1355Dev* dev, bool enable);
int admv1355_get_filter_load_en(Admv1355Dev* dev, bool* enable);

/** Set/get the filter LUT table select (0 = A, 1 = B) */
int admv1355_set_filter_table_sel(Admv1355Dev* dev, uint8_t table);
int admv1355_get_filter_table_sel(Admv1355Dev* dev, uint8_t* table);

/** Set/get the gain LUT enable */
int admv1355_set_gain_lut_en(Admv1355Dev* dev, bool enable);
int admv1355_get_gain_lut_en(Admv1355Dev* dev, bool* enable);

/** Set/get the gain LUT bypass enable */
int admv1355_set_gain_lut_bypass_en(Admv1355Dev* dev, bool enable);
int admv1355_get_gain_lut_bypass_en(Admv1355Dev* dev, bool* enable);

/** Set/get the gain LUT load enable */
int admv1355_set_gain_load_en(Admv1355Dev* dev, bool enable);
int admv1355_get_gain_load_en(Admv1355Dev* dev, bool* enable);

/** Set/get the GPO_G direct output value (0 - 31) */
int admv1355_set_gpo_g_direct(Admv1355Dev* dev, uint8_t val);
int admv1355_get_gpo_g_direct(Admv1355Dev* dev, uint8_t* val);

/** Set/get the GPO_G bypass-mode value (0 - 31) */
int admv1355_set_gpo_g_bypass(Admv1355Dev* dev, uint8_t val);
int admv1355_get_gpo_g_bypass(Admv1355Dev* dev, uint8_t* val);

/** Set/get the GPO_F direct output value (0 - 63) */
int admv1355_set_gpo_f_direct(Admv1355Dev* dev, uint8_t val);
int admv1355_get_gpo_f_direct(Admv1355Dev* dev, uint8_t* val);

/** Set/get the GPO_F output-enable mask (0 - 63) */
int admv1355_set_gpo_f_oe(Admv1355Dev* dev, uint8_t val);
int admv1355_get_gpo_f_oe(Admv1355Dev* dev, uint8_t* val);

/** Set/get the GPO_G output-enable mask (0 - 31) */
int admv1355_set_gpo_g_oe(Admv1355Dev* dev, uint8_t val);
int admv1355_get_gpo_g_oe(Admv1355Dev* dev, uint8_t* val);

int admv1355_set_filter_sm_step(Admv1355Dev* dev, uint8_t val);
int admv1355_get_filter_sm_step(Admv1355Dev* dev, uint8_t* val);

int admv1355_init(Admv1355Dev** device, Admv1355InitParam* init_param);

int admv1355_remove(Admv1355Dev* dev);

#endif /* ADMV1355_H_ */
