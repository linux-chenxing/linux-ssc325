/*
* mhal_rng_reg.h- Sigmastar
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
#ifndef _MHAL_RNG_REG_H_
#define _MHAL_RNG_REG_H_

// unit: ms< --> ( HZ / 1000 )
#define GmacInputRNGJiffThreshold    (10 * ( HZ / 1000 ))
#define GMAC_RIU_MAP                              0xBF200000
#define GMAC_RIU                                         ((unsigned short volatile *) RIU_MAP)
#define GMAC_REG_MIPS_BASE              (0x1D00)
#define GMAC_MIPS_REG(addr)                RIU[(addr<<1)+REG_MIPS_BASE]
#define GMAC_REG_RNG_OUT                 0x0e

#endif
