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
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/device.h>
#if defined(CONFIG_COMPAT)
#include <linux/compat.h>
#endif
#include "regMIU.h"

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
   {"OVERALL      ",0x00,0},
    {"GE           ",0x01,0},
    {"BDMA         ",0x02,0},
    {"MOVDMA       ",0x03,0},
    {"AESDMA       ",0x04,0},
    {"CMDQ0_R      ",0x05,0},
    {"CMDQ1_R      ",0x06,0},
    {"URDMA        ",0x07,0},
    {"CSI_TX_R     ",0x08,0},
    {"JPE_R        ",0x09,0},
    {"JPE_W        ",0x0A,0},
    {"JPD          ",0x0B,0},
    {"BACH         ",0x0C,0},
    {"BACH1        ",0x0D,0},
    {"IVE          ",0x0E,0},
    {"MCU51        ",0x0F,0},
    {"USB30        ",0x10,0},
    {"USB20        ",0x11,0},
    {"USB20_H      ",0x12,0},
    {"SD30         ",0x13,0},
    {"SDIO30       ",0x14,0},
    {"RSVD         ",0x15,1},
    {"SATA         ",0x16,0},
    {"EMAC         ",0x17,0},
    {"EMAC1        ",0x18,0},
    {"LDC          ",0x19,0},
    {"GOP          ",0x1A,0},
    {"GOP1_R       ",0x1B,0},
    {"GOP2_R       ",0x1C,0},
    {"GOP3_R       ",0x1D,0},
    {"GOP4_R       ",0x1E,0},
    {"GOP5_R       ",0x1F,0},
    {"VCODEC_R     ",0x20,0},
    {"VCODEC_W     ",0x21,0},
    {"ISP_ROT_R    ",0x22,0},
    {"ISP_MOT0W    ",0x23,0},
    {"ISP_MOT0R    ",0x24,0},
    {"ISP_MLOAD    ",0x25,0},
    {"GOP_DISP3    ",0x26,0},
    {"RSVD         ",0x27,1},
    {"RDMA_DIP     ",0x28,0},
    {"SC0_FRM_R    ",0x29,0},
    {"SC0_FRM_W    ",0x2A,0},
    {"SC1_FRM_W    ",0x2B,0},
    {"SC2_FRM_W    ",0x2C,0},
    {"SC3_FRM_RW   ",0x2D,0},
    {"SC4_FRM_RW   ",0x2E,0},
    {"SC5_FRM_RW   ",0x2F,0},
    {"RSVD         ",0x30,1},
    {"ISP_DMA_W    ",0x31,0},
    {"ISP_DMA_R    ",0x32,0},
    {"3DNR0_W      ",0x33,0},
    {"3DNR0_R      ",0x34,0},
    {"RSVD         ",0x35,1},
    {"PQ_VIP_DISP_W",0x36,1},
    {"DISP_CVBS_W  ",0x37,1},
    {"ISP_DMAG     ",0x38,0},
    {"GOP_DISP0    ",0x39,0},
    {"GOP_DISP1    ",0x3A,0},
    {"GOP_DISP2    ",0x3B,0},
    {"MOPROT0_Y    ",0x3C,0},
    {"MOPROT0_C    ",0x3D,0},
    {"MOPROT1_Y    ",0x3E,0},
    {"MOPROT1_C    ",0x3F,0},
    {"CPU_W        ",CPU_W_CLIENT_ID,0},
    {"DLA_HIWAY_R  ",DLA_HIWAY_R_ID,0},
    {"DLA_HIWAY_W  ",DLA_HIWAY_W_ID,0},
    {"CPU_R        ",CPU_R_CLIENT_ID,0},
};


#define MIU0_CLIENT_NUM sizeof(miu0_clients)/sizeof(miu0_clients[0])
#define MIU1_CLIENT_NUM sizeof(miu1_clients)/sizeof(miu1_clients[0])

