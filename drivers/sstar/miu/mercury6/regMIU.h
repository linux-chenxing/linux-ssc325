/*
* regMIU.h- Sigmastar
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
#ifndef _REG_MIU_H_
#define _REG_MIU_H_

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

#ifndef BIT
#define BIT(_bit_)                  (1 << (_bit_))
#endif

#define OVERALL_CLIENT_ID   (0x00)
#define CPU_W_CLIENT_ID     (0x70)
#define DLA_HIWAY_R_ID      (0x71)
#define DLA_HIWAY_W_ID      (0x72)
#define CPU_R_CLIENT_ID     (0x73)
#define MIU_GRP_CLIENT_NUM  (0x10)
#define MIU_GRP_NUM         (4)
#define MIU_ARB_CLIENT_NUM  (0x40)  // 16 clients per group, total 4 groups

#define BITS_RANGE(range)           (BIT(((1)?range)+1) - BIT((0)?range))
#define BITS_RANGE_VAL(x, range)    ((x & BITS_RANGE(range)) >> ((0)?range))

#define MIU_REG_BASE                (0x1200UL)
#define MIU1_REG_BASE               (0x0D00UL)
#define MIU2_REG_BASE               (0x62000UL)
#define PM_REG_BASE                 (0x1E00UL)
#define MIU_ATOP_BASE               (0x1000UL)
#define MIU1_ATOP_BASE              (0x0B00UL)
#define MIU2_ATOP_BASE              (0x62100UL)
#define CHIP_TOP_BASE               (0x1E00UL)
#define MIU_ARB_REG_BASE            (0x1100UL)
#define MIU1_ARB_REG_BASE           (0x0C00UL)
#define MIU2_ARB_REG_BASE           (0x62300UL)
#define MIU_MMU_REG_BASE            (0x1300UL)

#define MIU_PROTECT_DDR_SIZE        (MIU_REG_BASE+0xD3UL)
#define MIU_PROTECT_DDR_SIZE_MASK   BITS_RANGE(15:12)
#define MIU_PROTECT_DDR_32MB        (0x50UL)
#define MIU_PROTECT_DDR_64MB        (0x60UL)
#define MIU_PROTECT_DDR_128MB       (0x70UL)
#define MIU_PROTECT_DDR_256MB       (0x80UL)
#define MIU_PROTECT_DDR_512MB       (0x90UL)
#define MIU_PROTECT_DDR_1024MB      (0xA0UL)
#define MIU_PROTECT_DDR_2048MB      (0xB0UL)

#define MIU_PROTECT0_EN             (MIU_REG_BASE+0xD2UL)
#define MIU_PROTECT1_EN             (MIU_REG_BASE+0xD2UL)
#define MIU_PROTECT2_EN             (MIU_REG_BASE+0xD2UL)
#define MIU_PROTECT3_EN             (MIU_REG_BASE+0xD2UL)
#define MIU_PROTECT4_EN             (MIU_ARB_REG_BASE+0xDEUL)
#define MIU_PROTECT0_INVERT         (MIU_REG_BASE+0xD3UL)
#define MIU_PROTECT1_INVERT         (MIU_REG_BASE+0xD3UL)
#define MIU_PROTECT2_INVERT         (MIU_REG_BASE+0xD3UL)
#define MIU_PROTECT3_INVERT         (MIU_REG_BASE+0xD3UL)
#define MIU_PROTECT4_INVERT         (MIU_ARB_REG_BASE+0xDEUL)
#define MIU_PROTECT_ID0             (MIU_REG_BASE+0x2EUL)
#define MIU_PROTECT0_ID_ENABLE      (MIU_REG_BASE+0x20UL)
#define MIU_PROTECT1_ID_ENABLE      (MIU_REG_BASE+0x22UL)
#define MIU_PROTECT2_ID_ENABLE      (MIU_REG_BASE+0x24UL)
#define MIU_PROTECT3_ID_ENABLE      (MIU_REG_BASE+0x26UL)
#define MIU_PROTECT4_ID_ENABLE      (MIU_ARB_REG_BASE+0xDC)
#define MIU_PROTECT0_MSB            (MIU_REG_BASE+0xD0UL)
#define MIU_PROTECT1_MSB            (MIU_REG_BASE+0xD0UL)
#define MIU_PROTECT2_MSB            (MIU_REG_BASE+0xD0UL)
#define MIU_PROTECT3_MSB            (MIU_REG_BASE+0xD0UL)
#define MIU_PROTECT4_MSB            (MIU_REG_BASE+0xB2UL)
#define MIU_PROTECT0_START          (MIU_REG_BASE+0xC0UL)
#define MIU_PROTECT0_END            (MIU_REG_BASE+0xC2UL)
#define MIU_PROTECT1_START          (MIU_REG_BASE+0xC4UL)
#define MIU_PROTECT1_END            (MIU_REG_BASE+0xC6UL)
#define MIU_PROTECT2_START          (MIU_REG_BASE+0xC8UL)
#define MIU_PROTECT2_END            (MIU_REG_BASE+0xCAUL)
#define MIU_PROTECT3_START          (MIU_REG_BASE+0xCCUL)
#define MIU_PROTECT3_END            (MIU_REG_BASE+0xCEUL)
#define MIU_PROTECT4_START          (MIU_REG_BASE+0x72UL)
#define MIU_PROTECT4_END            (MIU_REG_BASE+0x92UL)
#define REG_MIU_PROTECT_LOADDR      (0x6DUL << 1)   //0xDA
#define REG_MIU_PROTECT_HIADDR      (0x6EUL << 1)   //0xDC
#define REG_MIU_PROTECT_STATUS      (0x6FUL << 1)   //0xDE

/*MMU protect regitsters*/
#define MMU_PROTECT0_CHK_FLG        (MIU_REG_BASE+0xB2UL)
#define MMU_PROTECT1_CHK_FLG        (MIU_REG_BASE+0xB2UL)
#define MMU_PROTECT2_CHK_FLG        (MIU_REG_BASE+0xB2UL)
#define MMU_PROTECT3_CHK_FLG        (MIU_REG_BASE+0xB2UL)
#define MMU_PROTECT4_CHK_FLG        (MIU_REG_BASE+0xB2UL)
#define MMU_PROTECT_ID0             (MIU_MMU_REG_BASE+0xEEUL)
#define MMU_PROTECT0_EN             (MIU_MMU_REG_BASE+0xD4UL)
#define MMU_PROTECT1_EN             (MIU_MMU_REG_BASE+0xD4UL)
#define MMU_PROTECT2_EN             (MIU_MMU_REG_BASE+0xD4UL)
#define MMU_PROTECT3_EN             (MIU_MMU_REG_BASE+0xD4UL)
#define MMU_PROTECT0_INVERT         (MIU_MMU_REG_BASE+0xD5UL)
#define MMU_PROTECT1_INVERT         (MIU_MMU_REG_BASE+0xD5UL)
#define MMU_PROTECT2_INVERT         (MIU_MMU_REG_BASE+0xD5UL)
#define MMU_PROTECT3_INVERT         (MIU_MMU_REG_BASE+0xD5UL)
#define MMU_PROTECT0_START          (MIU_MMU_REG_BASE+0xC0UL)
#define MMU_PROTECT0_END            (MIU_MMU_REG_BASE+0xC2UL)
#define MMU_PROTECT1_START          (MIU_MMU_REG_BASE+0xC4UL)
#define MMU_PROTECT1_END            (MIU_MMU_REG_BASE+0xC6UL)
#define MMU_PROTECT2_START          (MIU_MMU_REG_BASE+0xC8UL)
#define MMU_PROTECT2_END            (MIU_MMU_REG_BASE+0xCAUL)
#define MMU_PROTECT3_START          (MIU_MMU_REG_BASE+0xCCUL)
#define MMU_PROTECT3_END            (MIU_MMU_REG_BASE+0xCEUL)
#define MMU_PROTECT0_MSB            (MIU_MMU_REG_BASE+0xD0UL)
#define MMU_PROTECT1_MSB            (MIU_MMU_REG_BASE+0xD0UL)
#define MMU_PROTECT2_MSB            (MIU_MMU_REG_BASE+0xD2UL)
#define MMU_PROTECT3_MSB            (MIU_MMU_REG_BASE+0xD2UL)
#define MMU_PROTECT0_ID_ENABLE      (MIU_MMU_REG_BASE+0xD6UL)
#define MMU_PROTECT1_ID_ENABLE      (MIU_MMU_REG_BASE+0xD8UL)
#define MMU_PROTECT2_ID_ENABLE      (MIU_MMU_REG_BASE+0xDAUL)
#define MMU_PROTECT3_ID_ENABLE      (MIU_MMU_REG_BASE+0xDCUL)
#define REG_MMU_PROTECT_STATUS      0XDEUL
#define REG_MMU_PROTECT_LOADDR      0XEAUL
#define REG_MMU_PROTECT_HIADDR      0XECUL

// MMU Protection Status
#define REG_MMU_PROTECT_LOG_CLR     (BIT0)
#define REG_MMU_PROTECT_IRQ_MASK    (BIT1)
#define REG_MMU_PROTECT_HIT_FALG    (BIT4)
#define REG_MMU_PROTECT_HIT_ID      14:8
#define REG_MMU_PROTECT_HIT_NO      7:5

// MIU selection registers
#define REG_MIU_SEL0                (MIU_REG_BASE+0xf0UL)  //0x12F0
#define REG_MIU_SEL1                (MIU_REG_BASE+0xf2UL)  //0x12F1
#define REG_MIU_SEL2                (MIU_REG_BASE+0xf4UL)  //0x12F2
#define REG_MIU_SEL3                (MIU_REG_BASE+0xf6UL)  //0x12F3
#define REG_MIU_SELX(x)             (0xF0UL+x*2)

// MIU1
#define MIU1_PROTECT_DDR_SIZE       (MIU1_REG_BASE+0xD3UL)
#define MIU1_PROTECT_DDR_SIZE_MASK  BITS_RANGE(15:12)

#define MIU1_PROTECT0_EN            (MIU1_REG_BASE+0xD2UL)
#define MIU1_PROTECT1_EN            (MIU1_REG_BASE+0xD2UL)
#define MIU1_PROTECT2_EN            (MIU1_REG_BASE+0xD2UL)
#define MIU1_PROTECT3_EN            (MIU1_REG_BASE+0xD2UL)
#define MIU1_PROTECT4_EN            (MIU1_ARB_REG_BASE+0xDEUL)
#define MIU1_PROTECT_ID0            (MIU1_REG_BASE+0x2EUL)
#define MIU1_PROTECT0_ID_ENABLE     (MIU1_REG_BASE+0x20UL)
#define MIU1_PROTECT1_ID_ENABLE     (MIU1_REG_BASE+0x22UL)
#define MIU1_PROTECT2_ID_ENABLE     (MIU1_REG_BASE+0x24UL)
#define MIU1_PROTECT3_ID_ENABLE     (MIU1_REG_BASE+0x26UL)
#define MIU1_PROTECT4_ID_ENABLE     (MIU1_ARB_REG_BASE+0xDC)
#define MIU1_PROTECT0_MSB           (MIU1_REG_BASE+0xD0UL)
#define MIU1_PROTECT1_MSB           (MIU1_REG_BASE+0xD0UL)
#define MIU1_PROTECT2_MSB           (MIU1_REG_BASE+0xD0UL)
#define MIU1_PROTECT3_MSB           (MIU1_REG_BASE+0xD0UL)
#define MIU1_PROTECT4_MSB           (MIU1_REG_BASE+0xB2UL)
#define MIU1_PROTECT0_START         (MIU1_REG_BASE+0xC0UL)
#define MIU1_PROTECT0_END           (MIU1_REG_BASE+0xC2UL)
#define MIU1_PROTECT1_START         (MIU1_REG_BASE+0xC4UL)
#define MIU1_PROTECT1_END           (MIU1_REG_BASE+0xC6UL)
#define MIU1_PROTECT2_START         (MIU1_REG_BASE+0xC8UL)
#define MIU1_PROTECT2_END           (MIU1_REG_BASE+0xCAUL)
#define MIU1_PROTECT3_START         (MIU1_REG_BASE+0xCCUL)
#define MIU1_PROTECT3_END           (MIU1_REG_BASE+0xCEUL)
#define MIU1_PROTECT4_START         (MIU1_REG_BASE+0x72UL)
#define MIU1_PROTECT4_END           (MIU1_REG_BASE+0x92UL)

#define REG_MIU_I64_MODE            (BIT7)
#define REG_MIU_INIT_DONE           (BIT15)

// Protection Status
#define REG_MIU_PROTECT_LOG_CLR     (BIT0)
#define REG_MIU_PROTECT_IRQ_MASK    (BIT1)
#define REG_MIU_PROTECT_HIT_FALG    (BIT4)
#define REG_MIU_PROTECT_HIT_ID      14:8
#define REG_MIU_PROTECT_HIT_NO      7:5

#define REG_MIU_PROTECT_PWR_IRQ_MASK_OFFSET (MIU_REG_BASE+0xD8UL)
#define REG_MIU_PROTECT_PWR_IRQ_MASK_BIT    (BIT11)

// MMU Control Register
#define REG_MMU_CTRL                (MIU_MMU_REG_BASE+0xA0UL)
#define REG_MMU_RW_ENTRY            (MIU_MMU_REG_BASE+0xA2UL)
#define REG_MMU_W_DATA              (MIU_MMU_REG_BASE+0xA4UL)
#define REG_MMU_R_DATA              (MIU_MMU_REG_BASE+0xA6UL)
#define REG_MMU_CLIENT_ID_0_1       (MIU_MMU_REG_BASE+0xA8UL)
#define REG_MMU_CLIENT_ID_2_3       (MIU_MMU_REG_BASE+0xAAUL)
#define REG_MMU_CLIENT_ID_4_5       (MIU_MMU_REG_BASE+0xACUL)
#define REG_MMU_CLIENT_ID_6_7       (MIU_MMU_REG_BASE+0xAEUL)
#define REG_MMU_CLIENT_ID_SEL       (MIU_MMU_REG_BASE+0xB0UL)
#define REG_MMU_IRQ_CTRL            (MIU_MMU_REG_BASE+0xB2UL)
#define REG_MMU_COLLISION_ENTRY     (MIU_MMU_REG_BASE+0xB4UL)
#define REG_MMU_ACCESS              (MIU_MMU_REG_BASE+0xB6UL)
#define REG_MMU_INVALID_ENTRY       (MIU_MMU_REG_BASE+0xB8UL)
#define REG_MMU_INVALID_CLIENT_ID   (MIU_MMU_REG_BASE+0xBAUL)

// MMU Control
#define REG_MMU_CTRL_ENABLE         (BIT0)
#define REG_MMU_CTRL_PG_SIZE        2:1
#define REG_MMU_CTRL_PG_SIZE_128K   (~(BIT1))
#define REG_MMU_CTRL_PG_SIZE_256K   (BIT1)
#define REG_MMU_CTRL_PG_SIZE_512K   (BIT2)
#define REG_MMU_CTRL_RESET          (BIT4)
#define REG_MMU_CTRL_RESET_INIT_VAL (BIT5)
#define REG_MMU_CTRL_INIT_DONE      (BIT6)
#define REG_MMU_CTRL_REGION_MASK    11:7
#define REG_MMU_CTRL_RP_REGION_MASK 15:12

// Read/Write Entry
#define REG_MMU_RW_ENTRY_MODE       (BIT15)

// IRQ Control
#define REG_MMU_IRQ_RW_CLR          (BIT0)
#define REG_MMU_IRQ_RW_MASK         (BIT1)
#define REG_MMU_IRQ_RD_CLR          (BIT2)
#define REG_MMU_IRQ_RD_MASK         (BIT3)
#define REG_MMU_IRQ_WR_CLR          (BIT4)
#define REG_MMU_IRQ_WR_MASK         (BIT5)
#define REG_MMU_IRQ_RW_FLAG         (BIT6)
#define REG_MMU_IRQ_RD_FLAG         (BIT7)
#define REG_MMU_IRQ_WR_FLAG         (BIT8)
#define REG_MMU_IRQ_INVALID_RW      (BIT9)
#define REG_MMU_IRQ_INVALID_ID_MASK 6:0


#endif // _REG_MIU_H_
