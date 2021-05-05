/*
* ms_iic_io.h- Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/
#ifndef _DRV_IIC_IO_H_
#define _DRV_IIC_IO_H_
////////////////////////////////////////////////////////////////////////////////
// Define & data type
////////////////////////////////////////////////////////////////////////////////
#include <ms_iic.h>

typedef union _I2C_IOCTL_CMD{
	struct {
		U8            u8Port;
		HWI2C_CLK_SEL eClk;
	}Clk_Cfg;
	
}__attribute__((packed)) I2C_IOCTL_CMD;

#define I2C_IOCTL_MAGIC 'I'
#define IOCTL_I2C_SET_SPEED       _IOW(I2C_IOCTL_MAGIC, 0x1, I2C_IOCTL_CMD)

#endif // _DRV_IIC_IO_H_

