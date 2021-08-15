/*
* mhal_gpio.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
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
//#include "MsCommon.h"
//#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <asm/io.h>

#include "mhal_gpio.h"
#include "mhal_gpio_reg.h"
#include "ms_platform.h"
#include "gpio.h"
#include "irqs.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#if 1
#define _CONCAT( a, b )     a##b
#define CONCAT( a, b )      _CONCAT( a, b )

// Dummy
#define GPIO999_OEN     0, 0
#define GPIO999_OUT     0, 0
#define GPIO999_IN      0, 0

//infinity gpio reg start
#define GPIO0_PAD PAD_GPIO0
#define GPIO0_OEN 0x103c00, BIT5
#define GPIO0_IN  0x103c00, BIT0
#define GPIO0_OUT 0x103c00, BIT4

#define GPIO1_PAD PAD_GPIO1
#define GPIO1_OEN 0x103c02, BIT5
#define GPIO1_IN  0x103c02, BIT0
#define GPIO1_OUT 0x103c02, BIT4

#define GPIO2_PAD PAD_GPIO2
#define GPIO2_OEN 0x103c04, BIT5
#define GPIO2_IN  0x103c04, BIT0
#define GPIO2_OUT 0x103c04, BIT4

#define GPIO3_PAD PAD_GPIO3
#define GPIO3_OEN 0x103c06, BIT5
#define GPIO3_IN  0x103c06, BIT0
#define GPIO3_OUT 0x103c06, BIT4

#define GPIO4_PAD PAD_GPIO4
#define GPIO4_OEN 0x103c08, BIT5
#define GPIO4_IN  0x103c08, BIT0
#define GPIO4_OUT 0x103c08, BIT4

#define GPIO5_PAD PAD_GPIO5
#define GPIO5_OEN 0x103c0a, BIT5
#define GPIO5_IN  0x103c0a, BIT0
#define GPIO5_OUT 0x103c0a, BIT4

#define GPIO6_PAD PAD_GPIO6
#define GPIO6_OEN 0x103c0c, BIT5
#define GPIO6_IN  0x103c0c, BIT0
#define GPIO6_OUT 0x103c0c, BIT4

#define GPIO7_PAD PAD_GPIO7
#define GPIO7_OEN 0x103c0e, BIT5
#define GPIO7_IN  0x103c0e, BIT0
#define GPIO7_OUT 0x103c0e, BIT4

#define GPIO8_PAD PAD_GPIO8
#define GPIO8_OEN 0x103c10, BIT5
#define GPIO8_IN  0x103c10, BIT0
#define GPIO8_OUT 0x103c10, BIT4

#define GPIO9_PAD PAD_GPIO9
#define GPIO9_OEN 0x103c12, BIT5
#define GPIO9_IN  0x103c12, BIT0
#define GPIO9_OUT 0x103c12, BIT4

#define GPIO10_PAD PAD_GPIO10
#define GPIO10_OEN 0x103c14, BIT5
#define GPIO10_IN  0x103c14, BIT0
#define GPIO10_OUT 0x103c14, BIT4

#define GPIO11_PAD PAD_GPIO11
#define GPIO11_OEN 0x103c16, BIT5
#define GPIO11_IN  0x103c16, BIT0
#define GPIO11_OUT 0x103c16, BIT4

#define GPIO12_PAD PAD_GPIO12
#define GPIO12_OEN 0x103c18, BIT5
#define GPIO12_IN  0x103c18, BIT0
#define GPIO12_OUT 0x103c18, BIT4

#define GPIO13_PAD PAD_GPIO13
#define GPIO13_OEN 0x103c1a, BIT5
#define GPIO13_IN  0x103c1a, BIT0
#define GPIO13_OUT 0x103c1a, BIT4

#define GPIO14_PAD PAD_GPIO14
#define GPIO14_OEN 0x103c1c, BIT5
#define GPIO14_IN  0x103c1c, BIT0
#define GPIO14_OUT 0x103c1c, BIT4

#define GPIO15_PAD PAD_GPIO15
#define GPIO15_OEN 0x103c1e, BIT5
#define GPIO15_IN  0x103c1e, BIT0
#define GPIO15_OUT 0x103c1e, BIT4

#define GPIO16_PAD PAD_PWM0
#define GPIO16_OEN 0x103c20, BIT5
#define GPIO16_IN  0x103c20, BIT0
#define GPIO16_OUT 0x103c20, BIT4

#define GPIO17_PAD PAD_PWM1
#define GPIO17_OEN 0x103c22, BIT5
#define GPIO17_IN  0x103c22, BIT0
#define GPIO17_OUT 0x103c22, BIT4

#define GPIO18_PAD PAD_FUART_RX
#define GPIO18_OEN 0x103c28, BIT5
#define GPIO18_IN  0x103c28, BIT0
#define GPIO18_OUT 0x103c28, BIT4

#define GPIO19_PAD PAD_FUART_TX
#define GPIO19_OEN 0x103c2a, BIT5
#define GPIO19_IN  0x103c2a, BIT0
#define GPIO19_OUT 0x103c2a, BIT4

#define GPIO20_PAD PAD_FUART_CTS
#define GPIO20_OEN 0x103c2c, BIT5
#define GPIO20_IN  0x103c2c, BIT0
#define GPIO20_OUT 0x103c2c, BIT4

#define GPIO21_PAD PAD_FUART_RTS
#define GPIO21_OEN 0x103c2e, BIT5
#define GPIO21_IN  0x103c2e, BIT0
#define GPIO21_OUT 0x103c2e, BIT4

#define GPIO22_PAD PAD_UART0_RX
#define GPIO22_OEN 0x103c30, BIT5
#define GPIO22_IN  0x103c30, BIT0
#define GPIO22_OUT 0x103c30, BIT4

#define GPIO23_PAD PAD_UART0_TX
#define GPIO23_OEN 0x103c32, BIT5
#define GPIO23_IN  0x103c32, BIT0
#define GPIO23_OUT 0x103c32, BIT4

#define GPIO24_PAD PAD_UART1_RX
#define GPIO24_OEN 0x103c34, BIT5
#define GPIO24_IN  0x103c34, BIT0
#define GPIO24_OUT 0x103c34, BIT4

#define GPIO25_PAD PAD_UART1_TX
#define GPIO25_OEN 0x103c36, BIT5
#define GPIO25_IN  0x103c36, BIT0
#define GPIO25_OUT 0x103c36, BIT4

#define GPIO26_PAD PAD_SR_IO00
#define GPIO26_OEN 0x103c40, BIT5
#define GPIO26_IN  0x103c40, BIT0
#define GPIO26_OUT 0x103c40, BIT4

#define GPIO27_PAD PAD_SR_IO01
#define GPIO27_OEN 0x103c42, BIT5
#define GPIO27_IN  0x103c42, BIT0
#define GPIO27_OUT 0x103c42, BIT4

#define GPIO28_PAD PAD_SR_IO02
#define GPIO28_OEN 0x103c44, BIT5
#define GPIO28_IN  0x103c44, BIT0
#define GPIO28_OUT 0x103c44, BIT4

#define GPIO29_PAD PAD_SR_IO03
#define GPIO29_OEN 0x103c46, BIT5
#define GPIO29_IN  0x103c46, BIT0
#define GPIO29_OUT 0x103c46, BIT4

#define GPIO30_PAD PAD_SR_IO04
#define GPIO30_OEN 0x103c48, BIT5
#define GPIO30_IN  0x103c48, BIT0
#define GPIO30_OUT 0x103c48, BIT4

#define GPIO31_PAD PAD_SR_IO05
#define GPIO31_OEN 0x103c4a, BIT5
#define GPIO31_IN  0x103c4a, BIT0
#define GPIO31_OUT 0x103c4a, BIT4

#define GPIO32_PAD PAD_SR_IO06
#define GPIO32_OEN 0x103c4c, BIT5
#define GPIO32_IN  0x103c4c, BIT0
#define GPIO32_OUT 0x103c4c, BIT4

#define GPIO33_PAD PAD_SR_IO07
#define GPIO33_OEN 0x103c4e, BIT5
#define GPIO33_IN  0x103c4e, BIT0
#define GPIO33_OUT 0x103c4e, BIT4

#define GPIO34_PAD PAD_SR_IO08
#define GPIO34_OEN 0x103c50, BIT5
#define GPIO34_IN  0x103c50, BIT0
#define GPIO34_OUT 0x103c50, BIT4

#define GPIO35_PAD PAD_SR_IO09
#define GPIO35_OEN 0x103c52, BIT5
#define GPIO35_IN  0x103c52, BIT0
#define GPIO35_OUT 0x103c52, BIT4

#define GPIO36_PAD PAD_SR_IO10
#define GPIO36_OEN 0x103c54, BIT5
#define GPIO36_IN  0x103c54, BIT0
#define GPIO36_OUT 0x103c54, BIT4

#define GPIO37_PAD PAD_SR_IO11
#define GPIO37_OEN 0x103c56, BIT5
#define GPIO37_IN  0x103c56, BIT0
#define GPIO37_OUT 0x103c56, BIT4

#define GPIO38_PAD PAD_SR_IO12
#define GPIO38_OEN 0x103c58, BIT5
#define GPIO38_IN  0x103c58, BIT0
#define GPIO38_OUT 0x103c58, BIT4

#define GPIO39_PAD PAD_SR_IO13
#define GPIO39_OEN 0x103c5a, BIT5
#define GPIO39_IN  0x103c5a, BIT0
#define GPIO39_OUT 0x103c5a, BIT4

#define GPIO40_PAD PAD_SR_IO14
#define GPIO40_OEN 0x103c5c, BIT5
#define GPIO40_IN  0x103c5c, BIT0
#define GPIO40_OUT 0x103c5c, BIT4

#define GPIO41_PAD PAD_SR_IO15
#define GPIO41_OEN 0x103c5e, BIT5
#define GPIO41_IN  0x103c5e, BIT0
#define GPIO41_OUT 0x103c5e, BIT4

#define GPIO42_PAD PAD_SR_IO16
#define GPIO42_OEN 0x103c60, BIT5
#define GPIO42_IN  0x103c60, BIT0
#define GPIO42_OUT 0x103c60, BIT4

#define GPIO43_PAD PAD_SR_IO17
#define GPIO43_OEN 0x103c62, BIT5
#define GPIO43_IN  0x103c62, BIT0
#define GPIO43_OUT 0x103c62, BIT4

#define GPIO44_PAD PAD_NAND_CEZ
#define GPIO44_OEN 0x103c80, BIT5
#define GPIO44_IN  0x103c80, BIT0
#define GPIO44_OUT 0x103c80, BIT4

#define GPIO45_PAD PAD_NAND_ALE
#define GPIO45_OEN 0x103c82, BIT5
#define GPIO45_IN  0x103c82, BIT0
#define GPIO45_OUT 0x103c82, BIT4

#define GPIO46_PAD PAD_NAND_CLE
#define GPIO46_OEN 0x103c84, BIT5
#define GPIO46_IN  0x103c84, BIT0
#define GPIO46_OUT 0x103c84, BIT4

#define GPIO47_PAD PAD_NAND_WEZ
#define GPIO47_OEN 0x103c86, BIT5
#define GPIO47_IN  0x103c86, BIT0
#define GPIO47_OUT 0x103c86, BIT4

#define GPIO48_PAD PAD_NAND_WPZ
#define GPIO48_OEN 0x103c88, BIT5
#define GPIO48_IN  0x103c88, BIT0
#define GPIO48_OUT 0x103c88, BIT4

#define GPIO49_PAD PAD_NAND_REZ
#define GPIO49_OEN 0x103c8a, BIT5
#define GPIO49_IN  0x103c8a, BIT0
#define GPIO49_OUT 0x103c8a, BIT4

#define GPIO50_PAD PAD_NAND_RBZ
#define GPIO50_OEN 0x103c8c, BIT5
#define GPIO50_IN  0x103c8c, BIT0
#define GPIO50_OUT 0x103c8c, BIT4

#define GPIO51_PAD PAD_NAND_DA0
#define GPIO51_OEN 0x103c8e, BIT5
#define GPIO51_IN  0x103c8e, BIT0
#define GPIO51_OUT 0x103c8e, BIT4

#define GPIO52_PAD PAD_NAND_DA1
#define GPIO52_OEN 0x103c90, BIT5
#define GPIO52_IN  0x103c90, BIT0
#define GPIO52_OUT 0x103c90, BIT4

#define GPIO53_PAD PAD_NAND_DA2
#define GPIO53_OEN 0x103c92, BIT5
#define GPIO53_IN  0x103c92, BIT0
#define GPIO53_OUT 0x103c92, BIT4

#define GPIO54_PAD PAD_NAND_DA3
#define GPIO54_OEN 0x103c94, BIT5
#define GPIO54_IN  0x103c94, BIT0
#define GPIO54_OUT 0x103c94, BIT4

#define GPIO55_PAD PAD_NAND_DA4
#define GPIO55_OEN 0x103c96, BIT5
#define GPIO55_IN  0x103c96, BIT0
#define GPIO55_OUT 0x103c96, BIT4

#define GPIO56_PAD PAD_NAND_DA5
#define GPIO56_OEN 0x103c98, BIT5
#define GPIO56_IN  0x103c98, BIT0
#define GPIO56_OUT 0x103c98, BIT4

#define GPIO57_PAD PAD_NAND_DA6
#define GPIO57_OEN 0x103c9a, BIT5
#define GPIO57_IN  0x103c9a, BIT0
#define GPIO57_OUT 0x103c9a, BIT4

#define GPIO58_PAD PAD_NAND_DA7
#define GPIO58_OEN 0x103c9c, BIT5
#define GPIO58_IN  0x103c9c, BIT0
#define GPIO58_OUT 0x103c9c, BIT4

#define GPIO59_PAD PAD_SD_CLK
#define GPIO59_OEN 0x103ca0, BIT5
#define GPIO59_IN  0x103ca0, BIT0
#define GPIO59_OUT 0x103ca0, BIT4

#define GPIO60_PAD PAD_SD_CMD
#define GPIO60_OEN 0x103ca2, BIT5
#define GPIO60_IN  0x103ca2, BIT0
#define GPIO60_OUT 0x103ca2, BIT4

#define GPIO61_PAD PAD_SD_D0
#define GPIO61_OEN 0x103ca4, BIT5
#define GPIO61_IN  0x103ca4, BIT0
#define GPIO61_OUT 0x103ca4, BIT4

#define GPIO62_PAD PAD_SD_D1
#define GPIO62_OEN 0x103ca6, BIT5
#define GPIO62_IN  0x103ca6, BIT0
#define GPIO62_OUT 0x103ca6, BIT4

#define GPIO63_PAD PAD_SD_D2
#define GPIO63_OEN 0x103ca8, BIT5
#define GPIO63_IN  0x103ca8, BIT0
#define GPIO63_OUT 0x103ca8, BIT4

#define GPIO64_PAD PAD_SD_D3
#define GPIO64_OEN 0x103caa, BIT5
#define GPIO64_IN  0x103caa, BIT0
#define GPIO64_OUT 0x103caa, BIT4

#define GPIO65_PAD PAD_I2C0_SCL
#define GPIO65_OEN 0x103cc0, BIT5
#define GPIO65_IN  0x103cc0, BIT0
#define GPIO65_OUT 0x103cc0, BIT4

#define GPIO66_PAD PAD_I2C0_SDA
#define GPIO66_OEN 0x103cc2, BIT5
#define GPIO66_IN  0x103cc2, BIT0
#define GPIO66_OUT 0x103cc2, BIT4

#define GPIO67_PAD PAD_I2C1_SCL
#define GPIO67_OEN 0x103cc4, BIT5
#define GPIO67_IN  0x103cc4, BIT0
#define GPIO67_OUT 0x103cc4, BIT4

#define GPIO68_PAD PAD_I2C1_SDA
#define GPIO68_OEN 0x103cc6, BIT5
#define GPIO68_IN  0x103cc6, BIT0
#define GPIO68_OUT 0x103cc6, BIT4

#define GPIO69_PAD PAD_SPI0_CZ
#define GPIO69_OEN 0x103ce0, BIT5
#define GPIO69_IN  0x103ce0, BIT0
#define GPIO69_OUT 0x103ce0, BIT4

#define GPIO70_PAD PAD_SPI0_CK
#define GPIO70_OEN 0x103ce2, BIT5
#define GPIO70_IN  0x103ce2, BIT0
#define GPIO70_OUT 0x103ce2, BIT4

#define GPIO71_PAD PAD_SPI0_DI
#define GPIO71_OEN 0x103ce4, BIT5
#define GPIO71_IN  0x103ce4, BIT0
#define GPIO71_OUT 0x103ce4, BIT4

#define GPIO72_PAD PAD_SPI0_DO
#define GPIO72_OEN 0x103ce6, BIT5
#define GPIO72_IN  0x103ce6, BIT0
#define GPIO72_OUT 0x103ce6, BIT4

#define GPIO73_PAD PAD_SPI1_CZ
#define GPIO73_OEN 0x103ce8, BIT5
#define GPIO73_IN  0x103ce8, BIT0
#define GPIO73_OUT 0x103ce8, BIT4

#define GPIO74_PAD PAD_SPI1_CK
#define GPIO74_OEN 0x103cea, BIT5
#define GPIO74_IN  0x103cea, BIT0
#define GPIO74_OUT 0x103cea, BIT4

#define GPIO75_PAD PAD_SPI1_DI
#define GPIO75_OEN 0x103cec, BIT5
#define GPIO75_IN  0x103cec, BIT0
#define GPIO75_OUT 0x103cec, BIT4

#define GPIO76_PAD PAD_SPI1_DO
#define GPIO76_OEN 0x103cee, BIT5
#define GPIO76_IN  0x103cee, BIT0
#define GPIO76_OUT 0x103cee, BIT4
//infinity pm gpio
#define GPIO77_PAD PAD_PM_SD_CDZ
#define GPIO77_OEN 0x0f8e, BIT0
#define GPIO77_IN  0x0f8e, BIT2
#define GPIO77_OUT 0x0f8e, BIT1

#define GPIO78_PAD PAD_PM_IRIN
#define GPIO78_OEN 0x0f28, BIT0
#define GPIO78_IN  0x0f28, BIT2
#define GPIO78_OUT 0x0f28, BIT1

#define GPIO79_PAD PAD_PM_GPIO0
#define GPIO79_OEN 0x0f00, BIT0
#define GPIO79_IN  0x0f00, BIT2
#define GPIO79_OUT 0x0f00, BIT1

#define GPIO80_PAD PAD_PM_GPIO1
#define GPIO80_OEN 0x0f02, BIT0
#define GPIO80_IN  0x0f02, BIT2
#define GPIO80_OUT 0x0f02, BIT1

#define GPIO81_PAD PAD_PM_GPIO2
#define GPIO81_OEN 0x0f04, BIT0
#define GPIO81_IN  0x0f04, BIT2
#define GPIO81_OUT 0x0f04, BIT1

#define GPIO82_PAD PAD_PM_GPIO3
#define GPIO82_OEN 0x0f06, BIT0
#define GPIO82_IN  0x0f06, BIT2
#define GPIO82_OUT 0x0f06, BIT1

#define GPIO83_PAD PAD_PM_GPIO4
#define GPIO83_OEN 0x0f08, BIT0
#define GPIO83_IN  0x0f08, BIT2
#define GPIO83_OUT 0x0f08, BIT1

#define GPIO84_PAD PAD_PM_GPIO5
#define GPIO84_OEN 0x0f0a, BIT0
#define GPIO84_IN  0x0f0a, BIT2
#define GPIO84_OUT 0x0f0a, BIT1

#define GPIO85_PAD PAD_PM_GPIO6
#define GPIO85_OEN 0x0f0c, BIT0
#define GPIO85_IN  0x0f0c, BIT2
#define GPIO85_OUT 0x0f0c, BIT1

#define GPIO86_PAD PAD_PM_GPIO7
#define GPIO86_OEN 0x0f0e, BIT0
#define GPIO86_IN  0x0f0e, BIT2
#define GPIO86_OUT 0x0f0e, BIT1

#define GPIO87_PAD PAD_PM_GPIO8
#define GPIO87_OEN 0x0f10, BIT0
#define GPIO87_IN  0x0f10, BIT2
#define GPIO87_OUT 0x0f10, BIT1

#define GPIO88_PAD PAD_PM_GPIO9
#define GPIO88_OEN 0x0f12, BIT0
#define GPIO88_IN  0x0f12, BIT2
#define GPIO88_OUT 0x0f12, BIT1

#define GPIO89_PAD PAD_PM_GPIO10
#define GPIO89_OEN 0x0f14, BIT0
#define GPIO89_IN  0x0f14, BIT2
#define GPIO89_OUT 0x0f14, BIT1

#define GPIO90_PAD PAD_PM_SPI_CZ
#define GPIO90_OEN 0x0f30, BIT0
#define GPIO90_IN  0x0f30, BIT2
#define GPIO90_OUT 0x0f30, BIT1

#define GPIO91_PAD PAD_PM_SPI_CK
#define GPIO91_OEN 0x0f32, BIT0
#define GPIO91_IN  0x0f32, BIT2
#define GPIO91_OUT 0x0f32, BIT1

#define GPIO92_PAD PAD_PM_SPI_DI
#define GPIO92_OEN 0x0f34, BIT0
#define GPIO92_IN  0x0f34, BIT2
#define GPIO92_OUT 0x0f34, BIT1

#define GPIO93_PAD PAD_PM_SPI_DO
#define GPIO93_OEN 0x0f36, BIT0
#define GPIO93_IN  0x0f36, BIT2
#define GPIO93_OUT 0x0f36, BIT1

#define GPIO94_PAD PAD_PM_SPI_WPZ
#define GPIO94_OEN 0x0f88, BIT0
#define GPIO94_IN  0x0f88, BIT2
#define GPIO94_OUT 0x0f88, BIT1

#define GPIO95_PAD PAD_PM_SPI_HLD
#define GPIO95_OEN 0x0f8a, BIT0
#define GPIO95_IN  0x0f8a, BIT2
#define GPIO95_OUT 0x0f8a, BIT1

#define GPIO96_PAD PAD_PM_LED0
#define GPIO96_OEN 0x0f94, BIT0
#define GPIO96_IN  0x0f94, BIT2
#define GPIO96_OUT 0x0f94, BIT1

#define GPIO97_PAD PAD_PM_LED1
#define GPIO97_OEN 0x0f96, BIT0
#define GPIO97_IN  0x0f96, BIT2
#define GPIO97_OUT 0x0f96, BIT1
//SAR GPIO
#define GPIO98_PAD PAD_SAR_GPIO0
#define GPIO98_OEN 0x1423, BIT0
#define GPIO98_IN  0x1425, BIT0
#define GPIO98_OUT 0x1424, BIT0

#define GPIO99_PAD PAD_SAR_GPIO1
#define GPIO99_OEN 0x1423, BIT1
#define GPIO99_IN  0x1425, BIT1
#define GPIO99_OUT 0x1424, BIT1

#define GPIO100_PAD PAD_SAR_GPIO2
#define GPIO100_OEN 0x1423, BIT2
#define GPIO100_IN  0x1425, BIT2
#define GPIO100_OUT 0x1424, BIT2

#define GPIO101_PAD PAD_SAR_GPIO3
#define GPIO101_OEN 0x1423, BIT3
#define GPIO101_IN  0x1425, BIT3
#define GPIO101_OUT 0x1424, BIT3
//ETH GPIO
#define GPIO102_PAD PAD_ETH_RN
#define GPIO102_OEN 0x33ee, BIT4
#define GPIO102_IN  0x33f0, BIT4
#define GPIO102_OUT 0x33f0, BIT0

#define GPIO103_PAD PAD_ETH_RP
#define GPIO103_OEN 0x33ee, BIT5
#define GPIO103_IN  0x33f0, BIT5
#define GPIO103_OUT 0x33f0, BIT1

#define GPIO104_PAD PAD_ETH_TN
#define GPIO104_OEN 0x33ee, BIT6
#define GPIO104_IN  0x33f0, BIT6
#define GPIO104_OUT 0x33f0, BIT2

#define GPIO105_PAD PAD_ETH_TP
#define GPIO105_OEN 0x33ee, BIT7
#define GPIO105_IN  0x33f0, BIT7
#define GPIO105_OUT 0x33f0, BIT3
//USB GPIO
#define GPIO106_PAD PAD_USB_DM
#define GPIO106_OEN 0x14210a, BIT4
#define GPIO106_IN  0x142131, BIT5
#define GPIO106_OUT 0x14210a, BIT2

#define GPIO107_PAD PAD_USB_DP
#define GPIO107_OEN 0x14210a, BIT5
#define GPIO107_IN  0x142131, BIT4
#define GPIO107_OUT 0x14210a, BIT3
//infinity gpio reg end

U32 gChipBaseAddr=0xFD203C00;
U32 gPmSleepBaseAddr=0xFD001E00;
U32 gSarBaseAddr=0xFD002800;
U32 gRIUBaseAddr=0xFD000000;

#define MHal_CHIPTOP_REG(addr)             (*(volatile U8*)(gChipBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_PM_SLEEP_REG(addr)            (*(volatile U8*)(gPmSleepBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_SAR_GPIO_REG(addr)            (*(volatile U8*)(gSarBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_RIU_REG(addr)                 (*(volatile U8*)(gRIUBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static const struct gpio_setting
{
    U32 r_oen;
    U8  m_oen;
    U32 r_out;
    U8  m_out;
    U32 r_in;
    U8  m_in;
} gpio_table[] =
{
#define __GPIO__(_x_)   { CONCAT(CONCAT(GPIO, _x_), _OEN),   \
                          CONCAT(CONCAT(GPIO, _x_), _OUT),   \
                          CONCAT(CONCAT(GPIO, _x_), _IN) }
#define __GPIO(_x_)     __GPIO__(_x_)

//
// !! WARNING !! DO NOT MODIFIY !!!!
//
// These defines order must match following
// 1. the PAD name in GPIO excel
// 2. the perl script to generate the package header file
//
    //__GPIO(999), // 0 is not used

    __GPIO(0), __GPIO(1), __GPIO(2), __GPIO(3), __GPIO(4),
    __GPIO(5), __GPIO(6), __GPIO(7), __GPIO(8), __GPIO(9),
    __GPIO(10), __GPIO(11), __GPIO(12), __GPIO(13), __GPIO(14),
    __GPIO(15), __GPIO(16), __GPIO(17), __GPIO(18), __GPIO(19),
    __GPIO(20), __GPIO(21), __GPIO(22), __GPIO(23), __GPIO(24),
    __GPIO(25), __GPIO(26), __GPIO(27), __GPIO(28), __GPIO(29),
    __GPIO(30), __GPIO(31), __GPIO(32), __GPIO(33), __GPIO(34),
    __GPIO(35), __GPIO(36), __GPIO(37), __GPIO(38), __GPIO(39),
    __GPIO(40), __GPIO(41), __GPIO(42), __GPIO(43), __GPIO(44),
    __GPIO(45), __GPIO(46), __GPIO(47), __GPIO(48), __GPIO(49),
    __GPIO(50), __GPIO(51), __GPIO(52), __GPIO(53), __GPIO(54),
    __GPIO(55), __GPIO(56), __GPIO(57), __GPIO(58), __GPIO(59),
    __GPIO(60), __GPIO(61), __GPIO(62), __GPIO(63), __GPIO(64),
    __GPIO(65), __GPIO(66), __GPIO(67), __GPIO(68), __GPIO(69),
    __GPIO(70), __GPIO(71), __GPIO(72), __GPIO(73), __GPIO(74),
    __GPIO(75), __GPIO(76), __GPIO(77), __GPIO(78), __GPIO(79),
    __GPIO(80), __GPIO(81), __GPIO(82), __GPIO(83), __GPIO(84),
    __GPIO(85), __GPIO(86), __GPIO(87), __GPIO(88), __GPIO(89),
    __GPIO(90), __GPIO(91), __GPIO(92), __GPIO(93), __GPIO(94),
    __GPIO(95), __GPIO(96), __GPIO(97), __GPIO(98), __GPIO(99),
    __GPIO(100), __GPIO(101), __GPIO(102), __GPIO(103), __GPIO(104),
    __GPIO(105), __GPIO(106), __GPIO(107),
};
#endif

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

//the functions of this section set to initialize
void MHal_GPIO_Init(void)
{
//    printk("MHal_GPIO_Init gBaseAddr=%x\n",gPadBaseAddr);
    MHal_CHIPTOP_REG(REG_ALL_PAD_IN) &= ~BIT7;
}

void MHal_CHIPTOP_WriteRegBit(U32 u32Reg, U8 u8Enable, U8 u8BitMsk)
{
    if(u8Enable)
        MHal_CHIPTOP_REG(u32Reg) |= u8BitMsk;
    else
        MHal_CHIPTOP_REG(u32Reg) &= (~u8BitMsk);
}

U8 MHal_CHIPTOP_ReadRegBit(U32 u32Reg, U8 u8BitMsk)
{
    return ((MHal_CHIPTOP_REG(u32Reg)&u8BitMsk)? 1 : 0);
}

U8 MHal_CHIPTOP_ReadRegMsk(U32 u32Reg, U8 u8BitMsk)
{
    return (MHal_CHIPTOP_REG(u32Reg)&u8BitMsk);
}

void MHal_PM_SLEEP_WriteRegBit(U32 u32Reg, U8 u8Enable, U8 u8BitMsk)
{
    if(u8Enable)
        MHal_PM_SLEEP_REG(u32Reg) |= u8BitMsk;
    else
        MHal_PM_SLEEP_REG(u32Reg) &= (~u8BitMsk);
}

U8 MHal_PM_SLEEP_ReadRegBit(U32 u32Reg, U8 u8BitMsk)
{
    return ((MHal_PM_SLEEP_REG(u32Reg)&u8BitMsk)? 1 : 0);
}

U8 MHal_PM_SLEEP_ReadRegMsk(U32 u32Reg, U8 u8BitMsk)
{
    return (MHal_PM_SLEEP_REG(u32Reg)&u8BitMsk);
}

void MHal_SAR_GPIO_WriteRegBit(U32 u32Reg, U8 u8Enable, U8 u8BitMsk)
{
    if(u8Enable)
        MHal_SAR_GPIO_REG(u32Reg) |= u8BitMsk;
    else
        MHal_SAR_GPIO_REG(u32Reg) &= (~u8BitMsk);
}

void MHal_GPIO_Pad_Set(U8 u8IndexGPIO)
{
    switch(u8IndexGPIO)
    {
    case PAD_GPIO0:
    case PAD_GPIO1:
	case PAD_GPIO2:
	case PAD_GPIO3:
		//MHal_CHIPTOP_WriteRegBit(REG_FUART_MODE,DISABLE,BIT0|BIT1);
		//MHal_CHIPTOP_WriteRegBit(REG_I2S_MODE,DISABLE,BIT2);
        break;
	case PAD_GPIO4:
	case PAD_GPIO5:
        //MHal_CHIPTOP_WriteRegBit(REG_UART0_MODE,DISABLE,BIT5|BIT4);
		//MHal_CHIPTOP_WriteRegBit(REG_DMIC_MODE,DISABLE,BIT0);
        break;
	case PAD_GPIO6:
		//MHal_CHIPTOP_WriteRegBit(REG_UART1_MODE,DISABLE,BIT1|BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_DMIC_MODE,DISABLE,BIT0);
		break;
	case PAD_GPIO7:
		//MHal_CHIPTOP_WriteRegBit(REG_UART1_MODE,DISABLE,BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_ETH_MODE,DISABLE,BIT2);
        break;
	case PAD_GPIO8:
	case PAD_GPIO9:
	case PAD_GPIO10:
	case PAD_GPIO11:
		//MHal_CHIPTOP_WriteRegBit(REG_SPI0_MODE,DISABLE,BIT1|BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_ETH_MODE,DISABLE,BIT2);
		break;
	case PAD_GPIO12:
		//MHal_CHIPTOP_WriteRegBit(REG_SPI1_MODE,DISABLE,BIT5|BIT4);
		//MHal_CHIPTOP_WriteRegBit(REG_PWM0_MODE,DISABLE,BIT1|BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_ETH_MODE,DISABLE,BIT2);
		break;
	case PAD_GPIO13:
		//MHal_CHIPTOP_WriteRegBit(REG_SPI1_MODE,DISABLE,BIT5|BIT4);
		//MHal_CHIPTOP_WriteRegBit(REG_PWM1_MODE,DISABLE,BIT3|BIT2);
		//MHal_CHIPTOP_WriteRegBit(REG_ETH_MODE,DISABLE,BIT2);
		break;
	case PAD_GPIO14:
		//MHal_CHIPTOP_WriteRegBit(REG_SPI1_MODE,DISABLE,BIT5|BIT4);
		//MHal_CHIPTOP_WriteRegBit(REG_PWM2_MODE,DISABLE,BIT4);
		//MHal_CHIPTOP_WriteRegBit(REG_ETH_MODE,DISABLE,BIT2);
		break;
	case PAD_GPIO15:
		//MHal_CHIPTOP_WriteRegBit(REG_SPI1_MODE,DISABLE,BIT5|BIT4);
		//MHal_CHIPTOP_WriteRegBit(REG_PWM3_MODE,DISABLE,BIT6);
		//MHal_CHIPTOP_WriteRegBit(REG_ETH_MODE,DISABLE,BIT2);
		break;
	case PAD_PWM0:
		//MHal_CHIPTOP_WriteRegBit(REG_I2C0_MODE,DISABLE,BIT1|BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_I2C1_MODE,DISABLE,BIT5|BIT4);
		//MHal_CHIPTOP_WriteRegBit(REG_PWM0_MODE,DISABLE,BIT1|BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_TTL_MODE,DISABLE,BIT6);
		break;

	case PAD_PWM1:
		//MHal_CHIPTOP_WriteRegBit(REG_I2C0_MODE,DISABLE,BIT1|BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_I2C1_MODE,DISABLE,BIT5|BIT4);
		//MHal_CHIPTOP_WriteRegBit(REG_PWM1_MODE,DISABLE,BIT3|BIT2);
		break;
	case PAD_FUART_RX:
	case PAD_FUART_TX:
	case PAD_FUART_CTS:
	case PAD_FUART_RTS:
		//MHal_CHIPTOP_WriteRegBit(REG_EJ_MODE,DISABLE,BIT1|BIT0);
		break;
	case PAD_UART0_RX:
	case PAD_UART0_TX:
		//MHal_CHIPTOP_WriteRegBit(REG_UART0_MODE,DISABLE,BIT5|BIT4);
		break;
	case PAD_UART1_RX:
	case PAD_UART1_TX:
		//MHal_CHIPTOP_WriteRegBit(REG_UART1_MODE,DISABLE,BIT1|BIT0);
		break;
	case PAD_SR_IO00:
	case PAD_SR_IO01:
		//MHal_CHIPTOP_WriteRegBit(REG_I2C0_MODE,DISABLE,BIT1|BIT0);
        break;
	case PAD_SR_IO02:
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_OUT_MODE,DISABLE,BIT5|BIT4);
		//MHal_CHIPTOP_WriteRegBit(REG_SR_MODE,DISABLE,BIT2|BIT1|BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_CCIR_MODE,DISABLE,BIT4);
		break;
	case PAD_SR_IO03:
	case PAD_SR_IO04:
	case PAD_SR_IO05:
	case PAD_SR_IO06:
	case PAD_SR_IO07:
	case PAD_SR_IO08:
	case PAD_SR_IO09:
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_OUT_MODE,DISABLE,BIT5|BIT4);
		//MHal_CHIPTOP_WriteRegBit(REG_SR_MODE,DISABLE,BIT2|BIT1|BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_CCIR_MODE,DISABLE,BIT4);
		break;
	case PAD_SR_IO10:
	case PAD_SR_IO11:
	case PAD_SR_IO12:
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_OUT_MODE,DISABLE,BIT5|BIT4);
		break;
	case PAD_SR_IO13:
	case PAD_SR_IO14:
	case PAD_SR_IO15:
	case PAD_SR_IO16:
	case PAD_SR_IO17:
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_OUT_MODE,DISABLE,BIT5|BIT4);
		//MHal_CHIPTOP_WriteRegBit(REG_SR_MODE,DISABLE,BIT2|BIT1|BIT0);
		break;
	case PAD_NAND_ALE:
        break;
	case PAD_NAND_CLE:
	case PAD_NAND_CEZ:
	case PAD_NAND_WEZ:
	case PAD_NAND_WPZ:
	case PAD_NAND_REZ:
	case PAD_NAND_RBZ:
		//MHal_CHIPTOP_WriteRegBit(REG_NAND_MODE,DISABLE,BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_SD_MODE,DISABLE,BIT2);
		//MHal_CHIPTOP_WriteRegBit(REG_TTL_MODE,DISABLE,BIT6);
		break;
	case PAD_NAND_DA0:
	case PAD_NAND_DA1:
	case PAD_NAND_DA2:
	case PAD_NAND_DA3:
	case PAD_NAND_DA4:
	case PAD_NAND_DA5:
	case PAD_NAND_DA6:
	case PAD_NAND_DA7:
		//MHal_CHIPTOP_WriteRegBit(REG_NAND_MODE,DISABLE,BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_TTL_MODE,DISABLE,BIT6);
		break;
    case PAD_SD_CLK:
    case PAD_SD_CMD:
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
		break;
	case PAD_SD_D0:
	case PAD_SD_D1:
	case PAD_SD_D2:
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_OUT_MODE,DISABLE,BIT5|BIT4);
		break;
	case PAD_SD_D3:
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_OUT_MODE,DISABLE,BIT5|BIT4);
		//MHal_CHIPTOP_WriteRegBit(REG_SPI1_MODE,DISABLE,BIT5|BIT4);
		//MHal_CHIPTOP_WriteRegBit(REG_SDIO_MODE,DISABLE,BIT0);
		break;
	case PAD_I2C0_SCL:
	case PAD_I2C0_SDA:
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
		break;
	case PAD_I2C1_SCL:
	case PAD_I2C1_SDA:
		//MHal_CHIPTOP_WriteRegBit(REG_I2C1_MODE,DISABLE,BIT5|BIT4);
		break;

	case PAD_SPI0_CZ:
	case PAD_SPI0_CK:
	case PAD_SPI0_DI:
	case PAD_SPI0_DO:
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
		//MHal_CHIPTOP_WriteRegBit(REG_TEST_OUT_MODE,DISABLE,BIT5|BIT4);
		//MHal_CHIPTOP_WriteRegBit(REG_EJ_MODE,DISABLE,BIT1|BIT0);
		break;
	case PAD_SPI1_CZ:
	case PAD_SPI1_CK:
	case PAD_SPI1_DI:
	case PAD_SPI1_DO:
		//MHal_CHIPTOP_WriteRegBit(REG_SPI1_MODE,DISABLE,BIT5|BIT4);
		//MHal_CHIPTOP_WriteRegBit(REG_TTL_MODE,DISABLE,BIT6);
		break;
	case PAD_PM_IRIN:
		MHal_PM_SLEEP_WriteRegBit(REG_IRIN_MODE,ENABLE,BIT4);
		break;
	case PAD_SAR_GPIO0:
		MHal_SAR_GPIO_WriteRegBit(REG_SAR_MODE,DISABLE,BIT0);
		break;
	case PAD_SAR_GPIO1:
		MHal_SAR_GPIO_WriteRegBit(REG_SAR_MODE,DISABLE,BIT1);
		break;
	case PAD_SAR_GPIO2:
		MHal_SAR_GPIO_WriteRegBit(REG_SAR_MODE,DISABLE,BIT2);
		break;
	case PAD_SAR_GPIO3:
		MHal_SAR_GPIO_WriteRegBit(REG_SAR_MODE,DISABLE,BIT3);
		break;
	default:
		break;

    }
}
void MHal_GPIO_Pad_Oen(U8 u8IndexGPIO)
{
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_oen) &= (~gpio_table[u8IndexGPIO].m_oen);
}

void MHal_GPIO_Pad_Odn(U8 u8IndexGPIO)
{
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_oen) |= gpio_table[u8IndexGPIO].m_oen;
}

U8 MHal_GPIO_Pad_Level(U8 u8IndexGPIO)
{
    return ((MHal_RIU_REG(gpio_table[u8IndexGPIO].r_in)&gpio_table[u8IndexGPIO].m_in)? 1 : 0);
}

U8 MHal_GPIO_Pad_InOut(U8 u8IndexGPIO)
{
    return ((MHal_RIU_REG(gpio_table[u8IndexGPIO].r_oen)&gpio_table[u8IndexGPIO].m_oen)? 1 : 0);
}

void MHal_GPIO_Pull_High(U8 u8IndexGPIO)
{
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_out) |= gpio_table[u8IndexGPIO].m_out;
}

void MHal_GPIO_Pull_Low(U8 u8IndexGPIO)
{
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_out) &= (~gpio_table[u8IndexGPIO].m_out);
}

void MHal_GPIO_Set_High(U8 u8IndexGPIO)
{
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_oen) &= (~gpio_table[u8IndexGPIO].m_oen);
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_out) |= gpio_table[u8IndexGPIO].m_out;
}

void MHal_GPIO_Set_Low(U8 u8IndexGPIO)
{
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_oen) &= (~gpio_table[u8IndexGPIO].m_oen);
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_out) &= (~gpio_table[u8IndexGPIO].m_out);
}

void MHal_Enable_GPIO_INT(U8 u8IndexGPIO)
{
    switch(u8IndexGPIO)
    {
/*   case PAD_GPIO7:
        MHal_CHIPTOP_WriteRegBit(1,DISABLE,BIT7);
        MHal_CHIPTOP_WriteRegBit(2,ENABLE,BIT0);
        break;
    case PAD_GPIO8:
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_25,ENABLE,BIT1);
        break;
    case PAD_GPIO9:
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_25,ENABLE,BIT2);
        break;
    case PAD_GPIO13:
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_25,ENABLE,BIT3);
        break;
    case PAD_GPIO28:
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_25,ENABLE,BIT4);
        break;
    case PAD_GPIO29:
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_21,DISABLE,BIT2);
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_25,ENABLE,BIT5);
        break;
    case PAD_GPIO30:
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_20,DISABLE,BIT5);
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_02,DISABLE,BIT3);
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_20,DISABLE,BIT4);
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_25,ENABLE,BIT6);
        break;
    case PAD_GPIO31:
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_20,DISABLE,BIT5);
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_02,DISABLE,BIT3);
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_20,DISABLE,BIT4);
        MHal_CHIPTOP_WriteRegBit(REG_GPIO1_25,ENABLE,BIT7);
        break;*/
    default:
        break;
    }
}

int MHal_GPIO_To_Irq(U8 u8IndexGPIO)
{
    //160 is PMSLEEP virtual irq start
    if(u8IndexGPIO==PAD_PM_IRIN)
        return INT_PMSLEEP_IRIN + 160;
    else if(u8IndexGPIO>=PAD_PM_GPIO0 && u8IndexGPIO<=PAD_PM_GPIO10)
        return (u8IndexGPIO - PAD_PM_GPIO0 + (INT_PMSLEEP_GPIO_0+160));
    else
        return -1;
}

void MHal_GPIO_Set_POLARITY(U8 u8IndexGPIO,U8 reverse)
{
/*
IdxGPIO  GPIOx  IdxFIQ
70  --GPIO31 -- 63 -- ext_gpio_int[7]  -- reg_hst0_fiq_polarity_63_48_  -- [h00b,h00b]
71  --GPIO30 -- 58 -- ext_gpio_int[6]  -- reg_hst0_fiq_polarity_63_48_  -- [h00b,h00b]
72  --GPIO29 -- 57 -- ext_gpio_int[5]  -- reg_hst0_fiq_polarity_63_48_  -- [h00b,h00b]
73  -- GPIO28 -- 56 -- ext_gpio_int[4]  -- reg_hst0_fiq_polarity_63_48_  -- [h00b,h00b]
113 --GPIO13 -- 55 -- ext_gpio_int[3]  -- reg_hst0_fiq_polarity_63_48_  -- [h00b,h00b]
117 --GPIO9   -- 47 -- ext_gpio_int[2]  -- reg_hst0_fiq_polarity_47_32_  -- [h00a,h00a]
118 --GPIO8   -- 43 -- ext_gpio_int[1]  -- reg_hst0_fiq_polarity_47_32_  -- [h00a,h00a]
119 --GPIO7   -- 39 -- ext_gpio_int[0]  -- reg_hst0_fiq_polarity_47_32_  -- [h00a,h00a]
*/

    switch(u8IndexGPIO)
    {
/*    case 119:  //INT_FIQ_EXT_GPIO0
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 7); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 7); //Set To Raising edge trigger
        break;
    case 118:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 11); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 11); //Set To Raising edge trigger
        break;
    case 117:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 15); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 15); //Set To Raising edge trigger
        break;
    case 113:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 7); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 7); //Set To Raising edge trigger
        break;
    case 73:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 8); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 8); //Set To Raising edge trigger
        break;
    case 72:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 9); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 9); //Set To Raising edge trigger
        break;
    case 71:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 10); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 10); //Set To Raising edge trigger
        break;
    case 70:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 15); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 15); //Set To Raising edge trigger
        break;
*/
    default:
        break;
    }
}



