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

typedef enum {
     SPINAND_OTP_AVAIL = 0x0001,
     SPINAND_ALL_LOCK = 0x0002,
     SPINAND_EXT_ECC = 0x0004,
     SPINAND_ECC_RESERVED_NONE_CORRECTED = 0x0008,
     SPINAND_ECC_RESERVED_CORRECTED = 0x0010,
     SPINAND_NEED_QE = 0x0020,
     SPINAND_MULTI_DIES = 0x0040
} SPINAND_FUNC_FLAG_t;

typedef enum {
    SPINAND_NO_CR = 0,
    SPINAND_NONE_BUF_MODE,
    SPINAND_BUF_MODE,
} SPINAND_CR_TYPE_t;

typedef enum {
    SPINAND_CR_NONE = 0x00,
    SPINAND_CR_NEXT_STATUS = 0x01,
    SPINAND_CR_BUSY_AFTER_READ = 0x02,
    SPINAND_CR_BUSY_AFTER_NEXT = 0x04,
    SPINAND_CR_END_WITH_REST = 0x08,
    SPINAND_CR_BLOCK_WITH_LAST = 0x10,
} SPINAND_CR_CHECK_FLAG_t;

typedef struct {
    u8 u8_command;
    u32 u32_address;
    u8 u8_addressBytes;
    u8 u8_dummy;
    u16 u16_dataBytes;
    u16 u16_value;
} FLASH_CMD_SET_t;

typedef struct {
    FLASH_CMD_SET_t st_SRP0;
    FLASH_CMD_SET_t st_SRP1;
} SPINAND_SRP_t;

typedef struct {
    FLASH_CMD_SET_t st_complement;
    FLASH_CMD_SET_t st_topBottom;
    FLASH_CMD_SET_t st_blocks;
} SPINAND_MEMORY_PROTECT_t;

typedef struct {
    SPINAND_MEMORY_PROTECT_t st_blockStatus;
    SPINAND_SRP_t            st_srp;
} SPINAND_PROTECT_t;

typedef struct {
    FLASH_CMD_SET_t st_load;
    FLASH_CMD_SET_t st_noneBufModeCode;
} SPINAND_NONE_BUF_t;

typedef struct {
     FLASH_CMD_SET_t st_nextPage;
     FLASH_CMD_SET_t st_lastPage;
     FLASH_CMD_SET_t st_checkBusy;
     u8 u8_checkFlag; //reference SPINAND_CR_CHECK_FLAG_t
} SPINAND_BUF_MODE_t;

typedef struct {
    u8 u8_crType; //reference SPINAND_CR_TYPE_t
    union {
        SPINAND_NONE_BUF_t st_noneBufMode;
        SPINAND_BUF_MODE_t st_bufMode;
    } un_crProfile;
} SPINAND_CR_MODE_t;

typedef struct {
    FLASH_CMD_SET_t st_qeStatus;
    FLASH_CMD_SET_t st_read;
    FLASH_CMD_SET_t st_program;
    FLASH_CMD_SET_t st_random;
    SPINAND_CR_MODE_t st_crMode;
} SPINAND_ACCESS_CONFIG_t;

typedef struct {
        u32 u32_dieSize;
        FLASH_CMD_SET_t st_dieCode;
} SPINAND_DIE_t;

typedef struct {
        FLASH_CMD_SET_t st_otpLock;
        FLASH_CMD_SET_t st_otpEnabled;
} SPINAND_OTP_t;

typedef struct {
    FLASH_CMD_SET_t st_eccEnabled;
    SPINAND_OTP_t st_otp;
    SPINAND_DIE_t st_dieConfig;
    SPINAND_PROTECT_t st_protectStatus;
} SPINAND_EXT_CONFIG_t;

typedef struct {
    u16 u16_flags; //reference SPINAND_FUNC_FLAG_t
    u32 u32_maxWaitTime;
    SPINAND_EXT_CONFIG_t st_extConfig;
    SPINAND_ACCESS_CONFIG_t st_access;
    u32 u32_reserved;
} SPINAND_EXT_PROFILE_t;

typedef struct {
    u8 au8_magic[16];
    u32 u32_checksum;
    SPINAND_EXT_PROFILE_t st_profile;
} SPINAND_EXT_INFO_t;

typedef struct {
    u8   u8_IDByteCnt;
    u8   au8_ID[15];
    u16  u16_SpareByteCnt;
    u16  u16_PageByteCnt;
    u16  u16_BlkPageCnt;
    u16  u16_BlkCnt;
    u16  u16_SectorByteCnt;
    u8   u8PlaneCnt;
    u8   u8WrapConfig;
    u8   U8RIURead;
    u8   u8_MaxClk;
    u8   u8_UBOOTPBA;
    u8   u8_BL0PBA;
    u8   u8_BL1PBA;
    u8   u8_HashPBA[3][2];
    u8   u8_BootIdLoc;
    u8   u8_Reserved[24];//just for aligning to 64bytes + magic[16] = 80bytes
} __attribute__((aligned(16))) SPINAND_INFO_t;

/*
------------------------|
| SPINAND_INFO_t        |
|-----------------------|
| SPINAND_EXT_INFO_t    |
|-----------------------|
*/
#endif
