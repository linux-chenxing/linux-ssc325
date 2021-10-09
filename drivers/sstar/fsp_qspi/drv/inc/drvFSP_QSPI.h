/*
 *#############################################################################
 * SigmaStar trade secret
 * Copyright (c) 2006-2011 SigmaStar Technology Corp.
 * All rights reserved.
 *
 * Unless otherwise stipulated in writing, any and all information contained
 * herein regardless in any format shall remain the sole proprietary of
 * SigmaStar Technology Corp. and be kept in strict confidence
 * (SigmaStar Confidential Information) by the recipient.
 * Any unauthorized act including without limitation unauthorized disclosure,
 * copying, use, reproduction, sale, distribution, modification, disassembling,
 * reverse engineering and compiling of the contents of SigmaStar Confidential
 * Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
 * rights to any and all damages, losses, costs and expenses resulting therefrom.
 *
 *#############################################################################
 */

#ifndef _DRV_FSP_QSPI_H_
#define _DRV_FSP_QSPI_H_

#define FSP_USE_SINGLE_CMD             1
#define FSP_USE_TWO_CMDS            2
#define FSP_USE_THREE_CMDS          3

void DRV_FSP_use_quad_mode(u8 u8_en);
void DRV_FSP_init(u8 u8_cmd_cnt);
u8 DRV_FSP_is_done(void);
void DRV_FSP_use_outside_buf(u8 u8_which_cmd, u8 u8_replace_which_bytes, u32 u32_size);
void DRV_QSPI_use_sw_cs(u8 u8_enabled);
u8 DRV_QSPI_cmd_to_mode(u8 u8_cmd);
void DRV_QSPI_use_3bytes_address_mode(u8 u8_Cmd, u8 u8_dummy_cyc);
void DRV_QSPI_use_2bytes_address_mode(u8 u8_Cmd, u8 u8_dummy_cyc);
u8 DRV_FSP_set_which_cmd_size(u8 u8_which, u8 u8_count);
u8 DRV_FSP_set_rbf_size_after_which_cmd(u8 u8_which, u32 u32_receive);
u8 DRV_FSP_write_wbf(u8 *pu8_buf, u32 u32_size);
void DRV_FSP_trigger(void);
void DRV_FSP_clear_trigger(void);
u8 DRV_FSP_read_rbf(u8 *pu8_buf, u32 u32_size);
void DRV_QSPI_deselected_csz_time(u8 u8_clock);
void DRV_QSPI_pull_cs(u8 u8_pull_high);
void DRV_FSP_enable_outside_wbf(u8 u8_which_wbf, u8 u8_which_byte_replaced, u32 u32_size);
void DRV_FSP_disable_outside_wbf(void);
void DRV_FSP_QSPI_init(void);
void DRV_QSPI_set_timeout(u8 u8_enable, u32 u32_val);
#endif
