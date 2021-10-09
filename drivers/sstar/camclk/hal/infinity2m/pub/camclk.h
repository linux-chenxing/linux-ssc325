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
#define    CAMCLK_RTC_CLK_32K 35
#define    CAMCLK_pm_riu_w_clk_in 36
#define    CAMCLK_lpll_clk_div2 37
#define    CAMCLK_lpll_clk_div4 38
#define    CAMCLK_lpll_clk_div8 39
#define    CAMCLK_riu_w_clk_in 40
#define    CAMCLK_riu_w_clk_top 41
#define    CAMCLK_riu_w_clk_sc_gp 42
#define    CAMCLK_riu_w_clk_vhe_gp 43
#define    CAMCLK_riu_w_clk_dec_gp 44
#define    CAMCLK_riu_w_clk_hemcu_gp 45
#define    CAMCLK_riu_w_clk_mipi_if_gp 46
#define    CAMCLK_riu_w_clk_mcu_if_gp 47
#define    CAMCLK_miu_p 48
#define    CAMCLK_mspi0_p 49
#define    CAMCLK_mspi1_p 50
#define    CAMCLK_miu_sc_gp_p 51
#define    CAMCLK_miu2x_p 52
#define    CAMCLK_mcu_p 53
#define    CAMCLK_mcu_pm_p 54
#define    CAMCLK_sdio_p 55
#define    CAMCLK_fcie_p 56
#define    CAMCLK_tck_buf 57
#define    CAMCLK_pad2isp_sr_pclk 58
#define    CAMCLK_csi2_mac_p 59
#define    CAMCLK_mipi_tx_dsi_p 60
#define    CAMCLK_sc_pixel_p 61
#define    CAMCLK_ccir_in_clk 62
#define    CAMCLK_eth_buf 63
#define    CAMCLK_rmii_buf 64
#define    CAMCLK_emac_testrx125_in_lan 65
#define    CAMCLK_armpll_37p125m 66
#define    CAMCLK_hdmi_in 67
#define    CAMCLK_dac_in 68
#define    CAMCLK_miu_ff 69
#define    CAMCLK_miu_sc_gp 70
#define    CAMCLK_miu_dec_gp 71
#define    CAMCLK_miu_dig 72
#define    CAMCLK_miu_urdma 73
#define    CAMCLK_miu_miic0 74
#define    CAMCLK_miu_miic1 75
#define    CAMCLK_miu_dma0 76
#define    CAMCLK_riu 77
#define    CAMCLK_riu_nogating 78
#define    CAMCLK_riu_sc_gp 79
#define    CAMCLK_riu_dec_gp 80
#define    CAMCLK_riu_hemcu_gp 81
#define    CAMCLK_riu_mipi_gp 82
#define    CAMCLK_riu_mcu_if 83
#define    CAMCLK_miu2x 84
#define    CAMCLK_axi2x 85
#define    CAMCLK_mpll_144m 86
#define    CAMCLK_mpll_144m_div2 87
#define    CAMCLK_mpll_144m_div4 88
#define    CAMCLK_xtali_12m_div2 89
#define    CAMCLK_xtali_12m_div4 90
#define    CAMCLK_xtali_12m_div12 91
#define    CAMCLK_rtc_32k 92
#define    CAMCLK_rtc_32k_div4 93
#define    CAMCLK_live_pm 94
#define    CAMCLK_mcu_pm 95
#define    CAMCLK_riu_pm 96
#define    CAMCLK_miupll_clk 97
#define    CAMCLK_ddrpll_clk 98
#define    CAMCLK_lpll_clk 99
#define    CAMCLK_cpupll_clk 100
#define    CAMCLK_utmi 101
#define    CAMCLK_upll 102
#define    CAMCLK_fuart0_synth_out 103
#define    CAMCLK_miu 104
#define    CAMCLK_miu_xd2miu 105
#define    CAMCLK_bdma 106
#define    CAMCLK_ddr_syn 107
#define    CAMCLK_miu_rec 108
#define    CAMCLK_mcu 109
#define    CAMCLK_riubrdg 110
#define    CAMCLK_spi 111
#define    CAMCLK_uart0 112
#define    CAMCLK_uart1 113
#define    CAMCLK_uart2 114
#define    CAMCLK_fuart0_synth_in 115
#define    CAMCLK_fuart 116
#define    CAMCLK_mspi0 117
#define    CAMCLK_mspi1 118
#define    CAMCLK_mspi 119
#define    CAMCLK_miic0 120
#define    CAMCLK_miic1 121
#define    CAMCLK_bist 122
#define    CAMCLK_pwr_ctl 123
#define    CAMCLK_xtali 124
#define    CAMCLK_live_c 125
#define    CAMCLK_live 126
#define    CAMCLK_sata_phy_108 127
#define    CAMCLK_sata_phy_432 128
#define    CAMCLK_disp_432 129
#define    CAMCLK_bist_dec_gp 130
#define    CAMCLK_dec_pclk 131
#define    CAMCLK_dec_aclk 132
#define    CAMCLK_dec_bclk 133
#define    CAMCLK_dec_cclk 134
#define    CAMCLK_xtali_sc_gp 135
#define    CAMCLK_bist_sc_gp 136
#define    CAMCLK_emac_ahb 137
#define    CAMCLK_jpe 138
#define    CAMCLK_aesdma 139
#define    CAMCLK_sdio 140
#define    CAMCLK_dip 141
#define    CAMCLK_ge 142
#define    CAMCLK_mop 143
#define    CAMCLK_disp_216 144
#define    CAMCLK_sc_pixel 145
#define    CAMCLK_sata_pm 146
#define    CAMCLK_sata_axi 147
#define    CAMCLK_mipi_tx_dsi 148
#define    CAMCLK_mipi_tx_dsi_apb 149
#define    CAMCLK_hdmi 150
#define    CAMCLK_dac 151
#define    CAMCLK_emac1_tx 152
#define    CAMCLK_emac1_rx 153
#define    CAMCLK_emac1_tx_ref 154
#define    CAMCLK_emac1_rx_ref 155
#define    CAMCLK_emac_tx 156
#define    CAMCLK_emac_rx 157
#define    CAMCLK_emac_tx_ref 158
#define    CAMCLK_emac_rx_ref 159
#define    CAMCLK_hemcu_216m 160
#define    CAMCLK_spi_pm 161
#define    CAMCLK_pm_sleep 162
#define    CAMCLK_pwm 163
#define    CAMCLK_sar 164
#define    CAMCLK_rtc 165
#define    CAMCLK_ir 166
#endif
