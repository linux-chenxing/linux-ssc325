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
#define    CAMCLK_mpll_216m_div2 16
#define    CAMCLK_mpll_216m_div4 17
#define    CAMCLK_mpll_216m_div8 18
#define    CAMCLK_mpll_123m_div2 19
#define    CAMCLK_mpll_86m_div2 20
#define    CAMCLK_mpll_86m_div4 21
#define    CAMCLK_mpll_86m_div16 22
#define    CAMCLK_utmi_192m_div4 23
#define    CAMCLK_utmi_160m_div4 24
#define    CAMCLK_utmi_160m_div5 25
#define    CAMCLK_utmi_160m_div8 26
#define    CAMCLK_xtali_12m 27
#define    CAMCLK_xtali_12m_div8 28
#define    CAMCLK_xtali_12m_div16 29
#define    CAMCLK_xtali_12m_div40 30
#define    CAMCLK_xtali_12m_div64 31
#define    CAMCLK_xtali_12m_div128 32
#define    CAMCLK_xtali_24m 33
#define    CAMCLK_RTC_CLK_32K 34
#define    CAMCLK_pm_riu_w_clk_in 35
#define    CAMCLK_lpll_clk_div2 36
#define    CAMCLK_lpll_clk_div4 37
#define    CAMCLK_lpll_clk_div8 38
#define    CAMCLK_armpll_37p125m 39
#define    CAMCLK_riu_w_clk_in 40
#define    CAMCLK_riu_w_clk_top 41
#define    CAMCLK_riu_w_clk_sc_gp 42
#define    CAMCLK_riu_w_clk_vhe_gp 43
#define    CAMCLK_riu_w_clk_hemcu_gp 44
#define    CAMCLK_riu_w_clk_mipi_if_gp 45
#define    CAMCLK_riu_w_clk_mcu_if_gp 46
#define    CAMCLK_miu_p 47
#define    CAMCLK_mspi0_p 48
#define    CAMCLK_mspi1_p 49
#define    CAMCLK_miu_vhe_gp_p 50
#define    CAMCLK_miu_sc_gp_p 51
#define    CAMCLK_miu2x_p 52
#define    CAMCLK_mcu_p 53
#define    CAMCLK_mcu_pm_p 54
#define    CAMCLK_isp_p 55
#define    CAMCLK_fclk1_p 56
#define    CAMCLK_fclk2_p 57
#define    CAMCLK_sdio_p 58
#define    CAMCLK_tck_buf 59
#define    CAMCLK_pad2isp_sr_pclk 60
#define    CAMCLK_ccir_in_clk 61
#define    CAMCLK_eth_buf 62
#define    CAMCLK_rmii_buf 63
#define    CAMCLK_emac_testrx125_in_lan 64
#define    CAMCLK_miu_ff 65
#define    CAMCLK_miu_sc_gp 66
#define    CAMCLK_miu_vhe_gp 67
#define    CAMCLK_miu_dig 68
#define    CAMCLK_miu_xd2miu 69
#define    CAMCLK_miu_urdma 70
#define    CAMCLK_miu_bdma 71
#define    CAMCLK_miu_vhe 72
#define    CAMCLK_miu_mfeh 73
#define    CAMCLK_miu_mfe 74
#define    CAMCLK_miu_jpe1 75
#define    CAMCLK_miu_jpe0 76
#define    CAMCLK_miu_bach 77
#define    CAMCLK_miu_file 78
#define    CAMCLK_miu_uhc0 79
#define    CAMCLK_miu_emac 80
#define    CAMCLK_miu_cmdq 81
#define    CAMCLK_miu_isp_dnr 82
#define    CAMCLK_miu_isp_rot 83
#define    CAMCLK_miu_isp_dma 84
#define    CAMCLK_miu_isp_sta 85
#define    CAMCLK_miu_gop 86
#define    CAMCLK_miu_sc_dnr 87
#define    CAMCLK_miu_sc_dnr_sad 88
#define    CAMCLK_miu_sc_crop 89
#define    CAMCLK_miu_sc1_frm 90
#define    CAMCLK_miu_sc1_snp 91
#define    CAMCLK_miu_sc1_snpi 92
#define    CAMCLK_miu_sc1_dbg 93
#define    CAMCLK_miu_sc2_frm 94
#define    CAMCLK_miu_sc2_snpi 95
#define    CAMCLK_miu_sc3_frm 96
#define    CAMCLK_miu_fcie 97
#define    CAMCLK_miu_sdio 98
#define    CAMCLK_miu_ive 99
#define    CAMCLK_riu 100
#define    CAMCLK_riu_nogating 101
#define    CAMCLK_riu_sc_gp 102
#define    CAMCLK_riu_vhe_gp 103
#define    CAMCLK_riu_hemcu_gp 104
#define    CAMCLK_riu_mipi_gp 105
#define    CAMCLK_riu_mcu_if 106
#define    CAMCLK_miu2x 107
#define    CAMCLK_axi2x 108
#define    CAMCLK_tck 109
#define    CAMCLK_imi 110
#define    CAMCLK_gop0 111
#define    CAMCLK_gop1 112
#define    CAMCLK_gop2 113
#define    CAMCLK_mpll_144m 114
#define    CAMCLK_mpll_144m_div2 115
#define    CAMCLK_mpll_144m_div4 116
#define    CAMCLK_xtali_12m_div2 117
#define    CAMCLK_xtali_12m_div4 118
#define    CAMCLK_xtali_12m_div12 119
#define    CAMCLK_rtc_32k 120
#define    CAMCLK_rtc_32k_div4 121
#define    CAMCLK_live_pm 122
#define    CAMCLK_riu_pm 123
#define    CAMCLK_miupll_clk 124
#define    CAMCLK_ddrpll_clk 125
#define    CAMCLK_lpll_clk 126
#define    CAMCLK_cpupll_clk 127
#define    CAMCLK_utmi 128
#define    CAMCLK_upll 129
#define    CAMCLK_fuart0_synth_out 130
#define    CAMCLK_csi2_mac_p 131
#define    CAMCLK_miu 132
#define    CAMCLK_ddr_syn 133
#define    CAMCLK_miu_rec 134
#define    CAMCLK_mcu 135
#define    CAMCLK_riubrdg 136
#define    CAMCLK_bdma 137
#define    CAMCLK_spi 138
#define    CAMCLK_uart0 139
#define    CAMCLK_uart1 140
#define    CAMCLK_fuart0_synth_in 141
#define    CAMCLK_fuart 142
#define    CAMCLK_mspi0 143
#define    CAMCLK_mspi1 144
#define    CAMCLK_mspi 145
#define    CAMCLK_miic0 146
#define    CAMCLK_miic1 147
#define    CAMCLK_bist 148
#define    CAMCLK_pwr_ctl 149
#define    CAMCLK_xtali 150
#define    CAMCLK_live 151
#define    CAMCLK_sr_mclk 152
#define    CAMCLK_bist_vhe_gp 153
#define    CAMCLK_vhe 154
#define    CAMCLK_mfe 155
#define    CAMCLK_xtali_sc_gp 156
#define    CAMCLK_bist_sc_gp 157
#define    CAMCLK_emac_ahb 158
#define    CAMCLK_jpe 159
#define    CAMCLK_aesdma 160
#define    CAMCLK_sdio 161
#define    CAMCLK_sd 162
#define    CAMCLK_isp 163
#define    CAMCLK_fclk1 164
#define    CAMCLK_fclk2 165
#define    CAMCLK_odclk 166
#define    CAMCLK_dip 167
#define    CAMCLK_nlm 168
#define    CAMCLK_emac_tx 169
#define    CAMCLK_emac_rx 170
#define    CAMCLK_emac_tx_ref 171
#define    CAMCLK_emac_rx_ref 172
#define    CAMCLK_hemcu_216m 173
#define    CAMCLK_csi_mac 174
#define    CAMCLK_mac_lptx 175
#define    CAMCLK_ns 176
#define    CAMCLK_mcu_pm 177
#define    CAMCLK_spi_pm 178
#define    CAMCLK_pm_sleep 179
#define    CAMCLK_sar 180
#define    CAMCLK_rtc 181
#define    CAMCLK_ir 182
#endif
