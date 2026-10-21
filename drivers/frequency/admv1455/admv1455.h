#ifndef ADMV1455_H_
#define ADMV1455_H_

#include <stdbool.h>
#include <stdint.h>
#include "no_os_gpio.h"
#include "no_os_spi.h"
#include "no_os_util.h"

#define ADMV1455_REG_SPI_CONTROL 0x000
#define ADMV1455_REG_PRODUCT_ID_LSB 0x004
#define ADMV1455_REG_SCRATCH_PAD 0x00A
#define ADMV1455_REG_ADC_TEMP_READBACK 0x040
#define ADMV1455_REG_ADC_DETECTOR_READBACK 0x041
#define ADMV1455_REG_IF_MODE 0x184
#define ADMV1455_REG_IF_RES_CAL 0x188
#define ADMV1455_REG_FILTER_TABLE_EN 0x202
#define ADMV1455_REG_FILTER_TABLE_SEL 0x207
#define ADMV1455_REG_FILTER_LOAD_EN 0x208
#define ADMV1455_REG_EN_LOAD_G 0x281
#define ADMV1455_REG_GAIN_SPI_POINTER 0x282
#define ADMV1455_REG_GAIN_TABLE_EN 0x285
#define ADMV1455_REG_GAIN_LUT_MODE 0x286
#define ADMV1455_REG_BYPASS_GAIN_EN 0x28A
#define ADMV1455_REG_BYPASS_DSA1_DSA2 0x28B
#define ADMV1455_REG_BYPASS_DSA3_DSA4 0x28C
#define ADMV1455_REG_BYPASS_DSA5 0x28D
#define ADMV1455_REG_BYPASS_GPO_G 0x28E
#define ADMV1455_REG_IF_BPF_FINE 0x2A2
#define ADMV1455_REG_DIRECT_DSA1_DSA2 0x600
#define ADMV1455_REG_DIRECT_DSA3_DSA4 0x601
#define ADMV1455_REG_DIRECT_DSA5 0x602
#define ADMV1455_REG_DIRECT_GPO_G 0x603
#define ADMV1455_REG_GPO_FG_OE_LSB 0x780
#define ADMV1455_REG_GPO_FG_OE_MSB 0x781
#define ADMV1455_REG_DIRECT_0x800 0x800
#define ADMV1455_REG_DIRECT_0x801 0x801
#define ADMV1455_REG_DIRECT_0x802 0x802
#define ADMV1455_REG_DIRECT_0x803 0x803
#define ADMV1455_REG_DIRECT_0x804 0x804
#define ADMV1455_REG_DIRECT_GPO_F 0x805
#define ADMV1455_REG_DIRECT_DSA_OFF_12 0x806
#define ADMV1455_REG_DIRECT_DSA_OFF_34 0x807
#define ADMV1455_REG_DIRECT_DSA_OFF_5 0x808
#define ADMV1455_REG_DIRECT_LO_PHASE_Q 0x809
#define ADMV1455_REG_DIRECT_LO_PHASE_I 0x80A
#define ADMV1455_REG_DIRECT_DSA_IQ 0x80B
#define ADMV1455_REG_DIRECT_LO_DBLR 0x80C
#define ADMV1455_REG_FILTER_LUTA_BASE 0x900
#define ADMV1455_REG_FILTER_LUTB_BASE 0xAA0
#define ADMV1455_REG_GAIN_LUT_BASE 0xE00

#define ADMV1455_IF_RES_CAL_MSK NO_OS_GENMASK(3, 0)
#define ADMV1455_IF_BPF_FINE_MSK NO_OS_GENMASK(4, 0)
#define ADMV1455_DSA1_MSK NO_OS_GENMASK(3, 0)
#define ADMV1455_DSA2_MSK NO_OS_BIT(4)
#define ADMV1455_DSA3_MSK NO_OS_GENMASK(3, 0)
#define ADMV1455_DSA4_MSK NO_OS_GENMASK(7, 4)
#define ADMV1455_DSA5_MSK NO_OS_GENMASK(3, 0)
#define ADMV1455_GPO_G_MSK NO_OS_GENMASK(4, 0)
#define ADMV1455_DIRECT_RF_BAND_MSK NO_OS_BIT(7)
#define ADMV1455_DIRECT_IF_SUBBAND_MSK NO_OS_GENMASK(6, 5)
#define ADMV1455_DIRECT_LO_X1_FILTER_MSK NO_OS_GENMASK(4, 0)
#define ADMV1455_DIRECT_SIDEBAND_MSK NO_OS_BIT(7)
#define ADMV1455_DIRECT_RF_SUBBAND_MSK NO_OS_GENMASK(6, 5)
#define ADMV1455_DIRECT_LO_X3_FILTER_MSK NO_OS_GENMASK(4, 0)
#define ADMV1455_DIRECT_RF_LPF_MSK NO_OS_GENMASK(7, 4)
#define ADMV1455_DIRECT_LO_X4_FILTER_MSK NO_OS_GENMASK(3, 0)
#define ADMV1455_DIRECT_LO_TRAP_LSB_MSK NO_OS_GENMASK(7, 0)
#define ADMV1455_DIRECT_RF_HPF_MSK NO_OS_GENMASK(7, 4)
#define ADMV1455_DIRECT_BPF_COARSE_MSK NO_OS_GENMASK(3, 2)
#define ADMV1455_DIRECT_LO_TRAP_MSB_MSK NO_OS_GENMASK(1, 0)
#define ADMV1455_DIRECT_GPO_F_MSK NO_OS_GENMASK(5, 0)
#define ADMV1455_DIRECT_DSA1_OFF_MSK NO_OS_GENMASK(3, 0)
#define ADMV1455_DIRECT_DSA2_OFF_MSK NO_OS_GENMASK(5, 4)
#define ADMV1455_DIRECT_DSA3_OFF_MSK NO_OS_GENMASK(3, 0)
#define ADMV1455_DIRECT_DSA4_OFF_MSK NO_OS_GENMASK(7, 4)
#define ADMV1455_DIRECT_DSA5_OFF_MSK NO_OS_GENMASK(3, 0)
#define ADMV1455_DIRECT_LO_PHASE_Q_MSK NO_OS_GENMASK(4, 0)
#define ADMV1455_DIRECT_LO_BAND_MSK NO_OS_BIT(5)
#define ADMV1455_DIRECT_LO_PHASE_I_MSK NO_OS_GENMASK(4, 0)
#define ADMV1455_DIRECT_DSAI_MSK NO_OS_GENMASK(3, 0)
#define ADMV1455_DIRECT_DSAQ_MSK NO_OS_GENMASK(7, 4)
#define ADMV1455_DIRECT_LO_DBLR_MSK NO_OS_GENMASK(7, 0)
#define ADMV1455_GAIN_LUT_MODE_MSK NO_OS_GENMASK(1, 0)
#define ADMV1455_GAIN_SPI_POINTER_MSK NO_OS_GENMASK(6, 0)
#define ADMV1455_GPO_F_OE_MSK NO_OS_GENMASK(5, 0)
#define ADMV1455_GPO_G_OE_LSB_MSK NO_OS_GENMASK(7, 6)
#define ADMV1455_GPO_G_OE_MSB_MSK NO_OS_GENMASK(2, 0)

#define ADMV1455_FILTER_LUT_NUM_ENTRIES 32
#define ADMV1455_FILTER_LUT_ENTRY_SIZE 13
#define ADMV1455_FILTER_LUT_CONFIG_SIZE 4096
#define ADMV1455_GAIN_LUT_NUM_ENTRIES 67
#define ADMV1455_GAIN_LUT_ENTRY_SIZE 4
#define ADMV1455_GAIN_LUT_CONFIG_SIZE 4096
#define ADMV1455_SCRATCH_TEST_VAL 0xA5
#define ADMV1455_PRODUCT_ID 0x66
#define ADMV1455_SPI_SOFT_RESET 0x81
#define ADMV1455_SPI_4WIRE 0x18

typedef struct admv1455_dev {
	struct no_os_spi_desc* spi_desc;
	struct no_os_gpio_desc* gpio_reset;
	struct no_os_gpio_desc* gpio_cen;
	uint8_t chip_addr;
} Admv1455Dev;

typedef struct admv1455_init_param {
	struct no_os_spi_init_param* spi_init;
	struct no_os_gpio_init_param* gpio_reset;
	struct no_os_gpio_init_param* gpio_cen;
	uint8_t chip_addr;
	bool spi3wire;
} Admv1455InitParam;

typedef enum admv1455_lut {
	ADMV1455_FILTER_LUTA,
	ADMV1455_FILTER_LUTB,
	ADMV1455_GAIN_LUT,
} Admv1455Lut;

int admv1455_spi_read(Admv1455Dev* dev, uint16_t reg, uint8_t* value);
int admv1455_spi_write(Admv1455Dev* dev, uint16_t reg, uint8_t value);
int admv1455_spi_update_bits(Admv1455Dev* dev, uint16_t reg, uint8_t mask,
			     uint8_t value);
int admv1455_get_rf_band(Admv1455Dev* dev, uint8_t* band);
int admv1455_set_rf_band(Admv1455Dev* dev, uint8_t band);
int admv1455_get_mixer_sideband(Admv1455Dev* dev, uint8_t* sideband);
int admv1455_set_mixer_sideband(Admv1455Dev* dev, uint8_t sideband);
int admv1455_get_if_subband(Admv1455Dev* dev, uint8_t* subband);
int admv1455_set_if_subband(Admv1455Dev* dev, uint8_t subband);
int admv1455_get_rf_dsa(Admv1455Dev* dev, uint8_t* dsa1, uint8_t* dsa2,
			uint8_t* dsa3);
int admv1455_set_rf_dsa(Admv1455Dev* dev, uint8_t dsa1, uint8_t dsa2,
			uint8_t dsa3);
int admv1455_get_if_dsa(Admv1455Dev* dev, uint8_t* dsa4, uint8_t* dsa5);
int admv1455_set_if_dsa(Admv1455Dev* dev, uint8_t dsa4, uint8_t dsa5);
int admv1455_get_rf_dsa_bypass(Admv1455Dev* dev, uint8_t* dsa1, uint8_t* dsa2,
			       uint8_t* dsa3);
int admv1455_set_rf_dsa_bypass(Admv1455Dev* dev, uint8_t dsa1, uint8_t dsa2,
			       uint8_t dsa3);
int admv1455_get_if_dsa_bypass(Admv1455Dev* dev, uint8_t* dsa4, uint8_t* dsa5);
int admv1455_set_if_dsa_bypass(Admv1455Dev* dev, uint8_t dsa4, uint8_t dsa5);
int admv1455_get_if_iq_dsa(Admv1455Dev* dev, uint8_t* dsai, uint8_t* dsaq);
int admv1455_set_if_iq_dsa(Admv1455Dev* dev, uint8_t dsai, uint8_t dsaq);
int admv1455_get_rf_dsa_offset(Admv1455Dev* dev, uint8_t* dsa1, uint8_t* dsa2,
			       uint8_t* dsa3);
int admv1455_set_rf_dsa_offset(Admv1455Dev* dev, uint8_t dsa1, uint8_t dsa2,
			       uint8_t dsa3);
int admv1455_get_if_dsa_offset(Admv1455Dev* dev, uint8_t* dsa4, uint8_t* dsa5);
int admv1455_set_if_dsa_offset(Admv1455Dev* dev, uint8_t dsa4, uint8_t dsa5);
int admv1455_get_rf_direct_filter(Admv1455Dev* dev, uint8_t* lpf, uint8_t* hpf);
int admv1455_set_rf_direct_filter(Admv1455Dev* dev, uint8_t lpf, uint8_t hpf);
int admv1455_get_lo_phase(Admv1455Dev* dev, uint8_t* phase_i, uint8_t* phase_q);
int admv1455_set_lo_phase(Admv1455Dev* dev, uint8_t phase_i, uint8_t phase_q);
int admv1455_get_lo_band(Admv1455Dev* dev, uint8_t* band);
int admv1455_set_lo_band(Admv1455Dev* dev, uint8_t band);
int admv1455_get_lo_x1_filter(Admv1455Dev* dev, uint8_t* value);
int admv1455_set_lo_x1_filter(Admv1455Dev* dev, uint8_t value);
int admv1455_get_lo_x3_filter(Admv1455Dev* dev, uint8_t* value);
int admv1455_set_lo_x3_filter(Admv1455Dev* dev, uint8_t value);
int admv1455_get_lo_x4_filter(Admv1455Dev* dev, uint8_t* value);
int admv1455_set_lo_x4_filter(Admv1455Dev* dev, uint8_t value);
int admv1455_get_lo_trap(Admv1455Dev* dev, uint16_t* value);
int admv1455_set_lo_trap(Admv1455Dev* dev, uint16_t value);
int admv1455_get_lo_dbl_band(Admv1455Dev* dev, uint8_t* value);
int admv1455_set_lo_dbl_band(Admv1455Dev* dev, uint8_t value);
int admv1455_get_filter_load_en(Admv1455Dev* dev, bool* enable);
int admv1455_set_filter_load_en(Admv1455Dev* dev, bool enable);
int admv1455_get_filter_table_sel(Admv1455Dev* dev, uint8_t* table);
int admv1455_set_filter_table_sel(Admv1455Dev* dev, uint8_t table);
int admv1455_get_gain_lut_en(Admv1455Dev* dev, bool* enable);
int admv1455_set_gain_lut_en(Admv1455Dev* dev, bool enable);
int admv1455_get_gain_lut_bypass_en(Admv1455Dev* dev, bool* enable);
int admv1455_set_gain_lut_bypass_en(Admv1455Dev* dev, bool enable);
int admv1455_get_gain_load_en(Admv1455Dev* dev, bool* enable);
int admv1455_set_gain_load_en(Admv1455Dev* dev, bool enable);
int admv1455_get_gain_lut_mode(Admv1455Dev* dev, uint8_t* mode);
int admv1455_set_gain_lut_mode(Admv1455Dev* dev, uint8_t mode);
int admv1455_get_gain_spi_pointer(Admv1455Dev* dev, uint8_t* pointer);
int admv1455_set_gain_spi_pointer(Admv1455Dev* dev, uint8_t pointer);
int admv1455_get_gpo_f_direct(Admv1455Dev* dev, uint8_t* value);
int admv1455_set_gpo_f_direct(Admv1455Dev* dev, uint8_t value);
int admv1455_get_gpo_g_direct(Admv1455Dev* dev, uint8_t* value);
int admv1455_set_gpo_g_direct(Admv1455Dev* dev, uint8_t value);
int admv1455_get_gpo_g_bypass(Admv1455Dev* dev, uint8_t* value);
int admv1455_set_gpo_g_bypass(Admv1455Dev* dev, uint8_t value);
int admv1455_get_gpo_f_oe(Admv1455Dev* dev, uint8_t* value);
int admv1455_set_gpo_f_oe(Admv1455Dev* dev, uint8_t value);
int admv1455_get_gpo_g_oe(Admv1455Dev* dev, uint8_t* value);
int admv1455_set_gpo_g_oe(Admv1455Dev* dev, uint8_t value);
int admv1455_read_temperature(Admv1455Dev* dev, uint8_t* value);
int admv1455_read_detector(Admv1455Dev* dev, uint8_t* value);
int admv1455_read_lut_entry(Admv1455Dev* dev, Admv1455Lut table, uint8_t index,
			    uint8_t* data);
int admv1455_write_lut_entry(Admv1455Dev* dev, Admv1455Lut table, uint8_t index,
			     const uint8_t* data);
int admv1455_init(Admv1455Dev** device, const Admv1455InitParam* param);
int admv1455_remove(Admv1455Dev* dev);

#endif