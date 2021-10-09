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
    {"CEVAXM6_0_RW     ",0x01,1},
    {"CEVAXM6_1_RW      ",0x02,0},
    {"VD_R2I_R    ",0x03,0},
    {"VD_R2_SUBSYS_R    ",0x04,0},
    {"VD_R2D_RW    ",0x05,0},
    {"CEVAXM6_2_RW     ",0x06,0},
    {"CEVAXM6_3_RW   ",0x07,0},
    {"AUDIO_R    ",0x08,0},
    {"AUDIO_AU2_R     ",0x09,0},
    {"AUDIO_AU3_W    ",0x0A,0},
    {"CMDQ_R    ",0x0B,0},
    {"XD2MIU_RW     ",0x0C,0},
    {"UART_DMA_RW   ",0x0D,0},
    {"BDMA_RW      ",0x0E,0},
    {"DUMMY_G0CF     ",0x0F,1},
    {"SC1_CROP_LDC  ",0x10,0},
    {"ISP_GOP1_R",0x11,0},
    {"CMDQ_TOP_1_R",0x12,0},
    {"NOE_RW",0x13,0},
    {"USB30_RW",0x14,0},
    {"ISP_STA_W      ",0x15,0},
    {"ISP_AF_STA1_W     ",0x16,1},
    {"ISP_GOP2_R   ",0x17,0},
    {"EMAC_RW   ",0x18,0},
    {"IVE_TOP_RW",0x19,0},
    {"ISP_GOP3_R",0x1A,0},
    {"MIIC0_RW",0x1B,0},
    {"MIIC1_RW",0x1C,0},
    {"MIIC2_RW",0x1D,0},
    {"ISP_SC1_DBG_R     ",0x1E,0},
    {"ISP_CMDQ_TOP2_R   ",0x1F,0},
    {"SDIO_RW     ",0x20,1},
    {"USB30_1_RW     ",0x21,1},
    {"USB30_2_RW ",0x22,0},
    {"SD30_RW ",0x23,0},
    {"JPE_W   ",0x24,0},
    {"JPE_R   ",0x25,0},
    {"U3DEV_RW  ",0x26,0},
    {"JPD_RW    ",0x27,0},
    {"GMAC_RW    ",0x28,0},
    {"FCIE5_RW  ",0x29,0},
    {"SECGMAC  ",0x2A,0},
    {"USB30M1_HS_RW      ",0x2B,0},
    {"SATA0_RW     ",0x2C,1},
    {"SATA1_RW,     ",0x2D,1},
    {"USB20_RW    ",0x2E,0},
    {"USB20_P1_RW      ",0x2F,0},
    {"ISP_GOP4_R     ",0x30,1},
    {"ISOSC_BLKS_RW     ",0x31,1},
    {"CMDQ_TOP5_R ",0x32,0},
    {"ISP_GOP0_R ",0x33,0},
    {"SC1_FRAME_W   ",0x34,0},
    {"SC1_SNAPSHOT_W   ",0x35,0},
    {"SC2_FRAME_W  ",0x36,0},
    {"CMDQ_TOP4_R    ",0x37,0},
    {"MFE0_R    ",0x38,0},
    {"MFE0_W  ",0x39,0},
    {"SC3_FRAME_RW  ",0x3A,0},
    {"DUMMY_G3CB      ",0x3B,0},
    {"DUMMY_G3CC     ",0x3C,1},
    {"MFE1_R,     ",0x3D,1},
    {"MFE1_R    ",0x3E,0},
    {"ISP_MLOAD_R      ",0x3F,0},
    {"VE_W     ",0x40,1},
    {"EVD_ENG1_RW     ",0x41,1},
    {"MGWIN0_R ",0x42,0},
    {"MGWIN1_R ",0x43,0},
    {"HVD_RW   ",0x44,0},
    {"HVD1_RW   ",0x45,0},
    {"DDI_0_RW  ",0x46,0},
    {"EVD_ENG0_RW    ",0x47,0},
    {"MFDEC0_1_R    ",0x48,0},
    {"ISPSC_DMAG  ",0x49,0},
    {"EVD_BBU_R  ",0x4A,0},
    {"HVD_BBU_R      ",0x4B,0},
    {"SC1_IPMAIN_RW     ",0x4C,1},
    {"SC1_OPM_R,     ",0x4D,1},
    {"MFDEC_1_1_R    ",0x4E,0},
    {"LDC_R      ",0x4F,0},		
	{"GOP0_R     ",0x50,1},
    {"GOP1_R     ",0x51,1},
    {"AUTO_DOWNLOAD_R ",0x52,0},
    {"SC_DIPW_RW ",0x53,0},
    {"MVOP_128BIT_R   ",0x54,0},
    {"MVOP1_R   ",0x55,0},
    {"FRC_IPM0_W  ",0x56,0},
    {"SC_IPSUB_RW    ",0x57,0},
    {"FRC_OPM0_R    ",0x58,0},
    {"MDWIN0_W  ",0x59,0},
    {"MFDEC0_R  ",0x5A,0},
    {"MFDEC1_R      ",0x5B,0},
    {"MDWIN1_W     ",0x5C,1},
    {"SC_DYN_SCL_R,     ",0x5D,1},
    {"VE_R    ",0x5E,0},
    {"GE_RW      ",0x5F,0},
	{"ISP_DMAG0_W     ",0x60,1},
    {"ISP_DMAG0_R     ",0x61,1},
    {"ISP_DMAG1_W ",0x62,0},
    {"ISP_DMAG1_R ",0x63,0},
    {"ISP_DMAG2_RW   ",0x64,0},
    {"ISP_DMAG3_RW   ",0x65,0},
    {"ISP_DMAG4_W  ",0x66,0},
    {"ISP_DMAG4_R    ",0x67,0},
    {"ISP_DMAG_RW    ",0x68,0},
    {"DMA_GENERAL_RW  ",0x69,0},
    {"SC1_DNR_RW  ",0x6A,0},
    {"DUMMY_G6CB      ",0x6B,0},
    {"MHE0_R     ",0x6C,1},
    {"MHE0_W,     ",0x6D,1},
    {"MHE1_R    ",0x6E,0},
    {"MHE1_W      ",0x6F,0},	
	{"MIPS_RW     ",0x70,1},
    {"G3D_RW     ",0x71,1},
    {"DUMMY",0x72,0},
    {"DUMMY ",0x73,0},
    {"DUMMY   ",0x74,0},
    {"DUMMY   ",0x75,0},
    {"DUMMY  ",0x76,0},
    {"DUMMY    ",0x77,0},
    {"DUMMY    ",0x78,0},
    {"DUMMY  ",0x79,0},
    {"DUMMY  ",0x7A,0},
    {"DUMMY      ",0x7B,0},
    {"DUMMY     ",0x7C,1},
    {"DUMMY,     ",0x7D,1},
    {"DUMMY    ",0x7E,0},
    {"DUMMY      ",0x7F,0},
	
	
    {"CPU      ",CPU_CLIENT_ID,0},
    {"DLA_HIWAY",DLA_HIWAY_ID,0},
};



#define MIU0_CLIENT_NUM sizeof(miu0_clients)/sizeof(miu0_clients[0])
#define MIU1_CLIENT_NUM sizeof(miu1_clients)/sizeof(miu1_clients[0])

