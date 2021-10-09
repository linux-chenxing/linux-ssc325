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

#ifndef __MDRV_SNI_H
#define __MDRV_SNI_H
typedef struct {
    u8 u8_command;
    u32 u32_address;
    u8 u8_addressBytes;
    u8 u8_dummy;
    u16 u16_dataBytes;
    u16 u16_value;
} FLASH_CMD_SET_t;

typedef struct {
    u8 u8_needQE;
    FLASH_CMD_SET_t st_rQuadEnabled;
    FLASH_CMD_SET_t st_wQuadEnabled;
} SPINOR_QUAD_CFG_t;

typedef struct {
    FLASH_CMD_SET_t st_SRP0;
    FLASH_CMD_SET_t st_SRP1;
} SPINOR_SRP_t;

typedef struct {
    FLASH_CMD_SET_t st_complement;
    FLASH_CMD_SET_t st_topBottom;
    FLASH_CMD_SET_t st_blocks;
} SPINOR_MEMORY_PROTECT_t;

typedef struct {
    SPINOR_MEMORY_PROTECT_t st_blockStatus;
    SPINOR_SRP_t st_srp;
} SPINOR_PROTECT_STATUS_t;

typedef struct {
    u8 u8_IDByteCnt;
    u8 au8_ID[15];
    u8 u8_MaxClk;
    u16 u16_PageByteCnt;
    u16 u16_SectorByteCnt;
    u32 u32_BlkBytesCnt;
    u32 u32_Capacity;
    u32 u32_MaxWaitTime;
    u8 au8_reserved[8];
    SPINOR_PROTECT_STATUS_t st_rProtectStatus;
    SPINOR_PROTECT_STATUS_t st_wProtectStatus;
    SPINOR_QUAD_CFG_t st_QE;
    FLASH_CMD_SET_t st_readData;
    FLASH_CMD_SET_t st_program;
    u8 au8_venderName[16];
    u8 au8_partNumber[16];
} SPINOR_INFO_t;

#endif

