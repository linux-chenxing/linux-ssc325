/*
* 2_Infinity2_upll_p0_pd.h- Sigmastar
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
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00100840), 0x32);
//[1]: PD_UPLL
//[4]: PD_CLKO_UPLL_20M
//[5]: PD_CLKO_UPLL_320M
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00100841), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00100842), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00100844), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00100845), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0010084e), 0x04);
//[0]: EN_CLKO_UPLL_384M
//[1]: EN_UPLL_PRDT2
//[2]: GCR_UPLL_PD_CLKO_AUDIO