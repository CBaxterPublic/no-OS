/***************************************************************************//**
 *   @file   iio_admv1355.h
 *   @brief  Header file for admv1355 IIO Driver.
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
#ifndef IIO_ADMV1355_H
#define IIO_ADMV1355_H

#include "iio.h"
#include "admv1355.h"

typedef struct iio_channel IioChannel;
typedef struct iio_device IioDevice;
typedef struct iio_attribute IioAttribute;
typedef struct iio_ch_info IioChInfo;

typedef struct admv1355_iio_dev {
	Admv1355Dev* admv1355_dev;
	IioDevice* iio_dev;
} Admv1355IioDev;

typedef struct admv1355_iio_dev_init_param {
	Admv1355InitParam* admv1355_dev_init;
} Admv1355IioDevInitParam;

typedef enum admv1355_iio_dev_attr_id {
	ADMV1355_IIO_DEV_ATTR_RF_BAND,
	ADMV1355_IIO_DEV_ATTR_MIXER_SIDEBAND,
	ADMV1355_IIO_DEV_ATTR_IF_MODE,
	ADMV1355_IIO_DEV_ATTR_RF_DSA1,
	ADMV1355_IIO_DEV_ATTR_RF_DSA2,
	ADMV1355_IIO_DEV_ATTR_IF_DSAI,
	ADMV1355_IIO_DEV_ATTR_IF_DSAQ,
	ADMV1355_IIO_DEV_ATTR_LO_PHASE_I,
	ADMV1355_IIO_DEV_ATTR_LO_PHASE_Q,
	ADMV1355_IIO_DEV_ATTR_LO_IN,
	ADMV1355_IIO_DEV_ATTR_TEMP,
	ADMV1355_IIO_DEV_ATTR_RF_DSA1_BYPASS,
	ADMV1355_IIO_DEV_ATTR_RF_DSA2_BYPASS,
	ADMV1355_IIO_DEV_ATTR_RF_BYPASS_LPF_EN,
	ADMV1355_IIO_DEV_ATTR_RF_BYPASS_LPF_VAL,
	ADMV1355_IIO_DEV_ATTR_RF_BYPASS_HPF_EN,
	ADMV1355_IIO_DEV_ATTR_RF_BYPASS_HPF_VAL,
	ADMV1355_IIO_DEV_ATTR_RF_DIRECT_LPF_VAL,
	ADMV1355_IIO_DEV_ATTR_RF_DIRECT_HPF_VAL,
	ADMV1355_IIO_DEV_ATTR_LO_X4_FILTER,
	ADMV1355_IIO_DEV_ATTR_LO_X3_FILTER,
	ADMV1355_IIO_DEV_ATTR_LO_X1_FILTER,
	ADMV1355_IIO_DEV_ATTR_LO_TRAP,
	ADMV1355_IIO_DEV_ATTR_LO_DBL_BAND,
	ADMV1355_IIO_DEV_ATTR_LON_OFFSET_I,
	ADMV1355_IIO_DEV_ATTR_LON_OFFSET_Q,
	ADMV1355_IIO_DEV_ATTR_FILTER_LUT_EN,
	ADMV1355_IIO_DEV_ATTR_FILTER_LOAD_EN,
	ADMV1355_IIO_DEV_ATTR_FILTER_TABLE_SEL,
	ADMV1355_IIO_DEV_ATTR_GAIN_LUT_EN,
	ADMV1355_IIO_DEV_ATTR_GAIN_LUT_BYPASS_EN,
	ADMV1355_IIO_DEV_ATTR_GAIN_LOAD_EN,
	ADMV1355_IIO_DEV_ATTR_DIRECT_GPO_G,
	ADMV1355_IIO_DEV_ATTR_BYPASS_GPO_G,
	ADMV1355_IIO_DEV_ATTR_DIRECT_GPO_F,
	ADMV1355_IIO_DEV_ATTR_GPO_F_OE,
	ADMV1355_IIO_DEV_ATTR_GPO_G_OE,
	ADMV1355_IIO_DEV_ATTR_FILTER_SM_STEP,
	ADMV1355_IIO_DEV_ATTR_RF_BAND_AVAILABLE,
	ADMV1355_IIO_DEV_ATTR_MIXER_SIDEBAND_AVAILABLE,
	ADMV1355_IIO_DEV_ATTR_IF_MODE_AVAILABLE,
	ADMV1355_IIO_DEV_ATTR_RF_DSA1_AVAILABLE,
	ADMV1355_IIO_DEV_ATTR_RF_DSA2_AVAILABLE,
	ADMV1355_IIO_DEV_ATTR_RF_DSA1_BYPASS_AVAILABLE,
	ADMV1355_IIO_DEV_ATTR_RF_DSA2_BYPASS_AVAILABLE,
	ADMV1355_IIO_DEV_ATTR_IF_DSAI_AVAILABLE,
	ADMV1355_IIO_DEV_ATTR_IF_DSAQ_AVAILABLE,
	ADMV1355_IIO_DEV_ATTR_LO_X4_FILTER_AVAILABLE,
	ADMV1355_IIO_DEV_ATTR_LO_X3_FILTER_AVAILABLE,
	ADMV1355_IIO_DEV_ATTR_LO_X1_FILTER_AVAILABLE,
} Admv1355IioDevAttrId;

int admv1355_iio_init(Admv1355IioDev** iio_dev, Admv1355IioDevInitParam* init_param);

int admv1355_iio_remove(Admv1355IioDev* desc);

#endif /* IIO_ADMV1355_H */
