/*
* 1_Infinity2_usb20_p3_pd.h- Sigmastar
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
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00010b00), 0x05);
// [0]: HS_DM_PDN
// [2]: HS_TED_PDN
// [3]: HS_PREAMP_PDN
// [4]: FL_XCVR_PDN
// [6]: IREF_PDN
// [7]: REG_PDN
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00010b01), 0xff);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00010b08), 0x80);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00010b09), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00010b10), 0x00);
//[7]PD_BG_CURRENT
//[6]HS_TXSER_EN