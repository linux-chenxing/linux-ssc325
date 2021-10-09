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

#ifndef _DRV_SPINAND_H_
#define _DRV_SPINAND_H_

typedef enum _SPINAND_ERROR_NUM {
    ERR_SPINAND_SUCCESS =               0x00,
    ERR_SPINAND_ECC_CORRECTED =         0x01,
    ERR_SPINAND_ECC_NOT_CORRECTED =     0x02,
    ERR_SPINAND_ECC_RESERVED =          0x03,
    ERR_SPINAND_E_FAIL =                0x04,
    ERR_SPINAND_P_FAIL =                0x05,
    ERR_SPINAND_TIMEOUT =               0x06,
    ERR_SPINAND_INVALID =               0x07,
    ERR_SPINAND_DEVICE_FAILURE =        0x08,
    ERR_SPINAND_BDMA_FAILURE =          0x09
} SPINAND_FLASH_ERRNO_e;

typedef struct {
    u8 u8_rfc;
    u8 u8_dummy;
    u8 u8_programLoad;
    u8 u8_randomLoad;
    u32 u32_maxWaitTime;
} DRV_SPINAND_INFO_t;

#define SPI_TO_MIU_PATCH

// SPI NAND COMMAND
#define SPI_NAND_CMD_RFC                    0x03
#define SPI_NAND_CMD_FRFC                   0x0B

#define SPI_NAND_CMD_PGRD                   0x13
#define SPI_NAND_CMD_RDID                   0x9F
#define SPI_NAND_CMD_WREN                   0x06
#define SPI_NAND_CMD_WRDIS                  0x04
#define SPI_NAND_CMD_PL                     0x02
#define SPI_NAND_CMD_QPL                    0x32
#define SPI_NAND_CMD_RPL                    0x84
#define SPI_NAND_CMD_QRPL                   0x34
#define SPI_NAND_CMD_PE                     0x10
#define SPI_NAND_CMD_GF                     0x0F
#define SPI_NAND_CMD_RESET                  0xFF
#define SPI_NAND_REG_PROT                   0xA0
#define SPI_NAND_REG_FEAT                   0xB0
#define SPI_NAND_REG_STAT                   0xC0
        #define SPI_NAND_STAT_E_FAIL                    (0x01 << 2)
        #define SPI_NAND_STAT_P_FAIL                    (0x01 << 3)
        #define ECC_STATUS_PASS                         (0x00 << 4)
        #define ECC_NOT_CORRECTED                       (0x02 << 4)
        #define ECC_NO_CORRECTED                        (0x00 << 4)
        #define ECC_STATUS_MASK                         (0x03 << 4)
        #define ECC_STATUS_RESERVED                     (0x03 << 4)
        #define SPI_NAND_STAT_OIP                       (0x1)
        //#define ECC_STATUS_ERR                 (0x02 << 4)
#define SPI_NAND_REG_FUT                                0xD0
#define SPI_NAND_CMD_SF                                 0x1F
#define SPI_NAND_CMD_BE                                 0xD8

u8 DRV_SPINAND_receive_data(u8* pu8_buf, u32 u32_size);
u8 DRV_SPINAND_simple_transmission(u8* pu8_buf, u32 u32_size);
u8 DRV_SPINAND_complete_transmission(u8* pu8_buf, u32 u32_size);
u8 DRV_SPINAND_reset_status(void);
u8 DRV_SPINAND_reset(void);
u8 DRV_SPINAND_write_enable(void);
u8 DRV_SPINAND_write_disable(void);
u8 DRV_SPINAND_get_features(u8 u8_address, u8 *pu8_data, u8 u8_size);
u8 DRV_SPINAND_check_status(void);
u8 DRV_SPINAND_set_features(u8 u8_address, u8 *pu8_data, u8 u8_size);
u8 DRV_SPINAND_read_id(u8 *pu8_ids, u8 u8_bytes);
u8 DRV_SPINAND_cmd_read_id(void);
u8 DRV_SPINAND_page_read(u32 u32_row_address);
u8 DRV_SPINAND_page_read_with_status(u32 u32_row_address);
u8 DRV_SPINAND_cmd_normal_read_from_cache(u16 u16_col_address);
u8 DRV_SPINAND_cmd_read_status_register(void);
u8 DRV_SPINAND_random_program_load(u16 u16_col_address, u8* pu8_data, u32 u32_size);
u8 DRV_SPINAND_read_from_cache(u16 u16_col_address, u8 *pu8_data, u32 u32_size);
u8 DRV_SPINAND_block_erase(u32 u32_row_address);
u8 DRV_SPINAND_program_load_data(const u8* pu8_data, u32 u32_size);
u8 DRV_SPINAND_cmd_program_load(u16 u16_col_address);
u8 DRV_SPINAND_program_load(u16 u16_col_address, u8* pu8_data, u32 u32_size);
u8 DRV_SPINAND_program_execute(u32 u32_row_address);
void DRV_SPINAND_setup_timeout(u32 u32_timeout);
void DRV_SPINAND_setup_access(u8 u8_read, u8 u8_dummy_cycle, u8 u8_program_load, u8 u8_random_program_load);
void DRV_SPINAND_alloc_bdma_buffer(u32 u32DataSize);
void DRV_SPINAND_free_bdma_buffer(u32 u32DataSize);
void DRV_SPINAND_use_bdma(u8 u8_enabled);
u8 DRV_SPINAND_init(void);


#endif

