/*
* mhal_sata_host.h - Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: edie.chen <edie.chen@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef _MHAL_SATA_HOST_H_
#define _MHAL_SATA_HOST_H_

#if defined(CONFIG_ARM64)
extern ptrdiff_t mstar_pm_base;
#define SSTAR_RIU_BASE  mstar_pm_base
#else
//#define MSTAR_RIU_BASE  0xBF200000
//#define MSTAR_RIU_BASE 0x1F000000
//#define MSTAR_RIU_BASE 0x1F000000 //JFY
#define SSTAR_RIU_BASE 0xFD000000 //JFY
#endif

#define REG_CHIPTOP_BASE            0x100B00

#ifndef BIT	//for Linux_kernel type, BIT redefined in <linux/bitops.h>
    #define BIT(_bit_)                  (1 << (_bit_))
#endif
#define BMASK(_bits_)               (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))

#define REG_CKG_SATA_FCLK          (REG_CHIPTOP_BASE +  (0x32<<1))
#define CKG_SATA_FCLK_GATED        BIT(0)
#define CKG_SATA_FCLK_INVERT       BIT(1)
#define CKG_SATA_FCLK_MASK         BMASK(4:2)
#define CKG_SATA_FCLK_240MHZ       (0 << 2)
#define CKG_SATA_FCLK_216MHZ       (1 << 2)
#define CKG_SATA_FCLK_MIU_P           (3 << 2)

#define SATA_PORT_BASE_0 0x100000
#define SATA_PORT_BASE_1 0x110000

#define SATA_GHC_0          0x102B00
#define SATA_GHC_0_P0       0x102C00
#define SATA_GHC_0_MISC     0x102D00
#define SATA_GHC_0_MIUPORT  0x103700
#define SATA_GHC_0_PHY      0x103900

#define SATA_GHC_1          0x113100
#define SATA_GHC_1_P0       0x113200
#define SATA_GHC_1_MISC     0x113300
#define SATA_GHC_1_MIUPORT  0x113800
#define SATA_GHC_1_PHY      0x162A00//0x113900
/*
u32 virtAddr(u32 addr){
  addr=MSTAR_RIU_BASE+addr<<9;
  return (u32)ioremap(addr,0x200);
}
*/
/*
#define SATA_GHC_0_ADDRESS_START ioremap(MSTAR_RIU_BASE + (0x102B << 9),512)
#define SATA_GHC_0_ADDRESS_END   SATA_GHC_0_ADDRESS_START+0xFE
#define SATA_GHC_0_P0_ADDRESS_START (MSTAR_RIU_BASE + (0x113200 << 1))
#define SATA_GHC_0_P0_ADDRESS_END   (MSTAR_RIU_BASE + (0x1132FE << 1))
#define SATA_MISC_0_ADDRESS_START  (MSTAR_RIU_BASE + (0x113300 << 1))
#define SATA_MISC_0_ADDRESS_END    (MSTAR_RIU_BASE + (0x1133FE << 1))
*/
/*
u32 SATA_GHC_0_ADDRESS_START=(u32)virtAddr(0x102B);//(MSTAR_RIU_BASE + (0x102B00 << 1))//0xFD000000
u32 SATA_GHC_0_ADDRESS_END=SATA_GHC_0_ADDRESS_START+0xFE;//(MSTAR_RIU_BASE + (0x102BFE << 1))
u32 SATA_GHC_0_P0_ADDRESS_START=virtAddr(MSTAR_RIU_BASE+0x102C<<9);//(MSTAR_RIU_BASE + (0x102C00 << 1))
u32 SATA_GHC_0_P0_ADDRESS_END=SATA_GHC_0_P0_ADDRESS_START+0xFE;//(MSTAR_RIU_BASE + (0x102CFE << 1))
u32 SATA_MISC_0_ADDRESS_START=virtAddr(MSTAR_RIU_BASE+0x102D<<9);//(MSTAR_RIU_BASE + (0x102D00 << 1))
u32 SATA_MISC_0_ADDRESS_END=SATA_MISC_0_ADDRESS_START+0xFE;//(MSTAR_RIU_BASE + (0x102DFE << 1))



*/

#define SATA_GHC_0_ADDRESS_START    (SSTAR_RIU_BASE + (0x102B00 << 1))
#define SATA_GHC_0_ADDRESS_END      (SSTAR_RIU_BASE + (0x102BFE << 1))
#define SATA_GHC_0_P0_ADDRESS_START (SSTAR_RIU_BASE + (0x102C00 << 1))
#define SATA_GHC_0_P0_ADDRESS_END   (SSTAR_RIU_BASE + (0x102CFE << 1))
#define SATA_MISC_0_ADDRESS_START   (SSTAR_RIU_BASE + (0x102D00 << 1))
#define SATA_MISC_0_ADDRESS_END     (SSTAR_RIU_BASE + (0x102DFE << 1))


#define SATA_GHC_1_ADDRESS_START    (SSTAR_RIU_BASE + (0x113100 << 1))
#define SATA_GHC_1_ADDRESS_END      (SSTAR_RIU_BASE + (0x1131FE << 1))
#define SATA_GHC_1_P0_ADDRESS_START (SSTAR_RIU_BASE + (0x113200 << 1))
#define SATA_GHC_1_P0_ADDRESS_END   (SSTAR_RIU_BASE + (0x1132FE << 1))
#define SATA_MISC_1_ADDRESS_START   (SSTAR_RIU_BASE + (0x113300 << 1))
#define SATA_MISC_1_ADDRESS_END     (SSTAR_RIU_BASE + (0x1133FE << 1))

/*
#define SATA_GHC_0_ADDRESS_START (0xFD000000 + (0x102B00 << 1))
#define SATA_GHC_0_ADDRESS_END   (0xFD000000 + (0x102BFE << 1))
#define SATA_GHC_1_ADDRESS_START (0xFD000000 + (0x102C00 << 1))
#define SATA_GHC_1_ADDRESS_END   (0xFD000000 + (0x102CFE << 1))
#define SATA_MISC_ADDRESS_START  (0xFD000000 + (0x102D00 << 1))
#define SATA_MISC_ADDRESS_END    (0xFD000000 + (0x102DFE << 1))
*/

// FIXME: Needs to be removed since K6 does not support XIU mode.
#if defined(CONFIG_ARM64)
#define SATA_SDMAP_RIU_BASE         mstar_pm_base
#else
#define SATA_SDMAP_RIU_BASE         MSTAR_RIU_BASE //0xFD000000
#endif

// MIU interval, the gap between MIU0 and MIU1, chip dependent
#define MIU_INTERVAL_SATA 0x60000000 // for K6

// Local Definition, internal SATA MAC SRAM address
#define AHCI_P0CLB                  0x18001000
#define AHCI_P0FB                   0x18001100
#define AHCI_CTBA0                  0x18001200

#define SATA_PORT_NUM               1
#define SATA_CMD_HEADER_SIZE        0x400
#define SATA_FIS_SIZE               0x100

enum {
    /* global controller registers */
    SS_HOST_CAP        = 0x00,        /* host capabilities */
    SS_HOST_CTL        = (0x04 << 1), /* global host control */
    SS_HOST_IRQ_STAT   = (0x08 << 1), /* interrupt status */
    SS_HOST_PORTS_IMPL = (0x0c << 1), /* bitmap of implemented ports */
    SS_HOST_VERSION    = (0x10 << 1), /* AHCI spec. version compliancy */
    SS_HOST_CAP2       = (0x24 << 1), /* host capabilities, extended */

    /* HOST_CTL bits - HOST_CAP, 0x00 */
    //MS_HOST_RESET      = (1 << 0),  /* reset controller; self-clear */
    //MS_HOST_IRQ_EN     = (1 << 1),  /* global IRQ enable */
   // MS_HOST_AHCI_EN    = (1 << 31), /* AHCI enabled */

    /* Registers for each SATA port */
    SS_PORT_LST_ADDR       = 0x00, /* command list DMA addr */
    SS_PORT_LST_ADDR_HI    = (0x04 << 1), /* command list DMA addr hi */
    SS_PORT_FIS_ADDR       = (0x08 << 1), /* FIS rx buf addr */
    SS_PORT_FIS_ADDR_HI    = (0x0c << 1), /* FIS rx buf addr hi */
    SS_PORT_IRQ_STAT       = (0x10 << 1), /* interrupt status */
    SS_PORT_IRQ_MASK       = (0x14 << 1), /* interrupt enable/disable mask */
    SS_PORT_CMD            = (0x18 << 1), /* port command */
    SS_PORT_TFDATA         = (0x20 << 1), /* taskfile data */
    SS_PORT_SIG            = (0x24 << 1), /* device TF signature */
    SS_PORT_SCR_STAT       = (0x28 << 1), /* SATA phy register: SStatus */
    SS_PORT_SCR_CTL        = (0x2c << 1), /* SATA phy register: SControl */
    SS_PORT_SCR_ERR        = (0x30 << 1), /* SATA phy register: SError */
    SS_PORT_SCR_ACT        = (0x34 << 1), /* SATA phy register: SActive */
    SS_PORT_CMD_ISSUE      = (0x38 << 1), /* command issue */
    SS_PORT_SCR_NTF        = (0x3c << 1), /* SATA phy register: SNotification */
    SS_PORT_DMA_CTRL       = (0x70 << 1), /* SATA phy register: SNotification */

    /* PORT_CMD bits */
    SS_PORT_CMD_ASP        = (1 << 27), /* Aggressive Slumber/Partial */
    SS_PORT_CMD_ALPE       = (1 << 26), /* Aggressive Link PM enable */
    SS_PORT_CMD_ATAPI      = (1 << 24), /* Device is ATAPI */
    SS_PORT_CMD_FBSCP      = (1 << 22), /* FBS Capable Port */
    SS_PORT_CMD_PMP        = (1 << 17), /* PMP attached */
    SS_PORT_CMD_LIST_ON    = (1 << 15), /* cmd list DMA engine running */
    SS_PORT_CMD_FIS_ON     = (1 << 14), /* FIS DMA engine running */
    SS_PORT_CMD_FIS_RX     = (1 << 4), /* Enable FIS receive DMA engine */
    SS_PORT_CMD_CLO        = (1 << 3), /* Command list override */
    SS_PORT_CMD_POWER_ON   = (1 << 2), /* Power up device */
    SS_PORT_CMD_SPIN_UP    = (1 << 1), /* Spin up device */
    SS_PORT_CMD_START      = (1 << 0), /* Enable port DMA engine */

    SS_PORT_CMD_ICC_MASK   = (0xf << 28), /* i/f ICC state mask */
    SS_PORT_CMD_ICC_ACTIVE = (0x1 << 28), /* Put i/f in active state */
    SS_PORT_CMD_ICC_PARTIAL    = (0x2 << 28), /* Put i/f in partial state */
    SS_PORT_CMD_ICC_SLUMBER    = (0x6 << 28), /* Put i/f in slumber state */

    /*  Status Error  */
    SS_AHCI_PORT_SERR_RDIE  = (1 << 0),    /* Recovered Data Integrity Error */
    SS_AHCI_PORT_SERR_RCE   = (1 << 1),    /* Recovered Communications Error */
    /* Bit 2 ~ 7 Reserved */
    SS_AHCI_PORT_SERR_TDIE  = (1 << 8),    /* Transient Data Integrity Error */
    SS_AHCI_PORT_SERR_PCDIE = (1 << 9),    /* Persistent    Communication    or    Data    Integrity    Error */
    SS_AHCI_PORT_SERR_PE    = (1 << 10),   /* Protocol  Error */
    SS_AHCI_PORT_SERR_IE    = (1 << 11),   /* Internal  Error */
    /* Bit 15 ~ 12 Reserved */
    SS_AHCI_PORT_SERR_PRDYC = (1 << 16),   /* PhyRdy  Change */
    SS_AHCI_PORT_SERR_PIE   = (1 << 17),   /* Phy  Internal  Error */
    SS_AHCI_PORT_SERR_COMW  = (1 << 18),   /* Comm Wake Detect */
    SS_AHCI_PORT_SERR_TDE   = (1 << 19),   /* 10B  to  8B  Decode  Error  */
    SS_AHCI_PORT_SERR_DE    = (1 << 20),   /* Disparity Error <= Not Use by AHCI  */
    SS_AHCI_PORT_SERR_CRCE  = (1 << 21),   /* CRC Error  */
    SS_AHCI_PORT_SERR_HE    = (1 << 22),   /* Handshake  Error */
    SS_AHCI_PORT_SERR_LSE   = (1 << 23),   /* Link Sequence Error  */
    SS_AHCI_PORT_SERR_TSTE  = (1 << 24),   /* Transport  state  transition  error  */
    SS_AHCI_PORT_SERR_UFIS  = (1 << 25),   /* Unknown FIS Type  */
    SS_AHCI_PORT_SERR_EXC   = (1 << 26),   /* Exchanged :  a  change  in device  presence  has  been  detected */
    /* Bit 31 ~ 27 Reserved */
};

enum {
    E_PORT_SPEED_MASK = (0xF << 4),
    E_PORT_SPEED_NO_RESTRICTION = (0x0 < 4),
    E_PORT_SPEED_GEN1 = (0x1 << 4),
    E_PORT_SPEED_GEN2 = (0x2 << 4),
    E_PORT_SPEED_GEN3 = (0x3 << 4),

    E_PORT_DET_MASK = (0xF << 0),  //Device  Detection  Initialization
    E_PORT_DET_NODEVICE_DETECT = 0x0,
    E_PORT_DET_HW_RESET = 0x1, // Cause HW send initial sequence
    E_PORT_DET_DISABLE_PHY = 0x4, // Put PHY into Offline
    E_PORT_DET_DEVICE_NOEST = 0x1, // not established
    E_PORT_DET_DEVICE_EST = 0x3,  // established
    E_PORT_DET_PHY_OFFLINE = 0x4, // Put PHY into Offline

    DATA_COMPLETE_INTERRUPT = (1 << 31),

    /* PORT_IRQ_{STAT,MASK} bits */
    //PORT_IRQ_COLD_PRES  = (1 << 31), /* cold presence detect */
    //PORT_IRQ_TF_ERR     = (1 << 30), /* task file error */
    //PORT_IRQ_HBUS_ERR   = (1 << 29), /* host bus fatal error */
    //PORT_IRQ_HBUS_DATA_ERR  = (1 << 28), /* host bus data error */
    //PORT_IRQ_IF_ERR     = (1 << 27), /* interface fatal error */
    //PORT_IRQ_IF_NONFATAL    = (1 << 26), /* interface non-fatal error */
    //PORT_IRQ_OVERFLOW    = (1 << 24), /* xfer exhausted available S/G */
    //PORT_IRQ_BAD_PMP    = (1 << 23), /* incorrect port multiplier */

    //PORT_IRQ_PHYRDY     = (1 << 22), /* PhyRdy changed */
    //PORT_IRQ_DEV_ILCK   = (1 << 7), /* device interlock */
    //PORT_IRQ_CONNECT    = (1 << 6), /* port connect change status */
    //PORT_IRQ_SG_DONE    = (1 << 5), /* descriptor processed */
    //PORT_IRQ_UNK_FIS    = (1 << 4), /* unknown FIS rx'd */
    //PORT_IRQ_SDB_FIS    = (1 << 3), /* Set Device Bits FIS rx'd */
    //PORT_IRQ_DMAS_FIS   = (1 << 2), /* DMA Setup FIS rx'd */
    //PORT_IRQ_PIOS_FIS   = (1 << 1), /* PIO Setup FIS rx'd */
    //PORT_IRQ_D2H_REG_FIS    = (1 << 0), /* D2H Register FIS rx'd */

    //PORT_IRQ_FREEZE     = PORT_IRQ_HBUS_ERR |
    //    PORT_IRQ_IF_ERR |
    //    PORT_IRQ_CONNECT |
    //    PORT_IRQ_PHYRDY |
    //    PORT_IRQ_UNK_FIS |
    //    PORT_IRQ_BAD_PMP,
    //PORT_IRQ_ERROR      = PORT_IRQ_FREEZE |
    //    PORT_IRQ_TF_ERR |
    //    PORT_IRQ_HBUS_DATA_ERR,
    //DEF_PORT_IRQ        = PORT_IRQ_ERROR | PORT_IRQ_SG_DONE |
    //    PORT_IRQ_SDB_FIS | PORT_IRQ_DMAS_FIS |
    //    PORT_IRQ_PIOS_FIS | PORT_IRQ_D2H_REG_FIS,
};

/*
 *          Host Controller MISC Register
 */
enum
{
    SATA_MISC_CFIFO_ADDRL     =((0x10 <<1 )<<1),
    SATA_MISC_CFIFO_ADDRH     =((0x11 <<1 )<<1),
    SATA_MISC_CFIFO_WDATAL    =((0x12 <<1 )<<1),
    SATA_MISC_CFIFO_WDATAH    =((0x13 <<1 )<<1),
    SATA_MISC_CFIFO_RDATAL    =((0x14 <<1 )<<1),
    SATA_MISC_CFIFO_RDATAH    =((0x15 <<1 )<<1),
    SATA_MISC_CFIFO_RORW      =((0x16 <<1 )<<1),
    SATA_MISC_CFIFO_ACCESS    =((0x17 <<1 )<<1),
    SATA_MISC_ACCESS_MODE     =((0x18 <<1 )<<1),
    SATA_MISC_AMBA_MUXRST     =((0x21 <<1 )<<1),
    SATA_MISC_HBA_LADDR       =((0x24 <<1 )<<1),
    SATA_MISC_HBA_HADDR       =((0x25 <<1 )<<1),
    SATA_MISC_CMD_LADDR       =((0x26 <<1 )<<1),
    SATA_MISC_CMD_HADDR       =((0x27 <<1 )<<1),
    SATA_MISC_DATA_ADDR       =((0x28 <<1 )<<1),
    SATA_MISC_ENRELOAD        =((0x29 <<1 )<<1),
    SATA_MISC_AMBA_ARBIT      =((0x2A <<1 )<<1),
    SATA_MISC_AMBA_PGEN       =((0x30 <<1 )<<1),
    SATA_MISC_AGEN_F_VAL      =((0x35 <<1 )<<1),
    SATA_MISC_HOST_SWRST      =((0x50 <<1 )<<1),
    SATA_MISC_HOST_NEAR       =((0x51 <<1 )<<1),
    SATA_MISC_FPGA_EN         =((0x55 <<1 )<<1),
};

//void MHal_SATA_Clock_Config(phys_addr_t port_base, bool enable);
//void MHal_SATA_HW_Inital(phys_addr_t misc_base, phys_addr_t port_base, phys_addr_t hba_base);
//void MHal_SATA_Setup_Port_Implement(phys_addr_t misc_base, phys_addr_t port_base, phys_addr_t hba_base);
//phys_addr_t MHal_SATA_bus_address(phys_addr_t phy_address);
//u32 MHal_SATA_get_max_speed(void);

#define SSTAR_SATA_DTS_NAME "sstar,sata"
#define SSTAR_SATA1_DTS_NAME "sstar,sata1"

#endif
