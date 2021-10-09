/*
* mhal_iic.h- Sigmastar
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
#ifndef _HAL_HWI2C_H_
#define _HAL_HWI2C_H_

#ifdef _HAL_IIC_C_
#define _extern_HAL_IIC_
#else
#define _extern_HAL_IIC_ extern
#endif

#include <asm/types.h>
#include "mdrv_types.h"

////////////////////////////////////////////////////////////////////////////////
/// @file halHWI2C.h
/// @brief MIIC control functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Header Files
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Define & data type
////////////////////////////////////////////////////////////////////////////////
//v: value n: shift n bits
//v: value n: shift n bits
#define _LShift(v, n)       ((v) << (n))
#define _RShift(v, n)       ((v) >> (n))

#define HIGH_BYTE(val)      (u8)_RShift((val), 8)
#define LOW_BYTE(val)       ((u8)((val) & 0xFF))

#define __BIT(x)    ((u8)_LShift(1, x))
#define __BIT0       __BIT(0)
#define __BIT1       __BIT(1)
#define __BIT2       __BIT(2)
#define __BIT3       __BIT(3)
#define __BIT4       __BIT(4)
#define __BIT5       __BIT(5)
#define __BIT6       __BIT(6)
#define __BIT7       __BIT(7)
#if 0
//////////////////////////////////////////////////////////////////////////////////////
typedef unsigned int               bool;                            // 1 byte
/// data type unsigned char, data length 1 byte
typedef unsigned char               u8;                              // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short              u16;                             // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned int               u32;                             // 4 bytes
/// data type unsigned int64, data length 8 byte
typedef unsigned long         MS_U64;                             // 8 bytes
/// data type signed char, data length 1 byte
typedef signed char                 MS_S8;                              // 1 byte
/// data type signed short, data length 2 byte
typedef signed short                MS_S16;                             // 2 bytes
/// data type signed int, data length 4 byte
typedef signed int                 MS_S32;                             // 4 bytes
/// data type signed int64, data length 8 byte
typedef signed long            MS_S64;                             // 8 bytes
/////////////////////////////////////////////////////////////////////////////////
#endif

#define HWI2C_SET_RW_BIT(bRead, val) ((bRead) ? ((val) | __BIT0) : ((val) & ~__BIT0))

#define HAL_HWI2C_PORTS         4
#define HAL_HWI2C_PORT0         0
#define HAL_HWI2C_PORT1         1
#define HAL_HWI2C_PORT2         2
#define HAL_HWI2C_PORT3         3

typedef enum _HAL_HWI2C_STATE
{
	E_HAL_HWI2C_STATE_IDEL = 0,
	E_HAL_HWI2C_STATE_START,
	E_HAL_HWI2C_STATE_WRITE,
	E_HAL_HWI2C_STATE_READ,
	E_HAL_HWI2C_STATE_INT,
	E_HAL_HWI2C_STATE_WAIT,
	E_HAL_HWI2C_STATE_STOP
} HAL_HWI2C_STATE;


typedef enum _HAL_HWI2C_PORT
{
    E_HAL_HWI2C_PORT0_0 = 0, //disable port 0
    E_HAL_HWI2C_PORT0_1,
    E_HAL_HWI2C_PORT0_2,
    E_HAL_HWI2C_PORT0_3,
    E_HAL_HWI2C_PORT0_4,
    E_HAL_HWI2C_PORT0_5,
    E_HAL_HWI2C_PORT0_6,
    E_HAL_HWI2C_PORT0_7,

    E_HAL_HWI2C_PORT1_0,  //disable port 1
    E_HAL_HWI2C_PORT1_1,
    E_HAL_HWI2C_PORT1_2,
    E_HAL_HWI2C_PORT1_3,
    E_HAL_HWI2C_PORT1_4,
    E_HAL_HWI2C_PORT1_5,
    E_HAL_HWI2C_PORT1_6,
    E_HAL_HWI2C_PORT1_7,

    E_HAL_HWI2C_PORT2_0,  //disable port 2
    E_HAL_HWI2C_PORT2_1,
    E_HAL_HWI2C_PORT2_2,
    E_HAL_HWI2C_PORT2_3,
    E_HAL_HWI2C_PORT2_4,
    E_HAL_HWI2C_PORT2_5,
    E_HAL_HWI2C_PORT2_6,
    E_HAL_HWI2C_PORT2_7,

    E_HAL_HWI2C_PORT3_0, //disable port 3
    E_HAL_HWI2C_PORT3_1,
    E_HAL_HWI2C_PORT3_2,
    E_HAL_HWI2C_PORT3_3,
    E_HAL_HWI2C_PORT3_4,
    E_HAL_HWI2C_PORT3_5,
    E_HAL_HWI2C_PORT3_6,
    E_HAL_HWI2C_PORT3_7,

    E_HAL_HWI2C_PORT_NOSUP
}HAL_HWI2C_PORT;

typedef enum _HAL_HWI2C_CLKSEL
{
    E_HAL_HWI2C_CLKSEL_HIGH = 0,
    E_HAL_HWI2C_CLKSEL_NORMAL,
    E_HAL_HWI2C_CLKSEL_SLOW,
    E_HAL_HWI2C_CLKSEL_VSLOW,
    E_HAL_HWI2C_CLKSEL_USLOW,
    E_HAL_HWI2C_CLKSEL_UVSLOW,
    E_HAL_HWI2C_CLKSEL_NOSUP
}HAL_HWI2C_CLKSEL;

typedef enum _HAL_HWI2C_CLK_SEL
{
    E_HAL_HWI2C_CLK_25KHZ = 0,
    E_HAL_HWI2C_CLK_50KHZ,
    E_HAL_HWI2C_CLK_100KHZ,
    E_HAL_HWI2C_CLK_200KHZ,
    E_HAL_HWI2C_CLK_300KHZ,
    E_HAL_HWI2C_CLK_400KHZ,
    E_HAL_HWI2C_CLK_600KHZ,
    E_HAL_HWI2C_CLK_800KHZ,
    E_HAL_HWI2C_CLK_1000KHZ,
    E_HAL_HWI2C_CLK_1500KHZ,
    E_HAL_HWI2C_CLK_NOSUP      /// non-support speed
}HAL_HWI2C_CLK_SEL;

typedef enum {
    E_HAL_HWI2C_READ_MODE_DIRECT,                       ///< first transmit slave address + reg address and then start receive the data */
    E_HAL_HWI2C_READ_MODE_DIRECTION_CHANGE,             ///< slave address + reg address in write mode, direction change to read mode, repeat start slave address in read mode, data from device
    E_HAL_HWI2C_READ_MODE_DIRECTION_CHANGE_STOP_START,  ///< slave address + reg address in write mode + stop, direction change to read mode, repeat start slave address in read mode, data from device
    E_HAL_HWI2C_READ_MODE_MAX
} HAL_HWI2C_ReadMode;

typedef enum _HAL_HWI2C_DMA_ADDRMODE
{
    E_HAL_HWI2C_DMA_ADDR_NORMAL = 0,
    E_HAL_HWI2C_DMA_ADDR_10BIT,
    E_HAL_HWI2C_DMA_ADDR_MAX,
}HAL_HWI2C_DMA_ADDRMODE;

typedef enum _HAL_HWI2C_DMA_MIUPRI
{
    E_HAL_HWI2C_DMA_PRI_LOW = 0,
    E_HAL_HWI2C_DMA_PRI_HIGH,
    E_HAL_HWI2C_DMA_PRI_MAX,
}HAL_HWI2C_DMA_MIUPRI;

typedef enum _HAL_HWI2C_DMA_MIUCH
{
    E_HAL_HWI2C_DMA_MIU_CH0 = 0,
    E_HAL_HWI2C_DMA_MIU_CH1,
    E_HAL_HWI2C_DMA_MIU_MAX,
}HAL_HWI2C_DMA_MIUCH;

typedef struct _HAL_HWI2C_PinCfg
{
    u32  u32Reg;    /// register
    u8   u8BitPos;  /// bit position
    bool bEnable;   /// enable or disable
}HAL_HWI2C_PinCfg;

typedef struct _HAL_HWI2C_PortCfg //Synchronize with drvHWI2C.h
{
    u32                  u32DmaPhyAddr;  /// DMA physical address
    HAL_HWI2C_DMA_ADDRMODE  eDmaAddrMode;   /// DMA address mode
    HAL_HWI2C_DMA_MIUPRI    eDmaMiuPri;     /// DMA miu priroity
    HAL_HWI2C_DMA_MIUCH     eDmaMiuCh;      /// DMA miu channel
    bool                 bDmaEnable;     /// DMA enable

    HAL_HWI2C_PORT          ePort;          /// number
    HAL_HWI2C_CLK_SEL        eSpeed;         /// clock speed
    HAL_HWI2C_ReadMode      eReadMode;      /// read mode
    bool                 bEnable;        /// enable

}HAL_HWI2C_PortCfg;

/// I2C Configuration for initialization
typedef struct _HAL_HWI2C_CfgInit //Synchronize with drvHWI2C.h
{
    HAL_HWI2C_PortCfg   sCfgPort[4];    /// port cfg info
    HAL_HWI2C_PinCfg    sI2CPin;        /// pin info
    HAL_HWI2C_CLK_SEL    eSpeed;         /// speed
    HAL_HWI2C_PORT      ePort;          /// port
    HAL_HWI2C_ReadMode  eReadMode;      /// read mode

}HAL_HWI2C_CfgInit;

typedef struct _HAL_HWI2C_ClkCntCfg
{
    u16 u16ClkHCnt;
    u16 u16ClkLCnt;
    u16 u16StpCnt;
    u16 u16SdaCnt;
    u16 u16SttCnt;
    u16 u16LchCnt;
}HAL_HWI2C_ClkCntCfg;

////////////////////////////////////////////////////////////////////////////////
// Extern function
////////////////////////////////////////////////////////////////////////////////
_extern_HAL_IIC_ u32 MsOS_PA2KSEG1(u32 addr);
_extern_HAL_IIC_ u32 MsOS_VA2PA(u32 addr);

_extern_HAL_IIC_ void HAL_HWI2C_ExtraDelay(u32 u32Us);
_extern_HAL_IIC_ void HAL_HWI2C_SetIOMapBase(u32 u32Base,u32 u32ChipBase,u32 u32ClkBase);
_extern_HAL_IIC_ u8 HAL_HWI2C_ReadByte(u32 u32RegAddr);
_extern_HAL_IIC_ u16 HAL_HWI2C_Read2Byte(u32 u32RegAddr);
_extern_HAL_IIC_ u32 HAL_HWI2C_Read4Byte(u32 u32RegAddr);
_extern_HAL_IIC_ bool HAL_HWI2C_WriteByte(u32 u32RegAddr, u8 u8Val);
_extern_HAL_IIC_ bool HAL_HWI2C_Write2Byte(u32 u32RegAddr, u16 u16Val);
_extern_HAL_IIC_ bool HAL_HWI2C_Write4Byte(u32 u32RegAddr, u32 u32Val);
_extern_HAL_IIC_ bool HAL_HWI2C_WriteRegBit(u32 u32RegAddr, u8 u8Mask, bool bEnable);
_extern_HAL_IIC_ bool HAL_HWI2C_WriteByteMask(u32 u32RegAddr, u8 u8Val, u8 u8Mask);

_extern_HAL_IIC_ bool HAL_HWI2C_Init_Chip(void);
_extern_HAL_IIC_ bool HAL_HWI2C_IsMaster(void);
_extern_HAL_IIC_ bool HAL_HWI2C_Master_Enable(u16 u16PortOffset);
_extern_HAL_IIC_ bool HAL_HWI2C_SetPortRegOffset(HAL_HWI2C_PORT ePort, u16* pu16Offset);
_extern_HAL_IIC_ bool HAL_HWI2C_GetPortIdxByOffset(u16 u16Offset, u8* pu8Port);
_extern_HAL_IIC_ bool HAL_HWI2C_GetPortIdxByPort(HAL_HWI2C_PORT ePort, u8* pu8Port);
_extern_HAL_IIC_ bool HAL_HWI2C_SelectPort(int ePort);
_extern_HAL_IIC_ bool HAL_HWI2C_SetClk(u16 u16PortOffset, HAL_HWI2C_CLKSEL eClkSel);
_extern_HAL_IIC_ bool HAL_HWI2C_SetClkCnt(u16 u16PortOffset, HAL_HWI2C_ClkCntCfg *clkcnt);

_extern_HAL_IIC_ bool HAL_HWI2C_Start(u16 u16PortOffset);
_extern_HAL_IIC_ bool HAL_HWI2C_Stop(u16 u16PortOffset);
_extern_HAL_IIC_ bool HAL_HWI2C_ReadRdy(u16 u16PortOffset);
_extern_HAL_IIC_ bool HAL_HWI2C_SendData(u16 u16PortOffset, u8 u8Data);
_extern_HAL_IIC_ u8 HAL_HWI2C_RecvData(u16 u16PortOffset);
_extern_HAL_IIC_ bool HAL_HWI2C_Get_SendAck(u16 u16PortOffset);
_extern_HAL_IIC_ bool HAL_HWI2C_NoAck(u16 u16PortOffset);
_extern_HAL_IIC_ bool HAL_HWI2C_Ack(u16 u16PortOffset);
_extern_HAL_IIC_ u8 HAL_HWI2C_GetState(u16 u16PortOffset);
_extern_HAL_IIC_ bool HAL_HWI2C_Is_Idle(u16 u16PortOffset);
_extern_HAL_IIC_ bool HAL_HWI2C_Is_INT(u16 u16PortOffset);
_extern_HAL_IIC_ bool HAL_HWI2C_Clear_INT(u16 u16PortOffset);
_extern_HAL_IIC_ bool HAL_HWI2C_Reset(u16 u16PortOffset, bool bReset);
_extern_HAL_IIC_ bool HAL_HWI2C_Send_Byte(u16 u16PortOffset, u8 u8Data);
_extern_HAL_IIC_ bool HAL_HWI2C_Recv_Byte(u16 u16PortOffset, u8 *pData);

_extern_HAL_IIC_ bool HAL_HWI2C_DMA_Init(u16 u16PortOffset, HAL_HWI2C_PortCfg* pstPortCfg);
_extern_HAL_IIC_ bool HAL_HWI2C_DMA_Enable(u16 u16PortOffset, bool bEnable);
_extern_HAL_IIC_ bool HAL_HWI2C_DMA_ReadBytes(u16 u16PortOffset, u16 u16SlaveCfg, u32 uAddrCnt, u8 *pRegAddr, u32 uSize, u8 *pData);
_extern_HAL_IIC_ bool HAL_HWI2C_DMA_WriteBytes(u16 u16PortOffset, u16 u16SlaveCfg, u32 uAddrCnt, u8 *pRegAddr, u32 uSize, u8 *pData, u8 u8Sendstop);

_extern_HAL_IIC_ void HAL_HWI2C_Init_ExtraProc(void);
_extern_HAL_IIC_ bool HAL_HWI2C_WriteChipByteMask(u32 u32RegAddr, u8 u8Val, u8 u8Mask);
_extern_HAL_IIC_ u8 HAL_HWI2C_ReadChipByte(u32 u32RegAddr);
_extern_HAL_IIC_ bool HAL_HWI2C_WriteChipByte(u32 u32RegAddr, u8 u8Val);


#endif  //_MHAL_HWI2C_H_

