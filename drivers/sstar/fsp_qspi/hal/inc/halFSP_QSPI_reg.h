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

#ifndef _HAL_FSP_QSPI_REG_H_
#define _HAL_FSP_QSPI_REG_H_

#include <ms_platform.h>

#define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
#define BMASK(_bits_)               (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))

#define RIU_BASE_ADDR                       IO_ADDRESS(0x1F000000)

#define GET_REG8_ADDR(x)                    (((x) << 1) - ((x) & 1))
#define GET_REG16_ADDR(x)                   (x << 2)

#define GET_BANK_REG8_ADDR(x, y)                 (x + (y << 1) - ((y) & 1))
#define GET_BANK_REG16_ADDR(x, y)                (x + (y << 2))

#define REG_WRITE_U16(reg_addr, val)        REG_W((reg_addr)) = (val)
#define REG_W(Reg_Addr)                     (*(volatile u32*)(Reg_Addr))

#define GET_INFO_CHIP_INIT                  0x0020

#define REG_MAILBOX_BASE            GET_BANK_REG8_ADDR(RIU_BASE_ADDR, 0x100400)
#define DEBUG_GET_INFO              GET_BANK_REG16_ADDR(REG_MAILBOX_BASE, 0x02)
#define DEBUG_BAD_BLOCK             GET_BANK_REG16_ADDR(REG_MAILBOX_BASE, 0x03)
#define DEBYG_LoadBL                GET_BANK_REG16_ADDR(REG_MAILBOX_BASE, 0x04)
#define DEBYG_LoadBL_PAGE           GET_BANK_REG16_ADDR(REG_MAILBOX_BASE, 0x05)
#define DEBYG_LoadBL_PLANE          GET_BANK_REG16_ADDR(REG_MAILBOX_BASE, 0x06)
#define DEBUG_LoadBL_ECC            GET_BANK_REG16_ADDR(REG_MAILBOX_BASE, 0x07)
#define DEBUG_LoadBL_BadBlock       GET_BANK_REG16_ADDR(REG_MAILBOX_BASE, 0x08)
#define DEBUG_REG_FCIEECC_SECTOR    GET_BANK_REG16_ADDR(REG_MAILBOX_BASE, 0x09)
#define DEBUG_INIT_ECC              GET_BANK_REG16_ADDR(REG_MAILBOX_BASE, 0x0A)
#define DEBUG_BL_ECC                GET_BANK_REG16_ADDR(REG_MAILBOX_BASE, 0x0B)
#define DEBUG_REG_FCIEE_HIT         GET_BANK_REG16_ADDR(REG_MAILBOX_BASE, 0x0C)

#define REG_WRITE_U16(reg_addr, val)        REG_W((reg_addr)) = (val)
#define REG_W(Reg_Addr)                     (*(volatile u32*)(Reg_Addr))

#define GET_INFO_CHIP_INIT                  0x0020


#define FSP_CHK_NUM_WAITDONE                    10000
#define FSP_MAX_READ_BUF_CNT                    10

typedef enum _FSP_ERROR_NUM
{
    ERR_FSP_SUCCESS = 0x00,
    ERR_FSP_TIMEOUT = 0x01,
} FSP_ERRNO_e;

#define FSP_BASE_ADDR                           (0x100D00 << 1)
#define QSPI_BASE_ADDR                          (0x100E00 << 1)

#define FSP_PMBASE_ADDR                         (0x001600 << 1)//PM
#define QSPI_PMBASE_ADDR                        (0x001700 << 1)//PM

#define REG_FSP_QUAD_MODE			            0x75
#define FSP_QUAD_ENABLE                         1
#define REG_FSP_OUTSIDE_WBF_SIZE                0x78
#define FSP_WBF_OUTSIDE_EN                      (1 << 12)
#define REG_FSP_OUTSIDE_WBF_CTRL                0x79
#define FSP_WBF_REPLACED(x)                     ((x) & BMASK(7:0))
#define FSP_WBF_MODE(x)                         (((x) << 8) & BMASK(11:8))
#define FSP_OUTSIDE_WBF_SIZE(x)                 ((x) & BMASK(11:0))

#define REG_FSP_READ_BUFF                       0x65
#define REG_FSP_WRITE_SIZE                      0x6A
#define REG_FSP_READ_SIZE                       0x6B
#define REG_FSP_CTRL                            0x6C
#define REG_FSP_TRIGGER                         0x6D
    #define TRIGGER_FSP                             1
#define REG_FSP_DONE                            0x6E
    #define FSP_DONE                                1
#define REG_FSP_AUTO_CHECK_ERROR                0x6E
    #define FSP_AUTOCHECK_ERROR                     2
#define REG_FSP_CLEAR_DONE                      0x6F
    #define FSP_CLEAR_DONE                          1
#define REG_FSP_CTRL2                           0x75
#define REG_FSP_CTRL3                           0x75
#define REG_FSP_CTRL4                           0x76
    #define FSP_WDATA                           0x00
// FSP Register
#define REG_FSP_WRITE_BUFF                      0x60
#define REG_FSP_WRITE_BUFF_ONE                  0x61
#define REG_FSP_WRITE_BUFF_TWO                  0x62
#define REG_FSP_WRITE_BUFF_THREE                0x63
#define REG_FSP_WRITE_BUFF2                     0x70
    #define FSP_WRITE_BUF_JUMP_OFFSET               0x0A

#define REG_FSP_RDATA                           0x05
#define REG_FSP_WSIZE                           0x0A

#define FSP_SINGLE_WRITE_SIZE                   15
#define REG_FSP_RSIZE                           0x0B

#define FSP_ENABLE                              1
#define FSP_RESET                               2
#define FSP_INT                                 4
#define FSP_AUTOCHECK                           8
#define FSP_ENABLE_SECOND_CMD                   0x8000
#define FSP_ENABLE_THIRD_CMD                    0x4000


#define REG_SPI_CHIP_SELECT                     0x7A
#define REG_SPI_BURST_WRITE                     0x0A
    #define SPI_DISABLE_BURST                   0x02
    #define SPI_ENABLE_BURST                    0x01

#define REG_SPI_TIMEOUT_VAL_L                   (0x66)
#define REG_SPI_TIMEOUT_VAL_H                   (0x67)
#define REG_SPI_TIMEOUT_CTRL                    (0x67)

#define REG_SPI_TIMEOUT_VAL_MASK                BMASK(7:0)
#define REG_SPI_TIMEOUT_EN_MASK                 BMASK(15:15)
#define REG_SPI_TIMEOUT_RST_MASK                BMASK(14:14)

#define REG_SPI_TIMEOUT_DS                      BITS(15:15,0)
#define REG_SPI_TIMEOUT_EN                      BITS(15:15,1)
#define REG_SPI_TIMEOUT_NRST                    BITS(14:14,0)
#define REG_SPI_TIMEOUT_RST                     BITS(14:14,1)

#define REG_SPI_MODE_SEL                        0x72
    #define SPI_NORMAL_MODE                 0x00
    #define SPI_FAST_READ                   0x01
    #define SPI_CMD_3B                      0x02
    #define SPI_CMD_BB                      0x03
    #define SPI_CMD_6B                      0x0A
    #define SPI_CMD_EB                      0x0B
    #define REG_SPI_CMD_0B                      0x0C
    #define REG_SPI_CMD_4EB                     0x0D
#define REG_SPI_FUNC_SET                        0x7D
    #define SPI_ADDR2_EN                    0x800
    #define SPI_DUMMY_EN                    0x1000
    //only for two plane nand
    #define SPI_WRAP_EN                     0x2000

#define REG_SPI_CKG_SPI                         0x70
    #define SPI_USER_DUMMY_EN               0x10

#define REG_SPI_FSP_CZ_HIGH                     0x6d

#define PM_SLEEP_BASE_ADDR   (0x000E00 << 1)
#define CHIP_BASE_ADDR       (0x101E00 << 1)

#endif
