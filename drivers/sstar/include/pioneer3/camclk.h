/*
* camclk.h- Sigmastar
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

#ifndef __CAMCLK_H__
#define __CAMCLK_H__

#define    CAMCLK_VOID 0
#define    CAMCLK_utmi_480m 1
#define    CAMCLK_mpll_432m 2
#define    CAMCLK_upll_384m 3
#define    CAMCLK_upll_320m 4
#define    CAMCLK_mpll_288m 5
#define    CAMCLK_utmi_240m 6
#define    CAMCLK_mpll_216m 7
#define    CAMCLK_utmi_192m 8
#define    CAMCLK_mpll_172m 9
#define    CAMCLK_utmi_160m 10
#define    CAMCLK_mpll_123m 11
#define    CAMCLK_mpll_86m 12
#define    CAMCLK_mpll_288m_div2 13
#define    CAMCLK_mpll_288m_div4 14
#define    CAMCLK_mpll_288m_div8 15
#define    CAMCLK_mpll_288m_div32 16
#define    CAMCLK_mpll_216m_div2 17
#define    CAMCLK_mpll_216m_div4 18
#define    CAMCLK_mpll_216m_div8 19
#define    CAMCLK_mpll_123m_div2 20
#define    CAMCLK_mpll_86m_div2 21
#define    CAMCLK_mpll_86m_div4 22
#define    CAMCLK_mpll_86m_div16 23
#define    CAMCLK_utmi_192m_div4 24
#define    CAMCLK_utmi_160m_div4 25
#define    CAMCLK_utmi_160m_div5 26
#define    CAMCLK_utmi_160m_div8 27
#define    CAMCLK_xtali_12m 28
#define    CAMCLK_xtali_12m_div8 29
#define    CAMCLK_xtali_12m_div16 30
#define    CAMCLK_xtali_12m_div40 31
#define    CAMCLK_xtali_12m_div64 32
#define    CAMCLK_xtali_12m_div128 33
#define    CAMCLK_xtali_24m 34
#define    CAMCLK_lpll_clk 35
#define    CAMCLK_lpll_clk_div2 36
#define    CAMCLK_lpll_clk_div4 37
#define    CAMCLK_lpll_clk_div8 38
#define    CAMCLK_spi_synth_pll 39
#define    CAMCLK_fuart0_synth_out 40
#define    CAMCLK_miu_p 41
#define    CAMCLK_mcu_p 42
#define    CAMCLK_emac_testrx125_in_lan 43
#define    CAMCLK_armpll_37p125m 44
#define    CAMCLK_boot 45
#define    CAMCLK_mpll_144m 46
#define    CAMCLK_mpll_144m_div2 47
#define    CAMCLK_mpll_144m_div4 48
#define    CAMCLK_xtali_12m_div2 49
#define    CAMCLK_xtali_12m_div4 50
#define    CAMCLK_xtali_12m_div12 51
#define    CAMCLK_rtc_32k 52
#define    CAMCLK_rtc_32k_div4 53
#define    CAMCLK_ddr_syn 54
#define    CAMCLK_miu_rec 55
#define    CAMCLK_mcu 56
#define    CAMCLK_riubrdg 57
#define    CAMCLK_bdma 58
#define    CAMCLK_bdma2 59
#define    CAMCLK_bdma3 60
#define    CAMCLK_spi 61
#define    CAMCLK_uart0 62
#define    CAMCLK_uart1 63
#define    CAMCLK_uart2 64
#define    CAMCLK_fuart0_synth_in 65
#define    CAMCLK_fuart 66
#define    CAMCLK_mspi0 67
#define    CAMCLK_mspi1 68
#define    CAMCLK_miic0 69
#define    CAMCLK_miic1 70
#define    CAMCLK_keypad 71
#define    CAMCLK_bist 72
#define    CAMCLK_pwr_ctl 73
#define    CAMCLK_xtali 74
#define    CAMCLK_live_c 75
#define    CAMCLK_live 76
#define    CAMCLK_sr00_mclk 77
#define    CAMCLK_sr01_mclk 78
#define    CAMCLK_sata_phy_108 79
#define    CAMCLK_sata_phy_432 80
#define    CAMCLK_disp_432 81
#define    CAMCLK_pspi0 82
#define    CAMCLK_pspi1 83
#define    CAMCLK_upll_synth 84
#define    CAMCLK_xtali_sc_gp 85
#define    CAMCLK_bist_sc_gp 86
#define    CAMCLK_bist_isp_gp 87
#define    CAMCLK_bist_mcu 88
#define    CAMCLK_emac_ahb 89
#define    CAMCLK_jpe 90
#define    CAMCLK_aesdma 91
#define    CAMCLK_sdio 92
#define    CAMCLK_isp 93
#define    CAMCLK_fclk1 94
#define    CAMCLK_dip 95
#define    CAMCLK_ge 96
#define    CAMCLK_mop 97
#define    CAMCLK_disp_216 98
#define    CAMCLK_sc_pixel 99
#define    CAMCLK_sata_pm 100
#define    CAMCLK_sata_axi 101
#define    CAMCLK_mipi_tx_dsi 102
#define    CAMCLK_csi_mac_lptx_top_i_m00 103
#define    CAMCLK_csi_mac_top_i_m00 104
#define    CAMCLK_ns_top_i_m00 105
#define    CAMCLK_csi_mac_lptx_top_i_m01 106
#define    CAMCLK_csi_mac_top_i_m01 107
#define    CAMCLK_ns_top_i_m01 108
#define    CAMCLK_spi_pm 109
#define    CAMCLK_pm_sleep 110
#define    CAMCLK_pwm 111
#define    CAMCLK_sar 112
#define    CAMCLK_rtc 113
#define    CAMCLK_ir 114
#endif
