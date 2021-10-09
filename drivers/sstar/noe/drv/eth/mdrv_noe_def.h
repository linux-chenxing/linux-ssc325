/*
* mdrv_noe_def.h- Sigmastar
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
#ifndef _MDRV_NOE_DEF_H_
#define _MDRV_NOE_DEF_H_


typedef enum {
    E_NOE_DEV_MAIN = 0,
    E_NOE_DEV_PSEUDO,
    E_NOE_DEV_LAN,
    E_NOE_DEV_WAN
}EN_NOE_DEV_IDX;



struct net_device *MDrv_NOE_Get_Dev(EN_NOE_DEV_IDX idx);
#endif
