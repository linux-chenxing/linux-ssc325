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
    {"OVERALL  ",0x00,0},
    {"RSVD     ",0x01,1},
    {"VEN      ",0x02,0},
    {"USB30    ",0x03,0},
    {"JPE_R    ",0x04,0},
    {"JPE_W    ",0x05,0},
    {"BACH     ",0x06,0},
    {"AESDMA   ",0x07,0},
    {"USB20    ",0x08,0},
    {"EMAC     ",0x09,0},
    {"MCU51    ",0x0A,0},
    {"URDMA    ",0x0B,0},
    {"BDMA     ",0x0C,0},
    {"MOVDMA   ",0x0D,0},
    {"LDC      ",0x0E,0},
    {"RSVD     ",0x0F,1},
    {"CMDQ0_R  ",0x10,0},
    {"ISP_DMA_W",0x11,0},
    {"ISP_DMA_R",0x12,0},
    {"ISP_ROT_R",0x13,0},
    {"ISP_MLOAD",0x14,0},
    {"GOP      ",0x15,0},
    {"RSVD     ",0x16,1},
    {"DIP0_R   ",0x17,0},
    {"DIP0_W   ",0x18,0},
    {"SC0_FRM_W",0x19,0},
    {"SC0_FRM_R",0x1A,0},
    {"SC0_DBG_R",0x1B,0},
    {"SC1_FRM_W",0x1C,0},
    {"SC2_FRM_W",0x1D,0},
    {"SD30     ",0x1E,0},
    {"SDIO30   ",0x1F,0},
    {"RSVD     ",0x20,1},
    {"RSVD     ",0x21,1},
    {"CSI_TX_R ",0x22,0},
    {"ISP_DMAG ",0x23,0},
    {"GOP1_R   ",0x24,0},
    {"GOP2_R   ",0x25,0},
    {"USB20_H  ",0x26,0},
    {"MIIC2    ",0x27,0},
    {"MIIC1    ",0x28,0},
    {"3DNR0_W  ",0x29,0},
    {"3DNR0_R  ",0x2A,0},
    {"DLA      ",0x2B,0},
    {"RSVD     ",0x2C,1},
    {"RSVD     ",0x2D,1},
    {"MIIC0    ",0x2E,0},
    {"IVE      ",0x2F,0},
    {"CPU      ",CPU_CLIENT_ID,0},
    {"DLA_HIWAY",DLA_HIWAY_ID,0},
};



#define MIU0_CLIENT_NUM sizeof(miu0_clients)/sizeof(miu0_clients[0])
#define MIU1_CLIENT_NUM sizeof(miu1_clients)/sizeof(miu1_clients[0])

