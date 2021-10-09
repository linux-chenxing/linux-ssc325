/*
* ms_iic.c- Sigmastar
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
#include "drv_camclk_Api.h"
#include "cam_drv_i2c.h"
#include "ms_iic.h"
#include "mhal_iic.h"
#define I2C_ACCESS_DUMMY_TIME   5//3

// Local defines & local structures
////////////////////////////////////////////////////////////////////////////////
//define hwi2c ports
#define HWI2C_PORTS                   HAL_HWI2C_PORTS
#define HWI2C_PORT0                   HAL_HWI2C_PORT0
#define HWI2C_PORT1                   HAL_HWI2C_PORT1
#define HWI2C_PORT2                   HAL_HWI2C_PORT2
#define HWI2C_PORT3                   HAL_HWI2C_PORT3
#define HWI2C_STARTDLY                5 //us
#define HWI2C_STOPDLY                 5 //us
//define mutex
CamOsMutex_t i2cMutex[HWI2C_PORTM];
I2C_DMA HWI2C_DMA[HWI2C_PORTM];

#define HWI2C_MUTEX_CREATE(_P_)          //g_s32HWI2CMutex[_P_] = MsOS_CreateMutex(E_MSOS_FIFO, (char*)gu8HWI2CMutexName[_P_] , MSOS_PROCESS_SHARED)
#define HWI2C_MUTEX_LOCK(_P_)            //OS_OBTAIN_MUTEX(g_s32HWI2CMutex[_P_],MSOS_WAIT_FOREVER)
#define HWI2C_MUTEX_UNLOCK(_P_)          //OS_RELEASE_MUTEX(g_s32HWI2CMutex[_P_])
#define HWI2C_MUTEX_DELETE(_P_)          //OS_DELETE_MUTEX(g_s32HWI2CMutex[_P_])
#define MsOS_DelayTaskUs(x)				CamOsUsDelay(x)
#define MS_DEBUG_MSG(x)       x

//static MS_S32 g_s32HWI2CMutex[HWI2C_PORTM] = {-1,-1,-1,-1};
//static char gu8HWI2CMutexName[HWI2C_PORTM][13] = {"HWI2CMUTEXP0","HWI2CMUTEXP1","HWI2CMUTEXP2","HWI2CMUTEXP3"};

//#define EN_I2C_LOG
#ifdef EN_I2C_LOG
#define HWI2C_DBG_FUNC()               if (_geDbgLv >= E_HWI2C_DBGLV_ALL) \
                                            {MS_DEBUG_MSG(CamOsPrintf("\t====   %s   ====\n", __FUNCTION__);)}
#define HWI2C_DBG_INFO(x, args...)     if (_geDbgLv >= E_HWI2C_DBGLV_INFO ) \
                                            {MS_DEBUG_MSG(CamOsPrintf(x, ##args);)}
#define HWI2C_DBG_ERR(x, args...)      if (_geDbgLv >= E_HWI2C_DBGLV_ERR_ONLY) \
                                            {MS_DEBUG_MSG(CamOsPrintf(x, ##args);)}
#else
#define HWI2C_DBG_FUNC()               //CamOsPrintf("\t##########################   %s   ################################\n", __FUNCTION__)
#define HWI2C_DBG_INFO(x, args...)     //CamOsPrintf(x, ##args)
#define HWI2C_DBG_ERR(x, args...)      CamOsPrintf(x, ##args)
#endif

#ifdef CONFIG_MS_PADMUX
#include "mdrv_padmux.h"
#include "mdrv_puse.h"
#include "gpio.h"
#endif

//#define I2C_ACCESS_DUMMY_TIME   50
////////////////////////////////////////////////////////////////////////////////
// Local & Global Variables
////////////////////////////////////////////////////////////////////////////////
static bool _gbInit = FALSE;

#ifdef EN_I2C_LOG
static HWI2C_DbgLv _geDbgLv = E_HWI2C_DBGLV_INFO; //E_HWI2C_DBGLV_ERR_ONLY;
#endif

static HWI2C_State _geState = E_HWI2C_IDLE;
static u32 g_u32StartDelay = HWI2C_STARTDLY, g_u32StopDelay = HWI2C_STOPDLY;
static HWI2C_ReadMode g_HWI2CReadMode[HWI2C_PORTS];
//static HWI2C_PORT g_HWI2CPort[HWI2C_PORTS];
static u16 g_u16DelayFactor[HWI2C_PORTS];
static HAL_HWI2C_ClkCntCfg g_HWI2CClkCnt[HWI2C_PORTS];
static bool g_bDMAEnable[HWI2C_PORTS];

//static u8 g_HWI2CPortIdx = HWI2C_PORT0;

extern int  i2c_set_srcclk(u8 u8Port, HWI2C_SRC_CLK i2c_src_clk);
extern int  i2c_remove_srcclk(u8 u8Port);
extern void *MDrv_HWI2C_DMA_Alloc(I2C_DMA *dma, int i2cgroup);
extern void MDrv_HWI2C_DMA_DeAlloc(I2C_DMA *dma, int i2cgroup);
////////////////////////////////////////////////////////////////////////////////
// Local Function
////////////////////////////////////////////////////////////////////////////////
#define _MDrv_HWI2C_Send_Byte HAL_HWI2C_Send_Byte
#define _MDrv_HWI2C_Recv_Byte HAL_HWI2C_Recv_Byte

bool MDrv_HWI2C_Send_Byte(u16 u16PortOffset, u8 u8Data)
{
	return HAL_HWI2C_Send_Byte(u16PortOffset, u8Data);
}

bool MDrv_HWI2C_Recv_Byte(u16 u16PortOffset, u8 *pData)
{
	return HAL_HWI2C_Recv_Byte(u16PortOffset, pData);
}

bool MDrv_HWI2C_NoAck(u16 u16PortOffset)
{
    return HAL_HWI2C_NoAck(u16PortOffset);
}

bool MDrv_HWI2C_Reset(u16 u16PortOffset, bool bReset)
{
    return HAL_HWI2C_Reset(u16PortOffset, bReset);
}

bool MDrv_HWI2C_GetPortRegOffset(u8 u8Port, u16 *pu16Offset)
{
    HWI2C_DBG_FUNC();

    if(u8Port>=HWI2C_PORTS)
    {
        // HWI2C_DBG_ERR("Port index is %d >= max supported ports %d !\n", u8Port, HWI2C_PORTS);
        return FALSE;
    }
    return HAL_HWI2C_SetPortRegOffset(u8Port,pu16Offset);
}

bool _MDrv_HWI2C_ReadBytes(u8 u8Port, u16 u16SlaveCfg, u32 uAddrCnt, u8 *pRegAddr, u32 uSize, u8 *pData)
{
    u8 u8SlaveID = LOW_BYTE(u16SlaveCfg);
    u16 u16Offset = 0x00;
    u16 u16Dummy = I2C_ACCESS_DUMMY_TIME; // loop dummy
    bool bComplete = FALSE;
    u32 uAddrCntBkp,uSizeBkp;
    u8 *pRegAddrBkp,*pDataBkp;

    HWI2C_DBG_FUNC();

    _geState = E_HWI2C_READ_DATA;
    if (!pRegAddr)
        uAddrCnt = 0;
    if (!pData)
        uSize = 0;

    //check support port index
    if(u8Port>=HWI2C_PORTS)
    {
        // HWI2C_DBG_ERR("Port index is %d >= max supported ports %d !\n", u8Port, HWI2C_PORTS);
        return FALSE;
    }
    //no meaning operation
    if (!uSize)
    {
        // HWI2C_DBG_ERR("Read bytes error!\n");
        return FALSE;
    }

    //configure port register offset ==> important
    if(!MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset))
    {
        // HWI2C_DBG_ERR("Port index error!\n");
        return FALSE;
    }

    if(g_bDMAEnable[u8Port])
    {
        _geState = E_HWI2C_DMA_READ_DATA;
        return HAL_HWI2C_DMA_ReadBytes(u16Offset, u16SlaveCfg, uAddrCnt, pRegAddr, uSize, pData);
    }

    //start access routines
    uAddrCntBkp = uAddrCnt;
    pRegAddrBkp = pRegAddr;
    uSizeBkp = uSize;
    pDataBkp = pData;

    while (u16Dummy--)
    {
        if((g_HWI2CReadMode[u8Port]!=E_HWI2C_READ_MODE_DIRECT) && (uAddrCnt>0)&& (pRegAddr))
        {
            HAL_HWI2C_Start(u16Offset);
            //add extral delay by user configuration
            MsOS_DelayTaskUs(g_u32StartDelay);

            if (!_MDrv_HWI2C_Send_Byte(u16Offset,HWI2C_SET_RW_BIT(FALSE, u8SlaveID)))
                goto end;

            while(uAddrCnt--)
            {
                if (!_MDrv_HWI2C_Send_Byte(u16Offset,*pRegAddr))
                    goto end;
                pRegAddr++;
            }

            if(g_HWI2CReadMode[u8Port]==E_HWI2C_READ_MODE_DIRECTION_CHANGE_STOP_START)
            {
                HAL_HWI2C_Stop(u16Offset);
                //add extral delay by user configuration
                MsOS_DelayTaskUs(g_u32StopDelay);
            }
			else
			{
				HAL_HWI2C_Reset(u16Offset,TRUE);
				HAL_HWI2C_Reset(u16Offset,FALSE);
			}
        }

        //Very important to add delay to support all clock speeds
        //Strongly recommend that do not remove this delay routine
        HAL_HWI2C_ExtraDelay(g_u16DelayFactor[u8Port]);
        HAL_HWI2C_Start(u16Offset);

        //add extral delay by user configuration
        MsOS_DelayTaskUs(g_u32StartDelay);

        if (!_MDrv_HWI2C_Send_Byte(u16Offset,HWI2C_SET_RW_BIT(TRUE, u8SlaveID)))
            goto end;

        while(uSize)
        {
            ///////////////////////////////////
            //
            //  must set ACK/NAK before read ready
            //
            uSize--;
            if (uSize==0)
                HAL_HWI2C_NoAck(u16Offset);
            if (_MDrv_HWI2C_Recv_Byte(u16Offset,pData)==FALSE)
                goto end;
            pData++;
        }
        bComplete = TRUE;

    end:
        HAL_HWI2C_Stop(u16Offset);
        //add extral delay by user configuration
        MsOS_DelayTaskUs(g_u32StopDelay);
        if(u16Dummy&&(bComplete==FALSE))
        {
            uAddrCnt = uAddrCntBkp;
            pRegAddr = pRegAddrBkp;
            uSize = uSizeBkp;
            pData = pDataBkp;
            continue;
        }
        break;
    }
    _geState = E_HWI2C_IDLE;
	HAL_HWI2C_Reset(u16Offset,TRUE);
	HAL_HWI2C_Reset(u16Offset,FALSE);

    return bComplete;
}

bool _MDrv_HWI2C_WriteBytes(u8 u8Port, u16 u16SlaveCfg, u32 uAddrCnt, u8 *pRegAddr, u32 uSize, u8 *pData, u8 u8Sendstop)
{
    u8 u8SlaveID = LOW_BYTE(u16SlaveCfg);
    u16 u16Offset = 0x00;

    u16 u16Dummy = I2C_ACCESS_DUMMY_TIME; // loop dummy
    bool bComplete = FALSE;
    u32 uAddrCntBkp,uSizeBkp;
    u8 *pRegAddrBkp,*pDataBkp;

    HWI2C_DBG_FUNC();

    _geState = E_HWI2C_WRITE_DATA;
    if (!pRegAddr)
        uAddrCnt = 0;
    if (!pData)
        uSize = 0;

    //check support port index
    if(u8Port>=HWI2C_PORTS)
    {
        // HWI2C_DBG_ERR("Port index is %d >= max supported ports %d !\n", u8Port, HWI2C_PORTS);
        return FALSE;
    }
    //no meaning operation
    if (!uSize)
    {
        // HWI2C_DBG_ERR("Write bytes error!\n");
        return FALSE;
    }

    //configure port register offset ==> important
    if(!MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset))
    {
        // HWI2C_DBG_ERR("Port index error!\n");
        return FALSE;
    }

    if(g_bDMAEnable[u8Port])
    {
        _geState = E_HWI2C_DMA_WRITE_DATA;
        return HAL_HWI2C_DMA_WriteBytes(u16Offset, u16SlaveCfg, 0, pRegAddr, uSize, pData, u8Sendstop);
    }

    //start access routines
    uAddrCntBkp = uAddrCnt;
    pRegAddrBkp = pRegAddr;
    uSizeBkp = uSize;
    pDataBkp = pData;
    while(u16Dummy--)
    {
        HAL_HWI2C_Start(u16Offset);
        MsOS_DelayTaskUs(g_u32StartDelay);

        if (!_MDrv_HWI2C_Send_Byte(u16Offset,HWI2C_SET_RW_BIT(FALSE, u8SlaveID)))
        	{HWI2C_DBG_ERR("HWI2C_SET_RW_BIT error!\n");
            goto end;
        }
        while(uAddrCnt)
        {
            if (!_MDrv_HWI2C_Send_Byte(u16Offset, *pRegAddr))
            	{HWI2C_DBG_ERR("pRegAddr error!\n");
                goto end;
            }
            uAddrCnt--;
            pRegAddr++;
        }

        while(uSize)
        {
            if (!_MDrv_HWI2C_Send_Byte(u16Offset, *pData))
            	{HWI2C_DBG_ERR("pData error!\n");
                goto end;
            	}
            uSize--;
            pData++;
        }
        bComplete = TRUE;

    end:
        HAL_HWI2C_Stop(u16Offset);
        //add extral delay by user configuration
        MsOS_DelayTaskUs(g_u32StopDelay);
        if(u16Dummy&&(bComplete==FALSE))
        {
            uAddrCnt = uAddrCntBkp;
            pRegAddr = pRegAddrBkp;
            uSize = uSizeBkp;
            pData = pDataBkp;
            continue;
        }
        break;
    }
    _geState = E_HWI2C_IDLE;
	HAL_HWI2C_Reset(u16Offset,TRUE);
	HAL_HWI2C_Reset(u16Offset,FALSE);
    return bComplete;
}

#if defined(CONFIG_MS_PADMUX)
int MDrv_HWI2C_IsPadSet(void)
{
    // important: need to modify if more MDRV_PUSE_I2C? defined
    if (PAD_UNKNOWN != mdrv_padmux_getpad(MDRV_PUSE_I2C0_SCL) ||
        PAD_UNKNOWN != mdrv_padmux_getpad(MDRV_PUSE_I2C1_SCL))
    {
        return TRUE;
    }
    else{
        return FALSE;
    }
}
#endif

bool _MDrv_HWI2C_SelectPort(HWI2C_PORT ePort, u8 u8Port)
{
    u16 u16Offset = 0x00;
    //U8 u8Port = 0x00;
    bool bRet=TRUE;

    HWI2C_DBG_FUNC();

    //(2) Set pad mux for port number
#if defined(CONFIG_MS_PADMUX)
    if (0 == mdrv_padmux_active() ||
        FALSE == MDrv_HWI2C_IsPadSet() )
#endif
    {
        bRet &= HAL_HWI2C_SelectPort((HAL_HWI2C_PORT)ePort);
    }
    //(3) configure port register offset ==> important
    bRet &= MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset);

    //(4) master init
    bRet &= HAL_HWI2C_Master_Enable(u16Offset);

    return bRet;
}

bool _MDrv_HWI2C_SetDelayFactor(u8 u8Port, HWI2C_CLK_SEL eClk)
{
    switch(eClk)
    {
        case E_HWI2C_CLK_25KHZ:
            g_u16DelayFactor[u8Port] = (u16)(1<<5);
        break;
        case E_HWI2C_CLK_50KHZ:
            g_u16DelayFactor[u8Port] = (u16)(1<<4);
        break;
        case E_HWI2C_CLK_100KHZ:
            g_u16DelayFactor[u8Port] = (u16)(1<<3);
        break;
        case E_HWI2C_CLK_200KHZ:
            g_u16DelayFactor[u8Port] = (u16)(1<<2);
        break;
        case E_HWI2C_CLK_300KHZ:
            g_u16DelayFactor[u8Port] = (u16)(1<<1);
        break;
        case E_HWI2C_CLK_400KHZ:
        case E_HWI2C_CLK_600KHZ:
        case E_HWI2C_CLK_800KHZ:
        case E_HWI2C_CLK_1000KHZ:
        case E_HWI2C_CLK_1500KHZ:
            g_u16DelayFactor[u8Port] = (u16)(1<<0);
        break;
        default:
            HWI2C_DBG_ERR("[%s]Port%d clk: %u failed\n",__func__, u8Port, eClk);
            return FALSE;
        break;
    }
    return TRUE;
}

static bool _MDrv_HWI2C_SetClk(u8 u8Port, HWI2C_CLK_SEL eClk)
{
    u16 u16Offset = 0x00;
    int ret;
    HWI2C_SRC_CLK i2c_src_clk = E_HWI2C_SRC_CLK_12M;

    HWI2C_DBG_FUNC();
    HWI2C_DBG_INFO("Port%d clk: %u\n", u8Port, eClk);

    //check support port index
    if(u8Port >= HWI2C_PORTS)
    {
        // HWI2C_DBG_ERR("Port index is %d >= max supported ports %d !\n", u8Port, HWI2C_PORTS);
        return FALSE;
    }
    //check support clock speed
    if (eClk >= E_HWI2C_CLK_NOSUP)
    {
        // HWI2C_DBG_ERR("Clock [%u] is not supported!\n",eClk);
        return FALSE;
    }

    //configure port register offset ==> important
    if(!MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset))
    {
        // HWI2C_DBG_ERR("Port index error!\n");
        return FALSE;
    }
    if(!_MDrv_HWI2C_SetDelayFactor(u8Port, eClk)){
        return FALSE;
    }
    HWI2C_DBG_INFO("Port%d clk: %u offset:%d\n", u8Port, eClk, u16Offset);
    //switch case speed -> srcclk, clk cnt
    switch(eClk)
    {
        case E_HWI2C_CLK_25KHZ:
            i2c_src_clk = E_HWI2C_SRC_CLK_12M;
            g_HWI2CClkCnt[u8Port].u16ClkHCnt = 235;
            g_HWI2CClkCnt[u8Port].u16ClkLCnt = 237;
        break;
        case E_HWI2C_CLK_50KHZ:
            i2c_src_clk = E_HWI2C_SRC_CLK_12M;
            g_HWI2CClkCnt[u8Port].u16ClkHCnt = 115;
            g_HWI2CClkCnt[u8Port].u16ClkLCnt = 117;
        break;
        case E_HWI2C_CLK_100KHZ:
            i2c_src_clk = E_HWI2C_SRC_CLK_12M;
            g_HWI2CClkCnt[u8Port].u16ClkHCnt = 55;
            g_HWI2CClkCnt[u8Port].u16ClkLCnt = 57;
        break;
        case E_HWI2C_CLK_200KHZ:
            i2c_src_clk = E_HWI2C_SRC_CLK_12M;
            g_HWI2CClkCnt[u8Port].u16ClkHCnt = 25;
            g_HWI2CClkCnt[u8Port].u16ClkLCnt = 27;
        break;
        case E_HWI2C_CLK_300KHZ:
            i2c_src_clk = E_HWI2C_SRC_CLK_54M;
            g_HWI2CClkCnt[u8Port].u16ClkHCnt = 87;
            g_HWI2CClkCnt[u8Port].u16ClkLCnt = 89;
        break;
        case E_HWI2C_CLK_400KHZ:
            i2c_src_clk = E_HWI2C_SRC_CLK_54M;
            g_HWI2CClkCnt[u8Port].u16ClkHCnt = 64;
            g_HWI2CClkCnt[u8Port].u16ClkLCnt = 66;
        break;
        case E_HWI2C_CLK_600KHZ:
            i2c_src_clk = E_HWI2C_SRC_CLK_54M;
            g_HWI2CClkCnt[u8Port].u16ClkHCnt = 42;
            g_HWI2CClkCnt[u8Port].u16ClkLCnt = 44;
        break;
        case E_HWI2C_CLK_800KHZ:
            i2c_src_clk = E_HWI2C_SRC_CLK_54M;
            g_HWI2CClkCnt[u8Port].u16ClkHCnt = 30;
            g_HWI2CClkCnt[u8Port].u16ClkLCnt = 32;
        break;
        case E_HWI2C_CLK_1000KHZ:
            i2c_src_clk = E_HWI2C_SRC_CLK_72M;
            g_HWI2CClkCnt[u8Port].u16ClkHCnt = 33;
            g_HWI2CClkCnt[u8Port].u16ClkLCnt = 35;
        break;
        case E_HWI2C_CLK_1500KHZ:
            i2c_src_clk = E_HWI2C_SRC_CLK_72M;
            g_HWI2CClkCnt[u8Port].u16ClkHCnt = 21;
            g_HWI2CClkCnt[u8Port].u16ClkLCnt = 23;
        break;
        default:
            HWI2C_DBG_ERR("[%s]Port%d eClk%d failed\n", __func__, u8Port, eClk);
            return FALSE;
        break;
    }
    //HWI2C_DBG_ERR("[%s]Port%d eClk%d src%d\n", __func__, u8Port, eClk, i2c_src_clk);
    //HWI2C_DBG_ERR("[%s]g_HWI2CClkCnt[%d].u16ClkHCnt %d\n", __func__, u8Port, g_HWI2CClkCnt[u8Port].u16ClkHCnt);
    //HWI2C_DBG_ERR("[%s]g_HWI2CClkCnt[%d].u16ClkLCnt %d\n", __func__, u8Port, g_HWI2CClkCnt[u8Port].u16ClkLCnt);
    ret = i2c_set_srcclk(u8Port, i2c_src_clk);
    if(!ret){
        HWI2C_DBG_ERR("[%s]Failed\n", __func__);
        return FALSE;
    }

    return HAL_HWI2C_SetClkCnt(u16Offset, &g_HWI2CClkCnt[u8Port]);
}

bool _MDrv_HWI2C_SetReadMode(u8 u8Port, HWI2C_ReadMode eReadMode)
{
    HWI2C_DBG_FUNC();
    HWI2C_DBG_INFO("Port%d Readmode: %u\n", u8Port, eReadMode);
    //check support port index
    if(u8Port>=HWI2C_PORTS)
    {
        // HWI2C_DBG_ERR("Port index is %d >= max supported ports %d !\n", u8Port, HWI2C_PORTS);
        return FALSE;
    }
    if(eReadMode>=E_HWI2C_READ_MODE_MAX)
        return FALSE;
    g_HWI2CReadMode[u8Port] = eReadMode;
    return TRUE;
}

bool _MDrv_HWI2C_InitPort(HWI2C_UnitCfg *psCfg)
{
    u8 u8PortIdx = 0, u8Port = 0;
    u16 u16Offset = 0x00;
    bool bRet = TRUE;
    HWI2C_PortCfg stPortCfg;
    bool bPortRet = TRUE;
    
    HWI2C_DBG_FUNC();
    //(1) set default value for port variables
    for(u8PortIdx=0; u8PortIdx < HWI2C_PORTS; u8PortIdx++)
    {
        stPortCfg = psCfg->sCfgPort[u8PortIdx];
        if(stPortCfg.bEnable)
        {   
            //HWI2C_DBG_ERR("=========[%s] psCfg->eGroup %d usedrv %d  stPortCfg.ePort %d\n", __func__, psCfg->eGroup, psCfg->eIsUseMdrvPadmux, stPortCfg.ePort);
            if(psCfg->eIsUseMdrvPadmux == 1){
                u8Port = psCfg->eGroup;}
            else{
                bPortRet = HAL_HWI2C_GetPortIdxByPort(stPortCfg.ePort,&u8Port);}
                
            _MDrv_HWI2C_SelectPort(stPortCfg.ePort, u8Port);
            //HWI2C_DBG_ERR("=========[%s] psCfg->eGroup %d\n", __func__, psCfg->eGroup);
            if(psCfg->eIsUseMdrvPadmux == 1 || bPortRet)
            {
            	//set default clkcnt
                g_HWI2CClkCnt[u8Port].u16ClkHCnt = 25; 
                g_HWI2CClkCnt[u8Port].u16ClkLCnt = 27;
                g_HWI2CClkCnt[u8Port].u16SttCnt  = 38;
                g_HWI2CClkCnt[u8Port].u16StpCnt  = 38; 
                g_HWI2CClkCnt[u8Port].u16SdaCnt  =  5; 
                g_HWI2CClkCnt[u8Port].u16LchCnt  =  5;
                //HWI2C_DBG_ERR("[%s] u8Port %d %d\n", __func__, u8Port, stPortCfg.eSpeed);
                //set clock speed
                bRet &= _MDrv_HWI2C_SetClk(u8Port, stPortCfg.eSpeed);
                //set read mode
                bRet &= _MDrv_HWI2C_SetReadMode(u8Port, stPortCfg.eReadMode);
                //get port index
                bRet &= MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset);
                //master init
                bRet &= HAL_HWI2C_Master_Enable(u16Offset);
                //dma init
                bRet &= HAL_HWI2C_DMA_Init(u16Offset,(HAL_HWI2C_PortCfg*)&stPortCfg);
                g_bDMAEnable[u8Port] = stPortCfg.bDmaEnable;
                //dump port information
                CamOsPrintf("Port:%u Index=%u\n",u8Port,stPortCfg.ePort);
                CamOsPrintf("Enable=%u\n",stPortCfg.bEnable);
                CamOsPrintf("DmaReadMode:%u\n",stPortCfg.eReadMode);
                CamOsPrintf("Speed:%u\n",stPortCfg.eSpeed);
                CamOsPrintf("DmaEnable:%u\n",stPortCfg.bDmaEnable);
                CamOsPrintf("DmaAddrMode:%u\n",stPortCfg.eDmaAddrMode);
                CamOsPrintf("DmaMiuCh:%u\n",stPortCfg.eDmaMiuCh);
                CamOsPrintf("DmaMiuPri:%u\n",stPortCfg.eDmaMiuPri);
                CamOsPrintf("DmaPhyAddr:%x\n",stPortCfg.u32DmaPhyAddr);
            }
            else{
                HWI2C_DBG_ERR("[%s]Set Port failed %d, UseMdrvPadmux %d\n", __func__, stPortCfg.ePort, psCfg->eIsUseMdrvPadmux);
            }
        }
    }
    stPortCfg = psCfg->sCfgPort[0];
    bPortRet = TRUE;
    //(2) check initialized port : override above port configuration
    if(psCfg->eIsUseMdrvPadmux == 1){
        u8Port = psCfg->eGroup;}
    else{
        bPortRet = HAL_HWI2C_GetPortIdxByPort(stPortCfg.ePort,&u8Port);}
        
    _MDrv_HWI2C_SelectPort(stPortCfg.ePort, u8Port);
            
    if(psCfg->eIsUseMdrvPadmux == 1 || bPortRet)
    {
    	//set default clkcnt
        g_HWI2CClkCnt[u8Port].u16ClkHCnt = 25; 
        g_HWI2CClkCnt[u8Port].u16ClkLCnt = 27;
        g_HWI2CClkCnt[u8Port].u16SttCnt  = 38;
        g_HWI2CClkCnt[u8Port].u16StpCnt  = 38; 
        g_HWI2CClkCnt[u8Port].u16SdaCnt  =  5; 
        g_HWI2CClkCnt[u8Port].u16LchCnt  =  5;
        //set clock speed
        i2c_remove_srcclk(u8Port);
        bRet &=_MDrv_HWI2C_SetClk(u8Port, psCfg->eSpeed);
        //set read mode
        bRet &=_MDrv_HWI2C_SetReadMode(u8Port,psCfg->eReadMode);
        //get port index
        //configure port register offset ==> important
        bRet &= MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset);
        //master init
        bRet &= HAL_HWI2C_Master_Enable(u16Offset);
    }
    else{
        HWI2C_DBG_ERR("[%s]1.Set Port failed %d, UseMdrvPadmux %d\n", __func__, stPortCfg.ePort, psCfg->eIsUseMdrvPadmux);
    }
    //(3) dump allocated port information
    /*for(u8PortIdx=0; u8PortIdx < HWI2C_PORTS; u8PortIdx++)
    {
        HWI2C_DBG_INFO("HWI2C Allocated Port[%d] = 0x%02X\n",u8PortIdx,g_HWI2CPort[u8PortIdx]);
    }*/
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////
// Global Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_Init
/// @brief \b Function  \b Description: Init HWI2C driver
/// @param psCfg        \b IN: hw I2C config
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
bool MDrv_HWI2C_Init(HWI2C_UnitCfg *psCfg)
{
    bool bRet = TRUE;
    u8 u8Port=0;

    HWI2C_DBG_FUNC();

    HAL_HWI2C_SetIOMapBase(psCfg->eGroup,psCfg->eBaseAddr,psCfg->eChipAddr,psCfg->eClkAddr);

    //(2) Initialize pad mux and basic settings
    CamOsPrintf("Pinreg:%x bit:%u enable:%u speed:%u\n",psCfg->sI2CPin.u32Reg, psCfg->sI2CPin.u8BitPos, psCfg->sI2CPin.bEnable,psCfg->eSpeed);
    bRet &= HAL_HWI2C_Init_Chip();
    //(3) Initialize all port
    bRet &= _MDrv_HWI2C_InitPort(psCfg);
    //(4) Check final result
    if (!bRet)
    {
        HWI2C_DBG_ERR("I2C init fail!\n");
    }

    //(5) Extra procedure to do after initialization
    HAL_HWI2C_Init_ExtraProc();

    g_u32StartDelay = HWI2C_STARTDLY;
    g_u32StopDelay = HWI2C_STOPDLY;
    CamOsPrintf("START default delay %d(us)\n",(int)g_u32StartDelay);
    CamOsPrintf("STOP default delay %d(us)\n",(int)g_u32StopDelay);
    _gbInit = TRUE;

    CamOsPrintf("HWI2C_MUTEX_CREATE!\n");
    for(u8Port=0;u8Port<(u8)HWI2C_PORTS;u8Port++)
    {
        HWI2C_MUTEX_CREATE(u8Port);
    }
    CamOsPrintf("HWI2C(%d): initialized\n", psCfg->eGroup);
    return bRet;
}

void MDrv_HW_IIC_Init(void *base,void *chipbase,int i2cgroup,void *clkbase, int i2cpadmux, u8 IsUseMdrvPadmux, int i2cspeed, int i2c_enDma)
{
    HWI2C_UnitCfg pHwbuscfg[1];
    u8 j;

    CamOsMutexInit(&i2cMutex[i2cgroup]);
    memset(pHwbuscfg, 0, sizeof(HWI2C_UnitCfg));
    HWI2C_DMA[i2cgroup].i2c_virt_addr = (u8 *)MDrv_HWI2C_DMA_Alloc(HWI2C_DMA, i2cgroup);
    //We only initialze sCfgPort[0]
    for(j = 0 ; j < 1 ; j++)
    {
        pHwbuscfg[0].sCfgPort[j].bEnable = TRUE;
        //use default pad mode 1
        if(i2cgroup==0)
        {
            if(i2cpadmux == 1)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_1;
            else if(i2cpadmux == 2)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_2;
            else if(i2cpadmux == 3)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_3;
            else if(i2cpadmux == 4)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_4;
			else if(i2cpadmux == 5)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_5;
            else if(i2cpadmux == 6)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_6;
            else if(i2cpadmux == 7)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_7;
            else if(i2cpadmux == 8)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_8;
            else if(i2cpadmux == 9)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_9;
            else if(i2cpadmux == 10)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_10;
            else if(i2cpadmux == 11)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_11;
            else if(i2cpadmux == 12)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_12;
            else if(i2cpadmux == 13)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_13;
            else if(i2cpadmux == 14)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT0_14;
            else if(i2cpadmux == 0xff)
                pHwbuscfg[0].sCfgPort[j].ePort = 0xff;  //no define
        }else if(i2cgroup==1) {
            if(i2cpadmux == 1)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_1;
            else if(i2cpadmux == 2)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_2;
            else if(i2cpadmux == 3)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_3;
            else if(i2cpadmux == 4)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_4;
            else if(i2cpadmux == 5)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_5;
            else if(i2cpadmux == 6)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_6;
            else if(i2cpadmux == 7)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_7;
            else if(i2cpadmux == 8)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_8;
            else if(i2cpadmux == 9)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_9;
            else if(i2cpadmux == 10)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_10;
            else if(i2cpadmux == 11)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT1_11;
            else if(i2cpadmux == 0xff)
                pHwbuscfg[0].sCfgPort[j].ePort = 0xff;  //no define
        }else if(i2cgroup==2) {
            if(i2cpadmux == 1)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT2_1;
            else if(i2cpadmux == 2)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT2_2;
            else if(i2cpadmux == 3)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT2_3;
			else if(i2cpadmux == 4)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT2_4;
            else if(i2cpadmux == 5)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT2_5;
        }
        else if(i2cgroup==3) {
            if(i2cpadmux == 1)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT3_1;
            else if(i2cpadmux == 2)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT3_2;
            else if(i2cpadmux == 3)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT3_3;
            else if(i2cpadmux == 4)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT3_4;
            else if(i2cpadmux == 5)
                pHwbuscfg[0].sCfgPort[j].ePort = E_HAL_HWI2C_PORT3_5;
        }
        pHwbuscfg[0].sCfgPort[j].eSpeed= i2cspeed; //E_HWI2C_NORMAL; //E_HAL_HWI2C_CLKSEL_VSLOW;//pIIC_Param->u8ClockIIC;//
        pHwbuscfg[0].sCfgPort[j].eReadMode = E_HWI2C_READ_MODE_DIRECT;//pIIC_Param->u8IICReadMode;//
        if (i2c_enDma == -1)
        {
            if(i2cgroup==0)
                pHwbuscfg[0].sCfgPort[j].bDmaEnable = FALSE;  //Use default setting
            else
                pHwbuscfg[0].sCfgPort[j].bDmaEnable = TRUE;  //Use default setting */
        }
        else
        {
            pHwbuscfg[0].sCfgPort[j].bDmaEnable = i2c_enDma;
        }
        pHwbuscfg[0].sCfgPort[j].eDmaAddrMode = E_HWI2C_DMA_ADDR_NORMAL;  //Use default setting
        pHwbuscfg[0].sCfgPort[j].eDmaMiuPri = E_HWI2C_DMA_PRI_LOW;  //Use default setting
        pHwbuscfg[0].sCfgPort[j].eDmaMiuCh = E_HWI2C_DMA_MIU_CH0;  //Use default setting
        pHwbuscfg[0].sCfgPort[j].u32DmaPhyAddr = HWI2C_DMA[i2cgroup].i2c_dma_addr;  //Use default setting
        j++;
    }

	pHwbuscfg[0].sI2CPin.bEnable = FALSE;
	pHwbuscfg[0].sI2CPin.u8BitPos = 0;
	pHwbuscfg[0].sI2CPin.u32Reg = 0;
	pHwbuscfg[0].eSpeed = i2cspeed; //E_HWI2C_NORMAL; //E_HAL_HWI2C_CLKSEL_VSLOW;//pIIC_Param->u8ClockIIC;//
    pHwbuscfg[0].ePort = pHwbuscfg[0].sCfgPort[0].ePort; /// port
    pHwbuscfg[0].eReadMode = E_HWI2C_READ_MODE_DIRECT; //pIIC_Param->u8IICReadMode;//
    pHwbuscfg[0].eBaseAddr=(u32)base;
    pHwbuscfg[0].eChipAddr=(u32)chipbase;
    pHwbuscfg[0].eClkAddr=(u32)clkbase;
    pHwbuscfg[0].eGroup=i2cgroup;
    pHwbuscfg[0].eIsUseMdrvPadmux = IsUseMdrvPadmux;
    MDrv_HWI2C_Init(&pHwbuscfg[0]);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HW_IIC_DeInit
/// @brief \b Function  \b Description: de init iic
////////////////////////////////////////////////////////////////////////////////
void MDrv_HW_IIC_DeInit(int i2cgroup)
{
    //CamOsPrintf( "[%s] dma_free_coherent : %d\n", __func__, i2cgroup);
    MDrv_HWI2C_DMA_DeAlloc(HWI2C_DMA, i2cgroup);
    CamOsMutexDestroy(&i2cMutex[i2cgroup]);
}
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_Start
/// @brief \b Function  \b Description: send start bit
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
bool MDrv_HWI2C_Start(u16 u16PortOffset)
{
    HWI2C_DBG_FUNC();
    return HAL_HWI2C_Start(u16PortOffset);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_Stop
/// @brief \b Function  \b Description: send stop bit
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
bool MDrv_HWI2C_Stop(u16 u16PortOffset)
{
    HWI2C_DBG_FUNC();
    return HAL_HWI2C_Stop(u16PortOffset);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_GetPortIndex
/// @brief \b Function  \b Description: Get port index from port number
/// @param ePort        \b IN: port number
/// @param ePort        \b OUT: pointer to port index
/// @return             \b U8: Port Index
////////////////////////////////////////////////////////////////////////////////
bool MDrv_HWI2C_GetPortIndex(HWI2C_PORT ePort, u8* pu8Port)
{
    bool bRet=TRUE;

    HWI2C_DBG_FUNC();

    //(1) Get port index by port number
    bRet &= HAL_HWI2C_GetPortIdxByPort((HAL_HWI2C_PORT)ePort, pu8Port);
    HWI2C_DBG_INFO("ePort:0x%02X, u8Port:0x%02X\n",(u8)ePort,(u8)*pu8Port);

    return bRet;
}

bool MDrv_HWI2C_CheckAbility(HWI2C_DMA_HW_FEATURE etype,ms_i2c_feature_fp *fp)
{
	if(etype >= E_HWI2C_FEATURE_MAX){
		HWI2C_DBG_ERR("etype invalid %d\n", etype);
		return FALSE;
	}
	return HAL_HWI2C_CheckAbility(etype, fp);
}

//###################
//
//  Multi-Port Support: Port 0
//
//###################
#if 0
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_SelectPort
/// @brief \b Function  \b Description: Decide port index and pad mux for port number
/// @param ePort        \b IN: port number
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
bool MDrv_HWI2C_SelectPort(HWI2C_PORT ePort)
{
    HWI2C_DBG_FUNC();
    if(ePort >= E_HWI2C_PORT_NOSUP)
        return FALSE;
    return _MDrv_HWI2C_SelectPort(ePort);
}
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_SetClk
/// @brief \b Function  \b Description: Set HW I2C clock
/// @param eClk         \b IN: clock rate
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
bool MDrv_HWI2C_SetClk(HWI2C_CLKSEL eClk)
{
    HWI2C_DBG_FUNC();
    return _MDrv_HWI2C_SetClk(g_HWI2CPortIdx, eClk);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_SetReadMode
/// @brief \b Function  \b Description: Set HW I2C Read Mode
/// @param eClk         \b IN: ReadMode
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
bool MDrv_HWI2C_SetReadMode(HWI2C_ReadMode eReadMode)
{
    return _MDrv_HWI2C_SetReadMode(g_HWI2CPortIdx, eReadMode);
}
#endif
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_ReadByte
/// @brief \b Function  \b Description: read 1 byte data
/// @param u16SlaveCfg  \b IN: [15:8]: Channel number [7:0]:Slave ID
/// @param u8RegAddr    \b IN: target register address
/// @param pData        \b Out: read 1 byte data
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
bool MDrv_HWI2C_ReadByte(u16 u16SlaveCfg, u8 u8RegAddr, u8 *pData)
{
    HWI2C_DBG_FUNC();
    return MDrv_HWI2C_ReadBytes(u16SlaveCfg, 1, &u8RegAddr, 1, pData);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_ReadByte
/// @brief \b Function  \b Description: write 1 byte data
/// @param u16SlaveCfg  \b IN: [15:8]: Channel number [7:0]:Slave ID
/// @param u8RegAddr    \b IN: target register address
/// @param u8Data       \b IN: 1 byte data
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
bool MDrv_HWI2C_WriteByte(u16 u16SlaveCfg, u8 u8RegAddr, u8 u8Data)
{
    HWI2C_DBG_FUNC();
    return MDrv_HWI2C_WriteBytes(u16SlaveCfg, 1, &u8RegAddr, 1, &u8Data, 1);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_WriteBytes
/// @brief \b Function  \b Description: Init HWI2C driver
/// @param u16SlaveCfg  \b IN: [15:8]: Channel number [7:0]:Slave ID
/// @param uAddrCnt     \b IN: register address count
/// @param pRegAddr     \b IN: pointer to targert register address
/// @param uSize        \b IN: data length
/// @param pData        \b IN: data array
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
bool MDrv_HWI2C_WriteBytes(u16 u16SlaveCfg, u32 uAddrCnt, u8 *pRegAddr, u32 uSize, u8 *pData, u8 u8Sendstop)
{
    bool bRet;
    u8 u8Port;

    u8Port = HIGH_BYTE(u16SlaveCfg);
	//u8Port = g_HWI2CPortIdx;
    if(u8Port>=HWI2C_PORTS)
    {
        // HWI2C_DBG_ERR("Port index is %d >= max supported ports %d !\n", u8Port, HWI2C_PORTS);
        return FALSE;
    }
    //HWI2C_MUTEX_LOCK(u8Port);
	//mutex_lock(&i2cMutex);
    bRet = _MDrv_HWI2C_WriteBytes(u8Port,u16SlaveCfg,uAddrCnt,pRegAddr,uSize,pData, u8Sendstop);
   	//HWI2C_MUTEX_UNLOCK(u8Port);
	//mutex_unlock(&i2cMutex);
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_HWI2C_ReadBytes
/// @brief \b Function  \b Description: Init HWI2C driver
/// @param u16SlaveCfg  \b IN: [15:8]: Channel number [7:0]:Slave ID
/// @param uAddrCnt     \b IN: register address count
/// @param pRegAddr     \b IN: pointer to targert register address
/// @param uSize        \b IN: data length
/// @param pData        \b Out: read data aray
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
bool MDrv_HWI2C_ReadBytes(u16 u16SlaveCfg, u32 uAddrCnt, u8 *pRegAddr, u32 uSize, u8 *pData)
{
    bool bRet;
    u8 u8Port;

    u8Port = HIGH_BYTE(u16SlaveCfg);
    //u8Port = g_HWI2CPortIdx;
    if(u8Port>=HWI2C_PORTS)
    {
        // HWI2C_DBG_ERR("Port index is %d >= max supported ports %d !\n", u8Port, HWI2C_PORTS);
        return FALSE;
    }
    //HWI2C_MUTEX_LOCK(u8Port);
    //mutex_lock(&i2cMutex);
    bRet = _MDrv_HWI2C_ReadBytes(u8Port,u16SlaveCfg,uAddrCnt,pRegAddr,uSize,pData);
    //HWI2C_MUTEX_UNLOCK(u8Port);
    //mutex_unlock(&i2cMutex);
    return bRet;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

/*
+------------------------------------------------------------------------------
| FUNCTION    : ms_i2c_xfer_read
+------------------------------------------------------------------------------
| DESCRIPTION : This function is called by ms_i2c_xfer,
|                     used to read data from i2c bus
|           1. send start
|           2. send address + R (read bit), and wait ack
|           3. just set start_byte_read,
|              loop
|           4. wait interrupt is arised, then clear interrupt and read byte in
|           5. Auto generate NACK by IP,
|           6. the master does not acknowledge the final byte it receives.
|              This tells the slave that its transmission is done
|
| RETURN      : When the operation is success, it return 0.
|               Otherwise Negative number will be returned.
|
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| pmsg               | x |   | pass in the slave id (addr) and R/W flag
|--------------------+---+---+-------------------------------------------------
| pbuf                | x |   | the message buffer, the buffer used to fill
|                    |   |   | data readed
|--------------------+---+---+-------------------------------------------------
| length             | x |   | the byte to be readed from slave
+------------------------------------------------------------------------------
*/
char errBuf[4096];
char *perrBuf;

s32 ms_i2c_xfer_read(u8 u8Port, tI2cMsg *pmsg, u8 *pbuf, int length)
{
    bool ret = FALSE;
    u32 u32i = 0;
    u16 u16Offset = 0x00;

    if (NULL == pmsg)
    {
        CamOsPrintf("ERROR: in ms_i2c_xfer_read: pmsg is NULL pointer \r\n");
        return I2C_FAIL;
    }

    if (NULL == pbuf)
    {
        CamOsPrintf("ERROR: in ms_i2c_xfer_read: pbuf is NULL pointer \r\n");
        return I2C_FAIL;
    }

    //configure port register offset ==> important
    if(!MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset))
    {
        // HWI2C_DBG_ERR("Port index error!\n");
        return I2C_FAIL;
    }

    if(g_bDMAEnable[u8Port])
    {
        //CamOsPrintf("I2C read DMA: port = %#x\n", u8Port);
        ret = MDrv_HWI2C_ReadBytes((u8Port<< 8)|(((pmsg->addr & I2C_BYTE_MASK) << 1) | ((pmsg->flags & I2C_M_RD) ? 1 : 0)), 0, pbuf, length, pbuf);
        if(ret==FALSE)
        {
            perrBuf = &errBuf[0];
            memset(errBuf,0,4096);

            for (u32i = 0; u32i < length; u32i++)
            {
                perrBuf += sprintf(perrBuf,"%#x ", *pbuf);
                pbuf++;
            }
            CamOsPrintf("ERROR: Bus[%d] in ms_i2c_xfer_read: Slave dev NAK, Addr: %#x, Data: %s \r\n", (u16Offset/256),(((pmsg->addr & I2C_BYTE_MASK) << 1) | ((pmsg->flags & I2C_M_RD) ? 1 : 0)),errBuf);
            return I2C_TIMEOUT;
        } else {
            return I2C_SUCCESS;
        }
    }
    /* ***** 1. Send start bit ***** */
    if(!MDrv_HWI2C_Start(u16Offset))
    {
       CamOsPrintf("ERROR: in ms_i2c_xfer_read: Send Start error \r\n");
       return I2C_TIMEOUT;
    }
    // Delay for 1 SCL cycle 10us -> 4000T
    CamOsUsDelay(2);
    //LOOP_DELAY(8000); //20us

    /* ***** 2. Send slave id + read bit ***** */
    if (!MDrv_HWI2C_Send_Byte(u16Offset, ((pmsg->addr & I2C_BYTE_MASK) << 1) |
    			   ((pmsg->flags & I2C_M_RD) ? 1 : 0)))
    {

        perrBuf = &errBuf[0];
        memset(errBuf,0,4096);

        for (u32i = 0; u32i < length; u32i++)
        {
            perrBuf += sprintf(perrBuf,"%#x ", *pbuf);
            pbuf++;
        }
        CamOsPrintf("ERROR: Bus[%d] in ms_i2c_xfer_read: Slave dev NAK, Addr: %#x, Data: %s \r\n", (u16Offset/256),(((pmsg->addr & I2C_BYTE_MASK) << 1) | ((pmsg->flags & I2C_M_RD) ? 1 : 0)),errBuf);

        return I2C_TIMEOUT;
    }
    CamOsUsDelay(1);

    /* Read data */
    for (u32i = 0; u32i < length; u32i++)
    {
        /* ***** 6. Read byte data from slave ***** */
        if ((length-1) == u32i)
        {
            MDrv_HWI2C_NoAck(u16Offset);
        }
        ret = MDrv_HWI2C_Recv_Byte(u16Offset, pbuf);
        pbuf++;
    }

    return 0;
}

/*
+------------------------------------------------------------------------------
| FUNCTION    : ms_i2c_xfer_write
+------------------------------------------------------------------------------
| DESCRIPTION : This function is called by ms_i2c_xfer
|               used to write data to i2c bus the procedure is as following

|           1. send start
|           2. send address, and wait ack and clear interrupt in wait_ack()
|           loop
|           3. send byte
|           4. wait interrupt is arised, then clear interrupt
|                             and check if recieve ACK
|
| RETURN      : When the operation is success, it return 0.
|               Otherwise Negative number will be returned.
|
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| pmsg               | x |   | pass in the slave id (addr) and R/W flag
|--------------------+---+---+-------------------------------------------------
| pbuf                | x |   | the message buffer, the buffer used to fill
|                    |   |   | data readed
|--------------------+---+---+-------------------------------------------------
| length             | x |   | the byte to be writen from slave
+------------------------------------------------------------------------------
*/
s32 ms_i2c_xfer_write(u8 u8Port, tI2cMsg *pmsg, u8 *pbuf, int length, u8 u8Sendstop)
{
    u32 u32i = 0;
    u16 u16Offset = 0x00;
    s32 ret = FALSE;

    if (NULL == pmsg)
    {
        CamOsPrintf("ERROR: in ms_i2c_xfer_write: pmsg is NULL pointer \r\n");
        return I2C_FAIL;
    }

    if (NULL == pbuf)
    {
        CamOsPrintf("ERROR: in ms_i2c_xfer_write: pbuf is NULL pointer \r\n");
        return I2C_FAIL;
    }

    //configure port register offset ==> important
    if(!MDrv_HWI2C_GetPortRegOffset(u8Port,&u16Offset))
    {
        // HWI2C_DBG_ERR("Port index error!\n");
        return I2C_FAIL;
    }

    if(g_bDMAEnable[u8Port])
    {
        //CamOsPrintf("I2C write DMA: port = %#x\n", u8Port);
        ret = MDrv_HWI2C_WriteBytes((u8Port<< 8)|(((pmsg->addr & I2C_BYTE_MASK) << 1) | ((pmsg->flags & I2C_M_RD) ? 1 : 0)), length, pbuf, length, pbuf, u8Sendstop);
        if(ret==FALSE)
        {
            perrBuf = &errBuf[0];
            memset(errBuf,0,4096);

            for (u32i = 0; u32i < length; u32i++)
            {
                perrBuf += sprintf(perrBuf,"%#x ", *pbuf);
                pbuf++;
            }
            CamOsPrintf("ERROR: Bus[%d] in ms_i2c_xfer_write: Slave dev NAK, Addr: %#x, Data: %s \r\n", (u16Offset/256),(((pmsg->addr & I2C_BYTE_MASK) << 1) | ((pmsg->flags & I2C_M_RD) ? 1 : 0)),errBuf);
            return I2C_TIMEOUT;
        }else{
            return I2C_SUCCESS;
        }
    }

    /* ***** 1. Send start bit ***** */
    if(!MDrv_HWI2C_Start(u16Offset))
    {
        CamOsPrintf("ERROR: in ms_i2c_xfer_write: Send Start error \r\n");
        return I2C_TIMEOUT;
    }
    // Delay for 1 SCL cycle 10us -> 4000T
    //LOOP_DELAY(8000); //20us
    CamOsUsDelay(2);

    /* ***** 2. Send slave id + read bit ***** */
    if (!MDrv_HWI2C_Send_Byte(u16Offset, ((pmsg->addr & I2C_BYTE_MASK) << 1) |
                ((pmsg->flags & I2C_M_RD) ? 1 : 0)))
    {
        perrBuf = &errBuf[0];
        memset(errBuf,0,4096);

        for (u32i = 0; u32i < length; u32i++)
        {
            perrBuf += sprintf(perrBuf,"%#x ", *pbuf);
            pbuf++;
        }
        CamOsPrintf("ERROR: Bus[%d] in ms_i2c_xfer_write: Slave dev NAK, Addr: %#x, Data: %s \r\n", (u16Offset/256),(((pmsg->addr & I2C_BYTE_MASK) << 1) | ((pmsg->flags & I2C_M_RD) ? 1 : 0)),errBuf);
        return I2C_TIMEOUT;
     }

    /* ***** 3. Send register address and data to write ***** */
    /* we send register is first buffer */
    for (u32i = 0; u32i < length; u32i++)
    {
        /* ***** 4. Write high byte data to slave ***** */
        if(MDrv_HWI2C_Send_Byte(u16Offset, *pbuf))
        {
            pbuf++;
        }
        else
        {
            CamOsPrintf("ERROR: Bus[%d] in ms_i2c_xfer_write: Slave data NAK, Addr: %#x, Data: %#x \r\n", (u16Offset/256),(((pmsg->addr & I2C_BYTE_MASK) << 1) | ((pmsg->flags & I2C_M_RD) ? 1 : 0)),*pbuf);
            return I2C_TIMEOUT;
        }
    }

    return ret;
}

/*
+------------------------------------------------------------------------------
| FUNCTION    : ms_i2c_xfer
+------------------------------------------------------------------------------
| DESCRIPTION : This function will be called by i2c-core.c i2c-transfer()
|               i2c_master_send(), and i2c_master_recv()
|               We implement the I2C communication protocol here
|               Generic i2c master transfer entrypoint.
|
| RETURN      : When the operation is success, it return the number of message
|               requrested. Negative number when error occurs.
|
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| u8Port               | x |   | the adaptor which the communication will be
|                    |   |   | procceed
|--------------------+---+---+-------------------------------------------------
| pmsg               | x |   | the message buffer, the buffer with message to
|                    |   |   | be sent or used to fill data readed
|--------------------+---+---+-------------------------------------------------
| length                | x |   | number of message to be transfer
+------------------------------------------------------------------------------
*/
s32 ms_i2c_xfer_common(u8 u8Port, tI2cMsg *pmsg, int length)
{
    int i, err;
    u16 u16Offset = 0x00;
    bool bSendStop = 1;
    ms_i2c_feature_fp ms_i2c_nwrite_fp;
    tI2cMsg *ptmpmsg = pmsg;
    bool bDoRead = 0;

    HWI2C_DBG_INFO("ms_i2c_xfer_common: processing %d messages:\n", length);

    i = 0;
    err = 0;

    if (NULL == pmsg)
    {
        CamOsPrintf("ERROR: in ms_i2c_xfer_common: pmsg is NULL pointer \r\n");
        return I2C_FAIL;
    }

    //configure port register offset ==> important
    if(!MDrv_HWI2C_GetPortRegOffset(u8Port, &u16Offset))
    {
        // HWI2C_DBG_ERR("Port index error!\n");
        return I2C_FAIL;
    }

    CamOsMutexLock(&i2cMutex[u8Port]);
    MDrv_HWI2C_Reset(u16Offset,TRUE);
    CamOsUsDelay(1);
    MDrv_HWI2C_Reset(u16Offset,FALSE);
    CamOsUsDelay(1);

    //check read cmd
    for(i = 0; i < length; i++)
    {

        if(ptmpmsg->len && ptmpmsg->buf){
            if(ptmpmsg->flags & I2C_M_RD){
                bDoRead = 1;
                break;
            }
        }
        ptmpmsg++;
    }
    //query nwrite mode ability and proc nwrite
    if(MDrv_HWI2C_CheckAbility(E_HWI2C_FEATURE_NWRITE, &ms_i2c_nwrite_fp) && !bDoRead)
    {
        if(ms_i2c_nwrite_fp == NULL){
            return I2C_FAIL;
        }

        //HWI2C_DBG_ERR("ms_i2c_nwrite_fp num %d\n", num);
        ms_i2c_nwrite_fp(u16Offset, ((pmsg->addr & I2C_BYTE_MASK) << 1), pmsg, length);
    }
    else{
        /* in i2c-master_send or recv, the num is always 1,  */
        /* but use i2c_transfer() can set multiple message */

        for (i = 0; i < length; i++)
        {
#if 0
            CamOsPrintf(" #%d: %sing %d byte%s %s 0x%02x\n", i,
	                pmsg->flags & I2C_M_RD ? "read" : "writ",
	                pmsg->len, pmsg->len > 1 ? "s" : "",
	                pmsg->flags & I2C_M_RD ? "from" : "to", pmsg->addr);
#endif
            /* do Read/Write */
            if (pmsg->len && pmsg->buf) /* sanity check */
            {
                bSendStop = (pmsg->flags & I2C_CUST_M_NOSTOP) ? 0 : 1;

                if (pmsg->flags & I2C_M_RD)
                    err = ms_i2c_xfer_read(u8Port, pmsg, pmsg->buf, pmsg->len);
                else
                    err = ms_i2c_xfer_write(u8Port, pmsg, pmsg->buf, pmsg->len, bSendStop);

                if (err)
                {
                    CamOsMutexUnlock(&i2cMutex[u8Port]);
                    return err;
                }
            }
            pmsg++;        /* next message */
        }
    }
    /* ***** 6. Send stop bit ***** */
    /* finish the read/write, then issues the stop condition (P).
     * for repeat start, diposit stop, two start and one stop only
     */

    if(bSendStop)
    {
        MDrv_HWI2C_Stop(u16Offset);
    }

    CamOsMutexUnlock(&i2cMutex[u8Port]);
    return i;
}

