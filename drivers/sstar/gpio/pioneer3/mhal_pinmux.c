/*
* mhal_pinmux.c- Sigmastar
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
#include "ms_platform.h"
#include "mdrv_types.h"
#include "mhal_gpio.h"
#include "padmux.h"
#include "gpio.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define BASE_RIU_PA                         0xFD000000
#define PMSLEEP_BANK                        0x000E00
#define PM_SAR_BANK                         0x001400
#define ALBANY1_BANK                        0x003200
#define ALBANY2_BANK                        0x003300
#define CHIPTOP_BANK                        0x101E00
#define PADTOP_BANK                         0x103C00
#define PM_PADTOP_BANK                      0x003F00
#define UTMI0_BANK                          0x142100
#define ETH_BANK                          0x003300
#define ARBPHY_BANK                          0x003200


#define _GPIO_W_WORD(addr,val)              {(*(volatile u16*)(addr)) = (u16)(val);}
#define _GPIO_W_WORD_MASK(addr,val,mask)    {(*(volatile u16*)(addr)) = ((*(volatile u16*)(addr)) & ~(mask)) | ((u16)(val) & (mask));}
#define _GPIO_R_BYTE(addr)                  (*(volatile u8*)(addr))
#define _GPIO_R_WORD_MASK(addr,mask)        ((*(volatile u16*)(addr)) & (mask))

#define GET_BASE_ADDR_BY_BANK(x, y)         ((x) + ((y) << 1))
#define _RIUA_8BIT(bank , offset)           GET_BASE_ADDR_BY_BANK(BASE_RIU_PA, bank) + (((offset) & ~1)<<1) + ((offset) & 1)
#define _RIUA_16BIT(bank , offset)          GET_BASE_ADDR_BY_BANK(BASE_RIU_PA, bank) + ((offset)<<2)

/* SAR : SAR_BANK, R/W 8-bits */
#define REG_SAR_AISEL_8BIT          0x11*2
    #define REG_SAR_CH0_AISEL       BIT0
    #define REG_SAR_CH1_AISEL       BIT1
    #define REG_SAR_CH2_AISEL       BIT2
    #define REG_SAR_CH3_AISEL       BIT3
/* EMAC : ALBANY1_BANK */
#define REG_ATOP_RX_INOFF       0x69
    #define REG_ATOP_RX_INOFF_MASK  BIT15|BIT14

/* EMAC : ALBANY2_BANK */
#define REG_ETH_GPIO_EN         0x71
    #define REG_ETH_GPIO_EN_MASK    BIT3|BIT2|BIT1|BIT0
//#define REG_AUTO_GPIO_EN                        0x9
//    #define REG_AUTO_GPIO_EN_MASK                   BIT0|BIT1|BIT2
#define REG_EJ_MODE                             0x60
    #define REG_EJ_MODE_MASK                        BIT8|BIT9|BIT10
#define REG_ALL_PAD_IN                          0x28
    #define REG_ALL_PAD_IN_MASK                     BIT15
#define REG_TEST_IN_MODE                        0x12
    #define REG_TEST_IN_MODE_MASK                   BIT0|BIT1
#define REG_TEST_OUT_MODE                       0x12
    #define REG_TEST_OUT_MODE_MASK                  BIT4|BIT5
#define REG_I2C0_MODE                           0x6f
    #define REG_I2C0_MODE_MASK                      BIT0|BIT1|BIT2|BIT3
#define REG_I2C1_MODE                           0x6f
    #define REG_I2C1_MODE_MASK                      BIT4|BIT5|BIT6|BIT7
#define REG_SR0_MIPI_MODE                       0x74
    #define REG_SR0_MIPI_MODE_MASK                  BIT0
#define REG_SR1_MIPI_MODE                       0x74
    #define REG_SR1_MIPI_MODE_MASK                  BIT4|BIT5
#define REG_ISP_IR_MODE                         0x73
    #define REG_ISP_IR_MODE_MASK                    BIT0|BIT1
#define REG_SR0_CTRL_MODE                       0x74
    #define REG_SR0_CTRL_MODE_MASK                  BIT8|BIT9
#define REG_SR0_MCLK_MODE                       0x76
    #define REG_SR0_MCLK_MODE_MASK                  BIT0|BIT1
#define REG_SR0_PDN_MODE                        0x76
    #define REG_SR0_PDN_MODE_MASK                   BIT8
#define REG_SR0_RST_MODE                        0x76
    #define REG_SR0_RST_MODE_MASK                   BIT4|BIT5
#define REG_SR1_CTRL_MODE                       0x74
    #define REG_SR1_CTRL_MODE_MASK                  BIT12|BIT13
#define REG_SR1_MCLK_MODE                       0x77
    #define REG_SR1_MCLK_MODE_MASK                  BIT0|BIT1
#define REG_SR1_PDN_MODE                        0x77
    #define REG_SR1_PDN_MODE_MASK                   BIT8|BIT9
#define REG_SR1_RST_MODE                        0x77
    #define REG_SR1_RST_MODE_MASK                   BIT4|BIT5
#define REG_SR0_BT601_MODE                      0x75
    #define REG_SR0_BT601_MODE_MASK                 BIT0|BIT1
#define REG_SR0_BT656_MODE                      0x75
    #define REG_SR0_BT656_MODE_MASK                 BIT4|BIT5|BIT6
#define REG_ETH0_MODE                           0x6e
    #define REG_ETH0_MODE_MASK                      BIT0|BIT1|BIT2|BIT3
#define REG_TTL16_MODE                          0x6c
    #define REG_TTL16_MODE_MASK                     BIT0|BIT1|BIT2
#define REG_TTL18_MODE                          0x6c
    #define REG_TTL18_MODE_MASK                     BIT4|BIT5
#define REG_TTL24_MODE                          0x6c
    #define REG_TTL24_MODE_MASK                     BIT8|BIT9|BIT10
#define REG_RGB8_MODE                           0x6b
    #define REG_RGB8_MODE_MASK                      BIT0|BIT1
#define REG_BT656_OUT_MODE                      0x6c
    #define REG_BT656_OUT_MODE_MASK                 BIT12|BIT13
#define REG_PSPI0_SR_MODE                       0x69
    #define REG_PSPI0_SR_MODE_MASK                  BIT0|BIT1|BIT2
#define REG_PSPI0_G_MODE                        0x69
    #define REG_PSPI0_G_MODE_MASK                   BIT4|BIT5
#define REG_SPI0_MODE                           0x68
    #define REG_SPI0_MODE_MASK                      BIT0|BIT1|BIT2
#define REG_PSPI1_PL_MODE                       0x6a
    #define REG_PSPI1_PL_MODE_MASK                  BIT0|BIT1|BIT2|BIT3
#define REG_PSPI1_CS2_MODE                      0x6a
    #define REG_PSPI1_CS2_MODE_MASK                 BIT4|BIT5|BIT6|BIT7
#define REG_PSPI1_TE_MODE                       0x6a
    #define REG_PSPI1_TE_MODE_MASK                  BIT8|BIT9|BIT10|BIT11
#define REG_PSPI1_G_MODE                        0x6a
    #define REG_PSPI1_G_MODE_MASK                   BIT12|BIT13
#define REG_SPI1_MODE                           0x68
    #define REG_SPI1_MODE_MASK                      BIT8|BIT9|BIT10|BIT11
#define REG_SPI1_CS2_MODE                       0x68
    #define REG_SPI1_CS2_MODE_MASK                  BIT12|BIT13|BIT14
#define REG_SDIO_MODE                           0x67
    #define REG_SDIO_MODE_MASK                      BIT8|BIT9
#define REG_SD_CDZ_MODE                         0x67
    #define REG_SD_CDZ_MODE_MASK                    BIT0|BIT1
#define REG_KEY_READ0_MODE                      0x78
    #define REG_KEY_READ0_MODE_MASK                 BIT0|BIT1|BIT2|BIT3
#define REG_KEY_READ1_MODE                      0x78
    #define REG_KEY_READ1_MODE_MASK                 BIT4|BIT5|BIT6|BIT7
#define REG_KEY_READ2_MODE                      0x78
    #define REG_KEY_READ2_MODE_MASK                 BIT8|BIT9|BIT10|BIT11
#define REG_KEY_READ3_MODE                      0x78
    #define REG_KEY_READ3_MODE_MASK                 BIT12|BIT13|BIT14|BIT15
#define REG_KEY_READ4_MODE                      0x79
    #define REG_KEY_READ4_MODE_MASK                 BIT0|BIT1|BIT2|BIT3
#define REG_KEY_READ5_MODE                      0x79
    #define REG_KEY_READ5_MODE_MASK                 BIT4|BIT5|BIT6|BIT7
#define REG_KEY_READ6_MODE                      0x79
    #define REG_KEY_READ6_MODE_MASK                 BIT8|BIT9|BIT10|BIT11
#define REG_KEY_SCAN0_MODE                      0x7a
    #define REG_KEY_SCAN0_MODE_MASK                 BIT0|BIT1|BIT2|BIT3
#define REG_KEY_SCAN1_MODE                      0x7a
    #define REG_KEY_SCAN1_MODE_MASK                 BIT4|BIT5|BIT6|BIT7
#define REG_KEY_SCAN2_MODE                      0x7a
    #define REG_KEY_SCAN2_MODE_MASK                 BIT8|BIT9|BIT10|BIT11
#define REG_KEY_SCAN3_MODE                      0x7a
    #define REG_KEY_SCAN3_MODE_MASK                 BIT12|BIT13|BIT14|BIT15
#define REG_KEY_SCAN4_MODE                      0x7b
    #define REG_KEY_SCAN4_MODE_MASK                 BIT0|BIT1|BIT2|BIT3
#define REG_KEY_SCAN5_MODE                      0x7b
    #define REG_KEY_SCAN5_MODE_MASK                 BIT4|BIT5|BIT6|BIT7
#define REG_KEY_SCAN6_MODE                      0x7b
    #define REG_KEY_SCAN6_MODE_MASK                 BIT8|BIT9|BIT10|BIT11
#define REG_KEY_FIX_MODE                        0x79
    #define REG_KEY_FIX_MODE_MASK                   BIT12
#define REG_FUART_MODE                          0x6e
    #define REG_FUART_MODE_MASK                     BIT8|BIT9|BIT10|BIT11
#define REG_UART0_MODE                          0x6d
    #define REG_UART0_MODE_MASK                     BIT0|BIT1|BIT2
#define REG_UART1_MODE                          0x6d
    #define REG_UART1_MODE_MASK                     BIT4|BIT5|BIT6|BIT7
#define REG_UART2_MODE                          0x6d
    #define REG_UART2_MODE_MASK                     BIT8|BIT9|BIT10
#define REG_PWM0_MODE                           0x65
    #define REG_PWM0_MODE_MASK                      BIT0|BIT1|BIT2|BIT3
#define REG_PWM1_MODE                           0x65
    #define REG_PWM1_MODE_MASK                      BIT4|BIT5|BIT6|BIT7
#define REG_PWM2_MODE                           0x65
    #define REG_PWM2_MODE_MASK                      BIT8|BIT9|BIT10|BIT11
#define REG_PWM3_MODE                           0x65
    #define REG_PWM3_MODE_MASK                      BIT12|BIT13|BIT14|BIT15
#define REG_DMIC_MODE                           0x60
    #define REG_DMIC_MODE_MASK                      BIT0|BIT1|BIT2|BIT3
#define REG_I2S_MCK_MODE                        0x62
    #define REG_I2S_MCK_MODE_MASK                   BIT0|BIT1|BIT2
#define REG_I2S_RX_MODE                         0x62
    #define REG_I2S_RX_MODE_MASK                    BIT4|BIT5|BIT6
#define REG_I2S_TX_MODE                         0x62
    #define REG_I2S_TX_MODE_MASK                    BIT8|BIT9|BIT10
#define REG_I2S_RXTX_MODE                       0x62
    #define REG_I2S_RXTX_MODE_MASK                  BIT12|BIT13|BIT14
#define REG_BT1120_MODE                         0x72
    #define REG_BT1120_MODE_MASK                    BIT0|BIT1

#define REG_PM_SPI_GPIO                         0x35
    #define REG_PM_SPI_GPIO_MASK                    BIT0
#define REG_PM_SPIWPN_GPIO                      0x35
    #define REG_PM_SPIWPN_GPIO_MASK                 BIT4
#define REG_PM_SPIHOLDN_MODE                    0x35
    #define REG_PM_SPIHOLDN_MODE_MASK               BIT6|BIT7
#define REG_PM_SPICSZ1_GPIO                     0x35
    #define REG_PM_SPICSZ1_GPIO_MASK                BIT2
#define REG_PM_SPICSZ2_GPIO                     0x35
    #define REG_PM_SPICSZ2_GPIO_MASK                BIT3
#define REG_PM_PWM0_MODE                        0x28
    #define REG_PM_PWM0_MODE_MASK                   BIT0|BIT1
#define REG_PM_PWM1_MODE                        0x28
    #define REG_PM_PWM1_MODE_MASK                   BIT2|BIT3
#define REG_PM_PWM2_MODE                        0x28
    #define REG_PM_PWM2_MODE_MASK                   BIT6|BIT7
#define REG_PM_PWM3_MODE                        0x28
    #define REG_PM_PWM3_MODE_MASK                   BIT8|BIT9
#define REG_PM_UART1_MODE                       0x27
    #define REG_PM_UART1_MODE_MASK                  BIT8
#define REG_PM_VID_MODE                         0x28
    #define REG_PM_VID_MODE_MASK                    BIT12|BIT13
#define REG_PM_SD_CDZ_MODE                      0x28
    #define REG_PM_SD_CDZ_MODE_MASK                 BIT14
#define REG_PM_LED_MODE                         0x28
    #define REG_PM_LED_MODE_MASK                    BIT4|BIT5
#define REG_PM_PAD_EXT_MODE_0                   0x38
    #define REG_PM_PAD_EXT_MODE_0_MASK              BIT0
#define REG_PM_PAD_EXT_MODE_1                   0x38
    #define REG_PM_PAD_EXT_MODE_1_MASK              BIT1
#define REG_PM_PAD_EXT_MODE_2                   0x38
    #define REG_PM_PAD_EXT_MODE_2_MASK              BIT2
#define REG_PM_PAD_EXT_MODE_3                   0x38
    #define REG_PM_PAD_EXT_MODE_3_MASK              BIT3
#define REG_PM_PAD_EXT_MODE_4                   0x38
    #define REG_PM_PAD_EXT_MODE_4_MASK              BIT4
#define REG_PM_PAD_EXT_MODE_5                   0x38
    #define REG_PM_PAD_EXT_MODE_5_MASK              BIT5
#define REG_PM_PAD_EXT_MODE_6                   0x38
    #define REG_PM_PAD_EXT_MODE_6_MASK              BIT6
#define REG_PM_PAD_EXT_MODE_7                   0x38
    #define REG_PM_PAD_EXT_MODE_7_MASK              BIT7
#define REG_PM_PAD_EXT_MODE_8                   0x38
    #define REG_PM_PAD_EXT_MODE_8_MASK              BIT8
#define REG_PM_PAD_EXT_MODE_9                   0x38
    #define REG_PM_PAD_EXT_MODE_9_MASK              BIT9
#define REG_PM_PAD_EXT_MODE_10                  0x38
    #define REG_PM_PAD_EXT_MODE_10_MASK             BIT10
// PADMUX MASK MARCO END



#define PM_PADTOP_BANK        0x003F00
#define CHIPTOP_BANK          0x101E00
#define PADTOP_BANK           0x103C00
#define PADGPIO_BANK          0x103E00
#define PM_SAR_BANK           0x001400
#define PMSLEEP_BANK          0x000E00
#define PM_GPIO_BANK          0x000F00

#define REG_SR_IO00_GPIO_MODE                   0x0
    #define REG_SR_IO00_GPIO_MODE_MASK              BIT3
#define REG_SR_IO01_GPIO_MODE                   0x1
    #define REG_SR_IO01_GPIO_MODE_MASK              BIT3
#define REG_SR_IO02_GPIO_MODE                   0x2
    #define REG_SR_IO02_GPIO_MODE_MASK              BIT3
#define REG_SR_IO03_GPIO_MODE                   0x3
    #define REG_SR_IO03_GPIO_MODE_MASK              BIT3
#define REG_SR_IO04_GPIO_MODE                   0x4
    #define REG_SR_IO04_GPIO_MODE_MASK              BIT3
#define REG_SR_IO05_GPIO_MODE                   0x5
    #define REG_SR_IO05_GPIO_MODE_MASK              BIT3
#define REG_SR_IO06_GPIO_MODE                   0x6
    #define REG_SR_IO06_GPIO_MODE_MASK              BIT3
#define REG_SR_IO07_GPIO_MODE                   0x7
    #define REG_SR_IO07_GPIO_MODE_MASK              BIT3
#define REG_SR_IO08_GPIO_MODE                   0x8
    #define REG_SR_IO08_GPIO_MODE_MASK              BIT3
#define REG_SR_IO09_GPIO_MODE                   0x9
    #define REG_SR_IO09_GPIO_MODE_MASK              BIT3
#define REG_SR_IO10_GPIO_MODE                   0xA
    #define REG_SR_IO10_GPIO_MODE_MASK              BIT3
#define REG_SR_IO11_GPIO_MODE                   0xB
    #define REG_SR_IO11_GPIO_MODE_MASK              BIT3
#define REG_SR_IO12_GPIO_MODE                   0xC
    #define REG_SR_IO12_GPIO_MODE_MASK              BIT3
#define REG_SR_IO13_GPIO_MODE                   0xD
    #define REG_SR_IO13_GPIO_MODE_MASK              BIT3
#define REG_SR_IO14_GPIO_MODE                   0xE
    #define REG_SR_IO14_GPIO_MODE_MASK              BIT3
#define REG_SR_IO15_GPIO_MODE                   0xF
    #define REG_SR_IO15_GPIO_MODE_MASK              BIT3
#define REG_SR_IO16_GPIO_MODE                   0x10
    #define REG_SR_IO16_GPIO_MODE_MASK              BIT3
#define REG_TTL0_GPIO_MODE                      0x11
    #define REG_TTL0_GPIO_MODE_MASK                 BIT3
#define REG_TTL1_GPIO_MODE                      0x12
    #define REG_TTL1_GPIO_MODE_MASK                 BIT3
#define REG_TTL2_GPIO_MODE                      0x13
    #define REG_TTL2_GPIO_MODE_MASK                 BIT3
#define REG_TTL3_GPIO_MODE                      0x14
    #define REG_TTL3_GPIO_MODE_MASK                 BIT3
#define REG_TTL4_GPIO_MODE                      0x15
    #define REG_TTL4_GPIO_MODE_MASK                 BIT3
#define REG_TTL5_GPIO_MODE                      0x16
    #define REG_TTL5_GPIO_MODE_MASK                 BIT3
#define REG_TTL6_GPIO_MODE                      0x17
    #define REG_TTL6_GPIO_MODE_MASK                 BIT3
#define REG_TTL7_GPIO_MODE                      0x18
    #define REG_TTL7_GPIO_MODE_MASK                 BIT3
#define REG_TTL8_GPIO_MODE                      0x19
    #define REG_TTL8_GPIO_MODE_MASK                 BIT3
#define REG_TTL9_GPIO_MODE                      0x1A
    #define REG_TTL9_GPIO_MODE_MASK                 BIT3
#define REG_TTL10_GPIO_MODE                     0x1B
    #define REG_TTL10_GPIO_MODE_MASK                BIT3
#define REG_TTL11_GPIO_MODE                     0x1C
    #define REG_TTL11_GPIO_MODE_MASK                BIT3
#define REG_TTL12_GPIO_MODE                     0x1D
    #define REG_TTL12_GPIO_MODE_MASK                BIT3
#define REG_TTL13_GPIO_MODE                     0x1E
    #define REG_TTL13_GPIO_MODE_MASK                BIT3
#define REG_TTL14_GPIO_MODE                     0x1F
    #define REG_TTL14_GPIO_MODE_MASK                BIT3
#define REG_TTL15_GPIO_MODE                     0x20
    #define REG_TTL15_GPIO_MODE_MASK                BIT3
#define REG_TTL16_GPIO_MODE                     0x21
    #define REG_TTL16_GPIO_MODE_MASK                BIT3
#define REG_TTL17_GPIO_MODE                     0x22
    #define REG_TTL17_GPIO_MODE_MASK                BIT3
#define REG_TTL18_GPIO_MODE                     0x23
    #define REG_TTL18_GPIO_MODE_MASK                BIT3
#define REG_TTL19_GPIO_MODE                     0x24
    #define REG_TTL19_GPIO_MODE_MASK                BIT3
#define REG_TTL20_GPIO_MODE                     0x25
    #define REG_TTL20_GPIO_MODE_MASK                BIT3
#define REG_TTL21_GPIO_MODE                     0x26
    #define REG_TTL21_GPIO_MODE_MASK                BIT3
#define REG_KEY0_GPIO_MODE                      0x27
    #define REG_KEY0_GPIO_MODE_MASK                 BIT3
#define REG_KEY1_GPIO_MODE                      0x28
    #define REG_KEY1_GPIO_MODE_MASK                 BIT3
#define REG_KEY2_GPIO_MODE                      0x29
    #define REG_KEY2_GPIO_MODE_MASK                 BIT3
#define REG_KEY3_GPIO_MODE                      0x2A
    #define REG_KEY3_GPIO_MODE_MASK                 BIT3
#define REG_KEY4_GPIO_MODE                      0x2B
    #define REG_KEY4_GPIO_MODE_MASK                 BIT3
#define REG_KEY5_GPIO_MODE                      0x2C
    #define REG_KEY5_GPIO_MODE_MASK                 BIT3
#define REG_KEY6_GPIO_MODE                      0x2D
    #define REG_KEY6_GPIO_MODE_MASK                 BIT3
#define REG_KEY7_GPIO_MODE                      0x2E
    #define REG_KEY7_GPIO_MODE_MASK                 BIT3
#define REG_KEY8_GPIO_MODE                      0x2F
    #define REG_KEY8_GPIO_MODE_MASK                 BIT3
#define REG_KEY9_GPIO_MODE                      0x30
    #define REG_KEY9_GPIO_MODE_MASK                 BIT3
#define REG_KEY10_GPIO_MODE                      0x31
    #define REG_KEY10_GPIO_MODE_MASK                 BIT3
#define REG_KEY11_GPIO_MODE                      0x32
    #define REG_KEY11_GPIO_MODE_MASK                 BIT3
#define REG_KEY12_GPIO_MODE                      0x33
    #define REG_KEY12_GPIO_MODE_MASK                 BIT3
 #define REG_KEY13_GPIO_MODE                      0x34
    #define REG_KEY13_GPIO_MODE_MASK                 BIT3
#define REG_SD_D1_GPIO_MODE                     0x35
    #define REG_SD_D1_GPIO_MODE_MASK                BIT3
#define REG_SD_D0_GPIO_MODE                     0x36
    #define REG_SD_D0_GPIO_MODE_MASK                BIT3
#define REG_SD_CLK_GPIO_MODE                    0x37
    #define REG_SD_CLK_GPIO_MODE_MASK               BIT3
#define REG_SD_CMD_GPIO_MODE                    0x38
    #define REG_SD_CMD_GPIO_MODE_MASK               BIT3
#define REG_SD_D3_GPIO_MODE                     0x39
    #define REG_SD_D3_GPIO_MODE_MASK                BIT3
#define REG_SD_D2_GPIO_MODE                     0x3A
    #define REG_SD_D2_GPIO_MODE_MASK                BIT3
#define REG_SD_GPIO0_GPIO_MODE                  0x3B
    #define REG_SD_GPIO0_GPIO_MODE_MASK             BIT3
#define REG_SD_GPIO1_GPIO_MODE                  0x3C
    #define REG_SD_GPIO1_GPIO_MODE_MASK             BIT3
#define REG_GPIO0_GPIO_MODE                     0x3D
    #define REG_GPIO0_GPIO_MODE_MASK                BIT3
#define REG_GPIO1_GPIO_MODE                     0x3E
    #define REG_GPIO1_GPIO_MODE_MASK                BIT3
#define REG_GPIO2_GPIO_MODE                     0x3F
    #define REG_GPIO2_GPIO_MODE_MASK                BIT3
#define REG_GPIO3_GPIO_MODE                     0x40
    #define REG_GPIO3_GPIO_MODE_MASK                BIT3
#define REG_GPIO4_GPIO_MODE                     0x41
    #define REG_GPIO4_GPIO_MODE_MASK                BIT3
#define REG_GPIO5_GPIO_MODE                     0x42
    #define REG_GPIO5_GPIO_MODE_MASK                BIT3
#define REG_GPIO6_GPIO_MODE                     0x43
    #define REG_GPIO6_GPIO_MODE_MASK                BIT3
#define REG_GPIO7_GPIO_MODE                     0x44
    #define REG_GPIO7_GPIO_MODE_MASK                BIT3
#define REG_GPIO8_GPIO_MODE                     0x45
    #define REG_GPIO8_GPIO_MODE_MASK                BIT3
#define REG_GPIO9_GPIO_MODE                     0x46
    #define REG_GPIO9_GPIO_MODE_MASK                BIT3

#define REG_ETH_GPIO_EN_MODE 0x71
    #define REG_ETH_GPIO_EN_MODE_MASK                BIT0

#define REG_SAR_GPIO0_GPIO_MODE         0x11
    #define REG_SAR_GPIO0_GPIO_MODE_MASK    BIT0
#define REG_SAR_GPIO1_GPIO_MODE         0x11
    #define REG_SAR_GPIO1_GPIO_MODE_MASK    BIT1
#define REG_SAR_GPIO2_GPIO_MODE         0x11
    #define REG_SAR_GPIO2_GPIO_MODE_MASK    BIT2
#define REG_SAR_GPIO3_GPIO_MODE         0x11
    #define REG_SAR_GPIO3_GPIO_MODE_MASK    BIT3
    
//-------------------- configuration -----------------
#define ENABLE_CHECK_ALL_PAD_CONFLICT       0

//==============================================================================
//
//                              STRUCTURE
//
//==============================================================================

typedef struct stPadMux
{
    U16 padID;
    U32 base;
    U16 offset;
    U16 mask;
    U16 val;
    U16 mode;
} ST_PadMuxInfo;

typedef struct stPadMode
{
    U8  u8PadName[32];
    U32 u32ModeRIU;
    U32 u32ModeMask;
} ST_PadModeInfo;

const ST_PadMuxInfo m_stPadMuxTbl[] =
{

//    {PAD_SR_IO00,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT9,                   PINMUX_FOR_EJ_MODE_2},
    {PAD_SR_IO00,      PADGPIO_BANK,     REG_SR_IO00_GPIO_MODE,          REG_SR_IO00_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_SR_IO00,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO00,      CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_SR_IO00,      CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_I2C0_MODE_3},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT5,                   PINMUX_FOR_I2C1_MODE_2},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT4,                   PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5,                   PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_SR0_BT601_MODE_3},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT0,                   PINMUX_FOR_ETH0_MODE_1},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1,                   PINMUX_FOR_ETH0_MODE_2},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_ETH0_MODE_3},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2,                   PINMUX_FOR_ETH0_MODE_4},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT0,                   PINMUX_FOR_PSPI0_SR_MODE_1},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT1|BIT0,              PINMUX_FOR_PSPI0_SR_MODE_3},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT2|BIT0,              PINMUX_FOR_PSPI0_SR_MODE_5},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_PSPI0_G_MODE,               REG_PSPI0_G_MODE_MASK,              BIT4,                   PINMUX_FOR_PSPI0_G_MODE_1},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_PSPI0_G_MODE,               REG_PSPI0_G_MODE_MASK,              BIT5,                   PINMUX_FOR_PSPI0_G_MODE_2},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1,                   PINMUX_FOR_SPI0_MODE_2},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT0,                   PINMUX_FOR_PSPI1_PL_MODE_1},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_PSPI1_G_MODE,               REG_PSPI1_G_MODE_MASK,              BIT13,                  PINMUX_FOR_PSPI1_G_MODE_2},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT8,                   PINMUX_FOR_FUART_MODE_1},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT10|BIT9|BIT8,        PINMUX_FOR_FUART_MODE_7},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT4,                   PINMUX_FOR_UART1_MODE_1},
    {PAD_SR_IO00,      PADTOP_BANK,      REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT0,                   PINMUX_FOR_PWM0_MODE_1},

//    {PAD_SR_IO01,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT9,                   PINMUX_FOR_EJ_MODE_2},
    {PAD_SR_IO01,      PADGPIO_BANK,     REG_SR_IO01_GPIO_MODE,          REG_SR_IO01_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_SR_IO01,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO01,      CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_SR_IO01,      CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_I2C0_MODE_3},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT5,                   PINMUX_FOR_I2C1_MODE_2},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT4,                   PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5,                   PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1,                   PINMUX_FOR_SR0_BT601_MODE_2},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_SR0_BT601_MODE_3},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT6,                   PINMUX_FOR_SR0_BT656_MODE_4},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT0,                   PINMUX_FOR_ETH0_MODE_1},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1,                   PINMUX_FOR_ETH0_MODE_2},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_ETH0_MODE_3},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2,                   PINMUX_FOR_ETH0_MODE_4},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT0,                   PINMUX_FOR_PSPI0_SR_MODE_1},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT1,                   PINMUX_FOR_PSPI0_SR_MODE_2},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT1|BIT0,              PINMUX_FOR_PSPI0_SR_MODE_3},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT2,                   PINMUX_FOR_PSPI0_SR_MODE_4},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT2|BIT0,              PINMUX_FOR_PSPI0_SR_MODE_5},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT2|BIT1,              PINMUX_FOR_PSPI0_SR_MODE_6},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_PSPI0_G_MODE,               REG_PSPI0_G_MODE_MASK,              BIT4,                   PINMUX_FOR_PSPI0_G_MODE_1},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_PSPI0_G_MODE,               REG_PSPI0_G_MODE_MASK,              BIT5,                   PINMUX_FOR_PSPI0_G_MODE_2},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1,                   PINMUX_FOR_SPI0_MODE_2},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT0,                   PINMUX_FOR_PSPI1_PL_MODE_1},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_PSPI1_G_MODE,               REG_PSPI1_G_MODE_MASK,              BIT13,                  PINMUX_FOR_PSPI1_G_MODE_2},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT8,                   PINMUX_FOR_FUART_MODE_1},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT10|BIT9|BIT8,        PINMUX_FOR_FUART_MODE_7},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT4,                   PINMUX_FOR_UART1_MODE_1},
    {PAD_SR_IO01,      PADTOP_BANK,      REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT4,                   PINMUX_FOR_PWM1_MODE_1},

//    {PAD_SR_IO02,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT9,                   PINMUX_FOR_EJ_MODE_2},
    {PAD_SR_IO02,      PADGPIO_BANK,     REG_SR_IO02_GPIO_MODE,          REG_SR_IO02_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_SR_IO02,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO02,      CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_SR_IO02,      CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT4,                   PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5,                   PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1,                   PINMUX_FOR_SR0_BT601_MODE_2},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_SR0_BT601_MODE_3},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5,                   PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT6,                   PINMUX_FOR_SR0_BT656_MODE_4},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT0,                   PINMUX_FOR_ETH0_MODE_1},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1,                   PINMUX_FOR_ETH0_MODE_2},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_ETH0_MODE_3},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2,                   PINMUX_FOR_ETH0_MODE_4},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13,                  PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT0,                   PINMUX_FOR_PSPI0_SR_MODE_1},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT1,                   PINMUX_FOR_PSPI0_SR_MODE_2},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT1|BIT0,              PINMUX_FOR_PSPI0_SR_MODE_3},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT2,                   PINMUX_FOR_PSPI0_SR_MODE_4},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT2|BIT0,              PINMUX_FOR_PSPI0_SR_MODE_5},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT2|BIT1,              PINMUX_FOR_PSPI0_SR_MODE_6},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_PSPI0_G_MODE,               REG_PSPI0_G_MODE_MASK,              BIT4,                   PINMUX_FOR_PSPI0_G_MODE_1},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_PSPI0_G_MODE,               REG_PSPI0_G_MODE_MASK,              BIT5,                   PINMUX_FOR_PSPI0_G_MODE_2},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1,                   PINMUX_FOR_SPI0_MODE_2},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT0,                   PINMUX_FOR_PSPI1_PL_MODE_1},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_PSPI1_G_MODE,               REG_PSPI1_G_MODE_MASK,              BIT13,                  PINMUX_FOR_PSPI1_G_MODE_2},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT8,                   PINMUX_FOR_FUART_MODE_1},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT5,                   PINMUX_FOR_UART1_MODE_2},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_UART2_MODE,                 REG_UART2_MODE_MASK,                BIT8,                   PINMUX_FOR_UART2_MODE_1},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT8,                   PINMUX_FOR_PWM2_MODE_1},
    {PAD_SR_IO02,      PADTOP_BANK,      REG_I2S_MCK_MODE,               REG_I2S_MCK_MODE_MASK,              BIT0,                   PINMUX_FOR_I2S_MCK_MODE_1},

//    {PAD_SR_IO03,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT9,                   PINMUX_FOR_EJ_MODE_2},
    {PAD_SR_IO03,      PADGPIO_BANK,     REG_SR_IO03_GPIO_MODE,          REG_SR_IO03_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_SR_IO03,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO03,      CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_SR_IO03,      CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT4,                   PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5,                   PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1,                   PINMUX_FOR_SR0_BT601_MODE_2},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_SR0_BT601_MODE_3},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5,                   PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT6,                   PINMUX_FOR_SR0_BT656_MODE_4},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT0,                   PINMUX_FOR_ETH0_MODE_1},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1,                   PINMUX_FOR_ETH0_MODE_2},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_ETH0_MODE_3},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2,                   PINMUX_FOR_ETH0_MODE_4},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13,                  PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT0,                   PINMUX_FOR_PSPI0_SR_MODE_1},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT1,                   PINMUX_FOR_PSPI0_SR_MODE_2},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT1|BIT0,              PINMUX_FOR_PSPI0_SR_MODE_3},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT2,                   PINMUX_FOR_PSPI0_SR_MODE_4},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_PSPI0_G_MODE,               REG_PSPI0_G_MODE_MASK,              BIT4,                   PINMUX_FOR_PSPI0_G_MODE_1},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_PSPI0_G_MODE,               REG_PSPI0_G_MODE_MASK,              BIT5,                   PINMUX_FOR_PSPI0_G_MODE_2},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1,                   PINMUX_FOR_SPI0_MODE_2},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT0,                   PINMUX_FOR_PSPI1_PL_MODE_1},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_PSPI1_G_MODE,               REG_PSPI1_G_MODE_MASK,              BIT13,                  PINMUX_FOR_PSPI1_G_MODE_2},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT8,                   PINMUX_FOR_FUART_MODE_1},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT5,                   PINMUX_FOR_UART1_MODE_2},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_UART2_MODE,                 REG_UART2_MODE_MASK,                BIT8,                   PINMUX_FOR_UART2_MODE_1},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT12,                  PINMUX_FOR_PWM3_MODE_1},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT4,                   PINMUX_FOR_I2S_RX_MODE_1},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT8,                   PINMUX_FOR_I2S_TX_MODE_1},
    {PAD_SR_IO03,      PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT12,                  PINMUX_FOR_I2S_RXTX_MODE_1},

    {PAD_SR_IO04,      PADGPIO_BANK,     REG_SR_IO04_GPIO_MODE,          REG_SR_IO04_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SR_IO04,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO04,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO04,      CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_SR_IO04,      CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT2,                   PINMUX_FOR_I2C0_MODE_4},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT5|BIT4,              PINMUX_FOR_I2C1_MODE_3},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_SR0_MIPI_MODE,              REG_SR0_MIPI_MODE_MASK,             BIT0,                   PINMUX_FOR_SR0_MIPI_MODE_1},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5,                   PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1,                   PINMUX_FOR_SR0_BT601_MODE_2},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_SR0_BT601_MODE_3},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5,                   PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT6,                   PINMUX_FOR_SR0_BT656_MODE_4},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT0,                   PINMUX_FOR_ETH0_MODE_1},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1,                   PINMUX_FOR_ETH0_MODE_2},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_ETH0_MODE_3},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2,                   PINMUX_FOR_ETH0_MODE_4},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13,                  PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT0,                   PINMUX_FOR_PSPI0_SR_MODE_1},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT1,                   PINMUX_FOR_PSPI0_SR_MODE_2},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_PSPI0_G_MODE,               REG_PSPI0_G_MODE_MASK,              BIT5,                   PINMUX_FOR_PSPI0_G_MODE_2},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_PSPI1_CS2_MODE,             REG_PSPI1_CS2_MODE_MASK,            BIT5,                   PINMUX_FOR_PSPI1_CS2_MODE_2},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_PSPI1_TE_MODE,              REG_PSPI1_TE_MODE_MASK,             BIT9,                   PINMUX_FOR_PSPI1_TE_MODE_2},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_PSPI1_G_MODE,               REG_PSPI1_G_MODE_MASK,              BIT13,                  PINMUX_FOR_PSPI1_G_MODE_2},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT1,                   PINMUX_FOR_PWM0_MODE_2},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT0,                   PINMUX_FOR_DMIC_MODE_1},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2|BIT1|BIT0,         PINMUX_FOR_DMIC_MODE_7},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT4,                   PINMUX_FOR_I2S_RX_MODE_1},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT8,                   PINMUX_FOR_I2S_TX_MODE_1},
    {PAD_SR_IO04,      PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT12,                  PINMUX_FOR_I2S_RXTX_MODE_1},

    {PAD_SR_IO05,      PADGPIO_BANK,     REG_SR_IO05_GPIO_MODE,          REG_SR_IO05_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SR_IO05,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO05,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO05,      CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_SR_IO05,      CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT2,                   PINMUX_FOR_I2C0_MODE_4},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT5|BIT4,              PINMUX_FOR_I2C1_MODE_3},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_SR0_MIPI_MODE,              REG_SR0_MIPI_MODE_MASK,             BIT0,                   PINMUX_FOR_SR0_MIPI_MODE_1},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5,                   PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1,                   PINMUX_FOR_SR0_BT601_MODE_2},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_SR0_BT601_MODE_3},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT4,                   PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5,                   PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT6,                   PINMUX_FOR_SR0_BT656_MODE_4},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT0,                   PINMUX_FOR_ETH0_MODE_1},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1,                   PINMUX_FOR_ETH0_MODE_2},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_ETH0_MODE_3},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2,                   PINMUX_FOR_ETH0_MODE_4},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13,                  PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT0,                   PINMUX_FOR_PSPI0_SR_MODE_1},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_PSPI0_SR_MODE,              REG_PSPI0_SR_MODE_MASK,             BIT1,                   PINMUX_FOR_PSPI0_SR_MODE_2},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_PSPI0_G_MODE,               REG_PSPI0_G_MODE_MASK,              BIT5,                   PINMUX_FOR_PSPI0_G_MODE_2},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_PSPI1_G_MODE,               REG_PSPI1_G_MODE_MASK,              BIT13,                  PINMUX_FOR_PSPI1_G_MODE_2},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT5,                   PINMUX_FOR_PWM1_MODE_2},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT0,                   PINMUX_FOR_DMIC_MODE_1},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2|BIT1|BIT0,         PINMUX_FOR_DMIC_MODE_7},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT4,                   PINMUX_FOR_I2S_RX_MODE_1},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT5,                   PINMUX_FOR_I2S_RX_MODE_2},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT12,                  PINMUX_FOR_I2S_RXTX_MODE_1},
    {PAD_SR_IO05,      PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13,                  PINMUX_FOR_I2S_RXTX_MODE_2},

    {PAD_SR_IO06,      PADGPIO_BANK,     REG_SR_IO06_GPIO_MODE,          REG_SR_IO06_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SR_IO06,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO06,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO06,      CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_SR_IO06,      CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_I2C0_MODE_5},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT6,                   PINMUX_FOR_I2C1_MODE_4},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_SR0_MIPI_MODE,              REG_SR0_MIPI_MODE_MASK,             BIT0,                   PINMUX_FOR_SR0_MIPI_MODE_1},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1,                   PINMUX_FOR_SR0_BT601_MODE_2},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_SR0_BT601_MODE_3},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT4,                   PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5,                   PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT6,                   PINMUX_FOR_SR0_BT656_MODE_4},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT0,                   PINMUX_FOR_ETH0_MODE_1},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1,                   PINMUX_FOR_ETH0_MODE_2},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_ETH0_MODE_3},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2,                   PINMUX_FOR_ETH0_MODE_4},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13,                  PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_PSPI0_G_MODE,               REG_PSPI0_G_MODE_MASK,              BIT5,                   PINMUX_FOR_PSPI0_G_MODE_2},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_PSPI1_CS2_MODE,             REG_PSPI1_CS2_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_PSPI1_CS2_MODE_3},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_PSPI1_TE_MODE,              REG_PSPI1_TE_MODE_MASK,             BIT9|BIT8,              PINMUX_FOR_PSPI1_TE_MODE_3},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_PSPI1_G_MODE,               REG_PSPI1_G_MODE_MASK,              BIT13,                  PINMUX_FOR_PSPI1_G_MODE_2},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT9,                   PINMUX_FOR_FUART_MODE_2},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT11,                  PINMUX_FOR_FUART_MODE_8},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT9,                   PINMUX_FOR_PWM2_MODE_2},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT0,                   PINMUX_FOR_DMIC_MODE_1},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT8,                   PINMUX_FOR_I2S_TX_MODE_1},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT9,                   PINMUX_FOR_I2S_TX_MODE_2},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT12,                  PINMUX_FOR_I2S_RXTX_MODE_1},
    {PAD_SR_IO06,      PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13,                  PINMUX_FOR_I2S_RXTX_MODE_2},

    {PAD_SR_IO07,      PADGPIO_BANK,     REG_SR_IO07_GPIO_MODE,          REG_SR_IO07_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SR_IO07,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO07,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO07,      CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_SR_IO07,      CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_I2C0_MODE_5},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT6,                   PINMUX_FOR_I2C1_MODE_4},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_SR0_MIPI_MODE,              REG_SR0_MIPI_MODE_MASK,             BIT0,                   PINMUX_FOR_SR0_MIPI_MODE_1},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1,                   PINMUX_FOR_SR0_BT601_MODE_2},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_SR0_BT601_MODE_3},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT4,                   PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5,                   PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT6,                   PINMUX_FOR_SR0_BT656_MODE_4},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT0,                   PINMUX_FOR_ETH0_MODE_1},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1,                   PINMUX_FOR_ETH0_MODE_2},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_ETH0_MODE_3},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2,                   PINMUX_FOR_ETH0_MODE_4},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13,                  PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_PSPI0_G_MODE,               REG_PSPI0_G_MODE_MASK,              BIT5,                   PINMUX_FOR_PSPI0_G_MODE_2},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT1,                   PINMUX_FOR_PSPI1_PL_MODE_2},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_PSPI1_G_MODE,               REG_PSPI1_G_MODE_MASK,              BIT12,                  PINMUX_FOR_PSPI1_G_MODE_1},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_PSPI1_G_MODE,               REG_PSPI1_G_MODE_MASK,              BIT13,                  PINMUX_FOR_PSPI1_G_MODE_2},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT8,                   PINMUX_FOR_SPI1_MODE_1},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT9,                   PINMUX_FOR_FUART_MODE_2},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT11,                  PINMUX_FOR_FUART_MODE_8},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT13,                  PINMUX_FOR_PWM3_MODE_2},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT3,                   PINMUX_FOR_DMIC_MODE_8},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_I2S_MCK_MODE,               REG_I2S_MCK_MODE_MASK,              BIT1,                   PINMUX_FOR_I2S_MCK_MODE_2},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT5,                   PINMUX_FOR_I2S_RX_MODE_2},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT9,                   PINMUX_FOR_I2S_TX_MODE_2},
    {PAD_SR_IO07,      PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13,                  PINMUX_FOR_I2S_RXTX_MODE_2},

    {PAD_SR_IO08,      PADGPIO_BANK,     REG_SR_IO08_GPIO_MODE,          REG_SR_IO08_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SR_IO08,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO08,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT2|BIT1,              PINMUX_FOR_I2C0_MODE_6},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT6|BIT4,              PINMUX_FOR_I2C1_MODE_5},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT7|BIT5|BIT4,         PINMUX_FOR_I2C1_MODE_11},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_ISP_IR_MODE,                REG_ISP_IR_MODE_MASK,               BIT0,                   PINMUX_FOR_ISP_IR_MODE_1},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_SR1_CTRL_MODE,              REG_SR1_CTRL_MODE_MASK,             BIT12,                  PINMUX_FOR_SR1_CTRL_MODE_1},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_SR1_PDN_MODE,               REG_SR1_PDN_MODE_MASK,              BIT8,                   PINMUX_FOR_SR1_PDN_MODE_1},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1,                   PINMUX_FOR_SR0_BT601_MODE_2},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_SR0_BT601_MODE_3},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT4,                   PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT6,                   PINMUX_FOR_SR0_BT656_MODE_4},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT0,                   PINMUX_FOR_ETH0_MODE_1},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_ETH0_MODE_3},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT9,                   PINMUX_FOR_FUART_MODE_2},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_UART1_MODE_3},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_PWM0_MODE_3},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT3,                   PINMUX_FOR_DMIC_MODE_8},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT5,                   PINMUX_FOR_I2S_RX_MODE_2},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT9,                   PINMUX_FOR_I2S_TX_MODE_2},
    {PAD_SR_IO08,      PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13,                  PINMUX_FOR_I2S_RXTX_MODE_2},

    {PAD_SR_IO09,      PADGPIO_BANK,     REG_SR_IO09_GPIO_MODE,          REG_SR_IO09_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SR_IO09,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO09,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO09,      CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_SR_IO09,      CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_SR0_CTRL_MODE,              REG_SR0_CTRL_MODE_MASK,             BIT9,                   PINMUX_FOR_SR0_CTRL_MODE_2},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_SR0_MCLK_MODE,              REG_SR0_MCLK_MODE_MASK,             BIT1,                   PINMUX_FOR_SR0_MCLK_MODE_2},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_SR1_CTRL_MODE,              REG_SR1_CTRL_MODE_MASK,             BIT12,                  PINMUX_FOR_SR1_CTRL_MODE_1},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_SR1_CTRL_MODE,              REG_SR1_CTRL_MODE_MASK,             BIT13,                  PINMUX_FOR_SR1_CTRL_MODE_2},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_SR1_MCLK_MODE,              REG_SR1_MCLK_MODE_MASK,             BIT1,                   PINMUX_FOR_SR1_MCLK_MODE_2},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_SR1_RST_MODE,               REG_SR1_RST_MODE_MASK,              BIT4,                   PINMUX_FOR_SR1_RST_MODE_1},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1,                   PINMUX_FOR_SR0_BT601_MODE_2},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_SR0_BT601_MODE_3},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT4,                   PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5,                   PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT1,                   PINMUX_FOR_ETH0_MODE_2},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2,                   PINMUX_FOR_ETH0_MODE_4},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13,                  PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_PSPI0_G_MODE,               REG_PSPI0_G_MODE_MASK,              BIT5,                   PINMUX_FOR_PSPI0_G_MODE_2},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT1,                   PINMUX_FOR_PSPI1_PL_MODE_2},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_PSPI1_G_MODE,               REG_PSPI1_G_MODE_MASK,              BIT12,                  PINMUX_FOR_PSPI1_G_MODE_1},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_PSPI1_G_MODE,               REG_PSPI1_G_MODE_MASK,              BIT13,                  PINMUX_FOR_PSPI1_G_MODE_2},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT8,                   PINMUX_FOR_SPI1_MODE_1},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT9,                   PINMUX_FOR_FUART_MODE_2},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_UART1_MODE_3},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT5|BIT4,              PINMUX_FOR_PWM1_MODE_3},
    {PAD_SR_IO09,      PADTOP_BANK,      REG_I2S_MCK_MODE,               REG_I2S_MCK_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_I2S_MCK_MODE_3},

    {PAD_SR_IO10,      PADGPIO_BANK,     REG_SR_IO10_GPIO_MODE,          REG_SR_IO10_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SR_IO10,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO10,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_SR0_CTRL_MODE,              REG_SR0_CTRL_MODE_MASK,             BIT9,                   PINMUX_FOR_SR0_CTRL_MODE_2},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_SR0_RST_MODE,               REG_SR0_RST_MODE_MASK,              BIT5,                   PINMUX_FOR_SR0_RST_MODE_2},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_SR1_CTRL_MODE,              REG_SR1_CTRL_MODE_MASK,             BIT12,                  PINMUX_FOR_SR1_CTRL_MODE_1},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_SR1_CTRL_MODE,              REG_SR1_CTRL_MODE_MASK,             BIT13,                  PINMUX_FOR_SR1_CTRL_MODE_2},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_SR1_MCLK_MODE,              REG_SR1_MCLK_MODE_MASK,             BIT0,                   PINMUX_FOR_SR1_MCLK_MODE_1},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_SR1_RST_MODE,               REG_SR1_RST_MODE_MASK,              BIT5,                   PINMUX_FOR_SR1_RST_MODE_2},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1,                   PINMUX_FOR_SR0_BT601_MODE_2},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_SR0_BT601_MODE_3},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT4,                   PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5,                   PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13,                  PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_PSPI0_G_MODE,               REG_PSPI0_G_MODE_MASK,              BIT5,                   PINMUX_FOR_PSPI0_G_MODE_2},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT1,                   PINMUX_FOR_PSPI1_PL_MODE_2},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_PSPI1_G_MODE,               REG_PSPI1_G_MODE_MASK,              BIT12,                  PINMUX_FOR_PSPI1_G_MODE_1},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_PSPI1_G_MODE,               REG_PSPI1_G_MODE_MASK,              BIT13,                  PINMUX_FOR_PSPI1_G_MODE_2},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT8,                   PINMUX_FOR_SPI1_MODE_1},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT0,                   PINMUX_FOR_UART0_MODE_1},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_UART2_MODE,                 REG_UART2_MODE_MASK,                BIT9,                   PINMUX_FOR_UART2_MODE_2},
    {PAD_SR_IO10,      PADTOP_BANK,      REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT9|BIT8,              PINMUX_FOR_PWM2_MODE_3},

    {PAD_SR_IO11,      PADGPIO_BANK,     REG_SR_IO11_GPIO_MODE,          REG_SR_IO11_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SR_IO11,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO11,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT2|BIT1,              PINMUX_FOR_I2C0_MODE_6},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT6|BIT4,              PINMUX_FOR_I2C1_MODE_5},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT7|BIT5|BIT4,         PINMUX_FOR_I2C1_MODE_11},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_ISP_IR_MODE,                REG_ISP_IR_MODE_MASK,               BIT1,                   PINMUX_FOR_ISP_IR_MODE_2},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_SR0_CTRL_MODE,              REG_SR0_CTRL_MODE_MASK,             BIT8,                   PINMUX_FOR_SR0_CTRL_MODE_1},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_SR0_CTRL_MODE,              REG_SR0_CTRL_MODE_MASK,             BIT9,                   PINMUX_FOR_SR0_CTRL_MODE_2},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_SR0_PDN_MODE,               REG_SR0_PDN_MODE_MASK,              BIT8,                   PINMUX_FOR_SR0_PDN_MODE_1},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_SR1_CTRL_MODE,              REG_SR1_CTRL_MODE_MASK,             BIT13,                  PINMUX_FOR_SR1_CTRL_MODE_2},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_SR1_PDN_MODE,               REG_SR1_PDN_MODE_MASK,              BIT9,                   PINMUX_FOR_SR1_PDN_MODE_2},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1,                   PINMUX_FOR_SR0_BT601_MODE_2},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_SR0_BT601_MODE_3},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT4,                   PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5,                   PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT6,                   PINMUX_FOR_SR0_BT656_MODE_4},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13,                  PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT1,                   PINMUX_FOR_PSPI1_PL_MODE_2},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_PSPI1_CS2_MODE,             REG_PSPI1_CS2_MODE_MASK,            BIT6,                   PINMUX_FOR_PSPI1_CS2_MODE_4},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_PSPI1_TE_MODE,              REG_PSPI1_TE_MODE_MASK,             BIT10,                  PINMUX_FOR_PSPI1_TE_MODE_4},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_PSPI1_G_MODE,               REG_PSPI1_G_MODE_MASK,              BIT12,                  PINMUX_FOR_PSPI1_G_MODE_1},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT8,                   PINMUX_FOR_SPI1_MODE_1},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT0,                   PINMUX_FOR_UART0_MODE_1},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_UART2_MODE,                 REG_UART2_MODE_MASK,                BIT9,                   PINMUX_FOR_UART2_MODE_2},
    {PAD_SR_IO11,      PADTOP_BANK,      REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT13|BIT12,            PINMUX_FOR_PWM3_MODE_3},

    {PAD_SR_IO12,      PADGPIO_BANK,     REG_SR_IO12_GPIO_MODE,          REG_SR_IO12_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SR_IO12,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO12,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO12,      PADTOP_BANK,      REG_SR0_CTRL_MODE,              REG_SR0_CTRL_MODE_MASK,             BIT8,                   PINMUX_FOR_SR0_CTRL_MODE_1},
    {PAD_SR_IO12,      PADTOP_BANK,      REG_SR0_RST_MODE,               REG_SR0_RST_MODE_MASK,              BIT4,                   PINMUX_FOR_SR0_RST_MODE_1},
    {PAD_SR_IO12,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO12,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT4,                   PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR_IO12,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR_IO12,      PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_SR_IO12,      PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT1|BIT0,              PINMUX_FOR_PSPI1_PL_MODE_3},
    {PAD_SR_IO12,      PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT9,                   PINMUX_FOR_SPI1_MODE_2},

    {PAD_SR_IO13,      PADGPIO_BANK,     REG_SR_IO13_GPIO_MODE,          REG_SR_IO13_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SR_IO13,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO13,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO13,      PADTOP_BANK,      REG_SR0_CTRL_MODE,              REG_SR0_CTRL_MODE_MASK,             BIT8,                   PINMUX_FOR_SR0_CTRL_MODE_1},
    {PAD_SR_IO13,      PADTOP_BANK,      REG_SR0_MCLK_MODE,              REG_SR0_MCLK_MODE_MASK,             BIT0,                   PINMUX_FOR_SR0_MCLK_MODE_1},
    {PAD_SR_IO13,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO13,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT4,                   PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR_IO13,      PADTOP_BANK,      REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR_IO13,      PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_SR_IO13,      PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT1|BIT0,              PINMUX_FOR_PSPI1_PL_MODE_3},
    {PAD_SR_IO13,      PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT9,                   PINMUX_FOR_SPI1_MODE_2},

    {PAD_SR_IO14,      PADGPIO_BANK,     REG_SR_IO14_GPIO_MODE,          REG_SR_IO14_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SR_IO14,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO14,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO14,      PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT2|BIT1|BIT0,         PINMUX_FOR_I2C0_MODE_7},
    {PAD_SR_IO14,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT6|BIT5,              PINMUX_FOR_I2C1_MODE_6},
    {PAD_SR_IO14,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT7|BIT5|BIT4,         PINMUX_FOR_I2C1_MODE_11},
    {PAD_SR_IO14,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO14,      PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_SR_IO14,      PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT1|BIT0,              PINMUX_FOR_PSPI1_PL_MODE_3},
    {PAD_SR_IO14,      PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT9,                   PINMUX_FOR_SPI1_MODE_2},

    {PAD_SR_IO15,      PADGPIO_BANK,     REG_SR_IO15_GPIO_MODE,          REG_SR_IO15_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SR_IO15,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO15,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO15,      PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT2|BIT1|BIT0,         PINMUX_FOR_I2C0_MODE_7},
    {PAD_SR_IO15,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT6|BIT5,              PINMUX_FOR_I2C1_MODE_6},
    {PAD_SR_IO15,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT7|BIT5|BIT4,         PINMUX_FOR_I2C1_MODE_11},
    {PAD_SR_IO15,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT0,                   PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR_IO15,      PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_SR_IO15,      PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT1|BIT0,              PINMUX_FOR_PSPI1_PL_MODE_3},
    {PAD_SR_IO15,      PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT9,                   PINMUX_FOR_SPI1_MODE_2},

    {PAD_SR_IO16,      PADGPIO_BANK,     REG_SR_IO16_GPIO_MODE,          REG_SR_IO16_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SR_IO16,      CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SR_IO16,      CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SR_IO16,      PADTOP_BANK,      REG_ISP_IR_MODE,                REG_ISP_IR_MODE_MASK,               BIT1|BIT0,              PINMUX_FOR_ISP_IR_MODE_3},
    {PAD_SR_IO16,      PADTOP_BANK,      REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_SR0_BT601_MODE_3},
    {PAD_SR_IO16,      PADTOP_BANK,      REG_PSPI1_CS2_MODE,             REG_PSPI1_CS2_MODE_MASK,            BIT4,                   PINMUX_FOR_PSPI1_CS2_MODE_1},
    {PAD_SR_IO16,      PADTOP_BANK,      REG_PSPI1_TE_MODE,              REG_PSPI1_TE_MODE_MASK,             BIT8,                   PINMUX_FOR_PSPI1_TE_MODE_1},
    {PAD_SR_IO16,      PADTOP_BANK,      REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT11|BIT9|BIT8,        PINMUX_FOR_PWM2_MODE_11},

    //    {PAD_TTL0,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL0,         PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT8,                   PINMUX_FOR_EJ_MODE_1},
    {PAD_TTL0,         PADGPIO_BANK,     REG_TTL0_GPIO_MODE,          REG_TTL0_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_TTL0,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL0,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL0,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL0,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL0,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL0,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL0,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL0,         PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT0,                   PINMUX_FOR_I2C0_MODE_1},
    {PAD_TTL0,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_ETH0_MODE_5},
    {PAD_TTL0,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1,              PINMUX_FOR_ETH0_MODE_6},
    {PAD_TTL0,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL0,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL0,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL0,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL0,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL0,         PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2,                   PINMUX_FOR_PSPI1_PL_MODE_4},
    {PAD_TTL0,         PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT9|BIT8,              PINMUX_FOR_SPI1_MODE_3},
    {PAD_TTL0,         PADTOP_BANK,      REG_SD_CDZ_MODE,                REG_SD_CDZ_MODE_MASK,               BIT1,                   PINMUX_FOR_SD_CDZ_MODE_2},

 //    {PAD_TTL1,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL1,         PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT8,                   PINMUX_FOR_EJ_MODE_1},
    {PAD_TTL1,         PADGPIO_BANK,     REG_TTL1_GPIO_MODE,          REG_TTL1_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_TTL1,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL1,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL1,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL1,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL1,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL1,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL1,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL1,         PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT0,                   PINMUX_FOR_I2C0_MODE_1},
    {PAD_TTL1,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_ETH0_MODE_5},
    {PAD_TTL1,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1,              PINMUX_FOR_ETH0_MODE_6},
    {PAD_TTL1,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL1,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL1,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL1,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL1,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL1,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL1,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL1,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL1,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL1,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL1,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL1,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL1,         PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_RGB8_MODE_3},
    {PAD_TTL1,         PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2,                   PINMUX_FOR_PSPI1_PL_MODE_4},
    {PAD_TTL1,         PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT9|BIT8,              PINMUX_FOR_SPI1_MODE_3},
    {PAD_TTL1,         PADTOP_BANK,      REG_SDIO_MODE,                  REG_SDIO_MODE_MASK,                 BIT9,                   PINMUX_FOR_SDIO_MODE_2},


//    {PAD_TTL2,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL2,         PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT8,                   PINMUX_FOR_EJ_MODE_1},
    {PAD_TTL2,         PADGPIO_BANK,     REG_TTL2_GPIO_MODE,          REG_TTL2_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_TTL2,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL2,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL2,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL2,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL2,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL2,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL2,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL2,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_ETH0_MODE_5},
    {PAD_TTL2,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1,              PINMUX_FOR_ETH0_MODE_6},
    {PAD_TTL2,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL2,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL2,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL2,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL2,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL2,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL2,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL2,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL2,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL2,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL2,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL2,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL2,         PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_RGB8_MODE_3},
    {PAD_TTL2,         PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2,                   PINMUX_FOR_PSPI1_PL_MODE_4},
    {PAD_TTL2,         PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT9|BIT8,              PINMUX_FOR_SPI1_MODE_3},
    {PAD_TTL2,         PADTOP_BANK,      REG_SDIO_MODE,                  REG_SDIO_MODE_MASK,                 BIT9,                   PINMUX_FOR_SDIO_MODE_2},

//    {PAD_TTL3,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL3,         PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT8,                   PINMUX_FOR_EJ_MODE_1},
    {PAD_TTL3,         PADGPIO_BANK,     REG_TTL3_GPIO_MODE,          REG_TTL3_GPIO_MODE_MASK,         BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_TTL3,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL3,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL3,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL3,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL3,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL3,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL3,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL3,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_ETH0_MODE_5},
    {PAD_TTL3,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1,              PINMUX_FOR_ETH0_MODE_6},
    {PAD_TTL3,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL3,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL3,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL3,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL3,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL3,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL3,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL3,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL3,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL3,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL3,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL3,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL3,         PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_RGB8_MODE_3},
    {PAD_TTL3,         PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2,                   PINMUX_FOR_PSPI1_PL_MODE_4},
    {PAD_TTL3,         PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT9|BIT8,              PINMUX_FOR_SPI1_MODE_3},
    {PAD_TTL3,         PADTOP_BANK,      REG_SDIO_MODE,                  REG_SDIO_MODE_MASK,                 BIT9,                   PINMUX_FOR_SDIO_MODE_2},

    {PAD_TTL4,         PADGPIO_BANK,     REG_TTL4_GPIO_MODE,             REG_TTL4_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_TTL4,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL4,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL4,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL4,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL4,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL4,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL4,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL4,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL4,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_ETH0_MODE_5},
    {PAD_TTL4,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1,              PINMUX_FOR_ETH0_MODE_6},
    {PAD_TTL4,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL4,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL4,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL4,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL4,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL4,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL4,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL4,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL4,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL4,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL4,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL4,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL4,         PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_RGB8_MODE_3},
    {PAD_TTL4,         PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13|BIT12,            PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_TTL4,         PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_SPI0_MODE_5},
    {PAD_TTL4,         PADTOP_BANK,      REG_PSPI1_CS2_MODE,             REG_PSPI1_CS2_MODE_MASK,            BIT6|BIT4,              PINMUX_FOR_PSPI1_CS2_MODE_5},
    {PAD_TTL4,         PADTOP_BANK,      REG_PSPI1_TE_MODE,              REG_PSPI1_TE_MODE_MASK,             BIT10|BIT8,             PINMUX_FOR_PSPI1_TE_MODE_5},
    {PAD_TTL4,         PADTOP_BANK,      REG_SPI1_CS2_MODE,              REG_SPI1_CS2_MODE_MASK,             BIT12,                  PINMUX_FOR_SPI1_CS2_MODE_1},
    {PAD_TTL4,         PADTOP_BANK,      REG_SDIO_MODE,                  REG_SDIO_MODE_MASK,                 BIT9,                   PINMUX_FOR_SDIO_MODE_2},
    {PAD_TTL4,         PADTOP_BANK,      REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT2,                   PINMUX_FOR_PWM0_MODE_4},

    {PAD_TTL5,         PADGPIO_BANK,     REG_TTL5_GPIO_MODE,             REG_TTL5_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_TTL5,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL5,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL5,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL5,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL5,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL5,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL5,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL5,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL5,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_ETH0_MODE_5},
    {PAD_TTL5,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1,              PINMUX_FOR_ETH0_MODE_6},
    {PAD_TTL5,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL5,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL5,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL5,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL5,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL5,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL5,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL5,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL5,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL5,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL5,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL5,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL5,         PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_RGB8_MODE_3},
    {PAD_TTL5,         PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13|BIT12,            PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_TTL5,         PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_SPI0_MODE_5},
    {PAD_TTL5,         PADTOP_BANK,      REG_SDIO_MODE,                  REG_SDIO_MODE_MASK,                 BIT9,                   PINMUX_FOR_SDIO_MODE_2},
    {PAD_TTL5,         PADTOP_BANK,      REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT6,                   PINMUX_FOR_PWM1_MODE_4},

    {PAD_TTL6,         PADGPIO_BANK,     REG_TTL6_GPIO_MODE,             REG_TTL6_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_TTL6,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL6,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL6,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL6,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL6,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL6,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL6,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL6,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL6,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_ETH0_MODE_5},
    {PAD_TTL6,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1,              PINMUX_FOR_ETH0_MODE_6},
    {PAD_TTL6,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL6,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL6,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL6,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL6,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL6,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL6,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL6,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL6,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL6,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL6,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL6,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL6,         PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_RGB8_MODE_3},
    {PAD_TTL6,         PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13|BIT12,            PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_TTL6,         PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_SPI0_MODE_5},
    {PAD_TTL6,         PADTOP_BANK,      REG_SDIO_MODE,                  REG_SDIO_MODE_MASK,                 BIT9,                   PINMUX_FOR_SDIO_MODE_2},
    {PAD_TTL6,         PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT6,                   PINMUX_FOR_UART1_MODE_4},
    {PAD_TTL6,         PADTOP_BANK,      REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT10,                  PINMUX_FOR_PWM2_MODE_4},

    {PAD_TTL7,         PADGPIO_BANK,     REG_TTL7_GPIO_MODE,             REG_TTL7_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_TTL7,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL7,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL7,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL7,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL7,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL7,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL7,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL7,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL7,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_ETH0_MODE_5},
    {PAD_TTL7,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1,              PINMUX_FOR_ETH0_MODE_6},
    {PAD_TTL7,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL7,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL7,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL7,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL7,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL7,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL7,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL7,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL7,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL7,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL7,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL7,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL7,         PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_RGB8_MODE_3},
    {PAD_TTL7,         PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13|BIT12,            PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_TTL7,         PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_SPI0_MODE_5},
    {PAD_TTL7,         PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT6,                   PINMUX_FOR_UART1_MODE_4},
    {PAD_TTL7,         PADTOP_BANK,      REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT14,                  PINMUX_FOR_PWM3_MODE_4},

    {PAD_TTL8,         PADGPIO_BANK,     REG_TTL8_GPIO_MODE,             REG_TTL8_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_TTL8,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL8,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL8,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL8,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL8,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL8,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL8,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL8,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL8,         PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT3,                   PINMUX_FOR_I2C0_MODE_8},
    {PAD_TTL8,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_ETH0_MODE_5},
    {PAD_TTL8,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1,              PINMUX_FOR_ETH0_MODE_6},
    {PAD_TTL8,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL8,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL8,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL8,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL8,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL8,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL8,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL8,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL8,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL8,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL8,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL8,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL8,         PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1,                   PINMUX_FOR_RGB8_MODE_2},
    {PAD_TTL8,         PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_RGB8_MODE_3},
    {PAD_TTL8,         PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13|BIT12,            PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_TTL8,         PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_FUART_MODE_3},
    {PAD_TTL8,         PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT11|BIT8,             PINMUX_FOR_FUART_MODE_9},
    {PAD_TTL8,         PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT6|BIT4,              PINMUX_FOR_UART1_MODE_5},

    {PAD_TTL9,         PADGPIO_BANK,     REG_TTL9_GPIO_MODE,             REG_TTL9_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_TTL9,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL9,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL9,         CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL9,         CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL9,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL9,         PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL9,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL9,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL9,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL9,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL9,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL9,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL9,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL9,         PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT0,                   PINMUX_FOR_RGB8_MODE_1},
    {PAD_TTL9,         PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_RGB8_MODE_3},
    {PAD_TTL9,         PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13|BIT12,            PINMUX_FOR_BT656_OUT_MODE_3},

    {PAD_TTL10,        PADGPIO_BANK,     REG_TTL10_GPIO_MODE,            REG_TTL10_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_TTL10,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL10,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL10,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL10,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL10,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL10,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL10,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL10,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL10,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL10,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL10,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL10,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL10,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL10,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT0,                   PINMUX_FOR_RGB8_MODE_1},
    {PAD_TTL10,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_RGB8_MODE_3},
    {PAD_TTL10,        PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13|BIT12,            PINMUX_FOR_BT656_OUT_MODE_3},

    {PAD_TTL11,        PADGPIO_BANK,     REG_TTL11_GPIO_MODE,            REG_TTL11_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_TTL11,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL11,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL11,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL11,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL11,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL11,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL11,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL11,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL11,        PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT3,                   PINMUX_FOR_I2C0_MODE_8},
    {PAD_TTL11,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL11,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL11,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL11,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL11,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL11,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL11,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL11,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL11,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL11,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL11,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL11,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL11,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT0,                   PINMUX_FOR_RGB8_MODE_1},
    {PAD_TTL11,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1,                   PINMUX_FOR_RGB8_MODE_2},
    {PAD_TTL11,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_RGB8_MODE_3},
    {PAD_TTL11,        PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13|BIT12,            PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_TTL11,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_FUART_MODE_3},
    {PAD_TTL11,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT11|BIT8,             PINMUX_FOR_FUART_MODE_9},
    {PAD_TTL11,        PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT6|BIT4,              PINMUX_FOR_UART1_MODE_5},

//    {PAD_TTL12,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL12,        PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT10,                  PINMUX_FOR_EJ_MODE_4},
    {PAD_TTL12,        PADGPIO_BANK,     REG_TTL12_GPIO_MODE,            REG_TTL12_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_TTL12,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL12,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL12,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL12,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL12,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL12,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL12,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL12,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_7},
    {PAD_TTL12,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3,                   PINMUX_FOR_ETH0_MODE_8},
    {PAD_TTL12,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL12,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL12,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL12,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL12,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL12,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL12,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL12,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL12,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL12,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL12,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL12,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL12,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT0,                   PINMUX_FOR_RGB8_MODE_1},
    {PAD_TTL12,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1,                   PINMUX_FOR_RGB8_MODE_2},
    {PAD_TTL12,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_RGB8_MODE_3},
    {PAD_TTL12,        PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT12,                  PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_TTL12,        PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT13|BIT12,            PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_TTL12,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_FUART_MODE_3},
    {PAD_TTL12,        PADTOP_BANK,      REG_UART2_MODE,                 REG_UART2_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_UART2_MODE_3},

//    {PAD_TTL13,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL13,        PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT10,                  PINMUX_FOR_EJ_MODE_4},
    {PAD_TTL13,        PADGPIO_BANK,     REG_TTL13_GPIO_MODE,            REG_TTL13_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_TTL13,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL13,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL13,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL13,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL13,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL13,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL13,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL13,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_7},
    {PAD_TTL13,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3,                   PINMUX_FOR_ETH0_MODE_8},
    {PAD_TTL13,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL13,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL13,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL13,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL13,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL13,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL13,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL13,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL13,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL13,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL13,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL13,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL13,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT0,                   PINMUX_FOR_RGB8_MODE_1},
    {PAD_TTL13,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1,                   PINMUX_FOR_RGB8_MODE_2},
    {PAD_TTL13,        PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT12,                  PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_TTL13,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_FUART_MODE_3},
    {PAD_TTL13,        PADTOP_BANK,      REG_UART2_MODE,                 REG_UART2_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_UART2_MODE_3},
    {PAD_TTL13,        PADTOP_BANK,      REG_I2S_MCK_MODE,               REG_I2S_MCK_MODE_MASK,              BIT2,                   PINMUX_FOR_I2S_MCK_MODE_4},

//    {PAD_TTL14,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL14,        PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT10,                  PINMUX_FOR_EJ_MODE_4},
    {PAD_TTL14,        PADGPIO_BANK,     REG_TTL14_GPIO_MODE,            REG_TTL14_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_TTL14,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL14,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL14,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL14,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL14,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL14,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL14,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL14,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_7},
    {PAD_TTL14,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3,                   PINMUX_FOR_ETH0_MODE_8},
    {PAD_TTL14,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL14,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL14,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL14,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL14,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL14,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL14,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL14,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL14,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL14,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL14,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL14,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL14,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT0,                   PINMUX_FOR_RGB8_MODE_1},
    {PAD_TTL14,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1,                   PINMUX_FOR_RGB8_MODE_2},
    {PAD_TTL14,        PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT12,                  PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_TTL14,        PADTOP_BANK,      REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_PWM0_MODE_5},
    {PAD_TTL14,        PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT5|BIT4,              PINMUX_FOR_I2S_RX_MODE_3},
    {PAD_TTL14,        PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT9|BIT8,              PINMUX_FOR_I2S_TX_MODE_3},
    {PAD_TTL14,        PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13|BIT12,            PINMUX_FOR_I2S_RXTX_MODE_3},

//    {PAD_TTL15,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL15,        PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT10,                  PINMUX_FOR_EJ_MODE_4},
    {PAD_TTL15,        PADGPIO_BANK,     REG_TTL15_GPIO_MODE,            REG_TTL15_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_TTL15,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL15,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL15,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL15,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL15,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL15,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL15,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL15,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_7},
    {PAD_TTL15,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3,                   PINMUX_FOR_ETH0_MODE_8},
    {PAD_TTL15,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL15,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL15,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL15,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL15,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL15,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL15,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL15,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL15,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL15,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL15,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL15,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL15,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT0,                   PINMUX_FOR_RGB8_MODE_1},
    {PAD_TTL15,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1,                   PINMUX_FOR_RGB8_MODE_2},
    {PAD_TTL15,        PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT12,                  PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_TTL15,        PADTOP_BANK,      REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT6|BIT4,              PINMUX_FOR_PWM1_MODE_5},
    {PAD_TTL15,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT1,                   PINMUX_FOR_DMIC_MODE_2},
    {PAD_TTL15,        PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT5|BIT4,              PINMUX_FOR_I2S_RX_MODE_3},
    {PAD_TTL15,        PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT9|BIT8,              PINMUX_FOR_I2S_TX_MODE_3},
    {PAD_TTL15,        PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13|BIT12,            PINMUX_FOR_I2S_RXTX_MODE_3},

    {PAD_TTL16,        PADGPIO_BANK,     REG_TTL16_GPIO_MODE,            REG_TTL16_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_TTL16,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL16,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL16,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL16,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL16,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL16,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL16,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL16,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL16,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_7},
    {PAD_TTL16,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3,                   PINMUX_FOR_ETH0_MODE_8},
    {PAD_TTL16,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL16,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL16,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL16,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL16,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL16,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL16,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL16,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL16,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL16,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL16,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL16,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL16,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT0,                   PINMUX_FOR_RGB8_MODE_1},
    {PAD_TTL16,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1,                   PINMUX_FOR_RGB8_MODE_2},
    {PAD_TTL16,        PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT12,                  PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_TTL16,        PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2|BIT0,              PINMUX_FOR_PSPI1_PL_MODE_5},
    {PAD_TTL16,        PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10,                  PINMUX_FOR_SPI1_MODE_4},
    {PAD_TTL16,        PADTOP_BANK,      REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT10|BIT8,             PINMUX_FOR_PWM2_MODE_5},
    {PAD_TTL16,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT1,                   PINMUX_FOR_DMIC_MODE_2},
    {PAD_TTL16,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT3|BIT0,              PINMUX_FOR_DMIC_MODE_9},
    {PAD_TTL16,        PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT5|BIT4,              PINMUX_FOR_I2S_RX_MODE_3},
    {PAD_TTL16,        PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13|BIT12,            PINMUX_FOR_I2S_RXTX_MODE_3},

    {PAD_TTL17,        PADGPIO_BANK,     REG_TTL17_GPIO_MODE,            REG_TTL17_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_TTL17,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL17,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL17,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,                   PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_TTL17,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL17,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL17,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,                   PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_TTL17,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL17,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL17,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_7},
    {PAD_TTL17,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3,                   PINMUX_FOR_ETH0_MODE_8},
    {PAD_TTL17,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL17,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL17,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL17,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL17,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL17,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL17,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL17,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL17,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL17,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL17,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL17,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL17,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT0,                   PINMUX_FOR_RGB8_MODE_1},
    {PAD_TTL17,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1,                   PINMUX_FOR_RGB8_MODE_2},
    {PAD_TTL17,        PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT12,                  PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_TTL17,        PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2|BIT0,              PINMUX_FOR_PSPI1_PL_MODE_5},
    {PAD_TTL17,        PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10,                  PINMUX_FOR_SPI1_MODE_4},
    {PAD_TTL17,        PADTOP_BANK,      REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT14|BIT12,            PINMUX_FOR_PWM3_MODE_5},
    {PAD_TTL17,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT1,                   PINMUX_FOR_DMIC_MODE_2},
    {PAD_TTL17,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT3|BIT0,              PINMUX_FOR_DMIC_MODE_9},
    {PAD_TTL17,        PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT9|BIT8,              PINMUX_FOR_I2S_TX_MODE_3},
    {PAD_TTL17,        PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13|BIT12,            PINMUX_FOR_I2S_RXTX_MODE_3},

    {PAD_TTL18,        PADGPIO_BANK,     REG_TTL18_GPIO_MODE,            REG_TTL18_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_TTL18,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL18,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL18,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL18,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL18,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL18,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL18,        PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT3|BIT0,              PINMUX_FOR_I2C0_MODE_9},
    {PAD_TTL18,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_7},
    {PAD_TTL18,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3,                   PINMUX_FOR_ETH0_MODE_8},
    {PAD_TTL18,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL18,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL18,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL18,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL18,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL18,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL18,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL18,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL18,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL18,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL18,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL18,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL18,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT0,                   PINMUX_FOR_RGB8_MODE_1},
    {PAD_TTL18,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1,                   PINMUX_FOR_RGB8_MODE_2},
    {PAD_TTL18,        PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT12,                  PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_TTL18,        PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_SPI0_MODE_3},
    {PAD_TTL18,        PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2|BIT0,              PINMUX_FOR_PSPI1_PL_MODE_5},
    {PAD_TTL18,        PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10,                  PINMUX_FOR_SPI1_MODE_4},
    {PAD_TTL18,        PADTOP_BANK,      REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT2|BIT1,              PINMUX_FOR_PWM0_MODE_6},

    {PAD_TTL19,        PADGPIO_BANK,     REG_TTL19_GPIO_MODE,            REG_TTL19_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_TTL19,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL19,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL19,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_TTL19,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_TTL19,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_TTL19,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_TTL19,        PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT3|BIT0,              PINMUX_FOR_I2C0_MODE_9},
    {PAD_TTL19,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_7},
    {PAD_TTL19,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3,                   PINMUX_FOR_ETH0_MODE_8},
    {PAD_TTL19,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL19,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL19,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL19,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL19,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL19,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL19,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL19,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL19,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL19,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL19,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL19,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL19,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT0,                   PINMUX_FOR_RGB8_MODE_1},
    {PAD_TTL19,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1,                   PINMUX_FOR_RGB8_MODE_2},
    {PAD_TTL19,        PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT12,                  PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_TTL19,        PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_SPI0_MODE_3},
    {PAD_TTL19,        PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2|BIT0,              PINMUX_FOR_PSPI1_PL_MODE_5},
    {PAD_TTL19,        PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10,                  PINMUX_FOR_SPI1_MODE_4},
    {PAD_TTL19,        PADTOP_BANK,      REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT6|BIT5,              PINMUX_FOR_PWM1_MODE_6},

    {PAD_TTL20,        PADGPIO_BANK,     REG_TTL20_GPIO_MODE,            REG_TTL20_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_TTL20,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL20,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL20,        PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT6|BIT5|BIT4,         PINMUX_FOR_I2C1_MODE_7},
    {PAD_TTL20,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT2|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_7},
    {PAD_TTL20,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3,                   PINMUX_FOR_ETH0_MODE_8},
    {PAD_TTL20,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,                   PINMUX_FOR_TTL16_MODE_1},
    {PAD_TTL20,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,                   PINMUX_FOR_TTL16_MODE_2},
    {PAD_TTL20,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL20,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL20,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_TTL20,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_TTL20,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL20,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_TTL20,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_TTL20,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9|BIT8,              PINMUX_FOR_TTL24_MODE_3},
    {PAD_TTL20,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_TTL20,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_TTL20,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT0,                   PINMUX_FOR_RGB8_MODE_1},
    {PAD_TTL20,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1,                   PINMUX_FOR_RGB8_MODE_2},
    {PAD_TTL20,        PADTOP_BANK,      REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT12,                  PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_TTL20,        PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_SPI0_MODE_3},
    {PAD_TTL20,        PADTOP_BANK,      REG_PSPI1_CS2_MODE,             REG_PSPI1_CS2_MODE_MASK,            BIT6|BIT5,              PINMUX_FOR_PSPI1_CS2_MODE_6},
    {PAD_TTL20,        PADTOP_BANK,      REG_PSPI1_TE_MODE,              REG_PSPI1_TE_MODE_MASK,             BIT10|BIT9,             PINMUX_FOR_PSPI1_TE_MODE_6},
    {PAD_TTL20,        PADTOP_BANK,      REG_SPI1_CS2_MODE,              REG_SPI1_CS2_MODE_MASK,             BIT13,                  PINMUX_FOR_SPI1_CS2_MODE_2},
    {PAD_TTL20,        PADTOP_BANK,      REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT10|BIT9,             PINMUX_FOR_PWM2_MODE_6},

    {PAD_TTL21,        PADGPIO_BANK,     REG_TTL21_GPIO_MODE,            REG_TTL21_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_TTL21,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_TTL21,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_TTL21,        PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT6|BIT5|BIT4,         PINMUX_FOR_I2C1_MODE_7},
    {PAD_TTL21,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_TTL16_MODE_3},
    {PAD_TTL21,        PADTOP_BANK,      REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,                   PINMUX_FOR_TTL16_MODE_4},
    {PAD_TTL21,        PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5|BIT4,              PINMUX_FOR_TTL18_MODE_3},
    {PAD_TTL21,        PADTOP_BANK,      REG_RGB8_MODE,                  REG_RGB8_MODE_MASK,                 BIT1,                   PINMUX_FOR_RGB8_MODE_2},
    {PAD_TTL21,        PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_SPI0_MODE_3},
    {PAD_TTL21,        PADTOP_BANK,      REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT14|BIT13,            PINMUX_FOR_PWM3_MODE_6},

    {PAD_KEY0,         PADGPIO_BANK,     REG_KEY0_GPIO_MODE,             REG_KEY0_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_KEY0,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_KEY0,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT5,                   PINMUX_FOR_TTL18_MODE_2},
    {PAD_KEY0,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_KEY0,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_KEY0,         PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2|BIT1,              PINMUX_FOR_PSPI1_PL_MODE_6},
    {PAD_KEY0,         PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10|BIT8,             PINMUX_FOR_SPI1_MODE_5},
    {PAD_KEY0,         PADTOP_BANK,      REG_KEY_READ0_MODE,             REG_KEY_READ0_MODE_MASK,            BIT0,                   PINMUX_FOR_KEY_READ0_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_KEY_READ1_MODE,             REG_KEY_READ1_MODE_MASK,            BIT4,                   PINMUX_FOR_KEY_READ1_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_KEY_READ2_MODE,             REG_KEY_READ2_MODE_MASK,            BIT8,                   PINMUX_FOR_KEY_READ2_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_KEY_READ3_MODE,             REG_KEY_READ3_MODE_MASK,            BIT12,                  PINMUX_FOR_KEY_READ3_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_KEY_READ4_MODE,             REG_KEY_READ4_MODE_MASK,            BIT0,                   PINMUX_FOR_KEY_READ4_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_KEY_READ5_MODE,             REG_KEY_READ5_MODE_MASK,            BIT4,                   PINMUX_FOR_KEY_READ5_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_KEY_READ6_MODE,             REG_KEY_READ6_MODE_MASK,            BIT8,                   PINMUX_FOR_KEY_READ6_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_KEY_SCAN0_MODE,             REG_KEY_SCAN0_MODE_MASK,            BIT0,                   PINMUX_FOR_KEY_SCAN0_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_KEY_SCAN1_MODE,             REG_KEY_SCAN1_MODE_MASK,            BIT4,                   PINMUX_FOR_KEY_SCAN1_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_KEY_SCAN2_MODE,             REG_KEY_SCAN2_MODE_MASK,            BIT8,                   PINMUX_FOR_KEY_SCAN2_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_KEY_SCAN3_MODE,             REG_KEY_SCAN3_MODE_MASK,            BIT12,                  PINMUX_FOR_KEY_SCAN3_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_KEY_SCAN4_MODE,             REG_KEY_SCAN4_MODE_MASK,            BIT0,                   PINMUX_FOR_KEY_SCAN4_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_KEY_SCAN5_MODE,             REG_KEY_SCAN5_MODE_MASK,            BIT4,                   PINMUX_FOR_KEY_SCAN5_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_KEY_SCAN6_MODE,             REG_KEY_SCAN6_MODE_MASK,            BIT8,                   PINMUX_FOR_KEY_SCAN6_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_KEY_FIX_MODE,               REG_KEY_FIX_MODE_MASK,              BIT12,                  PINMUX_FOR_KEY_FIX_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT2|BIT1|BIT0,         PINMUX_FOR_PWM0_MODE_7},
    {PAD_KEY0,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT0,                   PINMUX_FOR_BT1120_MODE_1},
    {PAD_KEY0,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT1,                   PINMUX_FOR_BT1120_MODE_2},

    {PAD_KEY1,         PADGPIO_BANK,     REG_KEY1_GPIO_MODE,             REG_KEY1_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_KEY1,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_KEY1,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_KEY1,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT0,              PINMUX_FOR_ETH0_MODE_9},
    {PAD_KEY1,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1,              PINMUX_FOR_ETH0_MODE_10},
    {PAD_KEY1,         PADTOP_BANK,      REG_TTL18_MODE,                 REG_TTL18_MODE_MASK,                BIT4,                   PINMUX_FOR_TTL18_MODE_1},
    {PAD_KEY1,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_KEY1,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_KEY1,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_KEY1,         PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2|BIT1,              PINMUX_FOR_PSPI1_PL_MODE_6},
    {PAD_KEY1,         PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10|BIT8,             PINMUX_FOR_SPI1_MODE_5},
    {PAD_KEY1,         PADTOP_BANK,      REG_KEY_READ0_MODE,             REG_KEY_READ0_MODE_MASK,            BIT1,                   PINMUX_FOR_KEY_READ0_MODE_2},
    {PAD_KEY1,         PADTOP_BANK,      REG_KEY_READ1_MODE,             REG_KEY_READ1_MODE_MASK,            BIT5,                   PINMUX_FOR_KEY_READ1_MODE_2},
    {PAD_KEY1,         PADTOP_BANK,      REG_KEY_READ2_MODE,             REG_KEY_READ2_MODE_MASK,            BIT9,                   PINMUX_FOR_KEY_READ2_MODE_2},
    {PAD_KEY1,         PADTOP_BANK,      REG_KEY_READ3_MODE,             REG_KEY_READ3_MODE_MASK,            BIT13,                  PINMUX_FOR_KEY_READ3_MODE_2},
    {PAD_KEY1,         PADTOP_BANK,      REG_KEY_READ4_MODE,             REG_KEY_READ4_MODE_MASK,            BIT1,                   PINMUX_FOR_KEY_READ4_MODE_2},
    {PAD_KEY1,         PADTOP_BANK,      REG_KEY_READ5_MODE,             REG_KEY_READ5_MODE_MASK,            BIT5,                   PINMUX_FOR_KEY_READ5_MODE_2},
    {PAD_KEY1,         PADTOP_BANK,      REG_KEY_READ6_MODE,             REG_KEY_READ6_MODE_MASK,            BIT9,                   PINMUX_FOR_KEY_READ6_MODE_2},
    {PAD_KEY1,         PADTOP_BANK,      REG_KEY_SCAN0_MODE,             REG_KEY_SCAN0_MODE_MASK,            BIT1,                   PINMUX_FOR_KEY_SCAN0_MODE_2},
    {PAD_KEY1,         PADTOP_BANK,      REG_KEY_SCAN1_MODE,             REG_KEY_SCAN1_MODE_MASK,            BIT5,                   PINMUX_FOR_KEY_SCAN1_MODE_2},
    {PAD_KEY1,         PADTOP_BANK,      REG_KEY_SCAN2_MODE,             REG_KEY_SCAN2_MODE_MASK,            BIT9,                   PINMUX_FOR_KEY_SCAN2_MODE_2},
    {PAD_KEY1,         PADTOP_BANK,      REG_KEY_SCAN3_MODE,             REG_KEY_SCAN3_MODE_MASK,            BIT13,                  PINMUX_FOR_KEY_SCAN3_MODE_2},
    {PAD_KEY1,         PADTOP_BANK,      REG_KEY_SCAN4_MODE,             REG_KEY_SCAN4_MODE_MASK,            BIT1,                   PINMUX_FOR_KEY_SCAN4_MODE_2},
    {PAD_KEY1,         PADTOP_BANK,      REG_KEY_SCAN5_MODE,             REG_KEY_SCAN5_MODE_MASK,            BIT5,                   PINMUX_FOR_KEY_SCAN5_MODE_2},
    {PAD_KEY1,         PADTOP_BANK,      REG_KEY_SCAN6_MODE,             REG_KEY_SCAN6_MODE_MASK,            BIT9,                   PINMUX_FOR_KEY_SCAN6_MODE_2},
    {PAD_KEY1,         PADTOP_BANK,      REG_KEY_FIX_MODE,               REG_KEY_FIX_MODE_MASK,              BIT12,                  PINMUX_FOR_KEY_FIX_MODE_1},
    {PAD_KEY1,         PADTOP_BANK,      REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT6|BIT5|BIT4,         PINMUX_FOR_PWM1_MODE_7},
    {PAD_KEY1,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT0,                   PINMUX_FOR_BT1120_MODE_1},
    {PAD_KEY1,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT1,                   PINMUX_FOR_BT1120_MODE_2},

    {PAD_KEY2,         PADGPIO_BANK,     REG_KEY2_GPIO_MODE,             REG_KEY2_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_KEY2,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_KEY2,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_KEY2,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT0,              PINMUX_FOR_ETH0_MODE_9},
    {PAD_KEY2,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1,              PINMUX_FOR_ETH0_MODE_10},
    {PAD_KEY2,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_KEY2,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_KEY2,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_KEY2,         PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2|BIT1,              PINMUX_FOR_PSPI1_PL_MODE_6},
    {PAD_KEY2,         PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10|BIT8,             PINMUX_FOR_SPI1_MODE_5},
    {PAD_KEY2,         PADTOP_BANK,      REG_KEY_READ0_MODE,             REG_KEY_READ0_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_KEY_READ0_MODE_3},
    {PAD_KEY2,         PADTOP_BANK,      REG_KEY_READ1_MODE,             REG_KEY_READ1_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_KEY_READ1_MODE_3},
    {PAD_KEY2,         PADTOP_BANK,      REG_KEY_READ2_MODE,             REG_KEY_READ2_MODE_MASK,            BIT9|BIT8,              PINMUX_FOR_KEY_READ2_MODE_3},
    {PAD_KEY2,         PADTOP_BANK,      REG_KEY_READ3_MODE,             REG_KEY_READ3_MODE_MASK,            BIT13|BIT12,            PINMUX_FOR_KEY_READ3_MODE_3},
    {PAD_KEY2,         PADTOP_BANK,      REG_KEY_READ4_MODE,             REG_KEY_READ4_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_KEY_READ4_MODE_3},
    {PAD_KEY2,         PADTOP_BANK,      REG_KEY_READ5_MODE,             REG_KEY_READ5_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_KEY_READ5_MODE_3},
    {PAD_KEY2,         PADTOP_BANK,      REG_KEY_READ6_MODE,             REG_KEY_READ6_MODE_MASK,            BIT9|BIT8,              PINMUX_FOR_KEY_READ6_MODE_3},
    {PAD_KEY2,         PADTOP_BANK,      REG_KEY_SCAN0_MODE,             REG_KEY_SCAN0_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_KEY_SCAN0_MODE_3},
    {PAD_KEY2,         PADTOP_BANK,      REG_KEY_SCAN1_MODE,             REG_KEY_SCAN1_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_KEY_SCAN1_MODE_3},
    {PAD_KEY2,         PADTOP_BANK,      REG_KEY_SCAN2_MODE,             REG_KEY_SCAN2_MODE_MASK,            BIT9|BIT8,              PINMUX_FOR_KEY_SCAN2_MODE_3},
    {PAD_KEY2,         PADTOP_BANK,      REG_KEY_SCAN3_MODE,             REG_KEY_SCAN3_MODE_MASK,            BIT13|BIT12,            PINMUX_FOR_KEY_SCAN3_MODE_3},
    {PAD_KEY2,         PADTOP_BANK,      REG_KEY_SCAN4_MODE,             REG_KEY_SCAN4_MODE_MASK,            BIT1|BIT0,              PINMUX_FOR_KEY_SCAN4_MODE_3},
    {PAD_KEY2,         PADTOP_BANK,      REG_KEY_SCAN5_MODE,             REG_KEY_SCAN5_MODE_MASK,            BIT5|BIT4,              PINMUX_FOR_KEY_SCAN5_MODE_3},
    {PAD_KEY2,         PADTOP_BANK,      REG_KEY_SCAN6_MODE,             REG_KEY_SCAN6_MODE_MASK,            BIT9|BIT8,              PINMUX_FOR_KEY_SCAN6_MODE_3},
    {PAD_KEY2,         PADTOP_BANK,      REG_KEY_FIX_MODE,               REG_KEY_FIX_MODE_MASK,              BIT12,                  PINMUX_FOR_KEY_FIX_MODE_1},
    {PAD_KEY2,         PADTOP_BANK,      REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT10|BIT9|BIT8,        PINMUX_FOR_PWM2_MODE_7},
    {PAD_KEY2,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT0,                   PINMUX_FOR_BT1120_MODE_1},
    {PAD_KEY2,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT1,                   PINMUX_FOR_BT1120_MODE_2},

    {PAD_KEY3,         PADGPIO_BANK,     REG_KEY3_GPIO_MODE,             REG_KEY3_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_KEY3,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_KEY3,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_KEY3,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT0,              PINMUX_FOR_ETH0_MODE_9},
    {PAD_KEY3,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1,              PINMUX_FOR_ETH0_MODE_10},
    {PAD_KEY3,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_KEY3,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_KEY3,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_KEY3,         PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2|BIT1,              PINMUX_FOR_PSPI1_PL_MODE_6},
    {PAD_KEY3,         PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10|BIT8,             PINMUX_FOR_SPI1_MODE_5},
    {PAD_KEY3,         PADTOP_BANK,      REG_KEY_READ0_MODE,             REG_KEY_READ0_MODE_MASK,            BIT2,                   PINMUX_FOR_KEY_READ0_MODE_4},
    {PAD_KEY3,         PADTOP_BANK,      REG_KEY_READ1_MODE,             REG_KEY_READ1_MODE_MASK,            BIT6,                   PINMUX_FOR_KEY_READ1_MODE_4},
    {PAD_KEY3,         PADTOP_BANK,      REG_KEY_READ2_MODE,             REG_KEY_READ2_MODE_MASK,            BIT10,                  PINMUX_FOR_KEY_READ2_MODE_4},
    {PAD_KEY3,         PADTOP_BANK,      REG_KEY_READ3_MODE,             REG_KEY_READ3_MODE_MASK,            BIT14,                  PINMUX_FOR_KEY_READ3_MODE_4},
    {PAD_KEY3,         PADTOP_BANK,      REG_KEY_READ4_MODE,             REG_KEY_READ4_MODE_MASK,            BIT2,                   PINMUX_FOR_KEY_READ4_MODE_4},
    {PAD_KEY3,         PADTOP_BANK,      REG_KEY_READ5_MODE,             REG_KEY_READ5_MODE_MASK,            BIT6,                   PINMUX_FOR_KEY_READ5_MODE_4},
    {PAD_KEY3,         PADTOP_BANK,      REG_KEY_READ6_MODE,             REG_KEY_READ6_MODE_MASK,            BIT10,                  PINMUX_FOR_KEY_READ6_MODE_4},
    {PAD_KEY3,         PADTOP_BANK,      REG_KEY_SCAN0_MODE,             REG_KEY_SCAN0_MODE_MASK,            BIT2,                   PINMUX_FOR_KEY_SCAN0_MODE_4},
    {PAD_KEY3,         PADTOP_BANK,      REG_KEY_SCAN1_MODE,             REG_KEY_SCAN1_MODE_MASK,            BIT6,                   PINMUX_FOR_KEY_SCAN1_MODE_4},
    {PAD_KEY3,         PADTOP_BANK,      REG_KEY_SCAN2_MODE,             REG_KEY_SCAN2_MODE_MASK,            BIT10,                  PINMUX_FOR_KEY_SCAN2_MODE_4},
    {PAD_KEY3,         PADTOP_BANK,      REG_KEY_SCAN3_MODE,             REG_KEY_SCAN3_MODE_MASK,            BIT14,                  PINMUX_FOR_KEY_SCAN3_MODE_4},
    {PAD_KEY3,         PADTOP_BANK,      REG_KEY_SCAN4_MODE,             REG_KEY_SCAN4_MODE_MASK,            BIT2,                   PINMUX_FOR_KEY_SCAN4_MODE_4},
    {PAD_KEY3,         PADTOP_BANK,      REG_KEY_SCAN5_MODE,             REG_KEY_SCAN5_MODE_MASK,            BIT6,                   PINMUX_FOR_KEY_SCAN5_MODE_4},
    {PAD_KEY3,         PADTOP_BANK,      REG_KEY_SCAN6_MODE,             REG_KEY_SCAN6_MODE_MASK,            BIT10,                  PINMUX_FOR_KEY_SCAN6_MODE_4},
    {PAD_KEY3,         PADTOP_BANK,      REG_KEY_FIX_MODE,               REG_KEY_FIX_MODE_MASK,              BIT12,                  PINMUX_FOR_KEY_FIX_MODE_1},
    {PAD_KEY3,         PADTOP_BANK,      REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT14|BIT13|BIT12,      PINMUX_FOR_PWM3_MODE_7},
    {PAD_KEY3,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT0,                   PINMUX_FOR_BT1120_MODE_1},
    {PAD_KEY3,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT1,                   PINMUX_FOR_BT1120_MODE_2},

    {PAD_KEY4,         PADGPIO_BANK,     REG_KEY4_GPIO_MODE,             REG_KEY4_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_KEY4,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_KEY4,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_KEY4,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT0,              PINMUX_FOR_ETH0_MODE_9},
    {PAD_KEY4,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1,              PINMUX_FOR_ETH0_MODE_10},
    {PAD_KEY4,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_KEY4,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_KEY4,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_KEY4,         PADTOP_BANK,      REG_PSPI1_CS2_MODE,             REG_PSPI1_CS2_MODE_MASK,            BIT6|BIT5|BIT4,         PINMUX_FOR_PSPI1_CS2_MODE_7},
    {PAD_KEY4,         PADTOP_BANK,      REG_PSPI1_TE_MODE,              REG_PSPI1_TE_MODE_MASK,             BIT10|BIT9|BIT8,        PINMUX_FOR_PSPI1_TE_MODE_7},
    {PAD_KEY4,         PADTOP_BANK,      REG_SPI1_CS2_MODE,              REG_SPI1_CS2_MODE_MASK,             BIT13|BIT12,            PINMUX_FOR_SPI1_CS2_MODE_3},
    {PAD_KEY4,         PADTOP_BANK,      REG_KEY_READ0_MODE,             REG_KEY_READ0_MODE_MASK,            BIT2|BIT0,              PINMUX_FOR_KEY_READ0_MODE_5},
    {PAD_KEY4,         PADTOP_BANK,      REG_KEY_READ1_MODE,             REG_KEY_READ1_MODE_MASK,            BIT6|BIT4,              PINMUX_FOR_KEY_READ1_MODE_5},
    {PAD_KEY4,         PADTOP_BANK,      REG_KEY_READ2_MODE,             REG_KEY_READ2_MODE_MASK,            BIT10|BIT8,             PINMUX_FOR_KEY_READ2_MODE_5},
    {PAD_KEY4,         PADTOP_BANK,      REG_KEY_READ3_MODE,             REG_KEY_READ3_MODE_MASK,            BIT14|BIT12,            PINMUX_FOR_KEY_READ3_MODE_5},
    {PAD_KEY4,         PADTOP_BANK,      REG_KEY_READ4_MODE,             REG_KEY_READ4_MODE_MASK,            BIT2|BIT0,              PINMUX_FOR_KEY_READ4_MODE_5},
    {PAD_KEY4,         PADTOP_BANK,      REG_KEY_READ5_MODE,             REG_KEY_READ5_MODE_MASK,            BIT6|BIT4,              PINMUX_FOR_KEY_READ5_MODE_5},
    {PAD_KEY4,         PADTOP_BANK,      REG_KEY_READ6_MODE,             REG_KEY_READ6_MODE_MASK,            BIT10|BIT8,             PINMUX_FOR_KEY_READ6_MODE_5},
    {PAD_KEY4,         PADTOP_BANK,      REG_KEY_SCAN0_MODE,             REG_KEY_SCAN0_MODE_MASK,            BIT2|BIT0,              PINMUX_FOR_KEY_SCAN0_MODE_5},
    {PAD_KEY4,         PADTOP_BANK,      REG_KEY_SCAN1_MODE,             REG_KEY_SCAN1_MODE_MASK,            BIT6|BIT4,              PINMUX_FOR_KEY_SCAN1_MODE_5},
    {PAD_KEY4,         PADTOP_BANK,      REG_KEY_SCAN2_MODE,             REG_KEY_SCAN2_MODE_MASK,            BIT10|BIT8,             PINMUX_FOR_KEY_SCAN2_MODE_5},
    {PAD_KEY4,         PADTOP_BANK,      REG_KEY_SCAN3_MODE,             REG_KEY_SCAN3_MODE_MASK,            BIT14|BIT12,            PINMUX_FOR_KEY_SCAN3_MODE_5},
    {PAD_KEY4,         PADTOP_BANK,      REG_KEY_SCAN4_MODE,             REG_KEY_SCAN4_MODE_MASK,            BIT2|BIT0,              PINMUX_FOR_KEY_SCAN4_MODE_5},
    {PAD_KEY4,         PADTOP_BANK,      REG_KEY_SCAN5_MODE,             REG_KEY_SCAN5_MODE_MASK,            BIT6|BIT4,              PINMUX_FOR_KEY_SCAN5_MODE_5},
    {PAD_KEY4,         PADTOP_BANK,      REG_KEY_SCAN6_MODE,             REG_KEY_SCAN6_MODE_MASK,            BIT10|BIT8,             PINMUX_FOR_KEY_SCAN6_MODE_5},
    {PAD_KEY4,         PADTOP_BANK,      REG_KEY_FIX_MODE,               REG_KEY_FIX_MODE_MASK,              BIT12,                  PINMUX_FOR_KEY_FIX_MODE_1},
    {PAD_KEY4,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT0,                   PINMUX_FOR_BT1120_MODE_1},
    {PAD_KEY4,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT1,                   PINMUX_FOR_BT1120_MODE_2},

    {PAD_KEY5,         PADGPIO_BANK,     REG_KEY5_GPIO_MODE,             REG_KEY5_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_KEY5,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_KEY5,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_KEY5,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT0,              PINMUX_FOR_ETH0_MODE_9},
    {PAD_KEY5,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1,              PINMUX_FOR_ETH0_MODE_10},
    {PAD_KEY5,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_KEY5,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_KEY5,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_KEY5,         PADTOP_BANK,      REG_KEY_READ0_MODE,             REG_KEY_READ0_MODE_MASK,            BIT2|BIT1,              PINMUX_FOR_KEY_READ0_MODE_6},
    {PAD_KEY5,         PADTOP_BANK,      REG_KEY_READ1_MODE,             REG_KEY_READ1_MODE_MASK,            BIT6|BIT5,              PINMUX_FOR_KEY_READ1_MODE_6},
    {PAD_KEY5,         PADTOP_BANK,      REG_KEY_READ2_MODE,             REG_KEY_READ2_MODE_MASK,            BIT10|BIT9,             PINMUX_FOR_KEY_READ2_MODE_6},
    {PAD_KEY5,         PADTOP_BANK,      REG_KEY_READ3_MODE,             REG_KEY_READ3_MODE_MASK,            BIT14|BIT13,            PINMUX_FOR_KEY_READ3_MODE_6},
    {PAD_KEY5,         PADTOP_BANK,      REG_KEY_READ4_MODE,             REG_KEY_READ4_MODE_MASK,            BIT2|BIT1,              PINMUX_FOR_KEY_READ4_MODE_6},
    {PAD_KEY5,         PADTOP_BANK,      REG_KEY_READ5_MODE,             REG_KEY_READ5_MODE_MASK,            BIT6|BIT5,              PINMUX_FOR_KEY_READ5_MODE_6},
    {PAD_KEY5,         PADTOP_BANK,      REG_KEY_READ6_MODE,             REG_KEY_READ6_MODE_MASK,            BIT10|BIT9,             PINMUX_FOR_KEY_READ6_MODE_6},
    {PAD_KEY5,         PADTOP_BANK,      REG_KEY_SCAN0_MODE,             REG_KEY_SCAN0_MODE_MASK,            BIT2|BIT1,              PINMUX_FOR_KEY_SCAN0_MODE_6},
    {PAD_KEY5,         PADTOP_BANK,      REG_KEY_SCAN1_MODE,             REG_KEY_SCAN1_MODE_MASK,            BIT6|BIT5,              PINMUX_FOR_KEY_SCAN1_MODE_6},
    {PAD_KEY5,         PADTOP_BANK,      REG_KEY_SCAN2_MODE,             REG_KEY_SCAN2_MODE_MASK,            BIT10|BIT9,             PINMUX_FOR_KEY_SCAN2_MODE_6},
    {PAD_KEY5,         PADTOP_BANK,      REG_KEY_SCAN3_MODE,             REG_KEY_SCAN3_MODE_MASK,            BIT14|BIT13,            PINMUX_FOR_KEY_SCAN3_MODE_6},
    {PAD_KEY5,         PADTOP_BANK,      REG_KEY_SCAN4_MODE,             REG_KEY_SCAN4_MODE_MASK,            BIT2|BIT1,              PINMUX_FOR_KEY_SCAN4_MODE_6},
    {PAD_KEY5,         PADTOP_BANK,      REG_KEY_SCAN5_MODE,             REG_KEY_SCAN5_MODE_MASK,            BIT6|BIT5,              PINMUX_FOR_KEY_SCAN5_MODE_6},
    {PAD_KEY5,         PADTOP_BANK,      REG_KEY_SCAN6_MODE,             REG_KEY_SCAN6_MODE_MASK,            BIT10|BIT9,             PINMUX_FOR_KEY_SCAN6_MODE_6},
    {PAD_KEY5,         PADTOP_BANK,      REG_KEY_FIX_MODE,               REG_KEY_FIX_MODE_MASK,              BIT12,                  PINMUX_FOR_KEY_FIX_MODE_1},
    {PAD_KEY5,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT0,                   PINMUX_FOR_BT1120_MODE_1},
    {PAD_KEY5,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT1,                   PINMUX_FOR_BT1120_MODE_2},

    {PAD_KEY6,         PADGPIO_BANK,     REG_KEY6_GPIO_MODE,             REG_KEY6_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_KEY6,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_KEY6,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_KEY6,         PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT3|BIT1,              PINMUX_FOR_I2C0_MODE_10},
    {PAD_KEY6,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT0,              PINMUX_FOR_ETH0_MODE_9},
    {PAD_KEY6,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1,              PINMUX_FOR_ETH0_MODE_10},
    {PAD_KEY6,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_KEY6,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_KEY6,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_KEY6,         PADTOP_BANK,      REG_KEY_READ0_MODE,             REG_KEY_READ0_MODE_MASK,            BIT2|BIT1|BIT0,         PINMUX_FOR_KEY_READ0_MODE_7},
    {PAD_KEY6,         PADTOP_BANK,      REG_KEY_READ1_MODE,             REG_KEY_READ1_MODE_MASK,            BIT6|BIT5|BIT4,         PINMUX_FOR_KEY_READ1_MODE_7},
    {PAD_KEY6,         PADTOP_BANK,      REG_KEY_READ2_MODE,             REG_KEY_READ2_MODE_MASK,            BIT10|BIT9|BIT8,        PINMUX_FOR_KEY_READ2_MODE_7},
    {PAD_KEY6,         PADTOP_BANK,      REG_KEY_READ3_MODE,             REG_KEY_READ3_MODE_MASK,            BIT14|BIT13|BIT12,      PINMUX_FOR_KEY_READ3_MODE_7},
    {PAD_KEY6,         PADTOP_BANK,      REG_KEY_READ4_MODE,             REG_KEY_READ4_MODE_MASK,            BIT2|BIT1|BIT0,         PINMUX_FOR_KEY_READ4_MODE_7},
    {PAD_KEY6,         PADTOP_BANK,      REG_KEY_READ5_MODE,             REG_KEY_READ5_MODE_MASK,            BIT6|BIT5|BIT4,         PINMUX_FOR_KEY_READ5_MODE_7},
    {PAD_KEY6,         PADTOP_BANK,      REG_KEY_READ6_MODE,             REG_KEY_READ6_MODE_MASK,            BIT10|BIT9|BIT8,        PINMUX_FOR_KEY_READ6_MODE_7},
    {PAD_KEY6,         PADTOP_BANK,      REG_KEY_SCAN0_MODE,             REG_KEY_SCAN0_MODE_MASK,            BIT2|BIT1|BIT0,         PINMUX_FOR_KEY_SCAN0_MODE_7},
    {PAD_KEY6,         PADTOP_BANK,      REG_KEY_SCAN1_MODE,             REG_KEY_SCAN1_MODE_MASK,            BIT6|BIT5|BIT4,         PINMUX_FOR_KEY_SCAN1_MODE_7},
    {PAD_KEY6,         PADTOP_BANK,      REG_KEY_SCAN2_MODE,             REG_KEY_SCAN2_MODE_MASK,            BIT10|BIT9|BIT8,        PINMUX_FOR_KEY_SCAN2_MODE_7},
    {PAD_KEY6,         PADTOP_BANK,      REG_KEY_SCAN3_MODE,             REG_KEY_SCAN3_MODE_MASK,            BIT14|BIT13|BIT12,      PINMUX_FOR_KEY_SCAN3_MODE_7},
    {PAD_KEY6,         PADTOP_BANK,      REG_KEY_SCAN4_MODE,             REG_KEY_SCAN4_MODE_MASK,            BIT2|BIT1|BIT0,         PINMUX_FOR_KEY_SCAN4_MODE_7},
    {PAD_KEY6,         PADTOP_BANK,      REG_KEY_SCAN5_MODE,             REG_KEY_SCAN5_MODE_MASK,            BIT6|BIT5|BIT4,         PINMUX_FOR_KEY_SCAN5_MODE_7},
    {PAD_KEY6,         PADTOP_BANK,      REG_KEY_SCAN6_MODE,             REG_KEY_SCAN6_MODE_MASK,            BIT10|BIT9|BIT8,        PINMUX_FOR_KEY_SCAN6_MODE_7},
    {PAD_KEY6,         PADTOP_BANK,      REG_KEY_FIX_MODE,               REG_KEY_FIX_MODE_MASK,              BIT12,                  PINMUX_FOR_KEY_FIX_MODE_1},
    {PAD_KEY6,         PADTOP_BANK,      REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT1,                   PINMUX_FOR_UART0_MODE_2},
    {PAD_KEY6,         PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT6|BIT5,              PINMUX_FOR_UART1_MODE_6},
    {PAD_KEY6,         PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_DMIC_MODE_3},
    {PAD_KEY6,         PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT6,                   PINMUX_FOR_I2S_RX_MODE_4},
    {PAD_KEY6,         PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT14,                  PINMUX_FOR_I2S_RXTX_MODE_4},
    {PAD_KEY6,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT0,                   PINMUX_FOR_BT1120_MODE_1},
    {PAD_KEY6,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT1,                   PINMUX_FOR_BT1120_MODE_2},

    {PAD_KEY7,         PADGPIO_BANK,     REG_KEY7_GPIO_MODE,             REG_KEY7_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_KEY7,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_KEY7,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_KEY7,         PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT3|BIT1,              PINMUX_FOR_I2C0_MODE_10},
    {PAD_KEY7,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT0,              PINMUX_FOR_ETH0_MODE_9},
    {PAD_KEY7,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1,              PINMUX_FOR_ETH0_MODE_10},
    {PAD_KEY7,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT8,                   PINMUX_FOR_TTL24_MODE_1},
    {PAD_KEY7,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10,                  PINMUX_FOR_TTL24_MODE_4},
    {PAD_KEY7,         PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_TTL24_MODE_5},
    {PAD_KEY7,         PADTOP_BANK,      REG_KEY_READ0_MODE,             REG_KEY_READ0_MODE_MASK,            BIT3,                   PINMUX_FOR_KEY_READ0_MODE_8},
    {PAD_KEY7,         PADTOP_BANK,      REG_KEY_READ1_MODE,             REG_KEY_READ1_MODE_MASK,            BIT7,                   PINMUX_FOR_KEY_READ1_MODE_8},
    {PAD_KEY7,         PADTOP_BANK,      REG_KEY_READ2_MODE,             REG_KEY_READ2_MODE_MASK,            BIT11,                  PINMUX_FOR_KEY_READ2_MODE_8},
    {PAD_KEY7,         PADTOP_BANK,      REG_KEY_READ3_MODE,             REG_KEY_READ3_MODE_MASK,            BIT15,                  PINMUX_FOR_KEY_READ3_MODE_8},
    {PAD_KEY7,         PADTOP_BANK,      REG_KEY_READ4_MODE,             REG_KEY_READ4_MODE_MASK,            BIT3,                   PINMUX_FOR_KEY_READ4_MODE_8},
    {PAD_KEY7,         PADTOP_BANK,      REG_KEY_READ5_MODE,             REG_KEY_READ5_MODE_MASK,            BIT7,                   PINMUX_FOR_KEY_READ5_MODE_8},
    {PAD_KEY7,         PADTOP_BANK,      REG_KEY_READ6_MODE,             REG_KEY_READ6_MODE_MASK,            BIT11,                  PINMUX_FOR_KEY_READ6_MODE_8},
    {PAD_KEY7,         PADTOP_BANK,      REG_KEY_SCAN0_MODE,             REG_KEY_SCAN0_MODE_MASK,            BIT3,                   PINMUX_FOR_KEY_SCAN0_MODE_8},
    {PAD_KEY7,         PADTOP_BANK,      REG_KEY_SCAN1_MODE,             REG_KEY_SCAN1_MODE_MASK,            BIT7,                   PINMUX_FOR_KEY_SCAN1_MODE_8},
    {PAD_KEY7,         PADTOP_BANK,      REG_KEY_SCAN2_MODE,             REG_KEY_SCAN2_MODE_MASK,            BIT11,                  PINMUX_FOR_KEY_SCAN2_MODE_8},
    {PAD_KEY7,         PADTOP_BANK,      REG_KEY_SCAN3_MODE,             REG_KEY_SCAN3_MODE_MASK,            BIT15,                  PINMUX_FOR_KEY_SCAN3_MODE_8},
    {PAD_KEY7,         PADTOP_BANK,      REG_KEY_SCAN4_MODE,             REG_KEY_SCAN4_MODE_MASK,            BIT3,                   PINMUX_FOR_KEY_SCAN4_MODE_8},
    {PAD_KEY7,         PADTOP_BANK,      REG_KEY_SCAN5_MODE,             REG_KEY_SCAN5_MODE_MASK,            BIT7,                   PINMUX_FOR_KEY_SCAN5_MODE_8},
    {PAD_KEY7,         PADTOP_BANK,      REG_KEY_SCAN6_MODE,             REG_KEY_SCAN6_MODE_MASK,            BIT11,                  PINMUX_FOR_KEY_SCAN6_MODE_8},
    {PAD_KEY7,         PADTOP_BANK,      REG_KEY_FIX_MODE,               REG_KEY_FIX_MODE_MASK,              BIT12,                  PINMUX_FOR_KEY_FIX_MODE_1},
    {PAD_KEY7,         PADTOP_BANK,      REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT1,                   PINMUX_FOR_UART0_MODE_2},
    {PAD_KEY7,         PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT6|BIT5,              PINMUX_FOR_UART1_MODE_6},
    {PAD_KEY7,         PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_DMIC_MODE_3},
    {PAD_KEY7,         PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT3|BIT1,              PINMUX_FOR_DMIC_MODE_10},
    {PAD_KEY7,         PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT6,                   PINMUX_FOR_I2S_RX_MODE_4},
    {PAD_KEY7,         PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT14,                  PINMUX_FOR_I2S_RXTX_MODE_4},
    {PAD_KEY7,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT0,                   PINMUX_FOR_BT1120_MODE_1},
    {PAD_KEY7,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT1,                   PINMUX_FOR_BT1120_MODE_2},

    {PAD_KEY8,         PADGPIO_BANK,     REG_KEY8_GPIO_MODE,             REG_KEY8_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_KEY8,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_KEY8,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_KEY8,         PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT7,                   PINMUX_FOR_I2C1_MODE_8},
    {PAD_KEY8,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT0,              PINMUX_FOR_ETH0_MODE_9},
    {PAD_KEY8,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1,              PINMUX_FOR_ETH0_MODE_10},
    {PAD_KEY8,         PADTOP_BANK,      REG_KEY_READ0_MODE,             REG_KEY_READ0_MODE_MASK,            BIT3|BIT0,              PINMUX_FOR_KEY_READ0_MODE_9},
    {PAD_KEY8,         PADTOP_BANK,      REG_KEY_READ1_MODE,             REG_KEY_READ1_MODE_MASK,            BIT7|BIT4,              PINMUX_FOR_KEY_READ1_MODE_9},
    {PAD_KEY8,         PADTOP_BANK,      REG_KEY_READ2_MODE,             REG_KEY_READ2_MODE_MASK,            BIT11|BIT8,             PINMUX_FOR_KEY_READ2_MODE_9},
    {PAD_KEY8,         PADTOP_BANK,      REG_KEY_READ3_MODE,             REG_KEY_READ3_MODE_MASK,            BIT15|BIT12,            PINMUX_FOR_KEY_READ3_MODE_9},
    {PAD_KEY8,         PADTOP_BANK,      REG_KEY_READ4_MODE,             REG_KEY_READ4_MODE_MASK,            BIT3|BIT0,              PINMUX_FOR_KEY_READ4_MODE_9},
    {PAD_KEY8,         PADTOP_BANK,      REG_KEY_READ5_MODE,             REG_KEY_READ5_MODE_MASK,            BIT7|BIT4,              PINMUX_FOR_KEY_READ5_MODE_9},
    {PAD_KEY8,         PADTOP_BANK,      REG_KEY_READ6_MODE,             REG_KEY_READ6_MODE_MASK,            BIT11|BIT8,             PINMUX_FOR_KEY_READ6_MODE_9},
    {PAD_KEY8,         PADTOP_BANK,      REG_KEY_SCAN0_MODE,             REG_KEY_SCAN0_MODE_MASK,            BIT3|BIT0,              PINMUX_FOR_KEY_SCAN0_MODE_9},
    {PAD_KEY8,         PADTOP_BANK,      REG_KEY_SCAN1_MODE,             REG_KEY_SCAN1_MODE_MASK,            BIT7|BIT4,              PINMUX_FOR_KEY_SCAN1_MODE_9},
    {PAD_KEY8,         PADTOP_BANK,      REG_KEY_SCAN2_MODE,             REG_KEY_SCAN2_MODE_MASK,            BIT11|BIT8,             PINMUX_FOR_KEY_SCAN2_MODE_9},
    {PAD_KEY8,         PADTOP_BANK,      REG_KEY_SCAN3_MODE,             REG_KEY_SCAN3_MODE_MASK,            BIT15|BIT12,            PINMUX_FOR_KEY_SCAN3_MODE_9},
    {PAD_KEY8,         PADTOP_BANK,      REG_KEY_SCAN4_MODE,             REG_KEY_SCAN4_MODE_MASK,            BIT3|BIT0,              PINMUX_FOR_KEY_SCAN4_MODE_9},
    {PAD_KEY8,         PADTOP_BANK,      REG_KEY_SCAN5_MODE,             REG_KEY_SCAN5_MODE_MASK,            BIT7|BIT4,              PINMUX_FOR_KEY_SCAN5_MODE_9},
    {PAD_KEY8,         PADTOP_BANK,      REG_KEY_SCAN6_MODE,             REG_KEY_SCAN6_MODE_MASK,            BIT11|BIT8,             PINMUX_FOR_KEY_SCAN6_MODE_9},
    {PAD_KEY8,         PADTOP_BANK,      REG_KEY_FIX_MODE,               REG_KEY_FIX_MODE_MASK,              BIT12,                  PINMUX_FOR_KEY_FIX_MODE_1},
    {PAD_KEY8,         PADTOP_BANK,      REG_UART2_MODE,                 REG_UART2_MODE_MASK,                BIT10,                  PINMUX_FOR_UART2_MODE_4},
    {PAD_KEY8,         PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT1|BIT0,              PINMUX_FOR_DMIC_MODE_3},
    {PAD_KEY8,         PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT3|BIT1,              PINMUX_FOR_DMIC_MODE_10},
    {PAD_KEY8,         PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT6,                   PINMUX_FOR_I2S_RX_MODE_4},
    {PAD_KEY8,         PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT14,                  PINMUX_FOR_I2S_RXTX_MODE_4},
    {PAD_KEY8,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT0,                   PINMUX_FOR_BT1120_MODE_1},
    {PAD_KEY8,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT1|BIT0,              PINMUX_FOR_BT1120_MODE_3},

    {PAD_KEY9,         PADGPIO_BANK,     REG_KEY9_GPIO_MODE,             REG_KEY9_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_KEY9,         CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_KEY9,         CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_KEY9,         PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT7,                   PINMUX_FOR_I2C1_MODE_8},
    {PAD_KEY9,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT0,              PINMUX_FOR_ETH0_MODE_9},
    {PAD_KEY9,         PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1,              PINMUX_FOR_ETH0_MODE_10},
    {PAD_KEY9,         PADTOP_BANK,      REG_PSPI1_CS2_MODE,             REG_PSPI1_CS2_MODE_MASK,            BIT7,                   PINMUX_FOR_PSPI1_CS2_MODE_8},
    {PAD_KEY9,         PADTOP_BANK,      REG_PSPI1_TE_MODE,              REG_PSPI1_TE_MODE_MASK,             BIT11,                  PINMUX_FOR_PSPI1_TE_MODE_8},
    {PAD_KEY9,         PADTOP_BANK,      REG_SPI1_CS2_MODE,              REG_SPI1_CS2_MODE_MASK,             BIT14,                  PINMUX_FOR_SPI1_CS2_MODE_4},
    {PAD_KEY9,         PADTOP_BANK,      REG_KEY_READ0_MODE,             REG_KEY_READ0_MODE_MASK,            BIT3|BIT1,              PINMUX_FOR_KEY_READ0_MODE_10},
    {PAD_KEY9,         PADTOP_BANK,      REG_KEY_READ1_MODE,             REG_KEY_READ1_MODE_MASK,            BIT7|BIT5,              PINMUX_FOR_KEY_READ1_MODE_10},
    {PAD_KEY9,         PADTOP_BANK,      REG_KEY_READ2_MODE,             REG_KEY_READ2_MODE_MASK,            BIT11|BIT9,             PINMUX_FOR_KEY_READ2_MODE_10},
    {PAD_KEY9,         PADTOP_BANK,      REG_KEY_READ3_MODE,             REG_KEY_READ3_MODE_MASK,            BIT15|BIT13,            PINMUX_FOR_KEY_READ3_MODE_10},
    {PAD_KEY9,         PADTOP_BANK,      REG_KEY_READ4_MODE,             REG_KEY_READ4_MODE_MASK,            BIT3|BIT1,              PINMUX_FOR_KEY_READ4_MODE_10},
    {PAD_KEY9,         PADTOP_BANK,      REG_KEY_READ5_MODE,             REG_KEY_READ5_MODE_MASK,            BIT7|BIT5,              PINMUX_FOR_KEY_READ5_MODE_10},
    {PAD_KEY9,         PADTOP_BANK,      REG_KEY_READ6_MODE,             REG_KEY_READ6_MODE_MASK,            BIT11|BIT9,             PINMUX_FOR_KEY_READ6_MODE_10},
    {PAD_KEY9,         PADTOP_BANK,      REG_KEY_SCAN0_MODE,             REG_KEY_SCAN0_MODE_MASK,            BIT3|BIT1,              PINMUX_FOR_KEY_SCAN0_MODE_10},
    {PAD_KEY9,         PADTOP_BANK,      REG_KEY_SCAN1_MODE,             REG_KEY_SCAN1_MODE_MASK,            BIT7|BIT5,              PINMUX_FOR_KEY_SCAN1_MODE_10},
    {PAD_KEY9,         PADTOP_BANK,      REG_KEY_SCAN2_MODE,             REG_KEY_SCAN2_MODE_MASK,            BIT11|BIT9,             PINMUX_FOR_KEY_SCAN2_MODE_10},
    {PAD_KEY9,         PADTOP_BANK,      REG_KEY_SCAN3_MODE,             REG_KEY_SCAN3_MODE_MASK,            BIT15|BIT13,            PINMUX_FOR_KEY_SCAN3_MODE_10},
    {PAD_KEY9,         PADTOP_BANK,      REG_KEY_SCAN4_MODE,             REG_KEY_SCAN4_MODE_MASK,            BIT3|BIT1,              PINMUX_FOR_KEY_SCAN4_MODE_10},
    {PAD_KEY9,         PADTOP_BANK,      REG_KEY_SCAN5_MODE,             REG_KEY_SCAN5_MODE_MASK,            BIT7|BIT5,              PINMUX_FOR_KEY_SCAN5_MODE_10},
    {PAD_KEY9,         PADTOP_BANK,      REG_KEY_SCAN6_MODE,             REG_KEY_SCAN6_MODE_MASK,            BIT11|BIT9,             PINMUX_FOR_KEY_SCAN6_MODE_10},
    {PAD_KEY9,         PADTOP_BANK,      REG_KEY_FIX_MODE,               REG_KEY_FIX_MODE_MASK,              BIT12,                  PINMUX_FOR_KEY_FIX_MODE_1},
    {PAD_KEY9,         PADTOP_BANK,      REG_UART2_MODE,                 REG_UART2_MODE_MASK,                BIT10,                  PINMUX_FOR_UART2_MODE_4},
    {PAD_KEY9,         PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT10,                  PINMUX_FOR_I2S_TX_MODE_4},
    {PAD_KEY9,         PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT14,                  PINMUX_FOR_I2S_RXTX_MODE_4},
    {PAD_KEY9,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT0,                   PINMUX_FOR_BT1120_MODE_1},
    {PAD_KEY9,         PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT1|BIT0,              PINMUX_FOR_BT1120_MODE_3},

//    {PAD_KEY10,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_KEY10,        PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT9|BIT8,              PINMUX_FOR_EJ_MODE_3},
    {PAD_KEY10,        PADGPIO_BANK,     REG_KEY10_GPIO_MODE,             REG_KEY10_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_KEY10,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_KEY10,        PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT2,                   PINMUX_FOR_SPI0_MODE_4},
    {PAD_KEY10,        PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2|BIT1|BIT0,         PINMUX_FOR_PSPI1_PL_MODE_7},
    {PAD_KEY10,        PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10|BIT9,             PINMUX_FOR_SPI1_MODE_6},
    {PAD_KEY10,        PADTOP_BANK,      REG_KEY_READ0_MODE,             REG_KEY_READ0_MODE_MASK,            BIT3|BIT1|BIT0,         PINMUX_FOR_KEY_READ0_MODE_11},
    {PAD_KEY10,        PADTOP_BANK,      REG_KEY_READ1_MODE,             REG_KEY_READ1_MODE_MASK,            BIT7|BIT5|BIT4,         PINMUX_FOR_KEY_READ1_MODE_11},
    {PAD_KEY10,        PADTOP_BANK,      REG_KEY_READ2_MODE,             REG_KEY_READ2_MODE_MASK,            BIT11|BIT9|BIT8,        PINMUX_FOR_KEY_READ2_MODE_11},
    {PAD_KEY10,        PADTOP_BANK,      REG_KEY_READ3_MODE,             REG_KEY_READ3_MODE_MASK,            BIT15|BIT13|BIT12,      PINMUX_FOR_KEY_READ3_MODE_11},
    {PAD_KEY10,        PADTOP_BANK,      REG_KEY_READ4_MODE,             REG_KEY_READ4_MODE_MASK,            BIT3|BIT1|BIT0,         PINMUX_FOR_KEY_READ4_MODE_11},
    {PAD_KEY10,        PADTOP_BANK,      REG_KEY_READ5_MODE,             REG_KEY_READ5_MODE_MASK,            BIT7|BIT5|BIT4,         PINMUX_FOR_KEY_READ5_MODE_11},
    {PAD_KEY10,        PADTOP_BANK,      REG_KEY_READ6_MODE,             REG_KEY_READ6_MODE_MASK,            BIT11|BIT9|BIT8,        PINMUX_FOR_KEY_READ6_MODE_11},
    {PAD_KEY10,        PADTOP_BANK,      REG_KEY_SCAN0_MODE,             REG_KEY_SCAN0_MODE_MASK,            BIT3|BIT1|BIT0,         PINMUX_FOR_KEY_SCAN0_MODE_11},
    {PAD_KEY10,        PADTOP_BANK,      REG_KEY_SCAN1_MODE,             REG_KEY_SCAN1_MODE_MASK,            BIT7|BIT5|BIT4,         PINMUX_FOR_KEY_SCAN1_MODE_11},
    {PAD_KEY10,        PADTOP_BANK,      REG_KEY_SCAN2_MODE,             REG_KEY_SCAN2_MODE_MASK,            BIT11|BIT9|BIT8,        PINMUX_FOR_KEY_SCAN2_MODE_11},
    {PAD_KEY10,        PADTOP_BANK,      REG_KEY_SCAN3_MODE,             REG_KEY_SCAN3_MODE_MASK,            BIT15|BIT13|BIT12,      PINMUX_FOR_KEY_SCAN3_MODE_11},
    {PAD_KEY10,        PADTOP_BANK,      REG_KEY_SCAN4_MODE,             REG_KEY_SCAN4_MODE_MASK,            BIT3|BIT1|BIT0,         PINMUX_FOR_KEY_SCAN4_MODE_11},
    {PAD_KEY10,        PADTOP_BANK,      REG_KEY_SCAN5_MODE,             REG_KEY_SCAN5_MODE_MASK,            BIT7|BIT5|BIT4,         PINMUX_FOR_KEY_SCAN5_MODE_11},
    {PAD_KEY10,        PADTOP_BANK,      REG_KEY_SCAN6_MODE,             REG_KEY_SCAN6_MODE_MASK,            BIT11|BIT9|BIT8,        PINMUX_FOR_KEY_SCAN6_MODE_11},
    {PAD_KEY10,        PADTOP_BANK,      REG_KEY_FIX_MODE,               REG_KEY_FIX_MODE_MASK,              BIT12,                  PINMUX_FOR_KEY_FIX_MODE_1},
    {PAD_KEY10,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT10,                  PINMUX_FOR_FUART_MODE_4},
    {PAD_KEY10,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT11|BIT9,             PINMUX_FOR_FUART_MODE_10},
    {PAD_KEY10,        PADTOP_BANK,      REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT3,                   PINMUX_FOR_PWM0_MODE_8},
    {PAD_KEY10,        PADTOP_BANK,      REG_I2S_MCK_MODE,               REG_I2S_MCK_MODE_MASK,              BIT2|BIT0,              PINMUX_FOR_I2S_MCK_MODE_5},
    {PAD_KEY10,        PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT0,                   PINMUX_FOR_BT1120_MODE_1},
    {PAD_KEY10,        PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT1|BIT0,              PINMUX_FOR_BT1120_MODE_3},

//    {PAD_KEY11,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_KEY11,        PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT9|BIT8,              PINMUX_FOR_EJ_MODE_3},
    {PAD_KEY11,        PADGPIO_BANK,     REG_KEY11_GPIO_MODE,             REG_KEY11_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_KEY11,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_KEY11,        PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT2,                   PINMUX_FOR_SPI0_MODE_4},
    {PAD_KEY11,        PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2|BIT1|BIT0,         PINMUX_FOR_PSPI1_PL_MODE_7},
    {PAD_KEY11,        PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10|BIT9,             PINMUX_FOR_SPI1_MODE_6},
    {PAD_KEY11,        PADTOP_BANK,      REG_KEY_READ0_MODE,             REG_KEY_READ0_MODE_MASK,            BIT3|BIT2,              PINMUX_FOR_KEY_READ0_MODE_12},
    {PAD_KEY11,        PADTOP_BANK,      REG_KEY_READ1_MODE,             REG_KEY_READ1_MODE_MASK,            BIT7|BIT6,              PINMUX_FOR_KEY_READ1_MODE_12},
    {PAD_KEY11,        PADTOP_BANK,      REG_KEY_READ2_MODE,             REG_KEY_READ2_MODE_MASK,            BIT11|BIT10,            PINMUX_FOR_KEY_READ2_MODE_12},
    {PAD_KEY11,        PADTOP_BANK,      REG_KEY_READ3_MODE,             REG_KEY_READ3_MODE_MASK,            BIT15|BIT14,            PINMUX_FOR_KEY_READ3_MODE_12},
    {PAD_KEY11,        PADTOP_BANK,      REG_KEY_READ4_MODE,             REG_KEY_READ4_MODE_MASK,            BIT3|BIT2,              PINMUX_FOR_KEY_READ4_MODE_12},
    {PAD_KEY11,        PADTOP_BANK,      REG_KEY_READ5_MODE,             REG_KEY_READ5_MODE_MASK,            BIT7|BIT6,              PINMUX_FOR_KEY_READ5_MODE_12},
    {PAD_KEY11,        PADTOP_BANK,      REG_KEY_READ6_MODE,             REG_KEY_READ6_MODE_MASK,            BIT11|BIT10,            PINMUX_FOR_KEY_READ6_MODE_12},
    {PAD_KEY11,        PADTOP_BANK,      REG_KEY_SCAN0_MODE,             REG_KEY_SCAN0_MODE_MASK,            BIT3|BIT2,              PINMUX_FOR_KEY_SCAN0_MODE_12},
    {PAD_KEY11,        PADTOP_BANK,      REG_KEY_SCAN1_MODE,             REG_KEY_SCAN1_MODE_MASK,            BIT7|BIT6,              PINMUX_FOR_KEY_SCAN1_MODE_12},
    {PAD_KEY11,        PADTOP_BANK,      REG_KEY_SCAN2_MODE,             REG_KEY_SCAN2_MODE_MASK,            BIT11|BIT10,            PINMUX_FOR_KEY_SCAN2_MODE_12},
    {PAD_KEY11,        PADTOP_BANK,      REG_KEY_SCAN3_MODE,             REG_KEY_SCAN3_MODE_MASK,            BIT15|BIT14,            PINMUX_FOR_KEY_SCAN3_MODE_12},
    {PAD_KEY11,        PADTOP_BANK,      REG_KEY_SCAN4_MODE,             REG_KEY_SCAN4_MODE_MASK,            BIT3|BIT2,              PINMUX_FOR_KEY_SCAN4_MODE_12},
    {PAD_KEY11,        PADTOP_BANK,      REG_KEY_SCAN5_MODE,             REG_KEY_SCAN5_MODE_MASK,            BIT7|BIT6,              PINMUX_FOR_KEY_SCAN5_MODE_12},
    {PAD_KEY11,        PADTOP_BANK,      REG_KEY_SCAN6_MODE,             REG_KEY_SCAN6_MODE_MASK,            BIT11|BIT10,            PINMUX_FOR_KEY_SCAN6_MODE_12},
    {PAD_KEY11,        PADTOP_BANK,      REG_KEY_FIX_MODE,               REG_KEY_FIX_MODE_MASK,              BIT12,                  PINMUX_FOR_KEY_FIX_MODE_1},
    {PAD_KEY11,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT10,                  PINMUX_FOR_FUART_MODE_4},
    {PAD_KEY11,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT11|BIT9,             PINMUX_FOR_FUART_MODE_10},
    {PAD_KEY11,        PADTOP_BANK,      REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT7,                   PINMUX_FOR_PWM1_MODE_8},
    {PAD_KEY11,        PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT10,                  PINMUX_FOR_I2S_TX_MODE_4},
    {PAD_KEY11,        PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT0,                   PINMUX_FOR_BT1120_MODE_1},
    {PAD_KEY11,        PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT1|BIT0,              PINMUX_FOR_BT1120_MODE_3},

//    {PAD_KEY12,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_KEY12,        PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT9|BIT8,              PINMUX_FOR_EJ_MODE_3},
    {PAD_KEY12,        PADGPIO_BANK,     REG_KEY12_GPIO_MODE,             REG_KEY12_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_KEY12,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_KEY12,        PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT3|BIT1|BIT0,         PINMUX_FOR_I2C0_MODE_11},
    {PAD_KEY12,        PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT2,                   PINMUX_FOR_SPI0_MODE_4},
    {PAD_KEY12,        PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2|BIT1|BIT0,         PINMUX_FOR_PSPI1_PL_MODE_7},
    {PAD_KEY12,        PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10|BIT9,             PINMUX_FOR_SPI1_MODE_6},
    {PAD_KEY12,        PADTOP_BANK,      REG_KEY_READ0_MODE,             REG_KEY_READ0_MODE_MASK,            BIT3|BIT2|BIT0,         PINMUX_FOR_KEY_READ0_MODE_13},
    {PAD_KEY12,        PADTOP_BANK,      REG_KEY_READ1_MODE,             REG_KEY_READ1_MODE_MASK,            BIT7|BIT6|BIT4,         PINMUX_FOR_KEY_READ1_MODE_13},
    {PAD_KEY12,        PADTOP_BANK,      REG_KEY_READ2_MODE,             REG_KEY_READ2_MODE_MASK,            BIT11|BIT10|BIT8,       PINMUX_FOR_KEY_READ2_MODE_13},
    {PAD_KEY12,        PADTOP_BANK,      REG_KEY_READ3_MODE,             REG_KEY_READ3_MODE_MASK,            BIT15|BIT14|BIT12,      PINMUX_FOR_KEY_READ3_MODE_13},
    {PAD_KEY12,        PADTOP_BANK,      REG_KEY_READ4_MODE,             REG_KEY_READ4_MODE_MASK,            BIT3|BIT2|BIT0,         PINMUX_FOR_KEY_READ4_MODE_13},
    {PAD_KEY12,        PADTOP_BANK,      REG_KEY_READ5_MODE,             REG_KEY_READ5_MODE_MASK,            BIT7|BIT6|BIT4,         PINMUX_FOR_KEY_READ5_MODE_13},
    {PAD_KEY12,        PADTOP_BANK,      REG_KEY_READ6_MODE,             REG_KEY_READ6_MODE_MASK,            BIT11|BIT10|BIT8,       PINMUX_FOR_KEY_READ6_MODE_13},
    {PAD_KEY12,        PADTOP_BANK,      REG_KEY_SCAN0_MODE,             REG_KEY_SCAN0_MODE_MASK,            BIT3|BIT2|BIT0,         PINMUX_FOR_KEY_SCAN0_MODE_13},
    {PAD_KEY12,        PADTOP_BANK,      REG_KEY_SCAN1_MODE,             REG_KEY_SCAN1_MODE_MASK,            BIT7|BIT6|BIT4,         PINMUX_FOR_KEY_SCAN1_MODE_13},
    {PAD_KEY12,        PADTOP_BANK,      REG_KEY_SCAN2_MODE,             REG_KEY_SCAN2_MODE_MASK,            BIT11|BIT10|BIT8,       PINMUX_FOR_KEY_SCAN2_MODE_13},
    {PAD_KEY12,        PADTOP_BANK,      REG_KEY_SCAN3_MODE,             REG_KEY_SCAN3_MODE_MASK,            BIT15|BIT14|BIT12,      PINMUX_FOR_KEY_SCAN3_MODE_13},
    {PAD_KEY12,        PADTOP_BANK,      REG_KEY_SCAN4_MODE,             REG_KEY_SCAN4_MODE_MASK,            BIT3|BIT2|BIT0,         PINMUX_FOR_KEY_SCAN4_MODE_13},
    {PAD_KEY12,        PADTOP_BANK,      REG_KEY_SCAN5_MODE,             REG_KEY_SCAN5_MODE_MASK,            BIT7|BIT6|BIT4,         PINMUX_FOR_KEY_SCAN5_MODE_13},
    {PAD_KEY12,        PADTOP_BANK,      REG_KEY_SCAN6_MODE,             REG_KEY_SCAN6_MODE_MASK,            BIT11|BIT10|BIT8,       PINMUX_FOR_KEY_SCAN6_MODE_13},
    {PAD_KEY12,        PADTOP_BANK,      REG_KEY_FIX_MODE,               REG_KEY_FIX_MODE_MASK,              BIT12,                  PINMUX_FOR_KEY_FIX_MODE_1},
    {PAD_KEY12,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT10,                  PINMUX_FOR_FUART_MODE_4},
    {PAD_KEY12,        PADTOP_BANK,      REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT11,                  PINMUX_FOR_PWM2_MODE_8},
    {PAD_KEY12,        PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT10,                  PINMUX_FOR_I2S_TX_MODE_4},
    {PAD_KEY12,        PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT0,                   PINMUX_FOR_BT1120_MODE_1},
    {PAD_KEY12,        PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT1|BIT0,              PINMUX_FOR_BT1120_MODE_3},

//    {PAD_KEY13,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_KEY13,        PADTOP_BANK,      REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT9|BIT8,              PINMUX_FOR_EJ_MODE_3},
    {PAD_KEY13,        PADGPIO_BANK,     REG_KEY13_GPIO_MODE,             REG_KEY13_GPIO_MODE_MASK,            BIT3,                   PINMUX_FOR_GPIO_MODE},
    {PAD_KEY13,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_KEY13,        PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT3|BIT1|BIT0,         PINMUX_FOR_I2C0_MODE_11},
    {PAD_KEY13,        PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT2,                   PINMUX_FOR_SPI0_MODE_4},
    {PAD_KEY13,        PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT2|BIT1|BIT0,         PINMUX_FOR_PSPI1_PL_MODE_7},
    {PAD_KEY13,        PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10|BIT9,             PINMUX_FOR_SPI1_MODE_6},
    {PAD_KEY13,        PADTOP_BANK,      REG_KEY_READ0_MODE,             REG_KEY_READ0_MODE_MASK,            BIT3|BIT2|BIT1,         PINMUX_FOR_KEY_READ0_MODE_14},
    {PAD_KEY13,        PADTOP_BANK,      REG_KEY_READ1_MODE,             REG_KEY_READ1_MODE_MASK,            BIT7|BIT6|BIT5,         PINMUX_FOR_KEY_READ1_MODE_14},
    {PAD_KEY13,        PADTOP_BANK,      REG_KEY_READ2_MODE,             REG_KEY_READ2_MODE_MASK,            BIT11|BIT10|BIT9,       PINMUX_FOR_KEY_READ2_MODE_14},
    {PAD_KEY13,        PADTOP_BANK,      REG_KEY_READ3_MODE,             REG_KEY_READ3_MODE_MASK,            BIT15|BIT14|BIT13,      PINMUX_FOR_KEY_READ3_MODE_14},
    {PAD_KEY13,        PADTOP_BANK,      REG_KEY_READ4_MODE,             REG_KEY_READ4_MODE_MASK,            BIT3|BIT2|BIT1,         PINMUX_FOR_KEY_READ4_MODE_14},
    {PAD_KEY13,        PADTOP_BANK,      REG_KEY_READ5_MODE,             REG_KEY_READ5_MODE_MASK,            BIT7|BIT6|BIT5,         PINMUX_FOR_KEY_READ5_MODE_14},
    {PAD_KEY13,        PADTOP_BANK,      REG_KEY_READ6_MODE,             REG_KEY_READ6_MODE_MASK,            BIT11|BIT10|BIT9,       PINMUX_FOR_KEY_READ6_MODE_14},
    {PAD_KEY13,        PADTOP_BANK,      REG_KEY_SCAN0_MODE,             REG_KEY_SCAN0_MODE_MASK,            BIT3|BIT2|BIT1,         PINMUX_FOR_KEY_SCAN0_MODE_14},
    {PAD_KEY13,        PADTOP_BANK,      REG_KEY_SCAN1_MODE,             REG_KEY_SCAN1_MODE_MASK,            BIT7|BIT6|BIT5,         PINMUX_FOR_KEY_SCAN1_MODE_14},
    {PAD_KEY13,        PADTOP_BANK,      REG_KEY_SCAN2_MODE,             REG_KEY_SCAN2_MODE_MASK,            BIT11|BIT10|BIT9,       PINMUX_FOR_KEY_SCAN2_MODE_14},
    {PAD_KEY13,        PADTOP_BANK,      REG_KEY_SCAN3_MODE,             REG_KEY_SCAN3_MODE_MASK,            BIT15|BIT14|BIT13,      PINMUX_FOR_KEY_SCAN3_MODE_14},
    {PAD_KEY13,        PADTOP_BANK,      REG_KEY_SCAN4_MODE,             REG_KEY_SCAN4_MODE_MASK,            BIT3|BIT2|BIT1,         PINMUX_FOR_KEY_SCAN4_MODE_14},
    {PAD_KEY13,        PADTOP_BANK,      REG_KEY_SCAN5_MODE,             REG_KEY_SCAN5_MODE_MASK,            BIT7|BIT6|BIT5,         PINMUX_FOR_KEY_SCAN5_MODE_14},
    {PAD_KEY13,        PADTOP_BANK,      REG_KEY_SCAN6_MODE,             REG_KEY_SCAN6_MODE_MASK,            BIT11|BIT10|BIT9,       PINMUX_FOR_KEY_SCAN6_MODE_14},
    {PAD_KEY13,        PADTOP_BANK,      REG_KEY_FIX_MODE,               REG_KEY_FIX_MODE_MASK,              BIT12,                  PINMUX_FOR_KEY_FIX_MODE_1},
    {PAD_KEY13,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT10,                  PINMUX_FOR_FUART_MODE_4},
    {PAD_KEY13,        PADTOP_BANK,      REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT15,                  PINMUX_FOR_PWM3_MODE_8},
    {PAD_KEY13,        PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT0,                   PINMUX_FOR_BT1120_MODE_1},
    {PAD_KEY13,        PADTOP_BANK,      REG_BT1120_MODE,                REG_BT1120_MODE_MASK,               BIT1|BIT0,              PINMUX_FOR_BT1120_MODE_3},

    {PAD_SD_D1,        PADGPIO_BANK,     REG_SD_D1_GPIO_MODE,            REG_SD_D1_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SD_D1,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SD_D1,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SD_D1,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_SD_D1,        PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT3,                   PINMUX_FOR_PSPI1_PL_MODE_8},
    {PAD_SD_D1,        PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10|BIT9|BIT8,        PINMUX_FOR_SPI1_MODE_7},
    {PAD_SD_D1,        PADTOP_BANK,      REG_SDIO_MODE,                  REG_SDIO_MODE_MASK,                 BIT8,                   PINMUX_FOR_SDIO_MODE_1},
    {PAD_SD_D1,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_FUART_MODE_5},
    {PAD_SD_D1,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT11|BIT9|BIT8,        PINMUX_FOR_FUART_MODE_11},

    {PAD_SD_D0,        PADGPIO_BANK,     REG_SD_D0_GPIO_MODE,            REG_SD_D0_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SD_D0,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SD_D0,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SD_D0,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_SD_D0,        PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT3,                   PINMUX_FOR_PSPI1_PL_MODE_8},
    {PAD_SD_D0,        PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10|BIT9|BIT8,        PINMUX_FOR_SPI1_MODE_7},
    {PAD_SD_D0,        PADTOP_BANK,      REG_SDIO_MODE,                  REG_SDIO_MODE_MASK,                 BIT8,                   PINMUX_FOR_SDIO_MODE_1},
    {PAD_SD_D0,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_FUART_MODE_5},
    {PAD_SD_D0,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT11|BIT9|BIT8,        PINMUX_FOR_FUART_MODE_11},

    {PAD_SD_CLK,       PADGPIO_BANK,     REG_SD_CLK_GPIO_MODE,           REG_SD_CLK_GPIO_MODE_MASK,          BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SD_CLK,       CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SD_CLK,       CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SD_CLK,       PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_SD_CLK,       PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT3,                   PINMUX_FOR_PSPI1_PL_MODE_8},
    {PAD_SD_CLK,       PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10|BIT9|BIT8,        PINMUX_FOR_SPI1_MODE_7},
    {PAD_SD_CLK,       PADTOP_BANK,      REG_SDIO_MODE,                  REG_SDIO_MODE_MASK,                 BIT8,                   PINMUX_FOR_SDIO_MODE_1},
    {PAD_SD_CLK,       PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_FUART_MODE_5},

    {PAD_SD_CMD,       PADGPIO_BANK,     REG_SD_CMD_GPIO_MODE,           REG_SD_CMD_GPIO_MODE_MASK,          BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SD_CMD,       CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SD_CMD,       CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SD_CMD,       PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_SD_CMD,       PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT3,                   PINMUX_FOR_PSPI1_PL_MODE_8},
    {PAD_SD_CMD,       PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT10|BIT9|BIT8,        PINMUX_FOR_SPI1_MODE_7},
    {PAD_SD_CMD,       PADTOP_BANK,      REG_SDIO_MODE,                  REG_SDIO_MODE_MASK,                 BIT8,                   PINMUX_FOR_SDIO_MODE_1},
    {PAD_SD_CMD,       PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_FUART_MODE_5},
    {PAD_SD_CMD,       PADTOP_BANK,      REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT3|BIT0,              PINMUX_FOR_PWM0_MODE_9},
    {PAD_SD_CMD,       PADTOP_BANK,      REG_I2S_MCK_MODE,               REG_I2S_MCK_MODE_MASK,              BIT2|BIT1,              PINMUX_FOR_I2S_MCK_MODE_6},

    {PAD_SD_D3,        PADGPIO_BANK,     REG_SD_D3_GPIO_MODE,            REG_SD_D3_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SD_D3,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SD_D3,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SD_D3,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_SD_D3,        PADTOP_BANK,      REG_PSPI1_CS2_MODE,             REG_PSPI1_CS2_MODE_MASK,            BIT7|BIT4,              PINMUX_FOR_PSPI1_CS2_MODE_9},
    {PAD_SD_D3,        PADTOP_BANK,      REG_PSPI1_TE_MODE,              REG_PSPI1_TE_MODE_MASK,             BIT11|BIT8,             PINMUX_FOR_PSPI1_TE_MODE_9},
    {PAD_SD_D3,        PADTOP_BANK,      REG_SPI1_CS2_MODE,              REG_SPI1_CS2_MODE_MASK,             BIT14|BIT12,            PINMUX_FOR_SPI1_CS2_MODE_5},
    {PAD_SD_D3,        PADTOP_BANK,      REG_SDIO_MODE,                  REG_SDIO_MODE_MASK,                 BIT8,                   PINMUX_FOR_SDIO_MODE_1},
    {PAD_SD_D3,        PADTOP_BANK,      REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_UART0_MODE_3},
    {PAD_SD_D3,        PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT6|BIT5|BIT4,         PINMUX_FOR_UART1_MODE_7},
    {PAD_SD_D3,        PADTOP_BANK,      REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT7|BIT4,              PINMUX_FOR_PWM1_MODE_9},
    {PAD_SD_D3,        PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT6|BIT4,              PINMUX_FOR_I2S_RX_MODE_5},
    {PAD_SD_D3,        PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT10|BIT8,             PINMUX_FOR_I2S_TX_MODE_5},
    {PAD_SD_D3,        PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT14|BIT12,            PINMUX_FOR_I2S_RXTX_MODE_5},

    {PAD_SD_D2,        PADGPIO_BANK,     REG_SD_D2_GPIO_MODE,            REG_SD_D2_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SD_D2,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SD_D2,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_SD_D2,        PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_SD_D2,        PADTOP_BANK,      REG_SDIO_MODE,                  REG_SDIO_MODE_MASK,                 BIT8,                   PINMUX_FOR_SDIO_MODE_1},
    {PAD_SD_D2,        PADTOP_BANK,      REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT1|BIT0,              PINMUX_FOR_UART0_MODE_3},
    {PAD_SD_D2,        PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT6|BIT5|BIT4,         PINMUX_FOR_UART1_MODE_7},
    {PAD_SD_D2,        PADTOP_BANK,      REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT11|BIT8,             PINMUX_FOR_PWM2_MODE_9},
    {PAD_SD_D2,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2,                   PINMUX_FOR_DMIC_MODE_4},
    {PAD_SD_D2,        PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT6|BIT4,              PINMUX_FOR_I2S_RX_MODE_5},
    {PAD_SD_D2,        PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT10|BIT8,             PINMUX_FOR_I2S_TX_MODE_5},
    {PAD_SD_D2,        PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT14|BIT12,            PINMUX_FOR_I2S_RXTX_MODE_5},

    {PAD_SD_GPIO0,     PADGPIO_BANK,     REG_SD_GPIO0_GPIO_MODE,         REG_SD_GPIO0_GPIO_MODE_MASK,        BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SD_GPIO0,     CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SD_GPIO0,     PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_SD_GPIO0,     PADTOP_BANK,      REG_UART2_MODE,                 REG_UART2_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_UART2_MODE_5},
    {PAD_SD_GPIO0,     PADTOP_BANK,      REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT15|BIT12,            PINMUX_FOR_PWM3_MODE_9},
    {PAD_SD_GPIO0,     PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2,                   PINMUX_FOR_DMIC_MODE_4},
    {PAD_SD_GPIO0,     PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT3|BIT1|BIT0,         PINMUX_FOR_DMIC_MODE_11},
    {PAD_SD_GPIO0,     PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT6|BIT4,              PINMUX_FOR_I2S_RX_MODE_5},
    {PAD_SD_GPIO0,     PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT14|BIT12,            PINMUX_FOR_I2S_RXTX_MODE_5},

    {PAD_SD_GPIO1,     PADGPIO_BANK,     REG_SD_GPIO1_GPIO_MODE,         REG_SD_GPIO1_GPIO_MODE_MASK,        BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_SD_GPIO1,     CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_SD_GPIO1,     PADTOP_BANK,      REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT9,                   PINMUX_FOR_TTL24_MODE_2},
    {PAD_SD_GPIO1,     PADTOP_BANK,      REG_UART2_MODE,                 REG_UART2_MODE_MASK,                BIT10|BIT8,             PINMUX_FOR_UART2_MODE_5},
    {PAD_SD_GPIO1,     PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2,                   PINMUX_FOR_DMIC_MODE_4},
    {PAD_SD_GPIO1,     PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT3|BIT1|BIT0,         PINMUX_FOR_DMIC_MODE_11},
    {PAD_SD_GPIO1,     PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT10|BIT8,             PINMUX_FOR_I2S_TX_MODE_5},
    {PAD_SD_GPIO1,     PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT14|BIT12,            PINMUX_FOR_I2S_RXTX_MODE_5},

    {PAD_GPIO0,        PADGPIO_BANK,     REG_GPIO0_GPIO_MODE,            REG_GPIO0_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_GPIO0,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_GPIO0,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_GPIO0,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_GPIO0,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_GPIO0,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_11},
    {PAD_GPIO0,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT2,              PINMUX_FOR_ETH0_MODE_12},
    {PAD_GPIO0,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_DMIC_MODE_5},
    {PAD_GPIO0,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT3|BIT2,              PINMUX_FOR_DMIC_MODE_12},

    {PAD_GPIO1,        PADGPIO_BANK,     REG_GPIO1_GPIO_MODE,            REG_GPIO1_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_GPIO1,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_GPIO1,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_GPIO1,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_GPIO1,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_GPIO1,        PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT3|BIT2,              PINMUX_FOR_I2C0_MODE_12},
    {PAD_GPIO1,        PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT7|BIT4,              PINMUX_FOR_I2C1_MODE_9},
    {PAD_GPIO1,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_11},
    {PAD_GPIO1,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT2,              PINMUX_FOR_ETH0_MODE_12},
    {PAD_GPIO1,        PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT7,                   PINMUX_FOR_UART1_MODE_8},
    {PAD_GPIO1,        PADTOP_BANK,      REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT3|BIT1,              PINMUX_FOR_PWM0_MODE_10},
    {PAD_GPIO1,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_DMIC_MODE_5},
    {PAD_GPIO1,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT3|BIT2,              PINMUX_FOR_DMIC_MODE_12},

    {PAD_GPIO2,        PADGPIO_BANK,     REG_GPIO2_GPIO_MODE,            REG_GPIO2_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_GPIO2,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_GPIO2,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_GPIO2,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_GPIO2,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_GPIO2,        PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT3|BIT2,              PINMUX_FOR_I2C0_MODE_12},
    {PAD_GPIO2,        PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT7|BIT4,              PINMUX_FOR_I2C1_MODE_9},
    {PAD_GPIO2,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_11},
    {PAD_GPIO2,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT2,              PINMUX_FOR_ETH0_MODE_12},
    {PAD_GPIO2,        PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT7,                   PINMUX_FOR_UART1_MODE_8},
    {PAD_GPIO2,        PADTOP_BANK,      REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT7|BIT5,              PINMUX_FOR_PWM1_MODE_10},
    {PAD_GPIO2,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2|BIT0,              PINMUX_FOR_DMIC_MODE_5},
    {PAD_GPIO2,        PADTOP_BANK,      REG_I2S_MCK_MODE,               REG_I2S_MCK_MODE_MASK,              BIT2|BIT1|BIT0,         PINMUX_FOR_I2S_MCK_MODE_7},

    {PAD_GPIO3,        PADGPIO_BANK,     REG_GPIO3_GPIO_MODE,            REG_GPIO3_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_GPIO3,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_GPIO3,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_GPIO3,        CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,              PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_GPIO3,        CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,              PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_GPIO3,        PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT3|BIT2|BIT0,         PINMUX_FOR_I2C0_MODE_13},
    {PAD_GPIO3,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_11},
    {PAD_GPIO3,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT2,              PINMUX_FOR_ETH0_MODE_12},
    {PAD_GPIO3,        PADTOP_BANK,      REG_UART2_MODE,                 REG_UART2_MODE_MASK,                BIT10|BIT9,             PINMUX_FOR_UART2_MODE_6},
    {PAD_GPIO3,        PADTOP_BANK,      REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT11|BIT9,             PINMUX_FOR_PWM2_MODE_10},
    {PAD_GPIO3,        PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT6|BIT5,              PINMUX_FOR_I2S_RX_MODE_6},
    {PAD_GPIO3,        PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT14|BIT13,            PINMUX_FOR_I2S_RXTX_MODE_6},

    {PAD_GPIO4,        PADGPIO_BANK,     REG_GPIO4_GPIO_MODE,            REG_GPIO4_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_GPIO4,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_GPIO4,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_GPIO4,        PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT3|BIT2|BIT0,         PINMUX_FOR_I2C0_MODE_13},
    {PAD_GPIO4,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_11},
    {PAD_GPIO4,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT2,              PINMUX_FOR_ETH0_MODE_12},
    {PAD_GPIO4,        PADTOP_BANK,      REG_PSPI1_CS2_MODE,             REG_PSPI1_CS2_MODE_MASK,            BIT7|BIT5,              PINMUX_FOR_PSPI1_CS2_MODE_10},
    {PAD_GPIO4,        PADTOP_BANK,      REG_PSPI1_TE_MODE,              REG_PSPI1_TE_MODE_MASK,             BIT11|BIT9,             PINMUX_FOR_PSPI1_TE_MODE_10},
    {PAD_GPIO4,        PADTOP_BANK,      REG_SPI1_CS2_MODE,              REG_SPI1_CS2_MODE_MASK,             BIT14|BIT13,            PINMUX_FOR_SPI1_CS2_MODE_6},
    {PAD_GPIO4,        PADTOP_BANK,      REG_UART2_MODE,                 REG_UART2_MODE_MASK,                BIT10|BIT9,             PINMUX_FOR_UART2_MODE_6},
    {PAD_GPIO4,        PADTOP_BANK,      REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT15|BIT13,            PINMUX_FOR_PWM3_MODE_10},
    {PAD_GPIO4,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2|BIT1,              PINMUX_FOR_DMIC_MODE_6},
    {PAD_GPIO4,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT3|BIT2|BIT0,         PINMUX_FOR_DMIC_MODE_13},
    {PAD_GPIO4,        PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT6|BIT5,              PINMUX_FOR_I2S_RX_MODE_6},
    {PAD_GPIO4,        PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT14|BIT13,            PINMUX_FOR_I2S_RXTX_MODE_6},

    {PAD_GPIO5,        PADGPIO_BANK,     REG_GPIO5_GPIO_MODE,            REG_GPIO5_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_GPIO5,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_GPIO5,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_GPIO5,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_11},
    {PAD_GPIO5,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT2,              PINMUX_FOR_ETH0_MODE_12},
    {PAD_GPIO5,        PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT3|BIT0,              PINMUX_FOR_PSPI1_PL_MODE_9},
    {PAD_GPIO5,        PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT11,                  PINMUX_FOR_SPI1_MODE_8},
    {PAD_GPIO5,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT10|BIT9,             PINMUX_FOR_FUART_MODE_6},
    {PAD_GPIO5,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT11|BIT10,            PINMUX_FOR_FUART_MODE_12},
    {PAD_GPIO5,        PADTOP_BANK,      REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT2,                   PINMUX_FOR_UART0_MODE_4},
    {PAD_GPIO5,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2|BIT1,              PINMUX_FOR_DMIC_MODE_6},
    {PAD_GPIO5,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT3|BIT2|BIT0,         PINMUX_FOR_DMIC_MODE_13},
    {PAD_GPIO5,        PADTOP_BANK,      REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT6|BIT5,              PINMUX_FOR_I2S_RX_MODE_6},
    {PAD_GPIO5,        PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT14|BIT13,            PINMUX_FOR_I2S_RXTX_MODE_6},

    {PAD_GPIO6,        PADGPIO_BANK,     REG_GPIO6_GPIO_MODE,            REG_GPIO6_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_GPIO6,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_GPIO6,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_GPIO6,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_11},
    {PAD_GPIO6,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT2,              PINMUX_FOR_ETH0_MODE_12},
    {PAD_GPIO6,        PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT3|BIT0,              PINMUX_FOR_PSPI1_PL_MODE_9},
    {PAD_GPIO6,        PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT11,                  PINMUX_FOR_SPI1_MODE_8},
    {PAD_GPIO6,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT10|BIT9,             PINMUX_FOR_FUART_MODE_6},
    {PAD_GPIO6,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT11|BIT10,            PINMUX_FOR_FUART_MODE_12},
    {PAD_GPIO6,        PADTOP_BANK,      REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT2,                   PINMUX_FOR_UART0_MODE_4},
    {PAD_GPIO6,        PADTOP_BANK,      REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2|BIT1,              PINMUX_FOR_DMIC_MODE_6},
    {PAD_GPIO6,        PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT10|BIT9,             PINMUX_FOR_I2S_TX_MODE_6},
    {PAD_GPIO6,        PADTOP_BANK,      REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT14|BIT13,            PINMUX_FOR_I2S_RXTX_MODE_6},

    {PAD_GPIO7,        PADGPIO_BANK,     REG_GPIO7_GPIO_MODE,            REG_GPIO7_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_GPIO7,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_GPIO7,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_GPIO7,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_11},
    {PAD_GPIO7,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT2,              PINMUX_FOR_ETH0_MODE_12},
    {PAD_GPIO7,        PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT3|BIT0,              PINMUX_FOR_PSPI1_PL_MODE_9},
    {PAD_GPIO7,        PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT11,                  PINMUX_FOR_SPI1_MODE_8},
    {PAD_GPIO7,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT10|BIT9,             PINMUX_FOR_FUART_MODE_6},
    {PAD_GPIO7,        PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT7|BIT4,              PINMUX_FOR_UART1_MODE_9},
    {PAD_GPIO7,        PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT10|BIT9,             PINMUX_FOR_I2S_TX_MODE_6},

    {PAD_GPIO8,        PADGPIO_BANK,     REG_GPIO8_GPIO_MODE,            REG_GPIO8_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_GPIO8,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_GPIO8,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},
    {PAD_GPIO8,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT1|BIT0,         PINMUX_FOR_ETH0_MODE_11},
    {PAD_GPIO8,        PADTOP_BANK,      REG_ETH0_MODE,                  REG_ETH0_MODE_MASK,                 BIT3|BIT2,              PINMUX_FOR_ETH0_MODE_12},
    {PAD_GPIO8,        PADTOP_BANK,      REG_PSPI1_PL_MODE,              REG_PSPI1_PL_MODE_MASK,             BIT3|BIT0,              PINMUX_FOR_PSPI1_PL_MODE_9},
    {PAD_GPIO8,        PADTOP_BANK,      REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT11,                  PINMUX_FOR_SPI1_MODE_8},
    {PAD_GPIO8,        PADTOP_BANK,      REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT10|BIT9,             PINMUX_FOR_FUART_MODE_6},
    {PAD_GPIO8,        PADTOP_BANK,      REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT7|BIT4,              PINMUX_FOR_UART1_MODE_9},
    {PAD_GPIO8,        PADTOP_BANK,      REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT10|BIT9,             PINMUX_FOR_I2S_TX_MODE_6},

    {PAD_GPIO9,        PADGPIO_BANK,     REG_GPIO9_GPIO_MODE,            REG_GPIO9_GPIO_MODE_MASK,           BIT3,                   PINMUX_FOR_GPIO_MODE},
//    {PAD_GPIO9,        CHIPTOP_BANK,     REG_AUTO_GPIO_EN,               REG_AUTO_GPIO_EN_MASK,              BIT0,                   PINMUX_FOR_AUTO_GPIO_EN_1},
    {PAD_GPIO9,        CHIPTOP_BANK,     REG_ALL_PAD_IN,                 REG_ALL_PAD_IN_MASK,                BIT15,                  PINMUX_FOR_ALL_PAD_IN_1},

    {PAD_PM_SD_CDZ,    PADTOP_BANK,      REG_SD_CDZ_MODE,                REG_SD_CDZ_MODE_MASK,               BIT0,                   PINMUX_FOR_SD_CDZ_MODE_1},
    {PAD_PM_SD_CDZ,    PMSLEEP_BANK,     REG_PM_PWM3_MODE,               REG_PM_PWM3_MODE_MASK,              BIT8,                   PINMUX_FOR_PM_PWM3_MODE_1},
    {PAD_PM_SD_CDZ,    PMSLEEP_BANK,     REG_PM_SD_CDZ_MODE,             REG_PM_SD_CDZ_MODE_MASK,            BIT14,                  PINMUX_FOR_PM_SD_CDZ_MODE_1},
    {PAD_PM_SD_CDZ,    PMSLEEP_BANK,     REG_PM_PAD_EXT_MODE_0,          REG_PM_PAD_EXT_MODE_0_MASK,         BIT0,                   PINMUX_FOR_PM_PAD_EXT_MODE_0_1},

    {PAD_PM_SPI_CZ,    CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_PM_SPI_CZ,    CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_PM_SPI_CZ,    PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT0,                   PINMUX_FOR_SPI0_MODE_1},
    {PAD_PM_SPI_CZ,    PMSLEEP_BANK,     REG_PM_SPICSZ1_GPIO,            REG_PM_SPICSZ1_GPIO_MASK,           0,                      PINMUX_FOR_PM_SPICSZ1_GPIO_0},
    {PAD_PM_SPI_CZ,    PMSLEEP_BANK,     REG_PM_PAD_EXT_MODE_3,          REG_PM_PAD_EXT_MODE_3_MASK,         BIT3,                   PINMUX_FOR_PM_PAD_EXT_MODE_3_1},

    {PAD_PM_SPI_CK,    CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_PM_SPI_CK,    CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_PM_SPI_CK,    PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT0,                   PINMUX_FOR_SPI0_MODE_1},
    {PAD_PM_SPI_CK,    PMSLEEP_BANK,     REG_PM_SPI_GPIO,                REG_PM_SPI_GPIO_MASK,               0,                      PINMUX_FOR_PM_SPI_GPIO_0},
    {PAD_PM_SPI_CK,    PMSLEEP_BANK,     REG_PM_PAD_EXT_MODE_7,          REG_PM_PAD_EXT_MODE_7_MASK,         BIT7,                   PINMUX_FOR_PM_PAD_EXT_MODE_7_1},

    {PAD_PM_SPI_DI,    CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_PM_SPI_DI,    CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_PM_SPI_DI,    PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT0,                   PINMUX_FOR_SPI0_MODE_1},
    {PAD_PM_SPI_DI,    PMSLEEP_BANK,     REG_PM_SPI_GPIO,                REG_PM_SPI_GPIO_MASK,               0,                      PINMUX_FOR_PM_SPI_GPIO_0},
    {PAD_PM_SPI_DI,    PMSLEEP_BANK,     REG_PM_PAD_EXT_MODE_4,          REG_PM_PAD_EXT_MODE_4_MASK,         BIT4,                   PINMUX_FOR_PM_PAD_EXT_MODE_4_1},

    {PAD_PM_SPI_DO,    CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_PM_SPI_DO,    CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_PM_SPI_DO,    PADTOP_BANK,      REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT0,                   PINMUX_FOR_SPI0_MODE_1},
    {PAD_PM_SPI_DO,    PMSLEEP_BANK,     REG_PM_SPI_GPIO,                REG_PM_SPI_GPIO_MASK,               0,                      PINMUX_FOR_PM_SPI_GPIO_0},
    {PAD_PM_SPI_DO,    PMSLEEP_BANK,     REG_PM_PAD_EXT_MODE_6,          REG_PM_PAD_EXT_MODE_6_MASK,         BIT6,                   PINMUX_FOR_PM_PAD_EXT_MODE_6_1},

    {PAD_PM_SPI_WPZ,   CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_PM_SPI_WPZ,   CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_PM_SPI_WPZ,   PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT1,                   PINMUX_FOR_I2C0_MODE_2},
    {PAD_PM_SPI_WPZ,   PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT4,                   PINMUX_FOR_I2C1_MODE_1},
    {PAD_PM_SPI_WPZ,   PMSLEEP_BANK,     REG_PM_SPIWPN_GPIO,             REG_PM_SPIWPN_GPIO_MASK,            0,                      PINMUX_FOR_PM_SPIWPN_GPIO_0},
    {PAD_PM_SPI_WPZ,   PMSLEEP_BANK,     REG_PM_PAD_EXT_MODE_5,          REG_PM_PAD_EXT_MODE_5_MASK,         BIT5,                   PINMUX_FOR_PM_PAD_EXT_MODE_5_1},

    {PAD_PM_SPI_HLD,   PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT1,                   PINMUX_FOR_I2C0_MODE_2},
    {PAD_PM_SPI_HLD,   PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT4,                   PINMUX_FOR_I2C1_MODE_1},
    {PAD_PM_SPI_HLD,   PMSLEEP_BANK,     REG_PM_SPIHOLDN_MODE,           REG_PM_SPIHOLDN_MODE_MASK,          0,                      PINMUX_FOR_PM_SPIHOLDN_MODE_0},
    {PAD_PM_SPI_HLD,   PMSLEEP_BANK,     REG_PM_PWM2_MODE,               REG_PM_PWM2_MODE_MASK,              BIT6,                   PINMUX_FOR_PM_PWM2_MODE_1},
    {PAD_PM_SPI_HLD,   PMSLEEP_BANK,     REG_PM_PAD_EXT_MODE_8,          REG_PM_PAD_EXT_MODE_8_MASK,         BIT8,                   PINMUX_FOR_PM_PAD_EXT_MODE_8_1},

    {PAD_PM_LED0,      PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT3|BIT2|BIT1,         PINMUX_FOR_I2C0_MODE_14},
    {PAD_PM_LED0,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT7|BIT5,              PINMUX_FOR_I2C1_MODE_10},
    {PAD_PM_LED0,      PADTOP_BANK,      REG_UART2_MODE,                 REG_UART2_MODE_MASK,                BIT10|BIT9|BIT8,        PINMUX_FOR_UART2_MODE_7},
    {PAD_PM_LED0,      PMSLEEP_BANK,     REG_PM_SPICSZ2_GPIO,            REG_PM_SPICSZ2_GPIO_MASK,           0,                      PINMUX_FOR_PM_SPICSZ2_GPIO_0},
    {PAD_PM_LED0,      PMSLEEP_BANK,     REG_PM_PWM0_MODE,               REG_PM_PWM0_MODE_MASK,              BIT0,                   PINMUX_FOR_PM_PWM0_MODE_1},
    {PAD_PM_LED0,      PMSLEEP_BANK,     REG_PM_UART1_MODE,              REG_PM_UART1_MODE_MASK,             BIT8,                   PINMUX_FOR_PM_UART1_MODE_1},
    {PAD_PM_LED0,      PMSLEEP_BANK,     REG_PM_VID_MODE,                REG_PM_VID_MODE_MASK,               BIT12,                  PINMUX_FOR_PM_VID_MODE_1},
    {PAD_PM_LED0,      PMSLEEP_BANK,     REG_PM_VID_MODE,                REG_PM_VID_MODE_MASK,               BIT13,                  PINMUX_FOR_PM_VID_MODE_2},
    {PAD_PM_LED0,      PMSLEEP_BANK,     REG_PM_LED_MODE,                REG_PM_LED_MODE_MASK,               BIT4,                   PINMUX_FOR_PM_LED_MODE_1},
    {PAD_PM_LED0,      PMSLEEP_BANK,     REG_PM_LED_MODE,                REG_PM_LED_MODE_MASK,               BIT5,                   PINMUX_FOR_PM_LED_MODE_2},
    {PAD_PM_LED0,      PMSLEEP_BANK,     REG_PM_LED_MODE,                REG_PM_LED_MODE_MASK,               BIT5|BIT4,              PINMUX_FOR_PM_LED_MODE_3},
    {PAD_PM_LED0,      PMSLEEP_BANK,     REG_PM_PAD_EXT_MODE_9,          REG_PM_PAD_EXT_MODE_9_MASK,         BIT9,                   PINMUX_FOR_PM_PAD_EXT_MODE_9_1},

    {PAD_PM_LED1,      PADTOP_BANK,      REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT3|BIT2|BIT1,         PINMUX_FOR_I2C0_MODE_14},
    {PAD_PM_LED1,      PADTOP_BANK,      REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT7|BIT5,              PINMUX_FOR_I2C1_MODE_10},
    {PAD_PM_LED1,      PADTOP_BANK,      REG_UART2_MODE,                 REG_UART2_MODE_MASK,                BIT10|BIT9|BIT8,        PINMUX_FOR_UART2_MODE_7},
    {PAD_PM_LED1,      PMSLEEP_BANK,     REG_PM_PWM1_MODE,               REG_PM_PWM1_MODE_MASK,              BIT2,                   PINMUX_FOR_PM_PWM1_MODE_1},
    {PAD_PM_LED1,      PMSLEEP_BANK,     REG_PM_UART1_MODE,              REG_PM_UART1_MODE_MASK,             BIT8,                   PINMUX_FOR_PM_UART1_MODE_1},
    {PAD_PM_LED1,      PMSLEEP_BANK,     REG_PM_VID_MODE,                REG_PM_VID_MODE_MASK,               BIT12,                  PINMUX_FOR_PM_VID_MODE_1},
    {PAD_PM_LED1,      PMSLEEP_BANK,     REG_PM_VID_MODE,                REG_PM_VID_MODE_MASK,               BIT13|BIT12,            PINMUX_FOR_PM_VID_MODE_3},
    {PAD_PM_LED1,      PMSLEEP_BANK,     REG_PM_LED_MODE,                REG_PM_LED_MODE_MASK,               BIT4,                   PINMUX_FOR_PM_LED_MODE_1},
    {PAD_PM_LED1,      PMSLEEP_BANK,     REG_PM_PAD_EXT_MODE_10,         REG_PM_PAD_EXT_MODE_10_MASK,        BIT10,                  PINMUX_FOR_PM_PAD_EXT_MODE_10_1},

//    {PAD_ETH_RN,   CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
//    {PAD_ETH_RN,   CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
//    {PAD_ETH_RN,   ETH_BANK,     REG_ETH_GPIO_EN_MODE,              REG_ETH_GPIO_EN_MODE_MASK,             BIT0,                   PINMUX_FOR_ETH_GPIO_EN_MODE},

//    {PAD_ETH_RP,   CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
//    {PAD_ETH_RP,   CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
//    {PAD_ETH_RP,   ETH_BANK,     REG_ETH_GPIO_EN_MODE,              REG_ETH_GPIO_EN_MODE_MASK,             BIT1,                   PINMUX_FOR_ETH_GPIO_EN_MODE},

//    {PAD_ETH_TN,   CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
//    {PAD_ETH_TN,   CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
//    {PAD_ETH_TN,   ETH_BANK,     REG_ETH_GPIO_EN_MODE,              REG_ETH_GPIO_EN_MODE_MASK,             BIT2,                   PINMUX_FOR_ETH_GPIO_EN_MODE},

//    {PAD_ETH_TP,   CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
//    {PAD_ETH_TP,   CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
//    {PAD_ETH_TP,   ETH_BANK,     REG_ETH_GPIO_EN_MODE,              REG_ETH_GPIO_EN_MODE_MASK,             BIT3,                   PINMUX_FOR_ETH_GPIO_EN_MODE},

//    {PAD_SAR_GPIO0,   CHIPTOP_BANK,     REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,                   PINMUX_FOR_TEST_IN_MODE_2},
//    {PAD_SAR_GPIO0,   CHIPTOP_BANK,     REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,                   PINMUX_FOR_TEST_OUT_MODE_2},
//    {PAD_SAR_GPIO0, PM_SAR_BANK, REG_SAR_GPIO0_GPIO_MODE,        REG_SAR_GPIO0_GPIO_MODE_MASK,       BIT0,               PINMUX_FOR_GPIO_MODE},
//    {PAD_SAR_GPIO0, PM_SAR_BANK, REG_SAR_GPIO0_GPIO_MODE,        REG_SAR_GPIO0_GPIO_MODE_MASK,       BIT0,               PINMUX_FOR_GPIO_MODE},
//    {PAD_SAR_GPIO1, PM_SAR_BANK, REG_SAR_GPIO1_GPIO_MODE,        REG_SAR_GPIO1_GPIO_MODE_MASK,       BIT1,               PINMUX_FOR_GPIO_MODE},
//    {PAD_SAR_GPIO2, PM_SAR_BANK, REG_SAR_GPIO2_GPIO_MODE,        REG_SAR_GPIO2_GPIO_MODE_MASK,       BIT2,               PINMUX_FOR_GPIO_MODE},
//    {PAD_SAR_GPIO3, PM_SAR_BANK, REG_SAR_GPIO3_GPIO_MODE,        REG_SAR_GPIO3_GPIO_MODE_MASK,       BIT3,               PINMUX_FOR_GPIO_MODE},
//    {PAD_SAR_GPIO4, PM_SAR_BANK, REG_SAR_GPIO4_GPIO_MODE,        REG_SAR_GPIO4_GPIO_MODE_MASK,       BIT4,               PINMUX_FOR_GPIO_MODE},
};

static const ST_PadModeInfo m_stPadModeInfoTbl[] =
{
    {"GPIO",    0,                                              0},
//    {"AUTO_GPIO_EN_1",       _RIUA_16BIT(CHIPTOP_BANK,REG_AUTO_GPIO_EN),                REG_AUTO_GPIO_EN_MASK},
    {"EJ_MODE_1",            _RIUA_16BIT(PADTOP_BANK,REG_EJ_MODE),                      REG_EJ_MODE_MASK},
    {"EJ_MODE_2",            _RIUA_16BIT(PADTOP_BANK,REG_EJ_MODE),                      REG_EJ_MODE_MASK},
    {"EJ_MODE_3",            _RIUA_16BIT(PADTOP_BANK,REG_EJ_MODE),                      REG_EJ_MODE_MASK},
    {"EJ_MODE_4",            _RIUA_16BIT(PADTOP_BANK,REG_EJ_MODE),                      REG_EJ_MODE_MASK},
    {"ALL_PAD_IN_1",         _RIUA_16BIT(CHIPTOP_BANK,REG_ALL_PAD_IN),                  REG_ALL_PAD_IN_MASK},
    {"TEST_IN_MODE_1",       _RIUA_16BIT(CHIPTOP_BANK,REG_TEST_IN_MODE),                REG_TEST_IN_MODE_MASK},
    {"TEST_IN_MODE_2",       _RIUA_16BIT(CHIPTOP_BANK,REG_TEST_IN_MODE),                REG_TEST_IN_MODE_MASK},
    {"TEST_IN_MODE_3",       _RIUA_16BIT(CHIPTOP_BANK,REG_TEST_IN_MODE),                REG_TEST_IN_MODE_MASK},
    {"TEST_OUT_MODE_1",      _RIUA_16BIT(CHIPTOP_BANK,REG_TEST_OUT_MODE),               REG_TEST_OUT_MODE_MASK},
    {"TEST_OUT_MODE_2",      _RIUA_16BIT(CHIPTOP_BANK,REG_TEST_OUT_MODE),               REG_TEST_OUT_MODE_MASK},
    {"TEST_OUT_MODE_3",      _RIUA_16BIT(CHIPTOP_BANK,REG_TEST_OUT_MODE),               REG_TEST_OUT_MODE_MASK},
    {"I2C0_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                    REG_I2C0_MODE_MASK},
    {"I2C0_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                    REG_I2C0_MODE_MASK},
    {"I2C0_MODE_3",          _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                    REG_I2C0_MODE_MASK},
    {"I2C0_MODE_4",          _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                    REG_I2C0_MODE_MASK},
    {"I2C0_MODE_5",          _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                    REG_I2C0_MODE_MASK},
    {"I2C0_MODE_6",          _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                    REG_I2C0_MODE_MASK},
    {"I2C0_MODE_7",          _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                    REG_I2C0_MODE_MASK},
    {"I2C0_MODE_8",          _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                    REG_I2C0_MODE_MASK},
    {"I2C0_MODE_9",          _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                    REG_I2C0_MODE_MASK},
    {"I2C0_MODE_10",         _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                    REG_I2C0_MODE_MASK},
    {"I2C0_MODE_11",         _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                    REG_I2C0_MODE_MASK},
    {"I2C0_MODE_12",         _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                    REG_I2C0_MODE_MASK},
    {"I2C0_MODE_13",         _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                    REG_I2C0_MODE_MASK},
    {"I2C0_MODE_14",         _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                    REG_I2C0_MODE_MASK},
    {"I2C1_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_I2C1_MODE),                    REG_I2C1_MODE_MASK},
    {"I2C1_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_I2C1_MODE),                    REG_I2C1_MODE_MASK},
    {"I2C1_MODE_3",          _RIUA_16BIT(PADTOP_BANK,REG_I2C1_MODE),                    REG_I2C1_MODE_MASK},
    {"I2C1_MODE_4",          _RIUA_16BIT(PADTOP_BANK,REG_I2C1_MODE),                    REG_I2C1_MODE_MASK},
    {"I2C1_MODE_5",          _RIUA_16BIT(PADTOP_BANK,REG_I2C1_MODE),                    REG_I2C1_MODE_MASK},
    {"I2C1_MODE_6",          _RIUA_16BIT(PADTOP_BANK,REG_I2C1_MODE),                    REG_I2C1_MODE_MASK},
    {"I2C1_MODE_7",          _RIUA_16BIT(PADTOP_BANK,REG_I2C1_MODE),                    REG_I2C1_MODE_MASK},
    {"I2C1_MODE_8",          _RIUA_16BIT(PADTOP_BANK,REG_I2C1_MODE),                    REG_I2C1_MODE_MASK},
    {"I2C1_MODE_9",          _RIUA_16BIT(PADTOP_BANK,REG_I2C1_MODE),                    REG_I2C1_MODE_MASK},
    {"I2C1_MODE_10",         _RIUA_16BIT(PADTOP_BANK,REG_I2C1_MODE),                    REG_I2C1_MODE_MASK},
    {"I2C1_MODE_11",         _RIUA_16BIT(PADTOP_BANK,REG_I2C1_MODE),                    REG_I2C1_MODE_MASK},
    {"SR0_MIPI_MODE_1",      _RIUA_16BIT(PADTOP_BANK,REG_SR0_MIPI_MODE),                REG_SR0_MIPI_MODE_MASK},
    {"SR1_MIPI_MODE_1",      _RIUA_16BIT(PADTOP_BANK,REG_SR1_MIPI_MODE),                REG_SR1_MIPI_MODE_MASK},
    {"SR1_MIPI_MODE_2",      _RIUA_16BIT(PADTOP_BANK,REG_SR1_MIPI_MODE),                REG_SR1_MIPI_MODE_MASK},
    {"ISP_IR_MODE_1",        _RIUA_16BIT(PADTOP_BANK,REG_ISP_IR_MODE),                  REG_ISP_IR_MODE_MASK},
    {"ISP_IR_MODE_2",        _RIUA_16BIT(PADTOP_BANK,REG_ISP_IR_MODE),                  REG_ISP_IR_MODE_MASK},
    {"ISP_IR_MODE_3",        _RIUA_16BIT(PADTOP_BANK,REG_ISP_IR_MODE),                  REG_ISP_IR_MODE_MASK},
    {"SR0_CTRL_MODE_1",      _RIUA_16BIT(PADTOP_BANK,REG_SR0_CTRL_MODE),                REG_SR0_CTRL_MODE_MASK},
    {"SR0_CTRL_MODE_2",      _RIUA_16BIT(PADTOP_BANK,REG_SR0_CTRL_MODE),                REG_SR0_CTRL_MODE_MASK},
    {"SR0_MCLK_MODE_1",      _RIUA_16BIT(PADTOP_BANK,REG_SR0_MCLK_MODE),                REG_SR0_MCLK_MODE_MASK},
    {"SR0_MCLK_MODE_2",      _RIUA_16BIT(PADTOP_BANK,REG_SR0_MCLK_MODE),                REG_SR0_MCLK_MODE_MASK},
    {"SR0_PDN_MODE_1",       _RIUA_16BIT(PADTOP_BANK,REG_SR0_PDN_MODE),                 REG_SR0_PDN_MODE_MASK},
    {"SR0_RST_MODE_1",       _RIUA_16BIT(PADTOP_BANK,REG_SR0_RST_MODE),                 REG_SR0_RST_MODE_MASK},
    {"SR0_RST_MODE_2",       _RIUA_16BIT(PADTOP_BANK,REG_SR0_RST_MODE),                 REG_SR0_RST_MODE_MASK},
    {"SR1_CTRL_MODE_1",      _RIUA_16BIT(PADTOP_BANK,REG_SR1_CTRL_MODE),                REG_SR1_CTRL_MODE_MASK},
    {"SR1_CTRL_MODE_2",      _RIUA_16BIT(PADTOP_BANK,REG_SR1_CTRL_MODE),                REG_SR1_CTRL_MODE_MASK},
    {"SR1_MCLK_MODE_1",      _RIUA_16BIT(PADTOP_BANK,REG_SR1_MCLK_MODE),                REG_SR1_MCLK_MODE_MASK},
    {"SR1_MCLK_MODE_2",      _RIUA_16BIT(PADTOP_BANK,REG_SR1_MCLK_MODE),                REG_SR1_MCLK_MODE_MASK},
    {"SR1_PDN_MODE_1",       _RIUA_16BIT(PADTOP_BANK,REG_SR1_PDN_MODE),                 REG_SR1_PDN_MODE_MASK},
    {"SR1_PDN_MODE_2",       _RIUA_16BIT(PADTOP_BANK,REG_SR1_PDN_MODE),                 REG_SR1_PDN_MODE_MASK},
    {"SR1_RST_MODE_1",       _RIUA_16BIT(PADTOP_BANK,REG_SR1_RST_MODE),                 REG_SR1_RST_MODE_MASK},
    {"SR1_RST_MODE_2",       _RIUA_16BIT(PADTOP_BANK,REG_SR1_RST_MODE),                 REG_SR1_RST_MODE_MASK},
    {"SR0_BT601_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_SR0_BT601_MODE),               REG_SR0_BT601_MODE_MASK},
    {"SR0_BT601_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_SR0_BT601_MODE),               REG_SR0_BT601_MODE_MASK},
    {"SR0_BT601_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_SR0_BT601_MODE),               REG_SR0_BT601_MODE_MASK},
    {"SR0_BT656_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_SR0_BT656_MODE),               REG_SR0_BT656_MODE_MASK},
    {"SR0_BT656_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_SR0_BT656_MODE),               REG_SR0_BT656_MODE_MASK},
    {"SR0_BT656_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_SR0_BT656_MODE),               REG_SR0_BT656_MODE_MASK},
    {"SR0_BT656_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_SR0_BT656_MODE),               REG_SR0_BT656_MODE_MASK},
    {"ETH0_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_ETH0_MODE),                    REG_ETH0_MODE_MASK},
    {"ETH0_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_ETH0_MODE),                    REG_ETH0_MODE_MASK},
    {"ETH0_MODE_3",          _RIUA_16BIT(PADTOP_BANK,REG_ETH0_MODE),                    REG_ETH0_MODE_MASK},
    {"ETH0_MODE_4",          _RIUA_16BIT(PADTOP_BANK,REG_ETH0_MODE),                    REG_ETH0_MODE_MASK},
    {"ETH0_MODE_5",          _RIUA_16BIT(PADTOP_BANK,REG_ETH0_MODE),                    REG_ETH0_MODE_MASK},
    {"ETH0_MODE_6",          _RIUA_16BIT(PADTOP_BANK,REG_ETH0_MODE),                    REG_ETH0_MODE_MASK},
    {"ETH0_MODE_7",          _RIUA_16BIT(PADTOP_BANK,REG_ETH0_MODE),                    REG_ETH0_MODE_MASK},
    {"ETH0_MODE_8",          _RIUA_16BIT(PADTOP_BANK,REG_ETH0_MODE),                    REG_ETH0_MODE_MASK},
    {"ETH0_MODE_9",          _RIUA_16BIT(PADTOP_BANK,REG_ETH0_MODE),                    REG_ETH0_MODE_MASK},
    {"ETH0_MODE_10",         _RIUA_16BIT(PADTOP_BANK,REG_ETH0_MODE),                    REG_ETH0_MODE_MASK},
    {"ETH0_MODE_11",         _RIUA_16BIT(PADTOP_BANK,REG_ETH0_MODE),                    REG_ETH0_MODE_MASK},
    {"ETH0_MODE_12",         _RIUA_16BIT(PADTOP_BANK,REG_ETH0_MODE),                    REG_ETH0_MODE_MASK},
    {"TTL16_MODE_1",         _RIUA_16BIT(PADTOP_BANK,REG_TTL16_MODE),                   REG_TTL16_MODE_MASK},
    {"TTL16_MODE_2",         _RIUA_16BIT(PADTOP_BANK,REG_TTL16_MODE),                   REG_TTL16_MODE_MASK},
    {"TTL16_MODE_3",         _RIUA_16BIT(PADTOP_BANK,REG_TTL16_MODE),                   REG_TTL16_MODE_MASK},
    {"TTL16_MODE_4",         _RIUA_16BIT(PADTOP_BANK,REG_TTL16_MODE),                   REG_TTL16_MODE_MASK},
    {"TTL18_MODE_1",         _RIUA_16BIT(PADTOP_BANK,REG_TTL18_MODE),                   REG_TTL18_MODE_MASK},
    {"TTL18_MODE_2",         _RIUA_16BIT(PADTOP_BANK,REG_TTL18_MODE),                   REG_TTL18_MODE_MASK},
    {"TTL18_MODE_3",         _RIUA_16BIT(PADTOP_BANK,REG_TTL18_MODE),                   REG_TTL18_MODE_MASK},
    {"TTL24_MODE_1",         _RIUA_16BIT(PADTOP_BANK,REG_TTL24_MODE),                   REG_TTL24_MODE_MASK},
    {"TTL24_MODE_2",         _RIUA_16BIT(PADTOP_BANK,REG_TTL24_MODE),                   REG_TTL24_MODE_MASK},
    {"TTL24_MODE_3",         _RIUA_16BIT(PADTOP_BANK,REG_TTL24_MODE),                   REG_TTL24_MODE_MASK},
    {"TTL24_MODE_4",         _RIUA_16BIT(PADTOP_BANK,REG_TTL24_MODE),                   REG_TTL24_MODE_MASK},
    {"TTL24_MODE_5",         _RIUA_16BIT(PADTOP_BANK,REG_TTL24_MODE),                   REG_TTL24_MODE_MASK},
    {"RGB8_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_RGB8_MODE),                    REG_RGB8_MODE_MASK},
    {"RGB8_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_RGB8_MODE),                    REG_RGB8_MODE_MASK},
    {"RGB8_MODE_3",          _RIUA_16BIT(PADTOP_BANK,REG_RGB8_MODE),                    REG_RGB8_MODE_MASK},
    {"BT656_OUT_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_BT656_OUT_MODE),               REG_BT656_OUT_MODE_MASK},
    {"BT656_OUT_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_BT656_OUT_MODE),               REG_BT656_OUT_MODE_MASK},
    {"BT656_OUT_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_BT656_OUT_MODE),               REG_BT656_OUT_MODE_MASK},
    {"PSPI0_SR_MODE_1",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI0_SR_MODE),                REG_PSPI0_SR_MODE_MASK},
    {"PSPI0_SR_MODE_2",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI0_SR_MODE),                REG_PSPI0_SR_MODE_MASK},
    {"PSPI0_SR_MODE_3",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI0_SR_MODE),                REG_PSPI0_SR_MODE_MASK},
    {"PSPI0_SR_MODE_4",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI0_SR_MODE),                REG_PSPI0_SR_MODE_MASK},
    {"PSPI0_SR_MODE_5",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI0_SR_MODE),                REG_PSPI0_SR_MODE_MASK},
    {"PSPI0_SR_MODE_6",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI0_SR_MODE),                REG_PSPI0_SR_MODE_MASK},
    {"PSPI0_G_MODE_1",       _RIUA_16BIT(PADTOP_BANK,REG_PSPI0_G_MODE),                 REG_PSPI0_G_MODE_MASK},
    {"PSPI0_G_MODE_2",       _RIUA_16BIT(PADTOP_BANK,REG_PSPI0_G_MODE),                 REG_PSPI0_G_MODE_MASK},
    {"SPI0_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_SPI0_MODE),                    REG_SPI0_MODE_MASK},
    {"SPI0_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_SPI0_MODE),                    REG_SPI0_MODE_MASK},
    {"SPI0_MODE_3",          _RIUA_16BIT(PADTOP_BANK,REG_SPI0_MODE),                    REG_SPI0_MODE_MASK},
    {"SPI0_MODE_4",          _RIUA_16BIT(PADTOP_BANK,REG_SPI0_MODE),                    REG_SPI0_MODE_MASK},
    {"SPI0_MODE_5",          _RIUA_16BIT(PADTOP_BANK,REG_SPI0_MODE),                    REG_SPI0_MODE_MASK},
    {"PSPI1_PL_MODE_1",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_PL_MODE),                REG_PSPI1_PL_MODE_MASK},
    {"PSPI1_PL_MODE_2",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_PL_MODE),                REG_PSPI1_PL_MODE_MASK},
    {"PSPI1_PL_MODE_3",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_PL_MODE),                REG_PSPI1_PL_MODE_MASK},
    {"PSPI1_PL_MODE_4",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_PL_MODE),                REG_PSPI1_PL_MODE_MASK},
    {"PSPI1_PL_MODE_5",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_PL_MODE),                REG_PSPI1_PL_MODE_MASK},
    {"PSPI1_PL_MODE_6",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_PL_MODE),                REG_PSPI1_PL_MODE_MASK},
    {"PSPI1_PL_MODE_7",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_PL_MODE),                REG_PSPI1_PL_MODE_MASK},
    {"PSPI1_PL_MODE_8",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_PL_MODE),                REG_PSPI1_PL_MODE_MASK},
    {"PSPI1_PL_MODE_9",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_PL_MODE),                REG_PSPI1_PL_MODE_MASK},
    {"PSPI1_CS2_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_CS2_MODE),               REG_PSPI1_CS2_MODE_MASK},
    {"PSPI1_CS2_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_CS2_MODE),               REG_PSPI1_CS2_MODE_MASK},
    {"PSPI1_CS2_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_CS2_MODE),               REG_PSPI1_CS2_MODE_MASK},
    {"PSPI1_CS2_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_CS2_MODE),               REG_PSPI1_CS2_MODE_MASK},
    {"PSPI1_CS2_MODE_5",     _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_CS2_MODE),               REG_PSPI1_CS2_MODE_MASK},
    {"PSPI1_CS2_MODE_6",     _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_CS2_MODE),               REG_PSPI1_CS2_MODE_MASK},
    {"PSPI1_CS2_MODE_7",     _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_CS2_MODE),               REG_PSPI1_CS2_MODE_MASK},
    {"PSPI1_CS2_MODE_8",     _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_CS2_MODE),               REG_PSPI1_CS2_MODE_MASK},
    {"PSPI1_CS2_MODE_9",     _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_CS2_MODE),               REG_PSPI1_CS2_MODE_MASK},
    {"PSPI1_CS2_MODE_10",    _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_CS2_MODE),               REG_PSPI1_CS2_MODE_MASK},
    {"PSPI1_TE_MODE_1",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_TE_MODE),                REG_PSPI1_TE_MODE_MASK},
    {"PSPI1_TE_MODE_2",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_TE_MODE),                REG_PSPI1_TE_MODE_MASK},
    {"PSPI1_TE_MODE_3",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_TE_MODE),                REG_PSPI1_TE_MODE_MASK},
    {"PSPI1_TE_MODE_4",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_TE_MODE),                REG_PSPI1_TE_MODE_MASK},
    {"PSPI1_TE_MODE_5",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_TE_MODE),                REG_PSPI1_TE_MODE_MASK},
    {"PSPI1_TE_MODE_6",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_TE_MODE),                REG_PSPI1_TE_MODE_MASK},
    {"PSPI1_TE_MODE_7",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_TE_MODE),                REG_PSPI1_TE_MODE_MASK},
    {"PSPI1_TE_MODE_8",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_TE_MODE),                REG_PSPI1_TE_MODE_MASK},
    {"PSPI1_TE_MODE_9",      _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_TE_MODE),                REG_PSPI1_TE_MODE_MASK},
    {"PSPI1_TE_MODE_10",     _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_TE_MODE),                REG_PSPI1_TE_MODE_MASK},
    {"PSPI1_G_MODE_1",       _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_G_MODE),                 REG_PSPI1_G_MODE_MASK},
    {"PSPI1_G_MODE_2",       _RIUA_16BIT(PADTOP_BANK,REG_PSPI1_G_MODE),                 REG_PSPI1_G_MODE_MASK},
    {"SPI1_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_SPI1_MODE),                    REG_SPI1_MODE_MASK},
    {"SPI1_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_SPI1_MODE),                    REG_SPI1_MODE_MASK},
    {"SPI1_MODE_3",          _RIUA_16BIT(PADTOP_BANK,REG_SPI1_MODE),                    REG_SPI1_MODE_MASK},
    {"SPI1_MODE_4",          _RIUA_16BIT(PADTOP_BANK,REG_SPI1_MODE),                    REG_SPI1_MODE_MASK},
    {"SPI1_MODE_5",          _RIUA_16BIT(PADTOP_BANK,REG_SPI1_MODE),                    REG_SPI1_MODE_MASK},
    {"SPI1_MODE_6",          _RIUA_16BIT(PADTOP_BANK,REG_SPI1_MODE),                    REG_SPI1_MODE_MASK},
    {"SPI1_MODE_7",          _RIUA_16BIT(PADTOP_BANK,REG_SPI1_MODE),                    REG_SPI1_MODE_MASK},
    {"SPI1_MODE_8",          _RIUA_16BIT(PADTOP_BANK,REG_SPI1_MODE),                    REG_SPI1_MODE_MASK},
    {"SPI1_CS2_MODE_1",      _RIUA_16BIT(PADTOP_BANK,REG_SPI1_CS2_MODE),                REG_SPI1_CS2_MODE_MASK},
    {"SPI1_CS2_MODE_2",      _RIUA_16BIT(PADTOP_BANK,REG_SPI1_CS2_MODE),                REG_SPI1_CS2_MODE_MASK},
    {"SPI1_CS2_MODE_3",      _RIUA_16BIT(PADTOP_BANK,REG_SPI1_CS2_MODE),                REG_SPI1_CS2_MODE_MASK},
    {"SPI1_CS2_MODE_4",      _RIUA_16BIT(PADTOP_BANK,REG_SPI1_CS2_MODE),                REG_SPI1_CS2_MODE_MASK},
    {"SPI1_CS2_MODE_5",      _RIUA_16BIT(PADTOP_BANK,REG_SPI1_CS2_MODE),                REG_SPI1_CS2_MODE_MASK},
    {"SPI1_CS2_MODE_6",      _RIUA_16BIT(PADTOP_BANK,REG_SPI1_CS2_MODE),                REG_SPI1_CS2_MODE_MASK},
    {"SDIO_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_SDIO_MODE),                    REG_SDIO_MODE_MASK},
    {"SDIO_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_SDIO_MODE),                    REG_SDIO_MODE_MASK},
    {"SD_CDZ_MODE_1",        _RIUA_16BIT(PADTOP_BANK,REG_SD_CDZ_MODE),                  REG_SD_CDZ_MODE_MASK},
    {"SD_CDZ_MODE_2",        _RIUA_16BIT(PADTOP_BANK,REG_SD_CDZ_MODE),                  REG_SD_CDZ_MODE_MASK},
    {"KEY_READ0_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ0_MODE),               REG_KEY_READ0_MODE_MASK},
    {"KEY_READ0_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ0_MODE),               REG_KEY_READ0_MODE_MASK},
    {"KEY_READ0_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ0_MODE),               REG_KEY_READ0_MODE_MASK},
    {"KEY_READ0_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ0_MODE),               REG_KEY_READ0_MODE_MASK},
    {"KEY_READ0_MODE_5",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ0_MODE),               REG_KEY_READ0_MODE_MASK},
    {"KEY_READ0_MODE_6",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ0_MODE),               REG_KEY_READ0_MODE_MASK},
    {"KEY_READ0_MODE_7",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ0_MODE),               REG_KEY_READ0_MODE_MASK},
    {"KEY_READ0_MODE_8",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ0_MODE),               REG_KEY_READ0_MODE_MASK},
    {"KEY_READ0_MODE_9",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ0_MODE),               REG_KEY_READ0_MODE_MASK},
    {"KEY_READ0_MODE_10",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ0_MODE),               REG_KEY_READ0_MODE_MASK},
    {"KEY_READ0_MODE_11",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ0_MODE),               REG_KEY_READ0_MODE_MASK},
    {"KEY_READ0_MODE_12",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ0_MODE),               REG_KEY_READ0_MODE_MASK},
    {"KEY_READ0_MODE_13",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ0_MODE),               REG_KEY_READ0_MODE_MASK},
    {"KEY_READ0_MODE_14",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ0_MODE),               REG_KEY_READ0_MODE_MASK},
    {"KEY_READ1_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ1_MODE),               REG_KEY_READ1_MODE_MASK},
    {"KEY_READ1_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ1_MODE),               REG_KEY_READ1_MODE_MASK},
    {"KEY_READ1_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ1_MODE),               REG_KEY_READ1_MODE_MASK},
    {"KEY_READ1_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ1_MODE),               REG_KEY_READ1_MODE_MASK},
    {"KEY_READ1_MODE_5",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ1_MODE),               REG_KEY_READ1_MODE_MASK},
    {"KEY_READ1_MODE_6",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ1_MODE),               REG_KEY_READ1_MODE_MASK},
    {"KEY_READ1_MODE_7",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ1_MODE),               REG_KEY_READ1_MODE_MASK},
    {"KEY_READ1_MODE_8",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ1_MODE),               REG_KEY_READ1_MODE_MASK},
    {"KEY_READ1_MODE_9",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ1_MODE),               REG_KEY_READ1_MODE_MASK},
    {"KEY_READ1_MODE_10",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ1_MODE),               REG_KEY_READ1_MODE_MASK},
    {"KEY_READ1_MODE_11",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ1_MODE),               REG_KEY_READ1_MODE_MASK},
    {"KEY_READ1_MODE_12",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ1_MODE),               REG_KEY_READ1_MODE_MASK},
    {"KEY_READ1_MODE_13",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ1_MODE),               REG_KEY_READ1_MODE_MASK},
    {"KEY_READ1_MODE_14",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ1_MODE),               REG_KEY_READ1_MODE_MASK},
    {"KEY_READ2_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ2_MODE),               REG_KEY_READ2_MODE_MASK},
    {"KEY_READ2_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ2_MODE),               REG_KEY_READ2_MODE_MASK},
    {"KEY_READ2_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ2_MODE),               REG_KEY_READ2_MODE_MASK},
    {"KEY_READ2_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ2_MODE),               REG_KEY_READ2_MODE_MASK},
    {"KEY_READ2_MODE_5",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ2_MODE),               REG_KEY_READ2_MODE_MASK},
    {"KEY_READ2_MODE_6",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ2_MODE),               REG_KEY_READ2_MODE_MASK},
    {"KEY_READ2_MODE_7",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ2_MODE),               REG_KEY_READ2_MODE_MASK},
    {"KEY_READ2_MODE_8",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ2_MODE),               REG_KEY_READ2_MODE_MASK},
    {"KEY_READ2_MODE_9",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ2_MODE),               REG_KEY_READ2_MODE_MASK},
    {"KEY_READ2_MODE_10",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ2_MODE),               REG_KEY_READ2_MODE_MASK},
    {"KEY_READ2_MODE_11",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ2_MODE),               REG_KEY_READ2_MODE_MASK},
    {"KEY_READ2_MODE_12",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ2_MODE),               REG_KEY_READ2_MODE_MASK},
    {"KEY_READ2_MODE_13",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ2_MODE),               REG_KEY_READ2_MODE_MASK},
    {"KEY_READ2_MODE_14",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ2_MODE),               REG_KEY_READ2_MODE_MASK},
    {"KEY_READ3_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ3_MODE),               REG_KEY_READ3_MODE_MASK},
    {"KEY_READ3_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ3_MODE),               REG_KEY_READ3_MODE_MASK},
    {"KEY_READ3_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ3_MODE),               REG_KEY_READ3_MODE_MASK},
    {"KEY_READ3_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ3_MODE),               REG_KEY_READ3_MODE_MASK},
    {"KEY_READ3_MODE_5",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ3_MODE),               REG_KEY_READ3_MODE_MASK},
    {"KEY_READ3_MODE_6",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ3_MODE),               REG_KEY_READ3_MODE_MASK},
    {"KEY_READ3_MODE_7",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ3_MODE),               REG_KEY_READ3_MODE_MASK},
    {"KEY_READ3_MODE_8",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ3_MODE),               REG_KEY_READ3_MODE_MASK},
    {"KEY_READ3_MODE_9",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ3_MODE),               REG_KEY_READ3_MODE_MASK},
    {"KEY_READ3_MODE_10",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ3_MODE),               REG_KEY_READ3_MODE_MASK},
    {"KEY_READ3_MODE_11",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ3_MODE),               REG_KEY_READ3_MODE_MASK},
    {"KEY_READ3_MODE_12",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ3_MODE),               REG_KEY_READ3_MODE_MASK},
    {"KEY_READ3_MODE_13",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ3_MODE),               REG_KEY_READ3_MODE_MASK},
    {"KEY_READ3_MODE_14",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ3_MODE),               REG_KEY_READ3_MODE_MASK},
    {"KEY_READ4_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ4_MODE),               REG_KEY_READ4_MODE_MASK},
    {"KEY_READ4_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ4_MODE),               REG_KEY_READ4_MODE_MASK},
    {"KEY_READ4_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ4_MODE),               REG_KEY_READ4_MODE_MASK},
    {"KEY_READ4_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ4_MODE),               REG_KEY_READ4_MODE_MASK},
    {"KEY_READ4_MODE_5",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ4_MODE),               REG_KEY_READ4_MODE_MASK},
    {"KEY_READ4_MODE_6",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ4_MODE),               REG_KEY_READ4_MODE_MASK},
    {"KEY_READ4_MODE_7",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ4_MODE),               REG_KEY_READ4_MODE_MASK},
    {"KEY_READ4_MODE_8",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ4_MODE),               REG_KEY_READ4_MODE_MASK},
    {"KEY_READ4_MODE_9",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ4_MODE),               REG_KEY_READ4_MODE_MASK},
    {"KEY_READ4_MODE_10",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ4_MODE),               REG_KEY_READ4_MODE_MASK},
    {"KEY_READ4_MODE_11",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ4_MODE),               REG_KEY_READ4_MODE_MASK},
    {"KEY_READ4_MODE_12",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ4_MODE),               REG_KEY_READ4_MODE_MASK},
    {"KEY_READ4_MODE_13",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ4_MODE),               REG_KEY_READ4_MODE_MASK},
    {"KEY_READ4_MODE_14",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ4_MODE),               REG_KEY_READ4_MODE_MASK},
    {"KEY_READ5_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ5_MODE),               REG_KEY_READ5_MODE_MASK},
    {"KEY_READ5_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ5_MODE),               REG_KEY_READ5_MODE_MASK},
    {"KEY_READ5_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ5_MODE),               REG_KEY_READ5_MODE_MASK},
    {"KEY_READ5_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ5_MODE),               REG_KEY_READ5_MODE_MASK},
    {"KEY_READ5_MODE_5",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ5_MODE),               REG_KEY_READ5_MODE_MASK},
    {"KEY_READ5_MODE_6",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ5_MODE),               REG_KEY_READ5_MODE_MASK},
    {"KEY_READ5_MODE_7",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ5_MODE),               REG_KEY_READ5_MODE_MASK},
    {"KEY_READ5_MODE_8",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ5_MODE),               REG_KEY_READ5_MODE_MASK},
    {"KEY_READ5_MODE_9",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ5_MODE),               REG_KEY_READ5_MODE_MASK},
    {"KEY_READ5_MODE_10",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ5_MODE),               REG_KEY_READ5_MODE_MASK},
    {"KEY_READ5_MODE_11",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ5_MODE),               REG_KEY_READ5_MODE_MASK},
    {"KEY_READ5_MODE_12",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ5_MODE),               REG_KEY_READ5_MODE_MASK},
    {"KEY_READ5_MODE_13",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ5_MODE),               REG_KEY_READ5_MODE_MASK},
    {"KEY_READ5_MODE_14",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ5_MODE),               REG_KEY_READ5_MODE_MASK},
    {"KEY_READ6_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ6_MODE),               REG_KEY_READ6_MODE_MASK},
    {"KEY_READ6_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ6_MODE),               REG_KEY_READ6_MODE_MASK},
    {"KEY_READ6_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ6_MODE),               REG_KEY_READ6_MODE_MASK},
    {"KEY_READ6_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ6_MODE),               REG_KEY_READ6_MODE_MASK},
    {"KEY_READ6_MODE_5",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ6_MODE),               REG_KEY_READ6_MODE_MASK},
    {"KEY_READ6_MODE_6",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ6_MODE),               REG_KEY_READ6_MODE_MASK},
    {"KEY_READ6_MODE_7",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ6_MODE),               REG_KEY_READ6_MODE_MASK},
    {"KEY_READ6_MODE_8",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ6_MODE),               REG_KEY_READ6_MODE_MASK},
    {"KEY_READ6_MODE_9",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ6_MODE),               REG_KEY_READ6_MODE_MASK},
    {"KEY_READ6_MODE_10",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ6_MODE),               REG_KEY_READ6_MODE_MASK},
    {"KEY_READ6_MODE_11",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ6_MODE),               REG_KEY_READ6_MODE_MASK},
    {"KEY_READ6_MODE_12",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ6_MODE),               REG_KEY_READ6_MODE_MASK},
    {"KEY_READ6_MODE_13",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ6_MODE),               REG_KEY_READ6_MODE_MASK},
    {"KEY_READ6_MODE_14",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_READ6_MODE),               REG_KEY_READ6_MODE_MASK},
    {"KEY_SCAN0_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN0_MODE),               REG_KEY_SCAN0_MODE_MASK},
    {"KEY_SCAN0_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN0_MODE),               REG_KEY_SCAN0_MODE_MASK},
    {"KEY_SCAN0_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN0_MODE),               REG_KEY_SCAN0_MODE_MASK},
    {"KEY_SCAN0_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN0_MODE),               REG_KEY_SCAN0_MODE_MASK},
    {"KEY_SCAN0_MODE_5",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN0_MODE),               REG_KEY_SCAN0_MODE_MASK},
    {"KEY_SCAN0_MODE_6",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN0_MODE),               REG_KEY_SCAN0_MODE_MASK},
    {"KEY_SCAN0_MODE_7",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN0_MODE),               REG_KEY_SCAN0_MODE_MASK},
    {"KEY_SCAN0_MODE_8",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN0_MODE),               REG_KEY_SCAN0_MODE_MASK},
    {"KEY_SCAN0_MODE_9",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN0_MODE),               REG_KEY_SCAN0_MODE_MASK},
    {"KEY_SCAN0_MODE_10",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN0_MODE),               REG_KEY_SCAN0_MODE_MASK},
    {"KEY_SCAN0_MODE_11",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN0_MODE),               REG_KEY_SCAN0_MODE_MASK},
    {"KEY_SCAN0_MODE_12",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN0_MODE),               REG_KEY_SCAN0_MODE_MASK},
    {"KEY_SCAN0_MODE_13",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN0_MODE),               REG_KEY_SCAN0_MODE_MASK},
    {"KEY_SCAN0_MODE_14",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN0_MODE),               REG_KEY_SCAN0_MODE_MASK},
    {"KEY_SCAN1_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN1_MODE),               REG_KEY_SCAN1_MODE_MASK},
    {"KEY_SCAN1_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN1_MODE),               REG_KEY_SCAN1_MODE_MASK},
    {"KEY_SCAN1_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN1_MODE),               REG_KEY_SCAN1_MODE_MASK},
    {"KEY_SCAN1_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN1_MODE),               REG_KEY_SCAN1_MODE_MASK},
    {"KEY_SCAN1_MODE_5",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN1_MODE),               REG_KEY_SCAN1_MODE_MASK},
    {"KEY_SCAN1_MODE_6",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN1_MODE),               REG_KEY_SCAN1_MODE_MASK},
    {"KEY_SCAN1_MODE_7",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN1_MODE),               REG_KEY_SCAN1_MODE_MASK},
    {"KEY_SCAN1_MODE_8",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN1_MODE),               REG_KEY_SCAN1_MODE_MASK},
    {"KEY_SCAN1_MODE_9",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN1_MODE),               REG_KEY_SCAN1_MODE_MASK},
    {"KEY_SCAN1_MODE_10",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN1_MODE),               REG_KEY_SCAN1_MODE_MASK},
    {"KEY_SCAN1_MODE_11",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN1_MODE),               REG_KEY_SCAN1_MODE_MASK},
    {"KEY_SCAN1_MODE_12",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN1_MODE),               REG_KEY_SCAN1_MODE_MASK},
    {"KEY_SCAN1_MODE_13",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN1_MODE),               REG_KEY_SCAN1_MODE_MASK},
    {"KEY_SCAN1_MODE_14",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN1_MODE),               REG_KEY_SCAN1_MODE_MASK},
    {"KEY_SCAN2_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN2_MODE),               REG_KEY_SCAN2_MODE_MASK},
    {"KEY_SCAN2_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN2_MODE),               REG_KEY_SCAN2_MODE_MASK},
    {"KEY_SCAN2_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN2_MODE),               REG_KEY_SCAN2_MODE_MASK},
    {"KEY_SCAN2_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN2_MODE),               REG_KEY_SCAN2_MODE_MASK},
    {"KEY_SCAN2_MODE_5",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN2_MODE),               REG_KEY_SCAN2_MODE_MASK},
    {"KEY_SCAN2_MODE_6",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN2_MODE),               REG_KEY_SCAN2_MODE_MASK},
    {"KEY_SCAN2_MODE_7",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN2_MODE),               REG_KEY_SCAN2_MODE_MASK},
    {"KEY_SCAN2_MODE_8",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN2_MODE),               REG_KEY_SCAN2_MODE_MASK},
    {"KEY_SCAN2_MODE_9",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN2_MODE),               REG_KEY_SCAN2_MODE_MASK},
    {"KEY_SCAN2_MODE_10",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN2_MODE),               REG_KEY_SCAN2_MODE_MASK},
    {"KEY_SCAN2_MODE_11",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN2_MODE),               REG_KEY_SCAN2_MODE_MASK},
    {"KEY_SCAN2_MODE_12",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN2_MODE),               REG_KEY_SCAN2_MODE_MASK},
    {"KEY_SCAN2_MODE_13",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN2_MODE),               REG_KEY_SCAN2_MODE_MASK},
    {"KEY_SCAN2_MODE_14",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN2_MODE),               REG_KEY_SCAN2_MODE_MASK},
    {"KEY_SCAN3_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN3_MODE),               REG_KEY_SCAN3_MODE_MASK},
    {"KEY_SCAN3_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN3_MODE),               REG_KEY_SCAN3_MODE_MASK},
    {"KEY_SCAN3_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN3_MODE),               REG_KEY_SCAN3_MODE_MASK},
    {"KEY_SCAN3_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN3_MODE),               REG_KEY_SCAN3_MODE_MASK},
    {"KEY_SCAN3_MODE_5",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN3_MODE),               REG_KEY_SCAN3_MODE_MASK},
    {"KEY_SCAN3_MODE_6",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN3_MODE),               REG_KEY_SCAN3_MODE_MASK},
    {"KEY_SCAN3_MODE_7",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN3_MODE),               REG_KEY_SCAN3_MODE_MASK},
    {"KEY_SCAN3_MODE_8",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN3_MODE),               REG_KEY_SCAN3_MODE_MASK},
    {"KEY_SCAN3_MODE_9",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN3_MODE),               REG_KEY_SCAN3_MODE_MASK},
    {"KEY_SCAN3_MODE_10",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN3_MODE),               REG_KEY_SCAN3_MODE_MASK},
    {"KEY_SCAN3_MODE_11",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN3_MODE),               REG_KEY_SCAN3_MODE_MASK},
    {"KEY_SCAN3_MODE_12",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN3_MODE),               REG_KEY_SCAN3_MODE_MASK},
    {"KEY_SCAN3_MODE_13",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN3_MODE),               REG_KEY_SCAN3_MODE_MASK},
    {"KEY_SCAN3_MODE_14",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN3_MODE),               REG_KEY_SCAN3_MODE_MASK},
    {"KEY_SCAN4_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN4_MODE),               REG_KEY_SCAN4_MODE_MASK},
    {"KEY_SCAN4_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN4_MODE),               REG_KEY_SCAN4_MODE_MASK},
    {"KEY_SCAN4_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN4_MODE),               REG_KEY_SCAN4_MODE_MASK},
    {"KEY_SCAN4_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN4_MODE),               REG_KEY_SCAN4_MODE_MASK},
    {"KEY_SCAN4_MODE_5",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN4_MODE),               REG_KEY_SCAN4_MODE_MASK},
    {"KEY_SCAN4_MODE_6",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN4_MODE),               REG_KEY_SCAN4_MODE_MASK},
    {"KEY_SCAN4_MODE_7",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN4_MODE),               REG_KEY_SCAN4_MODE_MASK},
    {"KEY_SCAN4_MODE_8",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN4_MODE),               REG_KEY_SCAN4_MODE_MASK},
    {"KEY_SCAN4_MODE_9",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN4_MODE),               REG_KEY_SCAN4_MODE_MASK},
    {"KEY_SCAN4_MODE_10",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN4_MODE),               REG_KEY_SCAN4_MODE_MASK},
    {"KEY_SCAN4_MODE_11",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN4_MODE),               REG_KEY_SCAN4_MODE_MASK},
    {"KEY_SCAN4_MODE_12",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN4_MODE),               REG_KEY_SCAN4_MODE_MASK},
    {"KEY_SCAN4_MODE_13",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN4_MODE),               REG_KEY_SCAN4_MODE_MASK},
    {"KEY_SCAN4_MODE_14",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN4_MODE),               REG_KEY_SCAN4_MODE_MASK},
    {"KEY_SCAN5_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN5_MODE),               REG_KEY_SCAN5_MODE_MASK},
    {"KEY_SCAN5_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN5_MODE),               REG_KEY_SCAN5_MODE_MASK},
    {"KEY_SCAN5_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN5_MODE),               REG_KEY_SCAN5_MODE_MASK},
    {"KEY_SCAN5_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN5_MODE),               REG_KEY_SCAN5_MODE_MASK},
    {"KEY_SCAN5_MODE_5",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN5_MODE),               REG_KEY_SCAN5_MODE_MASK},
    {"KEY_SCAN5_MODE_6",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN5_MODE),               REG_KEY_SCAN5_MODE_MASK},
    {"KEY_SCAN5_MODE_7",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN5_MODE),               REG_KEY_SCAN5_MODE_MASK},
    {"KEY_SCAN5_MODE_8",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN5_MODE),               REG_KEY_SCAN5_MODE_MASK},
    {"KEY_SCAN5_MODE_9",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN5_MODE),               REG_KEY_SCAN5_MODE_MASK},
    {"KEY_SCAN5_MODE_10",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN5_MODE),               REG_KEY_SCAN5_MODE_MASK},
    {"KEY_SCAN5_MODE_11",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN5_MODE),               REG_KEY_SCAN5_MODE_MASK},
    {"KEY_SCAN5_MODE_12",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN5_MODE),               REG_KEY_SCAN5_MODE_MASK},
    {"KEY_SCAN5_MODE_13",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN5_MODE),               REG_KEY_SCAN5_MODE_MASK},
    {"KEY_SCAN5_MODE_14",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN5_MODE),               REG_KEY_SCAN5_MODE_MASK},
    {"KEY_SCAN6_MODE_1",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN6_MODE),               REG_KEY_SCAN6_MODE_MASK},
    {"KEY_SCAN6_MODE_2",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN6_MODE),               REG_KEY_SCAN6_MODE_MASK},
    {"KEY_SCAN6_MODE_3",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN6_MODE),               REG_KEY_SCAN6_MODE_MASK},
    {"KEY_SCAN6_MODE_4",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN6_MODE),               REG_KEY_SCAN6_MODE_MASK},
    {"KEY_SCAN6_MODE_5",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN6_MODE),               REG_KEY_SCAN6_MODE_MASK},
    {"KEY_SCAN6_MODE_6",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN6_MODE),               REG_KEY_SCAN6_MODE_MASK},
    {"KEY_SCAN6_MODE_7",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN6_MODE),               REG_KEY_SCAN6_MODE_MASK},
    {"KEY_SCAN6_MODE_8",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN6_MODE),               REG_KEY_SCAN6_MODE_MASK},
    {"KEY_SCAN6_MODE_9",     _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN6_MODE),               REG_KEY_SCAN6_MODE_MASK},
    {"KEY_SCAN6_MODE_10",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN6_MODE),               REG_KEY_SCAN6_MODE_MASK},
    {"KEY_SCAN6_MODE_11",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN6_MODE),               REG_KEY_SCAN6_MODE_MASK},
    {"KEY_SCAN6_MODE_12",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN6_MODE),               REG_KEY_SCAN6_MODE_MASK},
    {"KEY_SCAN6_MODE_13",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN6_MODE),               REG_KEY_SCAN6_MODE_MASK},
    {"KEY_SCAN6_MODE_14",    _RIUA_16BIT(PADTOP_BANK,REG_KEY_SCAN6_MODE),               REG_KEY_SCAN6_MODE_MASK},
    {"KEY_FIX_MODE_1",       _RIUA_16BIT(PADTOP_BANK,REG_KEY_FIX_MODE),                 REG_KEY_FIX_MODE_MASK},
    {"FUART_MODE_1",         _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                   REG_FUART_MODE_MASK},
    {"FUART_MODE_2",         _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                   REG_FUART_MODE_MASK},
    {"FUART_MODE_3",         _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                   REG_FUART_MODE_MASK},
    {"FUART_MODE_4",         _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                   REG_FUART_MODE_MASK},
    {"FUART_MODE_5",         _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                   REG_FUART_MODE_MASK},
    {"FUART_MODE_6",         _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                   REG_FUART_MODE_MASK},
    {"FUART_MODE_7",         _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                   REG_FUART_MODE_MASK},
    {"FUART_MODE_8",         _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                   REG_FUART_MODE_MASK},
    {"FUART_MODE_9",         _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                   REG_FUART_MODE_MASK},
    {"FUART_MODE_10",        _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                   REG_FUART_MODE_MASK},
    {"FUART_MODE_11",        _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                   REG_FUART_MODE_MASK},
    {"FUART_MODE_12",        _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                   REG_FUART_MODE_MASK},
    {"UART0_MODE_1",         _RIUA_16BIT(PADTOP_BANK,REG_UART0_MODE),                   REG_UART0_MODE_MASK},
    {"UART0_MODE_2",         _RIUA_16BIT(PADTOP_BANK,REG_UART0_MODE),                   REG_UART0_MODE_MASK},
    {"UART0_MODE_3",         _RIUA_16BIT(PADTOP_BANK,REG_UART0_MODE),                   REG_UART0_MODE_MASK},
    {"UART0_MODE_4",         _RIUA_16BIT(PADTOP_BANK,REG_UART0_MODE),                   REG_UART0_MODE_MASK},
    {"UART0_MODE_5",         _RIUA_16BIT(PADTOP_BANK,REG_UART0_MODE),                   REG_UART0_MODE_MASK},
    {"UART1_MODE_1",         _RIUA_16BIT(PADTOP_BANK,REG_UART1_MODE),                   REG_UART1_MODE_MASK},
    {"UART1_MODE_2",         _RIUA_16BIT(PADTOP_BANK,REG_UART1_MODE),                   REG_UART1_MODE_MASK},
    {"UART1_MODE_3",         _RIUA_16BIT(PADTOP_BANK,REG_UART1_MODE),                   REG_UART1_MODE_MASK},
    {"UART1_MODE_4",         _RIUA_16BIT(PADTOP_BANK,REG_UART1_MODE),                   REG_UART1_MODE_MASK},
    {"UART1_MODE_5",         _RIUA_16BIT(PADTOP_BANK,REG_UART1_MODE),                   REG_UART1_MODE_MASK},
    {"UART1_MODE_6",         _RIUA_16BIT(PADTOP_BANK,REG_UART1_MODE),                   REG_UART1_MODE_MASK},
    {"UART1_MODE_7",         _RIUA_16BIT(PADTOP_BANK,REG_UART1_MODE),                   REG_UART1_MODE_MASK},
    {"UART1_MODE_8",         _RIUA_16BIT(PADTOP_BANK,REG_UART1_MODE),                   REG_UART1_MODE_MASK},
    {"UART1_MODE_9",         _RIUA_16BIT(PADTOP_BANK,REG_UART1_MODE),                   REG_UART1_MODE_MASK},
    {"UART2_MODE_1",         _RIUA_16BIT(PADTOP_BANK,REG_UART2_MODE),                   REG_UART2_MODE_MASK},
    {"UART2_MODE_2",         _RIUA_16BIT(PADTOP_BANK,REG_UART2_MODE),                   REG_UART2_MODE_MASK},
    {"UART2_MODE_3",         _RIUA_16BIT(PADTOP_BANK,REG_UART2_MODE),                   REG_UART2_MODE_MASK},
    {"UART2_MODE_4",         _RIUA_16BIT(PADTOP_BANK,REG_UART2_MODE),                   REG_UART2_MODE_MASK},
    {"UART2_MODE_5",         _RIUA_16BIT(PADTOP_BANK,REG_UART2_MODE),                   REG_UART2_MODE_MASK},
    {"UART2_MODE_6",         _RIUA_16BIT(PADTOP_BANK,REG_UART2_MODE),                   REG_UART2_MODE_MASK},
    {"UART2_MODE_7",         _RIUA_16BIT(PADTOP_BANK,REG_UART2_MODE),                   REG_UART2_MODE_MASK},
    {"PWM0_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                    REG_PWM0_MODE_MASK},
    {"PWM0_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                    REG_PWM0_MODE_MASK},
    {"PWM0_MODE_3",          _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                    REG_PWM0_MODE_MASK},
    {"PWM0_MODE_4",          _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                    REG_PWM0_MODE_MASK},
    {"PWM0_MODE_5",          _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                    REG_PWM0_MODE_MASK},
    {"PWM0_MODE_6",          _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                    REG_PWM0_MODE_MASK},
    {"PWM0_MODE_7",          _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                    REG_PWM0_MODE_MASK},
    {"PWM0_MODE_8",          _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                    REG_PWM0_MODE_MASK},
    {"PWM0_MODE_9",          _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                    REG_PWM0_MODE_MASK},
    {"PWM0_MODE_10",         _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                    REG_PWM0_MODE_MASK},
    {"PWM1_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                    REG_PWM1_MODE_MASK},
    {"PWM1_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                    REG_PWM1_MODE_MASK},
    {"PWM1_MODE_3",          _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                    REG_PWM1_MODE_MASK},
    {"PWM1_MODE_4",          _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                    REG_PWM1_MODE_MASK},
    {"PWM1_MODE_5",          _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                    REG_PWM1_MODE_MASK},
    {"PWM1_MODE_6",          _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                    REG_PWM1_MODE_MASK},
    {"PWM1_MODE_7",          _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                    REG_PWM1_MODE_MASK},
    {"PWM1_MODE_8",          _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                    REG_PWM1_MODE_MASK},
    {"PWM1_MODE_9",          _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                    REG_PWM1_MODE_MASK},
    {"PWM1_MODE_10",         _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                    REG_PWM1_MODE_MASK},
    {"PWM2_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                    REG_PWM2_MODE_MASK},
    {"PWM2_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                    REG_PWM2_MODE_MASK},
    {"PWM2_MODE_3",          _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                    REG_PWM2_MODE_MASK},
    {"PWM2_MODE_4",          _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                    REG_PWM2_MODE_MASK},
    {"PWM2_MODE_5",          _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                    REG_PWM2_MODE_MASK},
    {"PWM2_MODE_6",          _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                    REG_PWM2_MODE_MASK},
    {"PWM2_MODE_7",          _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                    REG_PWM2_MODE_MASK},
    {"PWM2_MODE_8",          _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                    REG_PWM2_MODE_MASK},
    {"PWM2_MODE_9",          _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                    REG_PWM2_MODE_MASK},
    {"PWM2_MODE_10",         _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                    REG_PWM2_MODE_MASK},
    {"PWM2_MODE_11",         _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                    REG_PWM2_MODE_MASK},
    {"PWM3_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                    REG_PWM3_MODE_MASK},
    {"PWM3_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                    REG_PWM3_MODE_MASK},
    {"PWM3_MODE_3",          _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                    REG_PWM3_MODE_MASK},
    {"PWM3_MODE_4",          _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                    REG_PWM3_MODE_MASK},
    {"PWM3_MODE_5",          _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                    REG_PWM3_MODE_MASK},
    {"PWM3_MODE_6",          _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                    REG_PWM3_MODE_MASK},
    {"PWM3_MODE_7",          _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                    REG_PWM3_MODE_MASK},
    {"PWM3_MODE_8",          _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                    REG_PWM3_MODE_MASK},
    {"PWM3_MODE_9",          _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                    REG_PWM3_MODE_MASK},
    {"PWM3_MODE_10",         _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                    REG_PWM3_MODE_MASK},
    {"DMIC_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                    REG_DMIC_MODE_MASK},
    {"DMIC_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                    REG_DMIC_MODE_MASK},
    {"DMIC_MODE_3",          _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                    REG_DMIC_MODE_MASK},
    {"DMIC_MODE_4",          _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                    REG_DMIC_MODE_MASK},
    {"DMIC_MODE_5",          _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                    REG_DMIC_MODE_MASK},
    {"DMIC_MODE_6",          _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                    REG_DMIC_MODE_MASK},
    {"DMIC_MODE_7",          _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                    REG_DMIC_MODE_MASK},
    {"DMIC_MODE_8",          _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                    REG_DMIC_MODE_MASK},
    {"DMIC_MODE_9",          _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                    REG_DMIC_MODE_MASK},
    {"DMIC_MODE_10",         _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                    REG_DMIC_MODE_MASK},
    {"DMIC_MODE_11",         _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                    REG_DMIC_MODE_MASK},
    {"DMIC_MODE_12",         _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                    REG_DMIC_MODE_MASK},
    {"DMIC_MODE_13",         _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                    REG_DMIC_MODE_MASK},
    {"I2S_MCK_MODE_1",       _RIUA_16BIT(PADTOP_BANK,REG_I2S_MCK_MODE),                 REG_I2S_MCK_MODE_MASK},
    {"I2S_MCK_MODE_2",       _RIUA_16BIT(PADTOP_BANK,REG_I2S_MCK_MODE),                 REG_I2S_MCK_MODE_MASK},
    {"I2S_MCK_MODE_3",       _RIUA_16BIT(PADTOP_BANK,REG_I2S_MCK_MODE),                 REG_I2S_MCK_MODE_MASK},
    {"I2S_MCK_MODE_4",       _RIUA_16BIT(PADTOP_BANK,REG_I2S_MCK_MODE),                 REG_I2S_MCK_MODE_MASK},
    {"I2S_MCK_MODE_5",       _RIUA_16BIT(PADTOP_BANK,REG_I2S_MCK_MODE),                 REG_I2S_MCK_MODE_MASK},
    {"I2S_MCK_MODE_6",       _RIUA_16BIT(PADTOP_BANK,REG_I2S_MCK_MODE),                 REG_I2S_MCK_MODE_MASK},
    {"I2S_MCK_MODE_7",       _RIUA_16BIT(PADTOP_BANK,REG_I2S_MCK_MODE),                 REG_I2S_MCK_MODE_MASK},
    {"I2S_RX_MODE_1",        _RIUA_16BIT(PADTOP_BANK,REG_I2S_RX_MODE),                  REG_I2S_RX_MODE_MASK},
    {"I2S_RX_MODE_2",        _RIUA_16BIT(PADTOP_BANK,REG_I2S_RX_MODE),                  REG_I2S_RX_MODE_MASK},
    {"I2S_RX_MODE_3",        _RIUA_16BIT(PADTOP_BANK,REG_I2S_RX_MODE),                  REG_I2S_RX_MODE_MASK},
    {"I2S_RX_MODE_4",        _RIUA_16BIT(PADTOP_BANK,REG_I2S_RX_MODE),                  REG_I2S_RX_MODE_MASK},
    {"I2S_RX_MODE_5",        _RIUA_16BIT(PADTOP_BANK,REG_I2S_RX_MODE),                  REG_I2S_RX_MODE_MASK},
    {"I2S_RX_MODE_6",        _RIUA_16BIT(PADTOP_BANK,REG_I2S_RX_MODE),                  REG_I2S_RX_MODE_MASK},
    {"I2S_TX_MODE_1",        _RIUA_16BIT(PADTOP_BANK,REG_I2S_TX_MODE),                  REG_I2S_TX_MODE_MASK},
    {"I2S_TX_MODE_2",        _RIUA_16BIT(PADTOP_BANK,REG_I2S_TX_MODE),                  REG_I2S_TX_MODE_MASK},
    {"I2S_TX_MODE_3",        _RIUA_16BIT(PADTOP_BANK,REG_I2S_TX_MODE),                  REG_I2S_TX_MODE_MASK},
    {"I2S_TX_MODE_4",        _RIUA_16BIT(PADTOP_BANK,REG_I2S_TX_MODE),                  REG_I2S_TX_MODE_MASK},
    {"I2S_TX_MODE_5",        _RIUA_16BIT(PADTOP_BANK,REG_I2S_TX_MODE),                  REG_I2S_TX_MODE_MASK},
    {"I2S_TX_MODE_6",        _RIUA_16BIT(PADTOP_BANK,REG_I2S_TX_MODE),                  REG_I2S_TX_MODE_MASK},
    {"I2S_RXTX_MODE_1",      _RIUA_16BIT(PADTOP_BANK,REG_I2S_RXTX_MODE),                REG_I2S_RXTX_MODE_MASK},
    {"I2S_RXTX_MODE_2",      _RIUA_16BIT(PADTOP_BANK,REG_I2S_RXTX_MODE),                REG_I2S_RXTX_MODE_MASK},
    {"I2S_RXTX_MODE_3",      _RIUA_16BIT(PADTOP_BANK,REG_I2S_RXTX_MODE),                REG_I2S_RXTX_MODE_MASK},
    {"I2S_RXTX_MODE_4",      _RIUA_16BIT(PADTOP_BANK,REG_I2S_RXTX_MODE),                REG_I2S_RXTX_MODE_MASK},
    {"I2S_RXTX_MODE_5",      _RIUA_16BIT(PADTOP_BANK,REG_I2S_RXTX_MODE),                REG_I2S_RXTX_MODE_MASK},
    {"I2S_RXTX_MODE_6",      _RIUA_16BIT(PADTOP_BANK,REG_I2S_RXTX_MODE),                REG_I2S_RXTX_MODE_MASK},
    {"BT1120_MODE_1",        _RIUA_16BIT(PADTOP_BANK,REG_BT1120_MODE),                  REG_BT1120_MODE_MASK},
    {"BT1120_MODE_2",        _RIUA_16BIT(PADTOP_BANK,REG_BT1120_MODE),                  REG_BT1120_MODE_MASK},
    {"BT1120_MODE_3",        _RIUA_16BIT(PADTOP_BANK,REG_BT1120_MODE),                  REG_BT1120_MODE_MASK},
    {"PM_SPI_GPIO_0",        _RIUA_16BIT(PMSLEEP_BANK,REG_PM_SPI_GPIO),                 REG_PM_SPI_GPIO_MASK},
    {"PM_SPIWPN_GPIO_0",     _RIUA_16BIT(PMSLEEP_BANK,REG_PM_SPIWPN_GPIO),              REG_PM_SPIWPN_GPIO_MASK},
    {"PM_SPIHOLDN_MODE_0",   _RIUA_16BIT(PMSLEEP_BANK,REG_PM_SPIHOLDN_MODE),            REG_PM_SPIHOLDN_MODE_MASK},
    {"PM_SPICSZ1_GPIO_0",    _RIUA_16BIT(PMSLEEP_BANK,REG_PM_SPICSZ1_GPIO),             REG_PM_SPICSZ1_GPIO_MASK},
    {"PM_SPICSZ2_GPIO_0",    _RIUA_16BIT(PMSLEEP_BANK,REG_PM_SPICSZ2_GPIO),             REG_PM_SPICSZ2_GPIO_MASK},
    {"PM_PWM0_MODE_1",       _RIUA_16BIT(PMSLEEP_BANK,REG_PM_PWM0_MODE),                REG_PM_PWM0_MODE_MASK},
    {"PM_PWM1_MODE_1",       _RIUA_16BIT(PMSLEEP_BANK,REG_PM_PWM1_MODE),                REG_PM_PWM1_MODE_MASK},
    {"PM_PWM2_MODE_1",       _RIUA_16BIT(PMSLEEP_BANK,REG_PM_PWM2_MODE),                REG_PM_PWM2_MODE_MASK},
    {"PM_PWM3_MODE_1",       _RIUA_16BIT(PMSLEEP_BANK,REG_PM_PWM3_MODE),                REG_PM_PWM3_MODE_MASK},
    {"PM_UART1_MODE_1",      _RIUA_16BIT(PMSLEEP_BANK,REG_PM_UART1_MODE),               REG_PM_UART1_MODE_MASK},
    {"PM_VID_MODE_1",        _RIUA_16BIT(PMSLEEP_BANK,REG_PM_VID_MODE),                 REG_PM_VID_MODE_MASK},
    {"PM_VID_MODE_2",        _RIUA_16BIT(PMSLEEP_BANK,REG_PM_VID_MODE),                 REG_PM_VID_MODE_MASK},
    {"PM_VID_MODE_3",        _RIUA_16BIT(PMSLEEP_BANK,REG_PM_VID_MODE),                 REG_PM_VID_MODE_MASK},
    {"PM_SD_CDZ_MODE_1",     _RIUA_16BIT(PMSLEEP_BANK,REG_PM_SD_CDZ_MODE),              REG_PM_SD_CDZ_MODE_MASK},
    {"PM_LED_MODE_1",        _RIUA_16BIT(PMSLEEP_BANK,REG_PM_LED_MODE),                 REG_PM_LED_MODE_MASK},
    {"PM_LED_MODE_2",        _RIUA_16BIT(PMSLEEP_BANK,REG_PM_LED_MODE),                 REG_PM_LED_MODE_MASK},
    {"PM_LED_MODE_3",        _RIUA_16BIT(PMSLEEP_BANK,REG_PM_LED_MODE),                 REG_PM_LED_MODE_MASK},
    {"PM_PAD_EXT_MODE_0_1",  _RIUA_16BIT(PMSLEEP_BANK,REG_PM_PAD_EXT_MODE_0),           REG_PM_PAD_EXT_MODE_0_MASK},
    {"PM_PAD_EXT_MODE_1_1",  _RIUA_16BIT(PMSLEEP_BANK,REG_PM_PAD_EXT_MODE_1),           REG_PM_PAD_EXT_MODE_1_MASK},
    {"PM_PAD_EXT_MODE_2_1",  _RIUA_16BIT(PMSLEEP_BANK,REG_PM_PAD_EXT_MODE_2),           REG_PM_PAD_EXT_MODE_2_MASK},
    {"PM_PAD_EXT_MODE_3_1",  _RIUA_16BIT(PMSLEEP_BANK,REG_PM_PAD_EXT_MODE_3),           REG_PM_PAD_EXT_MODE_3_MASK},
    {"PM_PAD_EXT_MODE_4_1",  _RIUA_16BIT(PMSLEEP_BANK,REG_PM_PAD_EXT_MODE_4),           REG_PM_PAD_EXT_MODE_4_MASK},
    {"PM_PAD_EXT_MODE_5_1",  _RIUA_16BIT(PMSLEEP_BANK,REG_PM_PAD_EXT_MODE_5),           REG_PM_PAD_EXT_MODE_5_MASK},
    {"PM_PAD_EXT_MODE_6_1",  _RIUA_16BIT(PMSLEEP_BANK,REG_PM_PAD_EXT_MODE_6),           REG_PM_PAD_EXT_MODE_6_MASK},
    {"PM_PAD_EXT_MODE_7_1",  _RIUA_16BIT(PMSLEEP_BANK,REG_PM_PAD_EXT_MODE_7),           REG_PM_PAD_EXT_MODE_7_MASK},
    {"PM_PAD_EXT_MODE_8_1",  _RIUA_16BIT(PMSLEEP_BANK,REG_PM_PAD_EXT_MODE_8),           REG_PM_PAD_EXT_MODE_8_MASK},
    {"PM_PAD_EXT_MODE_9_1",  _RIUA_16BIT(PMSLEEP_BANK,REG_PM_PAD_EXT_MODE_9),           REG_PM_PAD_EXT_MODE_9_MASK},
    {"PM_PAD_EXT_MODE_10_1", _RIUA_16BIT(PMSLEEP_BANK,REG_PM_PAD_EXT_MODE_10),          REG_PM_PAD_EXT_MODE_10_MASK},
};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

//------------------------------------------------------------------------------
//  Function    : _HalCheckPin
//  Description :
//------------------------------------------------------------------------------
static S32 _HalCheckPin(U32 padID)
{
    if (GPIO_NR <= padID) {
        return FALSE;
    }
    return TRUE;
}


static void _HalSARGPIOWriteRegBit(u32 u32RegOffset, bool bEnable, U8 u8BitMsk)
{
    if (bEnable)
        _GPIO_R_BYTE(_RIUA_8BIT(PM_SAR_BANK, u32RegOffset)) |= u8BitMsk;
    else
        _GPIO_R_BYTE(_RIUA_8BIT(PM_SAR_BANK, u32RegOffset)) &= (~u8BitMsk);
}

static void _HalPadDisablePadMux(U32 u32PadModeID)
{
    if (_GPIO_R_WORD_MASK(m_stPadModeInfoTbl[u32PadModeID].u32ModeRIU, m_stPadModeInfoTbl[u32PadModeID].u32ModeMask)) {
        _GPIO_W_WORD_MASK(m_stPadModeInfoTbl[u32PadModeID].u32ModeRIU, 0, m_stPadModeInfoTbl[u32PadModeID].u32ModeMask);
    }
}


static S32 HalPadSetMode_General(U32 u32PadID, U32 u32Mode)
{
    U32 u32RegAddr = 0;
    U16 u16RegVal  = 0;
    U8  u8ModeIsFind = 0;
    U16 i = 0;

    for (i = 0; i < sizeof(m_stPadMuxTbl)/sizeof(struct stPadMux); i++)
    {
        if (u32PadID == m_stPadMuxTbl[i].padID)
        {
            u32RegAddr = _RIUA_16BIT(m_stPadMuxTbl[i].base, m_stPadMuxTbl[i].offset);

            if (u32Mode == m_stPadMuxTbl[i].mode)
            {
                u16RegVal = _GPIO_R_WORD_MASK(u32RegAddr, 0xFFFF);
                u16RegVal &= ~(m_stPadMuxTbl[i].mask);
                u16RegVal |= m_stPadMuxTbl[i].val; // CHECK Multi-Pad Mode

                _GPIO_W_WORD_MASK(u32RegAddr, u16RegVal, 0xFFFF);

                u8ModeIsFind = 1;
#if (ENABLE_CHECK_ALL_PAD_CONFLICT == 0)
                break;
#endif
            }
            else
            {   //Clear high priority setting
                u16RegVal = _GPIO_R_WORD_MASK(u32RegAddr, m_stPadMuxTbl[i].mask);
                if (u16RegVal == m_stPadMuxTbl[i].val)
                {
                    printk(KERN_INFO"[Padmux]reset PAD%d(reg 0x%x:%x; mask0x%x) t0 %s (org: %s)\n",
                           u32PadID,
                           m_stPadMuxTbl[i].base,
                           m_stPadMuxTbl[i].offset,
                           m_stPadMuxTbl[i].mask,
                           m_stPadModeInfoTbl[u32Mode].u8PadName,
                           m_stPadModeInfoTbl[m_stPadMuxTbl[i].mode].u8PadName);
                    if (m_stPadMuxTbl[i].val != 0)
                    {
                        _GPIO_W_WORD_MASK(u32RegAddr, 0, m_stPadMuxTbl[i].mask);
                    }
                    else
                    {
                        _GPIO_W_WORD_MASK(u32RegAddr, m_stPadMuxTbl[i].mask, m_stPadMuxTbl[i].mask);
                    }
                }
            }
        }
    }

    return (u8ModeIsFind) ? 0 : -1;
}
 
static S32 HalPadSetMode_MISC(U32 u32PadID, U32 u32Mode)
{
    if((u32PadID>=PAD_ETH_RN)&&(u32PadID<=PAD_ETH_TP) &&(u32Mode ==PINMUX_FOR_GPIO_MODE))//PAD_ETH_XX
	{
		_GPIO_W_WORD_MASK(_RIUA_16BIT(ARBPHY_BANK, 0x69), BIT14, BIT14);//0x32D3#6~#7=11b
		_GPIO_W_WORD_MASK(_RIUA_16BIT(ARBPHY_BANK, 0x69), BIT15, BIT15);
		_GPIO_W_WORD_MASK(_RIUA_16BIT(ARBPHY_BANK, 0x69), BIT10, BIT10);//0x32D3#3~#2=11b
		_GPIO_W_WORD_MASK(_RIUA_16BIT(ARBPHY_BANK, 0x69), BIT11, BIT11);
		_GPIO_W_WORD_MASK(_RIUA_16BIT(ETH_BANK, 0x44), BIT4, BIT4);//0x3388 #4=1b
	}

  switch(u32PadID)
    {
      /* lan-top */
        case PAD_ETH_RN:
            if (u32Mode == PINMUX_FOR_GPIO_MODE)
            {
                _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
                _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY1_BANK,REG_ATOP_RX_INOFF), BIT14, BIT14);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY2_BANK,REG_ETH_GPIO_EN), BIT0, BIT0);
            }
            else if (u32Mode == PINMUX_FOR_ETH_GPIO_EN_MODE)
            {
                _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
                _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY1_BANK,REG_ATOP_RX_INOFF), 0, BIT14);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY2_BANK,REG_ETH_GPIO_EN), 0, BIT0);
            }
            else
            {
                return -1;
            }
            break;
        case PAD_ETH_RP:
            if (u32Mode == PINMUX_FOR_GPIO_MODE) {
                _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
                _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY1_BANK,REG_ATOP_RX_INOFF), BIT14, BIT14);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY2_BANK,REG_ETH_GPIO_EN), BIT1, BIT1);
            }
            else if (u32Mode == PINMUX_FOR_ETH_GPIO_EN_MODE)
            {
                _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
                _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY1_BANK,REG_ATOP_RX_INOFF), 0, BIT14);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY2_BANK,REG_ETH_GPIO_EN), 0, BIT1);
            }
            else
            {
                return -1;
            }
            break;
        case PAD_ETH_TN:
            if (u32Mode == PINMUX_FOR_GPIO_MODE)
            {
                _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
                _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY1_BANK,REG_ATOP_RX_INOFF), BIT15, BIT15);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY2_BANK,REG_ETH_GPIO_EN), BIT2, BIT2);
            }
            else if (u32Mode == PINMUX_FOR_ETH_GPIO_EN_MODE)
            {
                _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
                _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY1_BANK,REG_ATOP_RX_INOFF), 0, BIT15);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY2_BANK,REG_ETH_GPIO_EN), 0, BIT2);
            }
            else
            {
                return -1;
            }
            break;
        case PAD_ETH_TP:
            if (u32Mode == PINMUX_FOR_GPIO_MODE)
            {
                _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
                _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY1_BANK,REG_ATOP_RX_INOFF), BIT15, BIT15);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY2_BANK,REG_ETH_GPIO_EN), BIT3, BIT3);
            }
            else if (u32Mode == PINMUX_FOR_ETH_GPIO_EN_MODE)
            {
                _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
                _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY1_BANK,REG_ATOP_RX_INOFF), 0, BIT15);
                _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY2_BANK,REG_ETH_GPIO_EN), 0, BIT3);
            }
            else
            {
                return -1;
            }
        break;
            /* SAR */
    case PAD_SAR_GPIO0: /* reg_sar_aisel; reg[1422]#5 ~ #0=0b */
        if (u32Mode == PINMUX_FOR_GPIO_MODE)
        {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _HalSARGPIOWriteRegBit(REG_SAR_AISEL_8BIT, 0, REG_SAR_CH0_AISEL);
        }
        else if (u32Mode == PINMUX_FOR_SAR_MODE)
        {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _HalSARGPIOWriteRegBit(REG_SAR_AISEL_8BIT, REG_SAR_CH0_AISEL, REG_SAR_CH0_AISEL);
        }
        else
        {
            return -1;
        }
        break;
    case PAD_SAR_GPIO1:
        if (u32Mode == PINMUX_FOR_GPIO_MODE)
        {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _HalSARGPIOWriteRegBit(REG_SAR_AISEL_8BIT, 0, REG_SAR_CH1_AISEL);
        }
        else if (u32Mode == PINMUX_FOR_SAR_MODE)
        {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _HalSARGPIOWriteRegBit(REG_SAR_AISEL_8BIT, REG_SAR_CH1_AISEL, REG_SAR_CH1_AISEL);
        }
        else
        {
            return -1;
        }
        break;
    case PAD_SAR_GPIO2:
        if (u32Mode == PINMUX_FOR_GPIO_MODE)
        {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _HalSARGPIOWriteRegBit(REG_SAR_AISEL_8BIT, 0, REG_SAR_CH2_AISEL);
        }
        else if (u32Mode == PINMUX_FOR_SAR_MODE)
        {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _HalSARGPIOWriteRegBit(REG_SAR_AISEL_8BIT, REG_SAR_CH2_AISEL, REG_SAR_CH2_AISEL);
        }
        else
        {
            return -1;
        }
        break;
    case PAD_SAR_GPIO3:
        if (u32Mode == PINMUX_FOR_GPIO_MODE)
        {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _HalSARGPIOWriteRegBit(REG_SAR_AISEL_8BIT, 0, REG_SAR_CH3_AISEL);
        }
        else if (u32Mode == PINMUX_FOR_SAR_MODE)
        {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _HalSARGPIOWriteRegBit(REG_SAR_AISEL_8BIT, REG_SAR_CH3_AISEL, REG_SAR_CH3_AISEL);
        }
        else
        {
            return -1;
        }
        break;
    }
      return 0;
}
 
//------------------------------------------------------------------------------
//  Function    : HalPadSetVal
//  Description :
//------------------------------------------------------------------------------
S32 HalPadSetVal(U32 u32PadID, U32 u32Mode)
{
    if (FALSE == _HalCheckPin(u32PadID)) {
        return FALSE;
    }
    if(u32PadID>=PAD_ETH_RN && u32PadID <= PAD_SAR_GPIO3)
        return HalPadSetMode_MISC(u32PadID, u32Mode);
    return HalPadSetMode_General(u32PadID, u32Mode);
}
//------------------------------------------------------------------------------
//  Function    : HalPadSet
//  Description :
//------------------------------------------------------------------------------
S32 HalPadSetMode(U32 u32Mode)
{
    U32 u32PadID;
    U16 k = 0;

    for (k = 0; k < sizeof(m_stPadMuxTbl)/sizeof(struct stPadMux); k++)
    {
        if (u32Mode == m_stPadMuxTbl[k].mode)
        {
            u32PadID = m_stPadMuxTbl[k].padID;
            if (HalPadSetMode_General( u32PadID, u32Mode) < 0)
            {
                return -1;
            }
        }
    }

    return 0;
}
