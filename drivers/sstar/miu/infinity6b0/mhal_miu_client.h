/*
* mhal_miu_client.h- Sigmastar
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
#include <linux/module.h>
#if defined(CONFIG_COMPAT)
#include <linux/compat.h>
#endif

// common for all MIU
struct miu_client {
    char* name;
    short bw_client_id;
    short bw_rsvd;
    short bw_enabled;
    short bw_dump_en;
    short bw_filter_en;
    short bw_max;
    short bw_avg;
    short bw_min;
    short effi_avg;
    short effi_min;
    short effi_max;
    short bw_max_div_effi;
    short bw_avg_div_effi;
};

static struct miu_client miu0_clients[] =
{
    {"OVERALL   ",0x00,0,1,0,0},
    {"VEN_R     ",0x01,0,0,0,0},
    {"VEN_W     ",0x02,0,0,0,0},
    {"RSVD      ",0x03,1,0,0,0},
    {"JPE_R     ",0x04,0,0,0,0},
    {"JPE_W     ",0x05,0,0,0,0},
    {"BACH_RW   ",0x06,0,0,0,0},
    {"AESDMA_RW ",0x07,0,0,0,0},
    {"USB20_RW  ",0x08,0,0,0,0},
    {"EMAC_RW   ",0x09,0,0,0,0},
    {"MCU51_RW  ",0x0A,0,0,0,0},
    {"URDMA_RW  ",0x0B,0,0,0,0},
    {"BDMA_RW   ",0x0C,0,0,0,0},
    {"MOVDMA0_RW",0x0D,0,0,0,0},
    {"GOP3_R    ",0x0E,0,0,0,0},
    {"RSVD      ",0x0F,1,0,0,0},
    {"CMDQ_R    ",0x10,0,0,0,0},
    {"ISP_DMA_W ",0x11,0,0,0,0},
    {"ISP_DMA_R ",0x12,0,0,0,0},
    {"ISP_ROT_R ",0x13,0,0,0,0},
    {"ISP_MLOAD ",0x14,0,0,0,0},
    {"GOP       ",0x15,0,0,0,0},
    {"RSVD      ",0x16,1,0,0,0},
    {"DIP0_R    ",0x17,0,0,0,0},
    {"DIP0_W    ",0x18,0,0,0,0},
    {"SC0_FRAME ",0x19,0,0,0,0},
    {"RSVD      ",0x1A,1,0,0,0},
    {"SC0_DBG_R ",0x1B,0,0,0,0},
    {"SC1_FRAME ",0x1C,0,0,0,0},
    {"SC2_FRAME ",0x1D,0,0,0,0},
    {"SD30_RW   ",0x1E,0,0,0,0},
    {"SDIO30_RW ",0x1F,0,0,0,0},
    {"RSVD      ",0x20,1,0,0,0},
    {"RSVD      ",0x21,1,0,0,0},
    {"RSVD      ",0x22,1,0,0,0},
    {"RSVD      ",0x23,1,0,0,0},
    {"GOP1_R    ",0x24,0,0,0,0},
    {"GOP2_R    ",0x25,0,0,0,0},
    {"USB20_H_RW",0x26,0,0,0,0},
    {"IVE_RW    ",0x27,0,0,0,0},
    {"MIIC1_RW  ",0x28,0,0,0,0},
    {"3DNR0_W   ",0x29,0,0,0,0},
    {"3DNR0_R   ",0x2A,0,0,0,0},
    {"CPU       ",0x70,0,0,0,0},
};

#define MIU0_CLIENT_NUM sizeof(miu0_clients)/sizeof(miu0_clients[0])
#define MIU1_CLIENT_NUM sizeof(miu1_clients)/sizeof(miu1_clients[0])

