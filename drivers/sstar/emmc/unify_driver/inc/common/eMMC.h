/*
* eMMC.h- Sigmastar
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

#ifndef eMMC_DRIVER_H
#define eMMC_DRIVER_H

//=====================================================================================
#include "../config/eMMC_config.h" // [CAUTION]: edit eMMC_config.h for your platform
//=====================================================================================
#include "eMMC_err_codes.h"

#define eMMC_DRIVER_VERSION   3 // used to sync with other SW stages (e.g. MBoot)

//===========================================================
// debug macro
//===========================================================
extern U32 gu32_eMMCDrvExtFlag;

#define eMMCDRV_EXTFLAG_DISABLE_LOG   BIT0
#define eMMC_DISABLE_LOG(yes)    \
    do{ \
        if(yes)  gu32_eMMCDrvExtFlag |= eMMCDRV_EXTFLAG_DISABLE_LOG; \
        else     gu32_eMMCDrvExtFlag &= ~eMMCDRV_EXTFLAG_DISABLE_LOG;\
    }while(0);
#define eMMC_IF_DISABLE_LOG()    \
    (gu32_eMMCDrvExtFlag & eMMCDRV_EXTFLAG_DISABLE_LOG)

#define eMMCDRV_EXTFLAG_BKG_SCAN      BIT1 // for Irwin
#define eMMC_SET_BKG_SCAN_HS200()  gu32_eMMCDrvExtFlag |= eMMCDRV_EXTFLAG_BKG_SCAN;
#define eMMC_CLR_BKG_SCAN_HS200()  gu32_eMMCDrvExtFlag &= ~eMMCDRV_EXTFLAG_BKG_SCAN;
#define eMMC_CHK_BKG_SCAN_HS200()  (gu32_eMMCDrvExtFlag & eMMCDRV_EXTFLAG_BKG_SCAN)


//===========================================================
// macro for Spec.
//===========================================================
#define ADDRESSING_MODE_BYTE      1 // 1 byte
#define ADDRESSING_MODE_SECTOR    2 // 512 bytes
#define ADDRESSING_MODE_4KB       3 // 4KB bytes

#define eMMC_DEVTYPE_HS400_1_8V   BIT6 // ECSD[196]
#define eMMC_DEVTYPE_HS200_1_8V   BIT4
#define eMMC_DEVTYPE_DDR          BIT2
#define eMMC_DEVTYPE_ALL          (eMMC_DEVTYPE_HS400_1_8V|eMMC_DEVTYPE_HS200_1_8V|eMMC_DEVTYPE_DDR)

#define eMMC_SPEED_OLD            0 // ECSD[185]
#define eMMC_SPEED_HIGH           1
#define eMMC_SPEED_HS200          2
#define eMMC_SPEED_HS400          3

#define eMMC_DRIVING_TYPE0        0 // x1
#define eMMC_DRIVING_TYPE1        1 // x1.5
#define eMMC_DRIVING_TYPE2        2 // x0.75
#define eMMC_DRIVING_TYPE3        3 // x0.5
#define eMMC_DRIVING_TYPE4        4 // x1.2

#define eMMC_FLAG_TRIM            BIT0
#define eMMC_FLAG_HPI_CMD12       BIT1
#define eMMC_FLAG_HPI_CMD13       BIT2

#define eMMC_PwrOffNotif_OFF      0
#define eMMC_PwrOffNotif_ON       1
#define eMMC_PwrOffNotif_SHORT    2
#define eMMC_PwrOffNotif_LONG     3

//-------------------------------------------------------
// Devices has to be in 512B block length mode by default
// after power-on, or software reset.
//-------------------------------------------------------

#define eMMC_SECTOR_512BYTE       0x200
#define eMMC_SECTOR_512BYTE_BITS  9
#define eMMC_SECTOR_512BYTE_MASK  (eMMC_SECTOR_512BYTE-1)

#define eMMC_SECTOR_BUF_16KB      (eMMC_SECTOR_512BYTE * 0x20)

#define eMMC_SECTOR_BYTECNT       eMMC_SECTOR_512BYTE
#define eMMC_SECTOR_BYTECNT_BITS  eMMC_SECTOR_512BYTE_BITS
//-------------------------------------------------------

#define eMMC_ExtCSD_SetBit        1
#define eMMC_ExtCSD_ClrBit        2
#define eMMC_ExtCSD_WByte         3

#define eMMC_CMD_BYTE_CNT         5
#define eMMC_R1_BYTE_CNT          5
#define eMMC_R1b_BYTE_CNT         5
#define eMMC_R2_BYTE_CNT          16
#define eMMC_R3_BYTE_CNT          5
#define eMMC_R4_BYTE_CNT          5
#define eMMC_R5_BYTE_CNT          5
#define eMMC_MAX_RSP_BYTE_CNT     eMMC_R2_BYTE_CNT

//===========================================================
// Partition Info parameters
//===========================================================

// internal data Sector Address
#define eMMC_ID_BYTE_CNT           15
#define eMMC_ID_FROM_CID_BYTE_CNT  10
#define eMMC_ID_DEFAULT_BYTE_CNT   11 // last byte means n GB

typedef eMMC_PACK0 struct _eMMC_NNI {

    U8  au8_Tag[16];
    U8  u8_IDByteCnt;
    U8  au8_ID[15];
    U32 u32_ChkSum;
    U16 u16_SpareByteCnt;
    U16 u16_PageByteCnt;
    U16 u16_BlkPageCnt;
    U16 u16_BlkCnt;
    U32 u32_Config;
    U16 u16_ECCType;
    U16 u16_SeqAccessTime;
    U8  au8_padding[12];
    U8  au8_Vendor[16];
    U8  au8_PartNumber[16];
    U8  u8_PairPageMapLoc;
    U8  u8_PairPageMapType;

} eMMC_PACK1 eMMC_NNI_t;


typedef eMMC_PACK0 struct _eMMC_PARTITION_RECORD {

    U16 u16_StartBlk;     // the start block index, reserved for UNFD internal use.
    U16 u16_BlkCnt;       // project team defined
    U16 u16_PartType;     // project team defined, e.g. eMMC_PART_XXX_0
    U16 u16_BackupBlkCnt; // reserved good blocks count for backup, UNFD internal use.
                          // e.g. u16BackupBlkCnt  = u16BlkCnt * 0.03 + 2
} eMMC_PACK1 eMMC_PARTITION_RECORD_t, *P_eMMC_PARTITION_RECORD_t;


typedef eMMC_PACK0 struct _eMMC_PNI {

    U32 u32_ChkSum;
    U16 u16_SpareByteCnt;
    U16 u16_PageByteCnt;
    U16 u16_BlkPageCnt;
    U16 u16_BlkCnt;
    U16 u16_PartCnt;
    U16 u16_UnitByteCnt;
    eMMC_PARTITION_RECORD_t records[];

} eMMC_PACK1 eMMC_PNI_t;


// Logical Disk Info
typedef struct _eMMC_DISK_INFO {

    U32 u32_StartSector;
    U32 u32_SectorCnt;

} eMMC_DISK_INFO_t, *P_eMMC_DISK_INFO_t;

//===========================================================
// DDR Timing Table
//===========================================================
typedef eMMC_PACK0 struct _eMMC_FCIE_DDRT_PARAM {

    U8 u8_DQS, u8_Cell;

} eMMC_PACK1 eMMC_FCIE_DDRT_PARAM_t;

#if 0
typedef eMMC_PACK0 struct _eMMC_FCIE_DDRT_WINDOW {

    U8 u8_Cnt;
    // DQS uses index, not reg value (see code)
    eMMC_FCIE_DDRT_PARAM_t aParam[2];
    U8 au8_DQSTryCellCnt[(BIT_DQS_MODE_MASK>>BIT_DQS_MODE_SHIFT)+1];
    U8 au8_DQSValidCellCnt[(BIT_DQS_MODE_MASK>>BIT_DQS_MODE_SHIFT)+1];

} eMMC_PACK1 eMMC_FCIE_DDRT_WINDOW_t;
#endif

typedef eMMC_PACK0 struct _eMMC_FCIE_DDRT_SET {

    U8 u8_Clk;
    eMMC_FCIE_DDRT_PARAM_t Param; // register values

} eMMC_PACK1 eMMC_FCIE_DDRT_SET_t;


#define eMMC_FCIE_DDRT_SET_CNT  12

#define eMMC_TIMING_SET_MAX      0
#define eMMC_TIMING_SET_MIN      1

// ----------------------------------------------
typedef eMMC_PACK0 struct _eMMC_FCIE_DDRT_TABLE {

    U8 u8_SetCnt, u8_CurSetIdx;

    #if !(defined(ENABLE_eMMC_ATOP) && ENABLE_eMMC_ATOP)
        // DDR48 (digital macro)
        eMMC_FCIE_DDRT_SET_t Set[eMMC_FCIE_DDRT_SET_CNT];
    #else
        // ATOP (for  DDR52, HS200, HS400)
        eMMC_FCIE_ATOP_SET_t Set[1];//eMMC_FCIE_VALID_CLK_CNT];
    #endif

    U32 u32_ChkSum; // put in the last
    U32 u32_VerNo; // for auto update

} eMMC_PACK1 eMMC_FCIE_TIMING_TABLE_t;

typedef eMMC_PACK0 struct _eMMC_FCIE_DDRT_EXT_TABLE {       //temp solution for monaco U02 HS400
    eMMC_FCIE_ATOP_SET_EXT_t Set;
    U32 u32_ChkSum; // put in the last
    U32 u32_VerNo; // for auto update
} eMMC_PACK1 eMMC_FCIE_TIMING_EXT_TABLE_t;

#define REG_OP_W    1
#define REG_OP_CLRBIT  2
#define REG_OP_SETBIT  3

typedef eMMC_PACK0 struct _eMMC_FCIE_REG_SET {      //total 10 bytes
    U32 u32_RegAddress;             //(BANK_ADDRESS + REGISTER OFFSET ) << 2
    U16 u16_RegValue;
    U16 u16_RegMask;
    U16 u16_OpCode;
} eMMC_PACK1 eMMC_FCIE_REG_SET_t;

typedef eMMC_PACK0 struct _eMMC_FCIE_GEN_TIMING_TABLE {
    U32 u32_ChkSum;
    U32 u32_VerNo; // for auto update
    U32 u32_Clk;
    U8 u8_SpeedMode;
    U8 u8_CurSetIdx;
    U8 u8_RegisterCnt;
    U8 u8_SetCnt;
    U8 au8_CID[eMMC_MAX_RSP_BYTE_CNT];
    U32 u32_Dummy[6];      //for extension
    eMMC_FCIE_REG_SET_t RegSet[45]; //at most 45 register set
} eMMC_PACK1 eMMC_FCIE_GEN_TIMING_TABLE_t;

#if defined(CONFIG_EMMC_FORCE_DDR52)
#define eMMC_TIMING_TABLE_VERSION        2
#else
#define eMMC_TIMING_TABLE_VERSION        4 // for CL.731742 & later
#endif
#define eMMC_TIMING_TABLE_CHKSUM_OFFSET  8

//===========================================================
// burst length for write speed
//===========================================================
typedef eMMC_PACK0 struct _eMMC_FCIE_WLen_TABLE {

    U16 u16_BestBrustLen, u16_WorstBrustLen;

    U16 u16_BestMBPerSec, u16_BestMBPerSecPoint;
    U16 u16_WorstMBPerSec, u16_WorstMBPerSecPoint;

    U32 u32_ChkSum; // put in the last

} eMMC_PACK1 eMMC_FCIE_WLen_TABLE_t;

//===========================================================
// Gernel Purpose Partition
//===========================================================
typedef eMMC_PACK0 struct _eMMC_GP_Part{
    U32 u32_PartSize;
    U8 u8_EnAttr;
    U8 u8_ExtAttr;
    U8 u8_RelW;
} eMMC_PACK1 eMMC_GP_Part_t;

//===========================================================
// driver flag (u32_DrvFlag)
//===========================================================
#define DRV_FLAG_INIT_DONE       BIT0 // include eMMC identify done

#define DRV_FLAG_GET_PART_INFO   BIT1
#define DRV_FLAG_RSP_WAIT_D0H    BIT2 // currently only R1b

#define DRV_FLAG_DDR_MODE        BIT3
#define DRV_FLAG_TUNING_TTABLE   BIT4 // to avoid retry & heavy log
#define DRV_FLAG_SPEED_MASK      (BIT7|BIT6|BIT5)
#define DRV_FLAG_SPEED_HIGH      BIT5
#define DRV_FLAG_SPEED_HS200     BIT6
#define DRV_FLAG_SPEED_HS400     BIT7
#define eMMC_IF_NORMAL_SDR()   (0==(g_eMMCDrv.u32_DrvFlag&DRV_FLAG_DDR_MODE)&&\
            DRV_FLAG_SPEED_HIGH==(g_eMMCDrv.u32_DrvFlag&DRV_FLAG_SPEED_MASK))
#define eMMC_SPEED_MODE()      (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_SPEED_MASK)
#define eMMC_IF_DDRT_TUNING()  (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_TUNING_TTABLE)

#define DRV_FLAG_PwrOffNotif_MASK   (BIT8|BIT9)
#define DRV_FLAG_PwrOffNotif_OFF    0
#define DRV_FLAG_PwrOffNotif_ON     BIT8
#define DRV_FLAG_PwrOffNotif_SHORT  BIT9
#define DRV_FLAG_PwrOffNotif_LONG   (BIT8|BIT9)

#define DRV_FLAG_RSPFROMRAM_SAVE    BIT10
#define DRV_FLAG_ERROR_RETRY        BIT11

typedef struct _eMMC_DRIVER
{
    U32 u32_ChkSum; // [8th ~ last-512] bytes
    U8 au8_Sig[4];  // 'e','M','M','C'

    // ----------------------------------------
    // Config from DTS
    // ----------------------------------------
    U16 u16_of_buswidth;

    // ----------------------------------------
    // FCIE
    // ----------------------------------------
    U16 u16_RCA;
    U32 u32_DrvFlag, u32_LastErrCode;
    U8  au8_Rsp[eMMC_MAX_RSP_BYTE_CNT];
    U8  au8_CSD[eMMC_MAX_RSP_BYTE_CNT];
    U8  au8_CID[eMMC_MAX_RSP_BYTE_CNT];
    U8  u8_PadType;
    U16 u16_Reg10_Mode;
    U32 u32_ClkKHz;
    U16 u16_ClkRegVal;
    eMMC_FCIE_TIMING_TABLE_t TimingTable_t;
        eMMC_FCIE_TIMING_EXT_TABLE_t TimingTable_Ext_t;
        eMMC_FCIE_GEN_TIMING_TABLE_t TimingTable_G_t;

    // ----------------------------------------
    // eMMC
    // ----------------------------------------
    // CSD
    U8  u8_SPEC_VERS;
    U8  u8_R_BL_LEN, u8_W_BL_LEN; // supported max blk len
    U16 u16_C_SIZE;
    U8  u8_TAAC, u8_NSAC, u8_Tran_Speed;
    U8  u8_C_SIZE_MULT;
    U8  u8_ERASE_GRP_SIZE, u8_ERASE_GRP_MULT;
    U8  u8_R2W_FACTOR;

    U8  u8_IfSectorMode;
    U32 u32_eMMCFlag;
    U32 u32_EraseUnitSize;

    // ExtCSD
    U32 u32_SEC_COUNT;
    U32 u32_BOOT_SEC_COUNT;

    #define BUS_WIDTH_1 1
    #define BUS_WIDTH_4 4
    #define BUS_WIDTH_8 8

    U8  u8_BUS_WIDTH;
    U8  u8_ErasedMemContent;
    U16 u16_ReliableWBlkCnt;
    U8  u8_ECSD185_HsTiming, u8_ECSD192_Ver, u8_ECSD196_DevType, u8_ECSD197_DriverStrength;
    U8  u8_ECSD248_CMD6TO, u8_ECSD247_PwrOffLongTO, u8_ECSD34_PwrOffCtrl;
    U8  u8_ECSD160_PartSupField, u8_ECSD224_HCEraseGRPSize, u8_ECSD221_HCWpGRPSize;
    U8  u8_ECSD159_MaxEnhSize_2, u8_ECSD158_MaxEnhSize_1, u8_ECSD157_MaxEnhSize_0;
    U8  u8_u8_ECSD155_PartSetComplete, u8_ECSD166_WrRelParam;
    U8  u8_ECSD184_Stroe_Support;

    // ----------------------------------------
    // CIS
    // ----------------------------------------
    // nni
    U8 u8_IDByteCnt, au8_ID[eMMC_ID_BYTE_CNT];
    U8 au8_Vendor[16], au8_PartNumber[16];

    // pni
    U32 au32_Pad[2]; // don't move

    U32 u32_PartDevNodeStartSector;
    U16 u16_PartDevNodeSectorCnt;
    U32 u32_FATSectorCnt;

    // ----------------------
    #if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
    // rsp from ram
    U8 au8_AllRsp[eMMC_SECTOR_512BYTE]; // last 4 bytes are CRC
    #endif

    // ----------------------
    #if defined(eMMC_BURST_LEN_AUTOCFG) && eMMC_BURST_LEN_AUTOCFG
    eMMC_FCIE_WLen_TABLE_t  BurstWriteLen_t;
    #endif

    // ----------------------
    #if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
    U32 u32_CNT_CMD17, u32_CNT_CMD24, u32_CNT_CMD18, u32_CNT_CMD25;
    U64 u64_CNT_TotalRBlk, u64_CNT_TotalWBlk;

    U32 u32_CNT_MinRBlk, u32_CNT_MinWBlk, u32_CNT_MaxRBlk, u32_CNT_MaxWBlk;
    U32 u32_RBlk_tmp, u32_WBlk_tmp;
    U32 au32_CNT_MinRBlk[0x200], au32_CNT_MinWBlk[0x200]; // for blk count < 0x200, how many times

    U32 u32_Addr_RLast, u32_Addr_WLast;
    U32 u32_Addr_RHitCnt, u32_Addr_WHitCnt;

    U32 u32_temp_count;
    #endif

    eMMC_GP_Part_t  GP_Part[4];

    U32 u32_EnUserStartAddr;
    U32 u32_EnUserSize;
    U8  u8_EnUserEnAttr;
    U8  u8_EnUserRelW;

    //FCIE5

    U16 u16_MacroToggleCnt;
    U16 u16_EmmcPll_IOBusWidth;
    U16 u16_EmmcPll_DqsPageByteCnt;
    U8 u8_MacroType;
    U8 u8_DefaultBusMode;
    U8 u8_HS400_mode;

    // misc
    U8  u8_disable_retry;

    U8  u8_make_sts_err;

    #define FCIE_NOT_MAKE_ERR           0
    #define FCIE_MAKE_RD_CRC_ERR        1
    #define FCIE_MAKE_WR_CRC_ERR        2
    #define FCIE_MAKE_WR_TOUT_ERR       3
    #define FCIE_MAKE_CMD_NO_RSP        4
    #define FCIE_MAKE_CMD_RSP_ERR       5
    #define FCIE_MAKE_RD_TOUT_ERR       6
    #define FCIE_MAKE_CARD_BUSY         7

    U8  u8_check_last_blk_crc;

} eMMC_DRIVER, *P_eMMC_DRIVER;

extern eMMC_DRIVER g_eMMCDrv;

// ADMA Descriptor
struct  _AdmaDescriptor{
    U32 u32_End     : 1;
    U32 u32_MiuSel  : 2;
    U32             : 13;
    U32 u32_JobCnt  : 16;
    U32 u32_Address;
    U32 u32_DmaLen;
    U32 u32_Dummy;
};

//===========================================================
// exposed APIs
//===========================================================
#include "../api/drv_eMMC.h"

//===========================================================
// internal used functions
//===========================================================
#include "eMMC_utl.h"
#include "eMMC_hal.h"

extern U32  eMMC_IPVerify_Main(void);
extern U32  eMMC_IPVerify_Main_Ex(U32 u32_DataPattern);
#define eMMC_TEST_READONLY   1
#define eMMC_TEST_WRITEONLY  2
extern U32  eMMC_IPVerify_WriteOnly(U16 u16_TestPattern);
extern U32  eMMC_IPVerify_ReadOnly(void);
extern U32  eMMC_IPVerify_SDRDDR_AllClkTemp(void);
extern void eMMCTest_DownCount(U32 u32_Sec);
extern U32  eMMC_IPVerify_Performance(void);
extern U32  eMMCTest_BlkWRC_ProbeTiming(U32 u32_eMMC_Addr);
extern U32  eMMCTest_KeepR_TestDDR(U32 u32_LoopCnt);
extern U32  eMMCTest_Lifetime(U8 u8_TestMode);
extern U32  eMMCTest_PwrCut_InitData(U8* u8_DataBuf, U32 u32_BlkStartAddr);
extern U32  eMMCTest_PwrCut_Test(U8* u8_DataBuf, U32 u32_BlkStartAddr);
extern U32  eMMCTest_PwrCut_Test2(U8* u8_DataBuf, U32 u32_BlkStartAddr);
//power cut
extern void eMMC_CheckPowerCut(void);
extern void eMMC_Prepare_Power_Saving_Mode_Queue(void);

#define eMMC_LIFETIME_TEST_FIXED    1
#define eMMC_LIFETIME_TEST_FILLED   2
#define eMMC_LIFETIME_TEST_RANDOM   3

extern U32  eMMC_BootMode(void);
#endif // eMMC_DRIVER_H

