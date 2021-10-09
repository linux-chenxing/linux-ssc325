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

#ifndef __SPINAND_BBT_H__
#define __SPINAND_BBT_H__

#include <linux/types.h>
#include <linux/mtd/mtd.h>

#define    BBT_HEADER               0x42425448
#define    NO_ERR_BLOCK             0xFFFFFFFF
#define    BLOCK_INFO_NUM           1024
#define    FACTORY_BAD_BLOCK        0x00
#define    ECC_CORRECT_BLOCK        0x01
#define    RUN_TIME_BAD_BLOCK       0x02
#define    ECC_CORR_NOTFIX_BLOCK    0x03
#define    GOOD_BLOCK               0x0f
#define    MAX_CIS_BLK_NUM          10


typedef struct block_info //1 byte records the status of 2 blocks
{
    u8 b0 : 4;    //block0, 0000:factory bad block  0001:Ecc correctable  0010:running-time bad block  1111:good block
    u8 b1 : 4;    //block0, 0000:factory bad block  0001:Ecc correctable  0010:running-time bad block  1111:good block
}BLOCK_INFO_t;

typedef struct bbt_info
{
    u32 u32_Header;                               //the header of bbt 0x42425448(bbth)
    u32 u32_Crc32;                                //the crc value of bbt
    BLOCK_INFO_t stBlkInfo[BLOCK_INFO_NUM];       //record the blocks' status,the maximum Block number is 1024*2=2048
}BBT_INFO_t;

u8 nand_load_bbt(struct mtd_info *mtd,u32 u32_address);
u8 nand_save_bbt(struct mtd_info *mtd,u32 u32_address, u8 u8_update);
u8 nand_bbt_get_blk_info(struct mtd_info *mtd,u32 u32Offset);
void nand_bbt_fill_blk_info(struct mtd_info *mtd,u32 u32Offset, u32 u32_address, u8 u8BlkType);

#endif
