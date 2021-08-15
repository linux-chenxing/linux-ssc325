//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
//******************************************************************************
//<MStar Software>
// UPLL P1 power down
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00100840), 0x32);
//[1]: PD_UPLL
//[4]: PD_CLKO_UPLL_20M
//[5]: PD_CLKO_UPLL_320M
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00100841), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00100842), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00100844), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00100845), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0010084e), 0x04);
//[0]: EN_CLKO_UPLL_384M
//[1]: EN_UPLL_PRDT2
//[2]: GCR_UPLL_PD_CLKO_AUDIO