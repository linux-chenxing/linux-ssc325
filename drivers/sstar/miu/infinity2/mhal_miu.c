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

#ifdef CONFIG_MSC006A_S01A_S_UVC
MS_U16 clientId_KernelProtect[IDNUM_KERNELPROTECT] =
{
    MIU_CLIENT_MIPS_RW, MIU_CLIENT_EMAC_RW,
    MIU_CLIENT_USB20_P1_RW, //MIU_CLIENT_USB20_RW,
    MIU_CLIENT_USB30M1_HS_RW, MIU_CLIENT_USB30_1_RW,
    MIU_CLIENT_USB30_2_RW, MIU_CLIENT_USB30_RW,
    MIU_CLIENT_U3DEV_RW, MIU_CLIENT_MIIC0_RW,
    MIU_CLIENT_MIIC1_RW, MIU_CLIENT_MIIC2_RW,
    MIU_CLIENT_SDIO_RW, MIU_CLIENT_SD30_RW,
    MIU_CLIENT_SATA1_RW, MIU_CLIENT_SATA0_RW,
    MIU_CLIENT_FCIE5_RW//emmc
};
#else //CONFIG_MSC005A_6A_S01A
MS_U16 clientId_KernelProtect[IDNUM_KERNELPROTECT] =
{
    MIU_CLIENT_MIPS_RW, MIU_CLIENT_EMAC_RW,
    MIU_CLIENT_USB20_P1_RW, MIU_CLIENT_USB20_RW,
    MIU_CLIENT_NOE_RW, MIU_CLIENT_SATA0_RW,
    MIU_CLIENT_SATA1_RW, MIU_CLIENT_USB30M1_HS_RW,
    MIU_CLIENT_USB30_1_RW, MIU_CLIENT_USB30_2_RW,
    MIU_CLIENT_USB30_RW, MIU_CLIENT_EVD_ENG0_RW,
    MIU_CLIENT_MIIC0_RW, MIU_CLIENT_MIIC1_RW,
    MIU_CLIENT_MIIC2_RW, MIU_CLIENT_GMAC_RW
};
#endif


EXPORT_SYMBOL(clientId_KernelProtect);

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
typedef enum
{
  E_CHIP_MIU_0 = 0,
  E_CHIP_MIU_1,
  E_CHIP_MIU_2,
  E_CHIP_MIU_3,
  E_CHIP_MIU_NUM,
} CHIP_MIU_ID;

//-------------------------------------------------------------------------------------------------
//  Macros
//-------------------------------------------------------------------------------------------------
#define _phy_to_miu_offset(MiuSel, Offset, PhysAddr) if (PhysAddr < ARM_MIU1_BASE_ADDR) \
                                                        {MiuSel = E_CHIP_MIU_0; Offset = PhysAddr;} \
                                                     else if ((PhysAddr >= ARM_MIU1_BASE_ADDR) && (PhysAddr < ARM_MIU2_BASE_ADDR)) \
                                                         {MiuSel = E_CHIP_MIU_1; Offset = PhysAddr - ARM_MIU1_BASE_ADDR;} \
                                                     else \
                                                         {MiuSel = E_CHIP_MIU_2; Offset = PhysAddr - ARM_MIU2_BASE_ADDR;}

#define _miu_offset_to_phy(MiuSel, Offset, PhysAddr) if (MiuSel == E_CHIP_MIU_0) \
                                                        {PhysAddr = Offset;} \
                                                     else if (MiuSel == E_CHIP_MIU_1) \
                                                         {PhysAddr = Offset + ARM_MIU1_BASE_ADDR;} \
                                                     else \
                                                         {PhysAddr = Offset + ARM_MIU2_BASE_ADDR;}

#define MIU_HAL_ERR(fmt, args...)   printk(KERN_ERR "miu hal error %s:%d" fmt,__FUNCTION__,__LINE__,## args)

//-------------------------------------------------------------------------------------------------
//  Local Variable
//-------------------------------------------------------------------------------------------------
//static MS_U32 _gMIU_MapBase = 0xBF200000;      //default set to MIPS platfrom
#if defined(CONFIG_ARM) || defined(CONFIG_MIPS)
static MS_U32 _gMIU_MapBase = 0xFD200000UL;   //default set to arm 32bit platfrom
#elif defined(CONFIG_ARM64)
extern ptrdiff_t mstar_pm_base;
static ptrdiff_t _gMIU_MapBase;
#endif

#if 0
MS_BOOL IDEnables[MIU_MAX_DEVICE][MIU_MAX_PROTECT_BLOCK][MIU_MAX_PROTECT_ID] = {{{0},{0},{0},{0}}, {{0},{0},{0},{0}}, {{0},{0},{0},{0}}}; //ID enable for protect block 0~3
MS_U8 IDs[MIU_MAX_DEVICE][MIU_MAX_PROTECT_ID] = {{0}, {0}, {0}}; //IDs for protection
#else
MS_BOOL IDEnables[MIU_MAX_DEVICE][MIU_MAX_PROTECT_BLOCK][MIU_MAX_PROTECT_ID] = {{{0},{0},{0},{0}}, {{0},{0},{0},{0}}}; //ID enable for protect block 0~3
MS_U16 IDs[MIU_MAX_DEVICE][MIU_MAX_PROTECT_ID] = {{0}, {0}}; //IDs for protection
#endif
MS_BOOL HAL_MIU_Get_IDEnables_Value(MS_U8 u8MiuDev, MS_U8 u8MiuBlockId, MS_U8 u8ProtectIdIndex)
{
    return IDEnables[u8MiuDev][u8MiuBlockId][u8ProtectIdIndex];
}

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
    #if MIU_NUM > 1
    for (i = 0; i < MIU1_CLIENT_NUM; i++) {
        if (miu1_clients[i].bw_client_id == id) {
            return miu1_clients[i].name;
        }
    }
    #endif
    return NULL;
}

MS_U32 HAL_MIU_BA2PA(MS_U32 u32BusAddr)
{
    MS_U32 u32PhyAddr = 0x0UL;

    // pa = ba - offset
        if( (u32BusAddr >= ARM_MIU0_BUS_BASE) && (u32BusAddr < ARM_MIU1_BUS_BASE) )     // MIU0
                u32PhyAddr = u32BusAddr - ARM_MIU0_BUS_BASE + ARM_MIU0_BASE_ADDR;
        else if( (u32BusAddr >= ARM_MIU1_BUS_BASE) && (u32BusAddr < ARM_MIU2_BUS_BASE)) // MIU1
                u32PhyAddr = u32BusAddr - ARM_MIU1_BUS_BASE + ARM_MIU1_BASE_ADDR;
        else
                u32PhyAddr = u32BusAddr - ARM_MIU2_BUS_BASE + ARM_MIU2_BASE_ADDR;       // MIU2

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
            return idx;
    }
    return (-1);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_ReadByte
/// @brief \b Function  \b Description: read 1 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <OUT>        \b None :
/// @param <RET>        \b MS_U8
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_U8 HAL_MIU_ReadByte(MS_U32 u32RegAddr)
{
#if defined(CONFIG_ARM64)
        _gMIU_MapBase = (mstar_pm_base + 0x00200000UL);
#endif
    return ((volatile MS_U8*)(_gMIU_MapBase))[(u32RegAddr << 1) - (u32RegAddr & 1)];
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_Read4Byte
/// @brief \b Function  \b Description: read 2 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <OUT>        \b None :
/// @param <RET>        \b MS_U16
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_U16 HAL_MIU_Read2Byte(MS_U32 u32RegAddr)
{
#if defined(CONFIG_ARM64)
        _gMIU_MapBase = (mstar_pm_base + 0x00200000UL);
#endif
    return ((volatile MS_U16*)(_gMIU_MapBase))[u32RegAddr];
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_Read4Byte
/// @brief \b Function  \b Description: read 4 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <OUT>        \b None :
/// @param <RET>        \b MS_U32
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_U32 HAL_MIU_Read4Byte(MS_U32 u32RegAddr)
{
    return (HAL_MIU_Read2Byte(u32RegAddr) | HAL_MIU_Read2Byte(u32RegAddr+2) << 16);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_WriteByte
/// @brief \b Function  \b Description: write 1 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u8Val : 1 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_WriteByte(MS_U32 u32RegAddr, MS_U8 u8Val)
{
    if (!u32RegAddr)
    {
        MIU_HAL_ERR("%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

#if defined(CONFIG_ARM64)
        _gMIU_MapBase = (mstar_pm_base + 0x00200000UL);
#endif
    ((volatile MS_U8*)(_gMIU_MapBase))[(u32RegAddr << 1) - (u32RegAddr & 1)] = u8Val;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_Write2Byte
/// @brief \b Function  \b Description: write 2 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u16Val : 2 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok FALSE: Fail
/// @param <GLOBAL>     \b None :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_Write2Byte(MS_U32 u32RegAddr, MS_U16 u16Val)
{
    if (!u32RegAddr)
    {
        MIU_HAL_ERR("%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

#if defined(CONFIG_ARM64)
        _gMIU_MapBase = (mstar_pm_base + 0x00200000UL);
#endif
    ((volatile MS_U16*)(_gMIU_MapBase))[u32RegAddr] = u16Val;
    return TRUE;
}

void HAL_MIU_SetProtectID(MS_U32 u32Reg, MS_U8 u8MiuDev, MS_U16 u8ClientID)
{
    MS_S16 sVal = HAL_MIU_GetClientInfo(u8MiuDev, (eMIUClientID)u8ClientID);
    MS_S16 sIDVal;

    if (0 > sVal)
        sVal = 0;

    sIDVal = HAL_MIU_ReadByte(u32Reg);
    sIDVal &= 0x80;
    sIDVal |= sVal;
    HAL_MIU_WriteByte(u32Reg, sIDVal);
}

MS_BOOL HAL_MIU_WriteRegBit(MS_U32 u32RegAddr, MS_U8 u8Mask, MS_BOOL bEnable)
{
    MS_U8 u8Val = HAL_MIU_ReadByte(u32RegAddr);
    if (!u32RegAddr)
    {
        MIU_HAL_ERR("%s reg error!\n", __FUNCTION__);
        return FALSE;
    }

    u8Val = HAL_MIU_ReadByte(u32RegAddr);
    u8Val = (bEnable) ? (u8Val | u8Mask) : (u8Val & ~u8Mask);
    HAL_MIU_WriteByte(u32RegAddr, u8Val);
    return TRUE;
}

MS_BOOL HAL_MIU_SetGroupID(MS_U8 u8MiuSel, MS_U8 u8Blockx, MS_U16 *pu8ProtectId, MS_U32 u32RegAddrID, MS_U32 u32RegAddrIDenable)
{
    MS_U32 u32index0, u32index1;
    MS_U16 u16ID;
    MS_U8 u8isfound0, u8isfound1;
    MS_U16 u16idenable;

    //reset IDenables for protect u8Blockx
    for(u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        IDEnables[u8MiuSel][u8Blockx][u32index0] = 0;
    }

    for(u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        u16ID = pu8ProtectId[u32index0];

        //Unused ID
        if(u16ID == 0)
           continue;

        u8isfound0 = FALSE;

        for(u32index1 = 0; u32index1 < MIU_MAX_PROTECT_ID; u32index1++)
        {
            if(IDs[u8MiuSel][u32index1] == u16ID)
            {
                //ID reused former setting
                IDEnables[u8MiuSel][u8Blockx][u32index1] = 1;
                u8isfound0 = TRUE;
                break;
            }
        }

        //Need to create new ID in IDs
        if(u8isfound0 != TRUE)
        {
            u8isfound1 = FALSE;

            for(u32index1 = 0; u32index1 < MIU_MAX_PROTECT_ID; u32index1++)
            {
                if(IDs[u8MiuSel][u32index1] == 0)
                {
                    IDs[u8MiuSel][u32index1] = u16ID;
                    IDEnables[u8MiuSel][u8Blockx][u32index1] = 1;
                    u8isfound1 = TRUE;
                    break;
                }
            }

            //ID overflow
            if(u8isfound1 == FALSE)
                return FALSE;
        }
    }

    u16idenable = 0;

    for(u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        if(IDEnables[u8MiuSel][u8Blockx][u32index0] == 1)
            u16idenable |= (1<<u32index0);
    }

    HAL_MIU_Write2Byte(u32RegAddrIDenable, u16idenable);

    for(u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
         HAL_MIU_SetProtectID(u32RegAddrID + u32index0, u8MiuSel, IDs[u8MiuSel][u32index0]);
    }

    return TRUE;
}

MS_BOOL HAL_MIU_ResetGroupID(MS_U8 u8MiuSel, MS_U8 u8Blockx, MS_U16 *pu8ProtectId, MS_U32 u32RegAddrID, MS_U32 u32RegAddrIDenable)
{
    MS_U32 u32index0, u32index1;
    MS_U8 u8isIDNoUse;
    MS_U16 u16idenable;

    //reset IDenables for protect u8Blockx
    for(u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        IDEnables[u8MiuSel][u8Blockx][u32index0] = 0;
    }

    u16idenable = 0x0UL;

    HAL_MIU_Write2Byte(u32RegAddrIDenable, u16idenable);

    for(u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
        u8isIDNoUse  = FALSE;

        for(u32index1 = 0; u32index1 < MIU_MAX_PROTECT_BLOCK; u32index1++)
        {
            if(IDEnables[u8MiuSel][u32index1][u32index0] == 1)
            {
                //protect ID is still be used
                u8isIDNoUse  = FALSE;
                break;
            }
            u8isIDNoUse  = TRUE;
        }

        if(u8isIDNoUse == TRUE)
            IDs[u8MiuSel][u32index0] = 0;
    }

    for(u32index0 = 0; u32index0 < MIU_MAX_PROTECT_ID; u32index0++)
    {
         HAL_MIU_SetProtectID(u32RegAddrID + u32index0, u8MiuSel, IDs[u8MiuSel][u32index0]);
    }

    return TRUE;
}

void HAL_MIU_Write2BytesBit(MS_U32 u32RegOffset, MS_BOOL bEnable, MS_U16 u16Mask)
{
    MS_U16 val = HAL_MIU_Read2Byte(u32RegOffset);
    val = (bEnable) ? (val | u16Mask) : (val & ~u16Mask);
    HAL_MIU_Write2Byte(u32RegOffset, val);
}

//-------------------------------------------------------------------------------------------------
//  MTLB HAL function
//-------------------------------------------------------------------------------------------------
//#ifdef CONFIG_MP_CMA_PATCH_DEBUG_STATIC_MIU_PROTECT

#define GET_HIT_BLOCK(regval)       BITS_RANGE_VAL(regval, REG_MIU_PROTECT_HIT_NO)
#define GET_HIT_CLIENT(regval)      BITS_RANGE_VAL(regval, REG_MIU_PROTECT_HIT_ID)

MS_BOOL HAL_MIU_CheckGroup(int group)
{
    if((group >=E_PROTECT_0) && (group <= E_MIU_PROTECT_MAX))
        return TRUE;
    else
        return FALSE;
}

MS_BOOL HAL_MIU_GetProtectInfo(MS_U8 u8MiuDev, MIU_PortectInfo *pInfo)
{
    MS_U16 ret = 0;
    MS_U16 loaddr = 0;
    MS_U16 hiaddr = 0;
    MS_U32 u32Reg = (u8MiuDev) ? MIU1_REG_BASE : MIU_REG_BASE;
    MS_U32 u32endAddr = 0;
    char clientName[40];

    if (!pInfo)
        return FALSE;

    memset(clientName, 0, sizeof(clientName));

    ret = HAL_MIU_Read2Byte(u32Reg+REG_MIU_PROTECT_STATUS);
    loaddr = HAL_MIU_Read2Byte(u32Reg+REG_MIU_PROTECT_LOADDR);
    hiaddr = HAL_MIU_Read2Byte(u32Reg+REG_MIU_PROTECT_HIADDR);

    if (REG_MIU_PROTECT_HIT_FALG & ret)
    {
        pInfo->bHit = TRUE;
        pInfo->u8Block = (MS_U8)GET_HIT_BLOCK(ret);
        pInfo->u8Group = (MS_U8)(GET_HIT_CLIENT(ret) >> 4);
        pInfo->u8ClientID = (MS_U8)(GET_HIT_CLIENT(ret) & 0x0F);
        pInfo->uAddress = (MS_U32)((hiaddr << 16) | loaddr) ;
        pInfo->uAddress = pInfo->uAddress * MIU_PROTECT_ADDRESS_UNIT;
        pInfo->u16ProtectType = HAL_MIU_Read2Byte(u32Reg + REG_MIU_PROTECT_ENGINE);
        u32endAddr = (pInfo->uAddress + MIU_PROTECT_ADDRESS_UNIT - 1);
        clientId_KernelProtectToName((MS_U8)(GET_HIT_CLIENT(ret)), clientName);

        if(pInfo->u8Block == 7)
        {
            printk(KERN_ERR "MIU%u OutofArea[W] Client:%s ID:%u-%u Hitted_Address(MIU):0x%x<->0x%x\n",
                        u8MiuDev, clientName, pInfo->u8Group, pInfo->u8ClientID, pInfo->uAddress, u32endAddr);
        }
        else if(pInfo->u8Block == 6)
        {
            printk(KERN_ERR "MIU%u OutofArea[R] Client:%s ID:%u-%u Hitted_Address(MIU):0x%x<->0x%x\n",
                        u8MiuDev, clientName, pInfo->u8Group, pInfo->u8ClientID, pInfo->uAddress, u32endAddr);
        }
        else
        {
            printk(KERN_ERR "MIU%u Block:%u Client:%s ID:%u-%u Hitted_Address(MIU):0x%x<->0x%x Type(0x69): 0x%x:\n",
                        u8MiuDev, pInfo->u8Block,clientName, pInfo->u8Group, pInfo->u8ClientID, pInfo->uAddress, u32endAddr, pInfo->u16ProtectType);
        }

        //clear log
        HAL_MIU_Write2BytesBit(u32Reg+REG_MIU_PROTECT_STATUS, TRUE, REG_MIU_PROTECT_LOG_CLR);
        HAL_MIU_Write2BytesBit(u32Reg+REG_MIU_PROTECT_STATUS, FALSE, REG_MIU_PROTECT_LOG_CLR);

    }

    return TRUE;
}
//#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MIU_GetDefaultClientID_KernelProtect()
/// @brief \b Function \b Description:  Get default client id array pointer for protect kernel
/// @param <RET>           \b     : The pointer of Array of client IDs
////////////////////////////////////////////////////////////////////////////////
MS_U16* HAL_MIU_GetDefaultClientID_KernelProtect()
{
     if(IDNUM_KERNELPROTECT > 0)
         return  (MS_U16 *)&clientId_KernelProtect[0];

     return NULL;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MIU_Protect()
/// @brief \b Function \b Description:  Enable/Disable MIU Protection mode
/// @param u8Blockx        \b IN     : MIU Block to protect (0 ~ 4)
/// @param *pu8ProtectId   \b IN     : Allow specified client IDs to write
/// @param u32Start        \b IN     : Starting bus address
/// @param u32End          \b IN     : End bus address
/// @param bSetFlag        \b IN     : Disable or Enable MIU protection
///                                      - -Disable(0)
///                                      - -Enable(1)
/// @param <OUT>           \b None    :
/// @param <RET>           \b None    :
/// @param <GLOBAL>        \b None    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_Protect(
                          MS_U8    u8Blockx,
                          MS_U16    *pu8ProtectId,
                          MS_U32   u32BusStart,
                          MS_U32   u32BusEnd,
                          PROTECT_CTRL  eSetFlag
                         )
{
    MS_U32 u32RegAddr;
    MS_U32 u32Reg;
    MS_U32 u32RegAddrStar;
    MS_U32 u32RegAddrMSB;
    MS_U32 u32RegAddrIDenable;
    MS_U32 u32MiuProtectEn;
    MS_U32 u32StartOffset;
    MS_U32 u32EndOffset;
    MS_U16 u16Data;
    MS_U16 u16Data1;
    MS_U16 u16Data2;
    MS_U8  u8Data;
    MS_U8  u8MiuSel;
    MS_U32 u32Start, u32End;

    u32Start = HAL_MIU_BA2PA(u32BusStart);
    u32End = HAL_MIU_BA2PA(u32BusEnd);

    // Get MIU selection and offset
    _phy_to_miu_offset(u8MiuSel, u32EndOffset, u32End);
    _phy_to_miu_offset(u8MiuSel, u32StartOffset, u32Start);

    u32Start = u32StartOffset;
    u32End = u32EndOffset;

    // Incorrect Block ID
    if(u8Blockx >= E_MIU_BLOCK_NUM)
    {
        MIU_HAL_ERR("Err: Out of the number of protect device\n");
        return FALSE;
    }
    else if(((u32Start & ((1 << MIU_PAGE_SHIFT) -1)) != 0) || ((u32End & ((1 << MIU_PAGE_SHIFT) -1)) != 0))
    {
        MIU_HAL_ERR("Err: Protected address should be aligned to 8KB\n");
        return FALSE;
    }
    else if(u32Start >= u32End)
    {
        MIU_HAL_ERR("Err: Start address is equal to or more than end address\n");
        return FALSE;
    }

    //write_enable
    u8Data = 1 << u8Blockx;

    if(u8MiuSel == E_CHIP_MIU_0)
    {
        u32RegAddrMSB = MIU_PROTECT0_MSB;
        u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);

        u32RegAddr = MIU_PROTECT0_ID0;
            u32MiuProtectEn=MIU_PROTECT_EN_INTERNAL;
        u32Reg = MIU_REG_BASE;

        switch (u8Blockx)
        {
            case E_MIU_BLOCK_0:
                u32RegAddrStar = MIU_PROTECT0_START;
                u32RegAddrIDenable = MIU_PROTECT0_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFFF0UL);
                break;
            case E_MIU_BLOCK_1:
                u32RegAddrStar = MIU_PROTECT1_START;
                u32RegAddrIDenable = MIU_PROTECT1_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFF0FUL);
                break;
            case E_MIU_BLOCK_2:
                u32RegAddrStar = MIU_PROTECT2_START;
                u32RegAddrIDenable = MIU_PROTECT2_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xF0FFUL);
                break;
            case E_MIU_BLOCK_3:
                u32RegAddrStar = MIU_PROTECT3_START;
                u32RegAddrIDenable = MIU_PROTECT3_ID_ENABLE;
                u16Data2 = (u16Data1 & 0x0FFFUL);
                break;
            default:
                        return FALSE;
        }
    }
    else if(u8MiuSel == E_CHIP_MIU_1)
    {
        u32RegAddrMSB = MIU1_PROTECT0_MSB;
        u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);

        u32RegAddr = MIU1_PROTECT0_ID0;
            u32MiuProtectEn=MIU1_PROTECT_EN;
        u32Reg = MIU1_REG_BASE;

        switch (u8Blockx)
        {
            case E_MIU_BLOCK_0:
                u32RegAddrStar = MIU1_PROTECT0_START;
                u32RegAddrIDenable = MIU1_PROTECT0_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFFF0UL);
                break;
            case E_MIU_BLOCK_1:
                u32RegAddrStar = MIU1_PROTECT1_START;
                u32RegAddrIDenable = MIU1_PROTECT1_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFF0FUL);
                break;
            case E_MIU_BLOCK_2:
                u32RegAddrStar = MIU1_PROTECT2_START;
                u32RegAddrIDenable = MIU1_PROTECT2_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xF0FFUL);
                break;
            case E_MIU_BLOCK_3:
                u32RegAddrStar = MIU1_PROTECT3_START;
                u32RegAddrIDenable = MIU1_PROTECT3_ID_ENABLE;
                u16Data2 = (u16Data1 & 0x0FFFUL);
                break;
            default:
                        return FALSE;
        }
     }
    else if(u8MiuSel == E_CHIP_MIU_2)
    {
        u32RegAddrMSB = MIU2_PROTECT0_MSB;
        u16Data1 = HAL_MIU_Read2Byte(u32RegAddrMSB);

        u32RegAddr = MIU2_PROTECT0_ID0;
            u32MiuProtectEn=MIU2_PROTECT_EN;
        u32Reg = MIU2_REG_BASE;

        switch (u8Blockx)
        {
            case E_MIU_BLOCK_0:
                u32RegAddrStar = MIU2_PROTECT0_START;
                u32RegAddrIDenable = MIU2_PROTECT0_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFFF0UL);
                break;
            case E_MIU_BLOCK_1:
                u32RegAddrStar = MIU2_PROTECT1_START;
                u32RegAddrIDenable = MIU2_PROTECT1_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xFF0FUL);
                break;
            case E_MIU_BLOCK_2:
                u32RegAddrStar = MIU2_PROTECT2_START;
                u32RegAddrIDenable = MIU2_PROTECT2_ID_ENABLE;
                u16Data2 = (u16Data1 & 0xF0FFUL);
                break;
            case E_MIU_BLOCK_3:
                u32RegAddrStar = MIU2_PROTECT3_START;
                u32RegAddrIDenable = MIU2_PROTECT3_ID_ENABLE;
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

    // Disable MIU protect
    HAL_MIU_WriteRegBit(u32MiuProtectEn,u8Data, DISABLE); //write disable
    HAL_MIU_WriteRegBit(u32MiuProtectEn,u8Data << 4, DISABLE); //read disable
    HAL_MIU_WriteRegBit(u32MiuProtectEn,u8Data << 8, DISABLE); //invert disable

    if ( eSetFlag )
    {
        // Set Protect IDs
        if(HAL_MIU_SetGroupID(u8MiuSel, u8Blockx, pu8ProtectId, u32RegAddr, u32RegAddrIDenable) == FALSE)
        {
            return FALSE;
        }

        // Set BIT29,30 of start/end address
        u16Data2 = u16Data2 | (MS_U16)((u32Start >> 29) << (u8Blockx*4));                                               // u16Data2 for start_ext addr
        u16Data1 = u16Data2 | (MS_U16)(((u32End - 1) >> 29) << (u8Blockx*4+2));
        HAL_MIU_Write2Byte(u32RegAddrMSB, u16Data1);

        // Start Address
        u16Data = (MS_U16)(u32Start >> MIU_PAGE_SHIFT);   //8k/unit
        HAL_MIU_Write2Byte(u32RegAddrStar , u16Data);

        // End Address
        u16Data = (MS_U16)((u32End >> MIU_PAGE_SHIFT)-1);   //8k/unit;
        HAL_MIU_Write2Byte(u32RegAddrStar + 2, u16Data);

        // Enable MIU protect
        if(eSetFlag == W_EN)
        {
            HAL_MIU_WriteRegBit(u32MiuProtectEn, u8Data, ENABLE); //write
        }
        else if(eSetFlag == R_EN)
        {
            HAL_MIU_WriteRegBit(u32MiuProtectEn, u8Data << 4, ENABLE); //read
        }
        else if(eSetFlag == WR_EN)
        {
            HAL_MIU_WriteRegBit(u32MiuProtectEn, u8Data, ENABLE); //write
            HAL_MIU_WriteRegBit(u32MiuProtectEn, u8Data << 4, ENABLE); //read
        }
        else if(eSetFlag == W_EN_INVERT)
        {
            HAL_MIU_WriteRegBit(u32MiuProtectEn, u8Data, ENABLE); //write
            HAL_MIU_WriteRegBit(u32MiuProtectEn+1, u8Data, ENABLE); //invert
        }
        else if(eSetFlag == R_EN_INVERT)
        {
            HAL_MIU_WriteRegBit(u32MiuProtectEn, u8Data << 4, ENABLE); //read
            HAL_MIU_WriteRegBit(u32MiuProtectEn+1, u8Data, ENABLE); //invert
        }
        else if(eSetFlag == WR_EN_INVERT)
        {
            HAL_MIU_WriteRegBit(u32MiuProtectEn, u8Data, ENABLE); //write
            HAL_MIU_WriteRegBit(u32MiuProtectEn, u8Data << 4, ENABLE); //read
            HAL_MIU_WriteRegBit(u32MiuProtectEn+1, u8Data, ENABLE); //invert
        }
        else
            printk(KERN_ERR"[miu protect] invalid operation !!\n");
    }
    else
    {
        // Reset Protect IDs
        HAL_MIU_ResetGroupID(u8MiuSel, u8Blockx, pu8ProtectId, u32RegAddr, u32RegAddrIDenable);
    }

    // clear log
    HAL_MIU_Write2BytesBit(u32Reg+REG_MIU_PROTECT_STATUS, TRUE, REG_MIU_PROTECT_LOG_CLR);
    HAL_MIU_Write2BytesBit(u32Reg+REG_MIU_PROTECT_STATUS, FALSE, REG_MIU_PROTECT_LOG_CLR);

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_MIU_ParseOccupiedResource
/// @brief \b Function  \b Description: Parse occupied resource to software structure
/// @return             \b 0: Fail 1: OK
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_ParseOccupiedResource(void)
{
    MS_U8  u8MiuSel;
    MS_U8  u8Blockx;
    MS_U8  u8ClientID;
    MS_U16 u16idenable;
    MS_U32 u32index;
    MS_U32 u32RegAddr;
    MS_U32 u32RegAddrIDenable;

    for(u8MiuSel = E_MIU_0; u8MiuSel < MIU_MAX_DEVICE; u8MiuSel++)
    {
        for(u8Blockx = E_MIU_BLOCK_0; u8Blockx < E_MIU_BLOCK_NUM; u8Blockx++)
        {
            if(u8MiuSel == E_MIU_0)
            {
                u32RegAddr = MIU_PROTECT0_ID0;

                switch (u8Blockx)
                {
                    case E_MIU_BLOCK_0:
                        u32RegAddrIDenable = MIU_PROTECT0_ID_ENABLE;
                        break;
                    case E_MIU_BLOCK_1:
                        u32RegAddrIDenable = MIU_PROTECT1_ID_ENABLE;
                        break;
                    case E_MIU_BLOCK_2:
                        u32RegAddrIDenable = MIU_PROTECT2_ID_ENABLE;
                        break;
                    case E_MIU_BLOCK_3:
                        u32RegAddrIDenable = MIU_PROTECT3_ID_ENABLE;
                        break;
                    default:
                        return false;
                }
            }
            else if(u8MiuSel == E_MIU_1)
            {
                u32RegAddr = MIU1_PROTECT0_ID0;

                switch (u8Blockx)
                {
                case E_MIU_BLOCK_0:
                     u32RegAddrIDenable = MIU1_PROTECT0_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_1:
                     u32RegAddrIDenable = MIU1_PROTECT1_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_2:
                     u32RegAddrIDenable = MIU1_PROTECT2_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_3:
                     u32RegAddrIDenable = MIU1_PROTECT3_ID_ENABLE;
                     break;
                 default:
                     return false;
                }
            }
            else if(u8MiuSel == E_MIU_2)
            {
                u32RegAddr = MIU2_PROTECT0_ID0;

                switch (u8Blockx)
                {
                case E_MIU_BLOCK_0:
                     u32RegAddrIDenable = MIU2_PROTECT0_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_1:
                     u32RegAddrIDenable = MIU2_PROTECT1_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_2:
                     u32RegAddrIDenable = MIU2_PROTECT2_ID_ENABLE;
                     break;
                 case E_MIU_BLOCK_3:
                     u32RegAddrIDenable = MIU2_PROTECT3_ID_ENABLE;
                     break;
                 default:
                     return false;
                }
            }
            else
            {
                printk(KERN_ERR "%s not support MIU%u!\n", __FUNCTION__, u8MiuSel );
                return FALSE;
            }

            u16idenable = HAL_MIU_Read2Byte(u32RegAddrIDenable);
            for(u32index = 0; u32index < MIU_MAX_PROTECT_ID; u32index++)
            {
                IDEnables[u8MiuSel][u8Blockx][u32index] = ((u16idenable >> u32index) & 0x1UL)? 1: 0;
            }
        }//for(u8Blockx = E_MIU_BLOCK_0; u8Blockx < E_MIU_BLOCK_NUM; u8Blockx++)

        for(u32index = 0; u32index < MIU_MAX_PROTECT_ID; u32index++)
        {
            u8ClientID = HAL_MIU_ReadByte(u32RegAddr + u32index) & 0x7F;
            IDs[u8MiuSel][u32index] = clientTbl[u8MiuSel][u8ClientID];
        }
    }//for(u8MiuSel = E_MIU_0; u8MiuSel < E_MIU_NUM; u8MiuSel++)

    return TRUE;
}

MS_BOOL HAL_Protect_Save(void)
{
    return TRUE;
}

MS_BOOL HAL_Protect_Restore(void)
{
    return TRUE;
}

unsigned int HAL_MIU_ProtectDramSize(void)
{
    MS_U8 u8Val = HAL_MIU_ReadByte(MIU_PROTECT_DDR_SIZE);
    u8Val = (u8Val >> 4) & 0xF;

    if (0 == u8Val)
    {
        printk("[%s][%d] MIU protect DRAM size is undefined. Using 0x40000000 as default\n", __FUNCTION__, __LINE__);
        return 0x40000000;
    }
    return (0x1 << (20 + u8Val));
}

int HAL_MIU_Info(MIU_DramInfo_Hal *pDramInfo)
{
    printk("Not support HAL_MIU_Info in this platform\n");

    return -1;
}

//// porting from k6 utopia
#define HAL_MIU_SSC_DBG(x)          // x
// #define MIU_HAL_ERR(x, args...)     {printf(x, ##args);}
#ifndef printf
#define printf printk
#endif

#define MPPL                        (432)
#define DDR_FACTOR                  (524288)
#define DDFSPAN_FACTOR              (131072)

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_MIU_SetSsc()
/// @brief \b Function \b Description: MDrv_MIU_SetSsc, @Step & Span
/// @param u16Fmodulation   \b IN : 20KHz ~ 40KHz (Input Value = 20 ~ 40)
/// @param u16FDeviation    \b IN  : under 0.1% ~ 2% (Input Value = 1 ~ 20)
/// @param bEnable          \b IN    :
/// @param None             \b OUT  :
/// @param None             \b RET  :
/// @param None             \b GLOBAL :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL HAL_MIU_SetSsc(MS_U8 u8MiuDev, MS_U16 u16Fmodulation, MS_U16 u16FDeviation, MS_BOOL bEnable)
{
    MS_U32 uDDFSET, uDDR_MHz, uDDFStep, uRegBase = MIU_ATOP_BASE;
    MS_U16 u16DDFSpan;
    MS_U16 u16Input_DIV_First,u16Input_DIV_Second,u16Loop_DIV_First,u16Loop_DIV_Second;
    MS_U8  u8Temp,i;

    //Pre check the input
    if (u8MiuDev == 1)
    {
#if 0
        if((HAL_MIU_Read2Byte(MIU1_REG_BASE)&BIT15))
        {
            uRegBase = MIU_ATOP_BASE+0x80;
        }
        else
        {
            printf("there is no MIU1\n");
            return 0;
        }
#else
        printf("there is no MIU1\n");
        return FALSE;
#endif
    }
    else
    {
        uRegBase = MIU_ATOP_BASE;
    }
    HAL_MIU_SSC_DBG(printf("MMIO base:%lx uRegBase:%lx\n", _gMIU_MapBase, uRegBase));

    if ((u16Fmodulation<20)||(u16Fmodulation>40))
    {
        MIU_HAL_ERR("SSC u16Fmodulation Error...(20KHz - 40KHz)\n");
        return 0;
    }

    if ((u16FDeviation<1)||(u16FDeviation>20))
    {
        MIU_HAL_ERR("SSC u16FDeviation Error...(0.1%% - 2%% ==> 1 ~20)\n");
        return 0;
    }

    HAL_MIU_SSC_DBG(printf("---> u16Fmodulation = %d u16FDeviation = %d \n",(int)u16Fmodulation,(int)u16FDeviation));
    //<1>.Caculate DDFM = (Loop_DIV_First * Loop_DIV_Second)/(Input_DIV_First * Input_DIV_Second);
    //Prepare Input_DIV_First
    u8Temp = ((MS_U16)(HAL_MIU_Read2Byte(uRegBase+MIU_DDRPLL_DIV_FIRST)&0x30)>>4);       //Bit 13,12 (0x110D36)
    u16Input_DIV_First = 0x01;
    for (i=0;i<u8Temp;i++)
        u16Input_DIV_First = u16Input_DIV_First << 1;
    //Prepare Input_DIV_Second
    u16Input_DIV_Second = (HAL_MIU_ReadByte(uRegBase+MIU_PLL_INPUT_DIV_2ND));     //Bit 0~7 (0x101222)
    if (u16Input_DIV_Second == 0)
        u16Input_DIV_Second = 1;
    //Prepare Loop_DIV_First
    u8Temp = ((HAL_MIU_ReadByte(uRegBase+MIU_DDRPLL_DIV_FIRST)&0xC0)>>6);         //Bit 15,14 (0x110D36)
    u16Loop_DIV_First = 0x01;
    for (i=0;i<u8Temp;i++)
        u16Loop_DIV_First = u16Loop_DIV_First << 1;

    //Prepare Loop_DIV_Second
    u16Loop_DIV_Second = (HAL_MIU_ReadByte(uRegBase+MIU_PLL_LOOP_DIV_2ND));      //Bit 0~7 (0x101223)
    if (u16Loop_DIV_Second == 0)
        u16Loop_DIV_Second = 1;

    //<2>.From DDFSET register to get DDRPLL
    uDDFSET = HAL_MIU_Read4Byte(uRegBase+MIU_DDFSET) & 0x00ffffff;
    //DDRPLL = MPPL * DDR_FACTOR * Loop_First * Loop_Second / DDFSET * Input_First * Input_Second
    HAL_MIU_SSC_DBG(printf("---> Loop_First:%u Loop_Second:%u\n", u16Loop_DIV_First, u16Loop_DIV_Second));
    HAL_MIU_SSC_DBG(printf("---> Input_first:%u Input_second:%u\n", u16Input_DIV_First, u16Input_DIV_Second));
    uDDR_MHz = (MPPL * DDR_FACTOR * u16Loop_DIV_First * u16Loop_DIV_Second)/ (uDDFSET*u16Input_DIV_First*u16Input_DIV_Second);
    HAL_MIU_SSC_DBG(printf("---> uDDFSET = 0x%lx\n",uDDFSET));
    HAL_MIU_SSC_DBG(printf("---> DDR_MHZ = 0x%lx (%d MHz)\n",uDDR_MHz,(int)uDDR_MHz));

    //<3>.Caculate DDFSPAN = (MPLL * DDFSPAN_FACTOR * MHz) / (DDFSET * Fmodulation * KHz)
    u16DDFSpan = (MS_U32)((DDFSPAN_FACTOR * MPPL/u16Fmodulation)* 1000/uDDFSET);
    HAL_MIU_SSC_DBG(printf("---> DDFSPAN = 0x%x (%d)\n",u16DDFSpan,(int)u16DDFSpan));
    if (u16DDFSpan > 0x3FFF)
    {
        u16DDFSpan = 0x3FFF;
        HAL_MIU_SSC_DBG(printf("??? DDFSPAN overflow > 0x3FFF, Fource set to 0x03FF\n"));
    }

    //Write to Register
    HAL_MIU_Write2Byte(uRegBase+MIU_DDFSPAN,u16DDFSpan);
    //<4>.Caculate DDFSTEP = (FDeviation*DDFSET/10)/(DDFSPAN*100)
    uDDFStep = (MS_U32)((u16FDeviation * (uDDFSET/10))/(u16DDFSpan*100));
    HAL_MIU_SSC_DBG(printf("---> DDFSTEP = 0x%lx (%lu)\n",uDDFStep,uDDFStep));
    //Write to Register
    uDDFStep &= (0x03FF);
    HAL_MIU_Write2Byte(uRegBase+MIU_DDFSTEP,(HAL_MIU_Read2Byte(uRegBase+MIU_DDFSTEP) & (~0x03FF))|uDDFStep);

    //<5>.Set ENABLE
    if(bEnable == ENABLE)
    {
        HAL_MIU_WriteByte(uRegBase+MIU_SSC_EN,(HAL_MIU_ReadByte(uRegBase+MIU_SSC_EN)|0xC0));
    }
    else
    {
        HAL_MIU_WriteByte(uRegBase+MIU_SSC_EN,(HAL_MIU_ReadByte(uRegBase+MIU_SSC_EN)&(~0xC0))|0x80);
    }
    return 1;
}
