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

//#include <common.h>
#include <linux/string.h>
#include <ms_platform.h>
#include <halFSP_QSPI_reg.h>

#if defined(FSP_QSPI_DUAL_BANK)
u8 FSPQSPI_mode = FPS_QSPI_NONPM;
#endif

void HAL_REG_Write2Byte(u32 u32_address, u16 u16_val)
{
    *(unsigned short volatile*)(RIU_BASE_ADDR + u32_address) = u16_val;
}

void HAL_REG_WriteByte(u32 u32_address, u8 u8_val)
{
    *(unsigned char volatile*)(RIU_BASE_ADDR + u32_address) = u8_val;
}

void HAL_CHIP_Write2Byte(u32 u32_address, u16 u16_val)
{
    // RIU[CHIP_BASE_ADDR + (u32RegAddr << 1)] = u16Val;
    *(unsigned short volatile*)(RIU_BASE_ADDR + CHIP_BASE_ADDR + (u32_address << 2)) = u16_val;
}


void HAL_PM_SLEEP_Write2Byte(u32 u32_address, u16 u16_val)
{
    *(unsigned short volatile*)(RIU_BASE_ADDR + PM_SLEEP_BASE_ADDR + (u32_address << 2))= u16_val;
}


void HAL_FSP_WriteByte(u32 u32_address, u8 u8_val)
{
    u32 u32_fsp_address;

    u32_fsp_address = FSP_PMBASE_ADDR;

#if defined(FSP_QSPI_DUAL_BANK)
    if (FPS_QSPI_NONPM == FSPQSPI_mode)
    {
        u32_fsp_address = FSP_BASE_ADDR;
    }
#endif
    *(unsigned char volatile*)(RIU_BASE_ADDR + u32_fsp_address + u32_address) = u8_val;
}

void HAL_FSP_Write2Byte(u32 u32_address, u16 u16_val)
{
    u32 u32_fsp_address;

    u32_fsp_address = FSP_PMBASE_ADDR;

#if defined(FSP_QSPI_DUAL_BANK)
    if (FPS_QSPI_NONPM == FSPQSPI_mode)
    {
        u32_fsp_address = FSP_BASE_ADDR;
    }
#endif

    *(unsigned short volatile*)(RIU_BASE_ADDR + u32_fsp_address + u32_address) = u16_val;
}

void HAL_QSPI_WriteByte(u32 u32_address, u16 u8_val)
{
    u32 u32_qspi_address;

    u32_qspi_address = QSPI_PMBASE_ADDR;

#if defined(FSP_QSPI_DUAL_BANK)
    if (FPS_QSPI_NONPM == FSPQSPI_mode)
    {
        u32_qspi_address = QSPI_BASE_ADDR;
    }
#endif
    *(unsigned char volatile*)(RIU_BASE_ADDR + u32_qspi_address + u32_address) = u8_val;
}

void HAL_QSPI_Write2Byte(u32 u32_address, u16 u16_val)
{
    u32 u32_qspi_address;

    u32_qspi_address = QSPI_PMBASE_ADDR;

#if defined(FSP_QSPI_DUAL_BANK)
    if (FPS_QSPI_NONPM == FSPQSPI_mode)
    {
        u32_qspi_address = QSPI_BASE_ADDR;
    }
#endif

    *(unsigned short volatile*)(RIU_BASE_ADDR + u32_qspi_address + u32_address) = u16_val;

}

void HAL_QSPI_Write2Byte_Mask(u32 u32_address, u16 u16_val,u16 u16_mask)
{
    u32 u32_qspi_address;
    u32 u16_reg_timeout_val;

    u32_qspi_address = QSPI_PMBASE_ADDR;

#if defined(FSP_QSPI_DUAL_BANK)
    if (FPS_QSPI_NONPM == FSPQSPI_mode)
    {
        u32_qspi_address = QSPI_BASE_ADDR;
    }
#endif
    u16_reg_timeout_val = (*(unsigned short volatile *)(RIU_BASE_ADDR + u32_qspi_address + u32_address));
    u16_reg_timeout_val = (u16_reg_timeout_val & ~u16_mask) | (u16_val & u16_mask);
    *(unsigned short volatile *)(RIU_BASE_ADDR + u32_qspi_address + u32_address) = u16_reg_timeout_val;
}

u8 HAL_QSPI_ReadByte(u32 u32_address)
{
    u32 u32_qspi_address;

    u32_qspi_address = QSPI_PMBASE_ADDR;

#if defined(FSP_QSPI_DUAL_BANK)
    if (FPS_QSPI_NONPM == FSPQSPI_mode)
    {
        u32_qspi_address = QSPI_BASE_ADDR;
    }
#endif

    return (*(unsigned char volatile *)(RIU_BASE_ADDR + u32_qspi_address + u32_address));
}
u16 HAL_FSP_Read2Byte(u32 u32_address)
{
    u32 u32_fsp_address;

    u32_fsp_address = FSP_PMBASE_ADDR;

#if defined(FSP_QSPI_DUAL_BANK)
    if (FPS_QSPI_NONPM == FSPQSPI_mode)
    {
        u32_fsp_address = FSP_BASE_ADDR;
    }
#endif

    return(*(unsigned short volatile*) (RIU_BASE_ADDR + u32_fsp_address + u32_address));
}

u8 HAL_FSP_ReadByte(u32 u32_address)
{
    u32 u32_fsp_address;

    u32_fsp_address = FSP_PMBASE_ADDR;

#if defined(FSP_QSPI_DUAL_BANK)
    if (FPS_QSPI_NONPM == FSPQSPI_mode)
    {
        u32_fsp_address = FSP_BASE_ADDR;
    }
#endif

    return (*(unsigned char volatile*)(RIU_BASE_ADDR + u32_fsp_address + u32_address));
}

