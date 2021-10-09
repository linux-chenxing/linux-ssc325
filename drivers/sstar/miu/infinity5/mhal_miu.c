/*
* mhal_miu.c- Sigmastar
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
#include <linux/printk.h>
#include <linux/delay.h>
#include "MsTypes.h"
#include "mdrv_types.h"
#include "mdrv_miu.h"
#include "regMIU.h"
#include "mhal_miu.h"
#include "registers.h"
#include "ms_platform.h"
#ifdef CONFIG_CAM_CLK
#include "camclk.h"
#include "drv_camclk_Api.h"
#endif
#include "mhal_miu_client.h"
//-------------------------------------------------------------------------------------------------
//  Macro Define
//-------------------------------------------------------------------------------------------------

#define _phy_to_miu_offset(MiuSel, Offset, PhysAddr) if (PhysAddr < ARM_MIU1_BASE_ADDR) \
                                                        {MiuSel = E_MIU_0; Offset = PhysAddr - ARM_MIU0_BASE_ADDR;} \
                                                     else if ((PhysAddr >= ARM_MIU1_BASE_ADDR) && (PhysAddr < ARM_MIU2_BASE_ADDR)) \
                                                        {MiuSel = E_MIU_1; Offset = PhysAddr - ARM_MIU1_BASE_ADDR;} \
                                                     else \
                                                        {MiuSel = E_MIU_2; Offset = PhysAddr - ARM_MIU2_BASE_ADDR;}

#define MIU_HAL_ERR(fmt, args...)   printk(KERN_ERR "miu hal error %s:%d" fmt,__FUNCTION__,__LINE__,## args)

//-------------------------------------------------------------------------------------------------
//  Local Variable
//-------------------------------------------------------------------------------------------------

static MS_U16 clientId_KernelProtect[IDNUM_KERNELPROTECT] =
{
    MIU_CLIENT_MIPS_RW,
    MIU_CLIENT_MCU51_RW,
    MIU_CLIENT_USB20_H_RW,
    MIU_CLIENT_USB20_RW,
    MIU_CLIENT_MIIC1_RW,
    MIU_CLIENT_MIIC2_RW,
    MIU_CLIENT_MIIC3_RW,
    MIU_CLIENT_IVE_RW,
    MIU_CLIENT_EMAC_RW,
    MIU_CLIENT_AESDMA_RW,
    MIU_CLIENT_SDIO_RW,
    MIU_CLIENT_FCIE_RW,
    MIU_CLIENT_URDMA_RW,
    MIU_CLIENT_BDMA_RW,
    MIU_CLIENT_MOVDMA0_RW,
    0,
};

#if defined(CONFIG_ARM) || defined(CONFIG_MIPS)
static MS_U32 m_u32MiuMapBase = 0xFD200000UL;   //default set to arm 32bit platform
#elif defined(CONFIG_ARM64)
extern ptrdiff_t mstar_pm_base;
static ptrdiff_t m_u32MiuMapBase;
#endif

static MS_BOOL IDEnables[MIU_MAX_DEVICE][MIU_MAX_PROTECT_BLOCK][MIU_MAX_PROTECT_ID] = {{{0},{0},{0},{0}}, {{0},{0},{0},{0}}}; //ID enable for protect block 0~3
static MS_U16 IDList[MIU_MAX_DEVICE][MIU_MAX_PROTECT_ID] = {{0}, {0}}; //IDList for protection

//-------------------------------------------------------------------------------------------------
//  MTLB HAL internal function
//-------------------------------------------------------------------------------------------------
const char* halClientIDTName(int id)
{
    int i = 0;
    for (i = 0; i < MIU0_CLIENT_NUM; i++) {
        if (miu0_clients[i].bw_client_id == id) {
            return miu0_clients[i].name;
        }
    }
    return NULL;
}
static MS_U32 HAL_MIU_BA2PA(MS_U32 u32BusAddr)
{
    MS_PHYADDR u32PhyAddr = 0x0UL;

    // pa = ba - offset
    if ((u32BusAddr >= ARM_MIU0_BUS_BASE) && (u32BusAddr < ARM_MIU1_BUS_BASE))
        u32PhyAddr = u32BusAddr - ARM_MIU0_BUS_BASE + ARM_MIU0_BASE_ADDR;
    else if ((u32BusAddr >= ARM_MIU1_BUS_BASE) && (u32BusAddr < ARM_MIU2_BUS_BASE))
        u32PhyAddr = u32BusAddr - ARM_MIU1_BUS_BASE + ARM_MIU1_BASE_ADDR;
    else
        u32PhyAddr = u32BusAddr - ARM_MIU2_BUS_BASE + ARM_MIU2_BASE_ADDR;

    return u32PhyAddr;
}

static MS_S16 HAL_MIU_GetClientIndex(MS_U8 u8MiuSel, eMIUClientID eClientID)
{
    MS_U8 idx = 0;

    if (MIU_MAX_DEVICE <= u8MiuSel) {
        MIU_HAL_ERR("%s not support MIU%u!\n", __FUNCTION__, u8MiuSel );
        return (-1);
    }

    for (idx = 0; idx < MIU0_CLIENT_NUM; idx++) {
        if (eClientID == miu0_clients[idx].bw_client_id)
            return eClientID;
    }
#if MIU_NUM > 1
    for (idx = 0; idx < MIU1_CLIENT_NUM; idx++) {
        if (eClientID == miu1_clients[idx].bw_client_id)
            return eClientID;
    }
 #endif
    return (-1);
}

static MS_U8 HAL_MIU_ReadByte(MS_U32 u32RegProtectId)
{
#if defined(CONFIG_ARM64)
    m_u32MiuMapBase = (mstar_pm_base + 0x00200000UL);
#endif
    return ((volatile MS_U8*)(m_u32MiuMapBase))[(u32RegProtectId << 1) - (u32RegProtectId & 1)];
}

static MS_U16 HAL_MIU_Read2Byte(MS_U32 u32RegProtectId)
{
#if defined(CONFIG_ARM64)
    m_u32MiuMapBase = (mstar_pm_base + 0x00200000UL);
#endif
    return ((volatile MS_U16*)(m_u32MiuMapBase))[u32RegProtectId];
}

static MS_BOOL HAL_MIU_WriteByte(MS_U32 u32RegProtectId, MS_U8 u8Val)
{
    if (!u32RegProtectId) {
        MIU_HAL_ERR("%s reg err\n", __FUNCTION__);
        return FALSE;
    }

#if defined(CONFIG_ARM64)
    m_u32MiuMapBase = (mstar_pm_base + 0x00200000UL);
#endif
    ((volatile MS_U8*)(m_u32MiuMapBase))[(u32RegProtectId << 1) - (u32RegProtectId & 1)] = u8Val;

    return TRUE;
}

static MS_BOOL HAL_MIU_Write2Byte(MS_U32 u32RegProtectId, MS_U16 u16Val)
{
    if (!u32RegProtectId) {
        MIU_HAL_ERR("%s reg err\n", __FUNCTION__);
        return FALSE;
    }

#if defined(CONFIG_ARM64)
    m_u32MiuMapBase = (mstar_pm_base + 0x00200000UL);
#endif
    ((volatile MS_U16*)(m_u32MiuMapBase))[u32RegProtectId] = u16Val;

    return TRUE;
}

static void HAL_MIU_WriteByteMask(MS_U32 u32RegOffset, MS_U8 u8Mask, MS_BOOL bEnable)
{
    MS_U8 u8Val = HAL_MIU_ReadByte(u32RegOffset);

    u8Val = (bEnable) ? (u8Val | u8Mask) : (u8Val & ~u8Mask);
    HAL_MIU_WriteByte(u32RegOffset, u8Val);
}

static void HAL_MIU_Write2BytesMask(MS_U32 u32RegOffset, MS_U16 u16Mask, MS_BOOL bEnable)
{
    MS_U16 u16Val = HAL_MIU_Read2Byte(u32RegOffset);

    u16Val = (bEnable) ? (u16Val | u16Mask) : (u16Val & ~u16Mask);
    HAL_MIU_Write2Byte(u32RegOffset, u16Val);
}

static void HAL_MIU_SetProtectIDReg(MS_U32 u32RegBase, MS_U8 u8MiuSel, MS_U16 u16ClientID)
{
    MS_S16 sVal = HAL_MIU_GetClientIndex(u8MiuSel, (eMIUClientID)u16ClientID);
    MS_S16 sIDVal = 0;

    if (0 > sVal) {
        sVal = 0;
    }

    sIDVal = HAL_MIU_ReadByte(u32RegBase);
    sIDVal &= 0x80;
    sIDVal |= sVal;
    HAL_MIU_WriteByte(u32RegBase, sIDVal);
}

static MS_BOOL HAL_MIU_SetGroupID(MS_U8 u8MiuSel, MS_U8 u8Blockx, MS_U16 *pu8ProtectId, MS_U32 u32RegAddrID, MS_U32 u32RegProtectIdEn)
{
    MS_U32 u32index0, u32index1;
    MS_U16 u16ID = 0;
    MS_U16 u16IdEnable = 0;
    MS_U8 u8isfound0, u8isfound1;

    // Reset IDEnables for protect u8Blockx
    for (u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        IDEnables[u8MiuSel][u8Blockx][u32index0] = 0;
    }

    for (u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        u16ID = pu8ProtectId[u32index0];

        // Unused ID
        if (u16ID == 0)
           continue;

        u8isfound0 = FALSE;

        for (u32index1 = 0; u32index1 < MIU_MAX_PROTECT_ID; u32index1++)
        {
            if (IDList[u8MiuSel][u32index1] == u16ID)
            {
                // ID reused former setting
                IDEnables[u8MiuSel][u8Blockx][u32index1] = 1;
                u8isfound0 = TRUE;
                break;
            }
        }

        // Need to create new ID in IDList
        if (u8isfound0 != TRUE)
        {
            u8isfound1 = FALSE;

            for (u32index1 = 0; u32index1 < MIU_MAX_PROTECT_ID; u32index1++)
            {
                if (IDList[u8MiuSel][u32index1] == 0)
                {
                    IDList[u8MiuSel][u32index1] = u16ID;
                    IDEnables[u8MiuSel][u8Blockx][u32index1] = 1;
                    u8isfound1 = TRUE;
                    break;
                }
            }

            // ID overflow
            if (u8isfound1 == FALSE) {
                return FALSE;
            }
        }
    }

    u16IdEnable = 0;

    for (u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        if (IDEnables[u8MiuSel][u8Blockx][u32index0] == 1) {
            u16IdEnable |= (1 << u32index0);
        }
    }

    HAL_MIU_Write2Byte(u32RegProtectIdEn, u16IdEnable);

    for (u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
         HAL_MIU_SetProtectIDReg(u32RegAddrID + u32index0, u8MiuSel, IDList[u8MiuSel][u32index0]);
    }

    return TRUE;
}

static MS_BOOL HAL_MIU_ResetGroupID(MS_U8 u8MiuSel, MS_U8 u8Blockx, MS_U16 *pu8ProtectId, MS_U32 u32RegAddrID, MS_U32 u32RegProtectIdEn)
{
    MS_U32 u32index0, u32index1;
    MS_U8 u8isIDNoUse = 0;
    MS_U16 u16IdEnable = 0;

    // Reset IDEnables for protect u8Blockx
    for (u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        IDEnables[u8MiuSel][u8Blockx][u32index0] = 0;
    }

    u16IdEnable = 0x0UL;

    HAL_MIU_Write2Byte(u32RegProtectIdEn, u16IdEnable);

    for (u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        u8isIDNoUse  = FALSE;

        for (u32index1 = 0; u32index1 < MIU_MAX_PROTECT_BLOCK; u32index1++)
        {
            if (IDEnables[u8MiuSel][u32index1][u32index0] == 1)
            {
                // Protect ID is still be used
                u8isIDNoUse = FALSE;
                break;
            }
            u8isIDNoUse  = TRUE;
        }

        if (u8isIDNoUse == TRUE) {
            IDList[u8MiuSel][u32index0] = 0;
        }
    }

    for (u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
         HAL_MIU_SetProtectIDReg(u32RegAddrID + u32index0, u8MiuSel, IDList[u8MiuSel][u32index0]);
    }

    return TRUE;
}

#define GET_HIT_BLOCK(regval)       BITS_RANGE_VAL(regval, REG_MIU_PROTECT_HIT_NO)
#define GET_HIT_CLIENT(regval)      BITS_RANGE_VAL(regval, REG_MIU_PROTECT_HIT_ID)

MS_BOOL HAL_MIU_GetHitProtectInfo(MS_U8 u8MiuSel, MIU_PortectInfo *pInfo)
{
    MS_U16 u16Ret = 0;
    MS_U16 u16LoAddr = 0;
    MS_U16 u16HiAddr = 0;
    MS_U32 u32RegBase = (u8MiuSel) ? MIU1_REG_BASE : MIU_REG_BASE;
    MS_U32 u32EndAddr = 0;
    char clientName[40];
    static MS_U16 u16HitCnt[MIU_MAX_DEVICE] = {0, };

    if (!pInfo) {
        return FALSE;
    }

    u16Ret      = HAL_MIU_Read2Byte(u32RegBase + REG_MIU_PROTECT_STATUS);
    u16LoAddr   = HAL_MIU_Read2Byte(u32RegBase + REG_MIU_PROTECT_LOADDR);
    u16HiAddr   = HAL_MIU_Read2Byte(u32RegBase + REG_MIU_PROTECT_HIADDR);

    if (REG_MIU_PROTECT_HIT_FALG & u16Ret)
    {
        pInfo->bHit         = TRUE;
        pInfo->u8Block      = (MS_U8)GET_HIT_BLOCK(u16Ret);
        pInfo->u8Group      = (MS_U8)(GET_HIT_CLIENT(u16Ret) >> 4);
        pInfo->u8ClientID   = (MS_U8)(GET_HIT_CLIENT(u16Ret) & 0x0F);
        pInfo->uAddress     = (MS_U32)((u16HiAddr << 16) | u16LoAddr);
        pInfo->uAddress     = pInfo->uAddress * MIU_PROTECT_ADDRESS_UNIT;

        u32EndAddr = (pInfo->uAddress + MIU_PROTECT_ADDRESS_UNIT - 1);

        strcpy(clientName, (char *)halClientIDTName((MS_U8)(GET_HIT_CLIENT(u16Ret))));
        printk(KERN_EMERG "MIU%u Block:%u Client:%s ID:%u-%u Hitted_Addr:0x%x<->0x%x\n",
               u8MiuSel, pInfo->u8Block, clientName,
               pInfo->u8Group, pInfo->u8ClientID,
               pInfo->uAddress, u32EndAddr);

        // Clear log
        HAL_MIU_Write2BytesMask(u32RegBase + REG_MIU_PROTECT_STATUS, REG_MIU_PROTECT_LOG_CLR, TRUE);
        HAL_MIU_Write2BytesMask(u32RegBase + REG_MIU_PROTECT_STATUS, REG_MIU_PROTECT_LOG_CLR, FALSE);

        // FIXME: Workaround for the unknown SC3_FRAME_W request even DMA disabled
        if ((u16HitCnt[u8MiuSel] == 0) && (MIU_CLIENT_SC3_FRAME_W == GET_HIT_CLIENT(u16Ret)))
        {
            pInfo->bHit = FALSE;
        }
        // FIXME: Workaround for the unknown VEN_R request addr over DRAM size
        if (MIU_CLIENT_VEN_R == GET_HIT_CLIENT(u16Ret))
        {
            pInfo->bHit = FALSE;
        }
        u16HitCnt[u8MiuSel]++;
    }

    return TRUE;
}

MS_BOOL HAL_MIU_GetProtectIdEnVal(MS_U8 u8MiuSel, MS_U8 u8BlockId, MS_U8 u8ProtectIdIndex)
{
    return IDEnables[u8MiuSel][u8BlockId][u8ProtectIdIndex];
}

MS_U16* HAL_MIU_GetDefaultKernelProtectClientID(void)
{
     if (IDNUM_KERNELPROTECT > 0) {
         return (MS_U16 *)&clientId_KernelProtect[0];
     }
     return NULL;
}

MS_U16* HAL_MIU_GetKernelProtectClientID(MS_U8 u8MiuSel)
{
     if (IDNUM_KERNELPROTECT > 0) {
         return (MS_U16 *)&IDList[u8MiuSel][0];
     }
     return NULL;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MIU_Protect()
/// @brief \b Function \b Description: Enable/Disable MIU Protection mode
/// @param u8Blockx        \b IN     : MIU Block to protect (0 ~ 4)
/// @param *pu8ProtectId   \b IN     : Allow specified client IDList to write
/// @param u32Start        \b IN     : Starting bus address
/// @param u32End          \b IN     : End bus address
/// @param bSetFlag        \b IN     : Disable or Enable MIU protection
///                                      - -Disable(0)
///                                      - -Enable(1)
/// @param <OUT>           \b None    :
/// @param <RET>           \b None    :
/// @param <GLOBAL>        \b None    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_Protect(    MS_U8   u8Blockx,
                            MS_U16  *pu8ProtectId,
                            MS_U32  u32BusStart,
                            MS_U32  u32BusEnd,
                            MS_BOOL bSetFlag)
{
    MS_U32 u32RegProtectId = 0;
    MS_U32 u32RegBase = 0;
    MS_U32 u32RegStartAddr = 0;
    MS_U32 u32RegAddrMSB = 0;
    MS_U32 u32RegProtectIdEn = 0;
    MS_U32 u32RegRWProtectEn = 0;
    MS_U32 u32StartOffset = 0;
    MS_U32 u32EndOffset = 0;
    MS_U8  u8MiuSel = 0;
    MS_U16 u16Data = 0;
    MS_U16 u16Data1 = 0;
    MS_U16 u16Data2 = 0;
    MS_U8  u8Data = 0;
    MS_U32 u32Start = 0, u32End = 0;

    u32Start = HAL_MIU_BA2PA(u32BusStart);
    u32End   = HAL_MIU_BA2PA(u32BusEnd);

    // Get MIU selection and offset
    _phy_to_miu_offset(u8MiuSel, u32EndOffset, u32End);
    _phy_to_miu_offset(u8MiuSel, u32StartOffset, u32Start);

    u32Start = u32StartOffset;
    u32End   = u32EndOffset;

    // Parameter check
    if (u8Blockx >= E_MIU_BLOCK_NUM)
    {
        MIU_HAL_ERR("Err: Blk Num out of range\n");
        return FALSE;
    }
    else if (((u32Start & ((1 << MIU_PAGE_SHIFT) -1)) != 0) ||
             ((u32End & ((1 << MIU_PAGE_SHIFT) -1)) != 0))
    {
        MIU_HAL_ERR("Err: Protected addr not 8KB aligned\n");
        return FALSE;
    }
    else if (u32Start >= u32End)
    {
        MIU_HAL_ERR("Err: Invalid end addr\n");
        return FALSE;
    }

    // Write_enable
    u8Data = 1 << u8Blockx;

    if (u8MiuSel == E_MIU_0)
    {
        u32RegAddrMSB = MIU_PROTECT0_MSB;
        u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);

        u32RegProtectId = MIU_PROTECT0_ID0;
        u32RegRWProtectEn = MIU_PROTECT_EN_INTERNAL;
        u32RegBase = MIU_REG_BASE;

        switch (u8Blockx)
        {
            case E_MIU_BLOCK_0:
                u32RegStartAddr = MIU_PROTECT0_START;
                u32RegProtectIdEn = MIU_PROTECT0_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFFF0UL);
                break;
            case E_MIU_BLOCK_1:
                u32RegStartAddr = MIU_PROTECT1_START;
                u32RegProtectIdEn = MIU_PROTECT1_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFF0FUL);
                break;
            case E_MIU_BLOCK_2:
                u32RegStartAddr = MIU_PROTECT2_START;
                u32RegProtectIdEn = MIU_PROTECT2_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xF0FFUL);
                break;
            case E_MIU_BLOCK_3:
                u32RegStartAddr = MIU_PROTECT3_START;
                u32RegProtectIdEn = MIU_PROTECT3_ID_ENABLE;
                u16Data2 = (u16Data1 & 0x0FFFUL);
                break;
            default:
                return FALSE;
        }
    }
    else if (u8MiuSel == E_MIU_1)
    {
        u32RegAddrMSB = MIU1_PROTECT0_MSB;
        u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);

        u32RegProtectId = MIU1_PROTECT0_ID0;
        u32RegRWProtectEn = MIU1_PROTECT_EN;
        u32RegBase = MIU1_REG_BASE;

        switch (u8Blockx)
        {
            case E_MIU_BLOCK_0:
                u32RegStartAddr = MIU1_PROTECT0_START;
                u32RegProtectIdEn = MIU1_PROTECT0_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFFF0UL);
                break;
            case E_MIU_BLOCK_1:
                u32RegStartAddr = MIU1_PROTECT1_START;
                u32RegProtectIdEn = MIU1_PROTECT1_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFF0FUL);
                break;
            case E_MIU_BLOCK_2:
                u32RegStartAddr = MIU1_PROTECT2_START;
                u32RegProtectIdEn = MIU1_PROTECT2_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xF0FFUL);
                break;
            case E_MIU_BLOCK_3:
                u32RegStartAddr = MIU1_PROTECT3_START;
                u32RegProtectIdEn = MIU1_PROTECT3_ID_ENABLE;
                u16Data2 = (u16Data1 & 0x0FFFUL);
                break;
            default:
                return FALSE;
        }
    }
    else
    {
        MIU_HAL_ERR("%s not support MIU%u!\n", __FUNCTION__, u8MiuSel );
        return FALSE;
    }

    // Disable MIU write protect
    HAL_MIU_WriteByteMask(u32RegRWProtectEn, u8Data, DISABLE);

    if (bSetFlag)
    {
        // Set Protect IDList
        if (HAL_MIU_SetGroupID(u8MiuSel, u8Blockx, pu8ProtectId, u32RegProtectId, u32RegProtectIdEn) == FALSE)
        {
            return FALSE;
        }

        // Set BIT29,30 of start/end address
        u16Data2 = u16Data2 | (MS_U16)((u32Start >> 29) << (u8Blockx*4));   // u16Data2 for start_ext addr
        u16Data1 = u16Data2 | (MS_U16)(((u32End - 1) >> 29) << (u8Blockx*4+2));
        HAL_MIU_Write2Byte(u32RegAddrMSB, u16Data1);

        // Start Address
        u16Data = (MS_U16)(u32Start >> MIU_PAGE_SHIFT);     // 8k unit
        HAL_MIU_Write2Byte(u32RegStartAddr, u16Data);

        // End Address
        u16Data = (MS_U16)((u32End >> MIU_PAGE_SHIFT)-1);   // 8k unit;
        HAL_MIU_Write2Byte(u32RegStartAddr + 2, u16Data);

        // Enable MIU write protect
        HAL_MIU_WriteByteMask(u32RegRWProtectEn, u8Data, ENABLE);
    }
    else
    {
        // Reset Protect IDList
        HAL_MIU_ResetGroupID(u8MiuSel, u8Blockx, pu8ProtectId, u32RegProtectId, u32RegProtectIdEn);
    }

    // Clear log
    HAL_MIU_Write2BytesMask(u32RegBase + REG_MIU_PROTECT_STATUS, REG_MIU_PROTECT_LOG_CLR, TRUE);
    HAL_MIU_Write2BytesMask(u32RegBase + REG_MIU_PROTECT_STATUS, REG_MIU_PROTECT_LOG_CLR, FALSE);

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_ParseOccupiedResource
/// @brief \b Function  \b Description: Parse occupied resource to software structure
/// @return             \b 0: Fail 1: OK
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_ParseOccupiedResource(void)
{
    MS_U8  u8MiuSel = 0;
    MS_U8  u8Blockx = 0;
    MS_U8  u8ClientID = 0;
    MS_U16 u16IdEnable = 0;
    MS_U32 u32index = 0;
    MS_U32 u32RegProtectId = 0;
    MS_U32 u32RegProtectIdEn = 0;

    for (u8MiuSel = E_MIU_0; u8MiuSel < MIU_MAX_DEVICE; u8MiuSel++)
    {
        for (u8Blockx = E_MIU_BLOCK_0; u8Blockx < E_MIU_BLOCK_NUM; u8Blockx++)
        {
            if (u8MiuSel == E_MIU_0)
            {
                u32RegProtectId = MIU_PROTECT0_ID0;

                switch (u8Blockx)
                {
                    case E_MIU_BLOCK_0:
                        u32RegProtectIdEn = MIU_PROTECT0_ID_ENABLE;
                        break;
                    case E_MIU_BLOCK_1:
                        u32RegProtectIdEn = MIU_PROTECT1_ID_ENABLE;
                        break;
                    case E_MIU_BLOCK_2:
                        u32RegProtectIdEn = MIU_PROTECT2_ID_ENABLE;
                        break;
                    case E_MIU_BLOCK_3:
                        u32RegProtectIdEn = MIU_PROTECT3_ID_ENABLE;
                        break;
                    default:
                        return FALSE;
                }
            }
            else if (u8MiuSel == E_MIU_1)
            {
                u32RegProtectId = MIU1_PROTECT0_ID0;

                switch (u8Blockx)
                {
                case E_MIU_BLOCK_0:
                     u32RegProtectIdEn = MIU1_PROTECT0_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_1:
                     u32RegProtectIdEn = MIU1_PROTECT1_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_2:
                     u32RegProtectIdEn = MIU1_PROTECT2_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_3:
                     u32RegProtectIdEn = MIU1_PROTECT3_ID_ENABLE;
                     break;
                 default:
                     return FALSE;
                }
            }
            else
            {
                MIU_HAL_ERR("%s not support MIU%u!\n", __FUNCTION__, u8MiuSel);
                return FALSE;
            }

            u16IdEnable = HAL_MIU_Read2Byte(u32RegProtectIdEn);

            for (u32index = 0; u32index < MIU_MAX_PROTECT_ID; u32index++)
            {
                IDEnables[u8MiuSel][u8Blockx][u32index] = ((u16IdEnable >> u32index) & 0x1UL) ? 1: 0;
            }
        }

        for (u32index = 0; u32index < MIU_MAX_PROTECT_ID; u32index++)
        {
            u8ClientID = HAL_MIU_ReadByte(u32RegProtectId + u32index) & 0x7F;
            IDList[u8MiuSel][u32index] = u8ClientID;
        }
    }

    return TRUE;
}

unsigned int HAL_MIU_ProtectDramSize(void)
{
    MS_U8 u8Val = HAL_MIU_ReadByte(MIU_PROTECT_DDR_SIZE);

    u8Val = (u8Val >> 4) & 0xF;

    if (0 == u8Val) {
        MIU_HAL_ERR("MIU protect size undefined. Using 0x40000000\n");
        return 0x40000000;
    }
    return (0x1 << (20 + u8Val));
}

int HAL_MIU_Info(MIU_DramInfo_Hal *pDramInfo)
{
    printk("Not support HAL_MIU_Info in this platform\n");

    return -1;
}
