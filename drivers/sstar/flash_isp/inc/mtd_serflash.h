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

#ifndef _MTD_SERFLASH_H_
#define _MTD_SERFLASH_H_
#include <linux/types.h>
typedef struct {
    u32 u32_eraseSize;
    u32 u32_writeSize;
    u32 u32_capacity;
} FLASH_INFO_t;


#define MS_SPI_ADDR			    IO_ADDRESS(0x14000000)

#define FLASH_GET_OFFSET(offset)            (MS_SPI_ADDR + (offset))
#define BLOCK_ERASE_SIZE                    0x10000

#define FLASH_CIS_LOAD_OFFSET           0x08
#define FLASH_SEARCH_END                (0x10000)

#define FLASH_DEFAULT_SNI_OFFSET        0x20000

#define BIT_FILTER(x)           while(1) {\
                                if ((x) & 0x1) break; \
                                (x) = (x) >> 1; \
                                \}

#endif
