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
#ifndef __MHAL_MIU_CLIENT_H__
#define __MHAL_MIU_CLIENT_H__
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/device.h>
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
    {"OVERALL   ",0x00,0},
    {"DIP0_R    ",0x01,0},
    {"DIP0_W    ",0x02,0},
    {"LDC_R     ",0x03,0},
    {"SC2_FRM_W ",0x04,0},
    {"SC3_FRM   ",0x05,0},
    {"RSC_R     ",0x06,0},
    {"SC1_DBG_R ",0x07,0},
    {"CMDQ0_R   ",0x08,0},
    {"MOVDMA0   ",0x09,0},
    {"EMAC      ",0x0A,0},
    {"2DGE      ",0x0B,0},
    {"3DNR0_R   ",0x0C,0},
    {"3DNR0_W   ",0x0D,0},
    {"GOP4_R    ",0x0E,0},
    {"RSVD      ",0x0F,1},
    {"ISP_DMAG0 ",0x10,0},
    {"ISP_DMAG1 ",0x11,0},
    {"ISP_DMAG2 ",0x12,0},
    {"GOP2_R    ",0x13,0},
    {"GOP3_R    ",0x14,0},
    {"ISP_DMAG  ",0x15,0},
    {"ISP_STA   ",0x16,0},
    {"ISP_STA1_W",0x17,0},
    {"CMDQ1_R   ",0x18,0},
    {"MOVDMA1   ",0x19,0},
    {"MCU51     ",0x1A,0},
    {"DLA       ",0x1B,0},
    {"IVE       ",0x1C,0},
    {"RSVD      ",0x1D,1},
    {"RSVD      ",0x1E,1},
    {"RSVD      ",0x1F,1},
    {"RSVD      ",0x20,1},
    {"SC_ROT_R  ",0x21,0},
    {"SC_AIP_W  ",0x22,0},
    {"SC0_FRM_W ",0x23,0},
    {"SC0_SNP_W ",0x24,0},
    {"SC1_FRM_W ",0x25,0},
    {"GOP0_R    ",0x26,0},
    {"3DNR1_R   ",0x27,0},
    {"3DNR1_W   ",0x28,0},
    {"CMDQ2_R   ",0x29,0},
    {"BDMA      ",0x2A,0},
    {"AESDMA    ",0x2B,0},
    {"USB20     ",0x2C,0},
    {"USB20_H   ",0x2D,0},
    {"MIIC1     ",0x2E,0},
    {"URDMA     ",0x2F,0},
    {"VEN_R     ",0x30,0},
    {"VEN_W     ",0x31,0},
    {"JPE_W     ",0x32,0},
    {"JPE_R     ",0x33,0},
    {"DIP1_R    ",0x34,0},
    {"DIP1_W    ",0x35,0},
    {"GOP1_R    ",0x36,0},
    {"BACH0     ",0x37,0},
    {"BACH1     ",0x38,0},
    {"CMDQ3_R   ",0x39,0},
    {"SDIO      ",0x3A,0},
    {"FCIE      ",0x3B,0},
    {"MIIC2     ",0x3C,0},
    {"MIIC3     ",0x3D,0},
    {"RSVD      ",0x3E,1},
    {"RSVD      ",0x3F,1},
    {"CPU       ",CPU_CLIENT_ID,0},
};


#if MIU_NUM > 1
static struct miu_client miu1_clients[] =
{
    {"OVERALL   ",0x00,0,1,0,0},
};
#endif

#define MIU0_CLIENT_NUM sizeof(miu0_clients)/sizeof(miu0_clients[0])
#define MIU1_CLIENT_NUM sizeof(miu1_clients)/sizeof(miu1_clients[0])

#endif
