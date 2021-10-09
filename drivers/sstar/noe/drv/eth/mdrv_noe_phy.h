/*
* mdrv_noe_phy.h- Sigmastar
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
#ifndef _MDRV_NOE_PHY_
#define _MDRV_NOE_PHY_

#define EV_ICPLUS_PHY_ID0 0x0243
#define EV_ICPLUS_PHY_ID1 0x0D90
#define EV_MARVELL_PHY_ID0 0x0141
#define EV_MARVELL_PHY_ID1 0x0CC2
#define EV_VTSS_PHY_ID0 0x0007
#define EV_VTSS_PHY_ID1 0x0421
#define EV_ATHEROS_PHY_ID0 0x004d
#define EV_ATHEROS_PHY_ID1 0xd072
#define EV_REALTEK_PHY_ID0 0x001C
#define EV_REALTEK_PHY_ID1 0xC800

struct st_drv_phy_config {
    EN_NOE_GE_MAC ge;
    unsigned char mii_force_mode;
    EN_NOE_SPEED speed;
    EN_NOE_DUPLEX duplex;
};

void MDrv_NOE_PHY_Set_Config(u32 phy_addr, struct st_drv_phy_config *info);

#endif /* _MDRV_NOE_PHY_ */

