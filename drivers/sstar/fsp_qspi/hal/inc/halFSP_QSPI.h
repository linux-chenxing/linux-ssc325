/*
 *#############################################################################
 * SigmaStar trade secret
 * Copyright (c) 2006-2011 SigmaStar Technology Corp.
 * All rights reserved.
 *
 * Unless otherwise stipulated in writing, any and all information contained
 * herein regardless in any format shall remain the sole proprietary of
 * SigmaStar Technology Corp. and be kept in strict confidence
 * (SigmaStar Confidential Information) by the recipient.
 * Any unauthorized act including without limitation unauthorized disclosure,
 * copying, use, reproduction, sale, distribution, modification, disassembling,
 * reverse engineering and compiling of the contents of SigmaStar Confidential
 * Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
 * rights to any and all damages, losses, costs and expenses resulting therefrom.
 *
 *#############################################################################
 */

#ifndef _HAL_FSP_QSPI_H
#define _HAL_FSP_QSPI_H

//#include "stdcomp.h"

#define REG_SPI_SW_CS_EN                        0x01
#define REG_SPI_SW_CS_PULL_HIGH                 0x02
void HAL_REG_Write2Byte(u32 u32_address, u16 u16_val);
void HAL_REG_WriteByte(u32 u32_address, u8 u8_val);
void HAL_CHIP_Write2Byte(u32 u32_address, u16 u16_val);
void HAL_PM_SLEEP_Write2Byte(u32 u32_address, u16 u16_val);
void HAL_FSP_WriteByte(u32 u32_address, u8 u8_val);
void HAL_FSP_Write2Byte(u32 u32_address, u16 u16_val);
void HAL_QSPI_WriteByte(u32 u32_address, u16 u8_val);
void HAL_QSPI_Write2Byte(u32 u32_address, u16 u16_val);
void HAL_QSPI_Write2Byte_Mask(u32 u32_address, u16 u16_val,u16 u16_mask);
u16 HAL_FSP_Read2Byte(u32 u32_address);
u8 HAL_FSP_ReadByte(u32 u32_address);
u8 HAL_QSPI_ReadByte(u32 u32_address);

#endif
