/*
* cam_drv_i2c.c- Sigmastar
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

#include "cam_drv_i2c.h"
#include "ms_iic.h"

s32 CamI2cOpen(tI2cHandle *pHandle, u8 nPortNum)
{
#if defined(__RTK_OS__)
    pHandle->nPortNum = nPortNum;

#elif defined(__KERNEL__)
    pHandle->pAdapter = (void *)i2c_get_adapter(nPortNum);

#endif
    return 0;
}

s32 CamI2cTransfer(tI2cHandle *pHandle, tI2cMsg *pMsg, u32 nMsgNum)
{
#if defined(__RTK_OS__)
    return ms_i2c_xfer_common(pHandle->nPortNum, pMsg, nMsgNum);

#elif defined(__KERNEL__)
    return i2c_transfer((struct i2c_adapter *)pHandle->pAdapter, pMsg, nMsgNum);

#endif
}

s32 CamI2cClose(tI2cHandle *pHandle)
{
#if defined(__RTK_OS__)
    pHandle->nPortNum = (-1);

#elif defined(__KERNEL__)

#endif
    return 0;
}

s32 CamI2cSetClk(void *pHandle, u32 clk)
{
    //ToDo
    return 0;
}

#if defined(__KERNEL__)
EXPORT_SYMBOL(CamI2cOpen);
EXPORT_SYMBOL(CamI2cTransfer);
EXPORT_SYMBOL(CamI2cClose);
#endif