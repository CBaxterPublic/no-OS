/***************************************************************************//**
 *   @file   iio_admv1455.h
 *   @brief  Header file for admv1455 IIO Driver.
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
#ifndef IIO_ADMV1455_H_
#define IIO_ADMV1455_H_

#include "admv1455.h"
#include "iio_types.h"

typedef struct iio_channel IioChannel;
typedef struct iio_device IioDevice;
typedef struct iio_attribute IioAttribute;
typedef struct iio_ch_info IioChInfo;

typedef struct admv1455_iio_dev {
	Admv1455Dev* admv1455_dev;
	IioDevice* iio_dev;
} Admv1455IioDev;

typedef struct admv1455_iio_dev_init_param {
	Admv1455InitParam* admv1455_dev_init;
} Admv1455IioDevInitParam;

typedef enum admv1455_iio_dev_attr_id {
	ADMV1455_RF_BAND,
	ADMV1455_IF_SUBBAND,
	ADMV1455_LO_TRAP,
	ADMV1455_GPO_G_OE,
	ADMV1455_DIRECT_DSA1,
	ADMV1455_DIRECT_DSA2,
	ADMV1455_DIRECT_DSA3,
	ADMV1455_DIRECT_DSA4,
	ADMV1455_DIRECT_DSA5,
	ADMV1455_BYPASS_DSA1,
	ADMV1455_BYPASS_DSA2,
	ADMV1455_BYPASS_DSA3,
	ADMV1455_BYPASS_DSA4,
	ADMV1455_BYPASS_DSA5,
	ADMV1455_LO_SIDEBAND,
	ADMV1455_LO_BAND,
	ADMV1455_LO_X1_FILTER,
	ADMV1455_LO_X3_FILTER,
	ADMV1455_LO_X4_FILTER,
	ADMV1455_LO_DOUBLER,
	ADMV1455_DSAI,
	ADMV1455_DSAQ,
	ADMV1455_LPF_VALUE,
	ADMV1455_HPF_VALUE,
	ADMV1455_DSA1_OFFSET,
	ADMV1455_DSA2_OFFSET,
	ADMV1455_DSA3_OFFSET,
	ADMV1455_DSA4_OFFSET,
	ADMV1455_DSA5_OFFSET,
	ADMV1455_LO_PHASE_I,
	ADMV1455_LO_PHASE_Q,
	ADMV1455_FILTER_LOAD_EN,
	ADMV1455_FILTER_TABLE_SEL,
	ADMV1455_DIRECT_GPO_F,
	ADMV1455_BYPASS_GAIN_TABLE_EN,
	ADMV1455_BYPASS_GPO_G,
	ADMV1455_GAIN_TABLE_EN,
	ADMV1455_GAIN_LOAD_EN,
	ADMV1455_DIRECT_GPO_G,
	ADMV1455_GAIN_LUT_MODE,
	ADMV1455_GAIN_SPI_POINTER,
	ADMV1455_GPO_F_OE,
	ADMV1455_CONTROL_COUNT,
} Admv1455IioDevAttrId;

int admv1455_iio_init(Admv1455IioDev** device,
		      const Admv1455IioDevInitParam* param);
int admv1455_iio_remove(Admv1455IioDev* dev);

#endif