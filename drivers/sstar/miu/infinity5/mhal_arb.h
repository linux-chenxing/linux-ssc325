/*
* mhal_arb.h- Sigmastar
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
#ifndef __HAL_ARB__
#define __HAL_ARB__

#include "MsTypes.h"
#include "mdrv_types.h"
#include "registers.h"
#include "ms_platform.h"
#include "regMIU.h"




#define CONFIG_SUPPORT_ARB_v1

struct arb_flowctrl {
    unsigned char cnt0_id;      // client ID, bit[3:0]: ID0, bit[7:4]: ID1
    unsigned char cnt0_period;  // mask period, (max. 255)
    unsigned char cnt0_enable;  // client enable, bit[3:0]: ID0, bit[7:4]: ID1
    unsigned char cnt1_id;      // client ID, bit[3:0]: ID0, bit[7:4]: ID1
    unsigned char cnt1_period;  // mask period, (max. 255)
    unsigned char cnt1_enable;  // client enable, bit[3:0]: ID0, bit[7:4]: ID1
};

struct miu_arb {
    char name[12];                          // device name (miu_arbx)
    char priority[MIU_ARB_CLIENT_NUM];      // client priority (0/1/2/3)
    char burst[MIU_ARB_CLIENT_NUM];         // client burst length (8/16/32/64), 0 => no limit
    char promote[MIU_ARB_CLIENT_NUM];       // client promote enable/disable
    struct arb_flowctrl fctrl[MIU_GRP_NUM]; // flow control, mask request for a period
    char dump;                              // dump mode: readable text, register table
    short client_selected;
};

#ifndef MIU_NUM
#define MIU_NUM         (1)
#endif

/* Bandwidth adjustment related */
// policy
#define MIU_ARB_POLICY_DEF      0
#define MIU_ARB_POLICY_BGA2_DEF 1
#define MIU_ARB_POLICY_NUM      2
// dump
#define MIU_ARB_DUMP_TEXT   0
#define MIU_ARB_DUMP_REG    1
#define MIU_ARB_DUMP_MAX    2
// group priority
#define MIU_ARB_OG_PRIO_0   0
#define MIU_ARB_OG_PRIO_1   1
#define MIU_ARB_OG_PRIO_2   2
#define MIU_ARB_OG_PRIO_3   3
#define MIU_ARB_OG_PRIO_NUM 4

struct miu_burst {
    short len;
    short idx;
};

struct miu_policy {
    char *name;
    char idx;
};

struct miu_reg_addr {
    u32 flowctrl0;
    u32 flowctrl1;
    u32 priority0;
    u32 priority1;
    u32 nolimit;
    u32 burst0;
    u32 burst1;
    u32 ctrl;
    u32 promote;
};

struct miu_reg_val {
    u32 flowctrl;
    u32 priority;
    u32 burst;
    u16 nolimit;
    u16 ctrl;
    u16 promote;
};

struct miu_policy_tbl {
    struct miu_reg_val val[MIU_NUM][MIU_GRP_NUM];
};

struct miu_arb_grp_reg {
    u16 cfg;                // inner group arbitration config
        #define REG_IGRP_CFG(g)         (REG_ID_20 + BK_REG(0x10*g))
        #define IGCFG_ROUND_ROBIN_BIT   0x0001 // bit 0
        #define IGCFG_FIX_PRIO_BIT      0x0002 // bit 1
        #define IGCFG_MBR_LIMIT_EN_BIT  0x0004 // bit 2
        #define IGCFG_GRP_LIMIT_EN_BIT  0x0008 // bit 3
        #define IGCFG_FCTL0_EN_BIT      0x0100 // bit 8
        #define IGCFG_FCTL1_EN_BIT      0x0200 // bit 9
    u16 burst;              // bit[7:0] member burst length; bit[15:8] group burst length
        #define REG_IGRP_BURST(g)       (REG_ID_21 + BK_REG(0x10*g))
        #define IGBURST_MBR_SHIFT       0
        #define IGBURST_MBR_MASK        0x00FF
        #define IGBURST_GRP_SHIFT       8
        #define IGBURST_GRP_MASK        0xFF00
    u16 flowctrl0;
        #define REG_IGRP_FCTL0(g)       (REG_ID_2A + BK_REG(0x10*g))
        #define IGFCTL_ID0_SHIFT        0
        #define IGFCTL_ID0_MASK         0x000F
        #define IGFCTL_ID1_SHIFT        4
        #define IGFCTL_ID1_MASK         0x00F0
        #define IGFCTL_PERIOD_SHIFT     8
        #define IGFCTL_PERIOD_MASK      0xFF00
    u16 flowctrl1;
        #define REG_IGRP_FCTL1(g)       (REG_ID_2B + BK_REG(0x10*g))
    u16 mbr_priority;
        #define REG_IGRP_MBR_PRIO(g)    (REG_ID_2C + BK_REG(0x10*g))
        #define IGMBR_PRIO(mbr,pri)     (pri << mbr)
        #define IGMBR_PRIO_MASK(mbr)    (1 << mbr)
        #define IGMBR_PRIO_SHIFT(mbr)   (mbr)
    u16 mbr_nolimit;
        #define REG_IGRP_MBR_NOLIMIT(g) (REG_ID_2E + BK_REG(0x10*g))
        #define IGMBR_NOLIMIT_EN(mbr)   (1 << mbr)
};

struct miu_arb_reg {
    struct miu_arb_grp_reg grp[MIU_GRP_NUM];
    u16 cfg;                // outer groups arbitration config
        #define REG_OGRP_CFG            (REG_ID_7F)
        #define OGCFG_GRP_PRIO_SHIRT(p) (p << 1)
        #define OGCFG_GRP_PRIO_MASK(p)  (0x3 << (p << 1))
        #define OGCFG_FIX_PRIO_EN       0x0100  // bit 8
        #define OGCFG_ROUND_ROBIN_EN    0x1000  // bit 12
};

struct miu_arb_handle {
    char name[12];          // device name (miu_arbx)
    char dump;              // dump mode: readable text, register table
    char group_selected;    // group selected, 0~(MIU_GRP_NUM-1)
    char client_selected;   // client selected, 1~(MIU_CLIENT_NUM-2)
};

// priority
#define MIU_ARB_PRIO_1ST    0
#define MIU_ARB_PRIO_2ND    1
#define MIU_ARB_PRIO_3RD    2
#define MIU_ARB_PRIO_4TH    3

// burst
#define MIU_ARB_BURST_NOLIM 4
#define MIU_ARB_BURST_OPT   5   // no limit + 4 different burst length option

// group flow control
#define MIU_ARB_CNT_PERIOD_MAX          (0xFF)
#define MIU_ARB_GET_CNT_EN(en)          (en&0xFF)
#define MIU_ARB_GET_CNT_ID0(id)         (id&0x0F)
#define MIU_ARB_GET_CNT_ID1(id)         ((id>>4)&0x0F)
#define MIU_ARB_GET_CNT_ID0_EN(en)      (en&0x0F)
#define MIU_ARB_GET_CNT_ID1_EN(en)      ((en>>4)&0x0F)
#define MIU_ARB_SET_CNT_ID0(id, i)      id = ((id&0xF0)|i)
#define MIU_ARB_SET_CNT_ID1(id, i)      id = ((id&0x0F)|(i<<4))
#define MIU_ARB_SET_CNT_ID0_EN(en, e)   en = ((en&0xF0)|e)
#define MIU_ARB_SET_CNT_ID1_EN(en, e)   en = ((en&0x0F)|(e<<4))

// dump
#define MIU_ARB_DUMP_TEXT   0
#define MIU_ARB_DUMP_REG    1
#define MIU_ARB_DUMP_MAX    2

// policy
#define MIU_ARB_POLICY_RR   0
#define MIU_ARB_POLICY_RT   1
#define MIU_ARB_POLICY_NUM  2





/*=============================================================*/
// Local variable
/*=============================================================*/
bool halARBFlowctrl_is_enable(unsigned char miu, short client, unsigned char *period);
void _load_policy(int idx);
bool halARBFlowctrl_enable(unsigned char miu, short client, bool enable, char period);
int halARBGetCurPolicy(void);
short halARBClientReserved(int id);
void halARBPriority_store(u32 input, U8 m);
int halARBPolicySelect(unsigned char m);
int halARBPriorityShow(unsigned char m, int c);
void halARBBurstStore(unsigned char m);
int halARBGetBurstMapLen(int i);
int halARBGetArbPolicyBurst(unsigned char m, int c);
void halARBSetPromote(unsigned char m, int c,int input);
int halARBGetPromote(unsigned char m, int c);
int halARBGetPolicyIdx(int i);
char* halARBGetPolicyName(int i);
void* halARBGetFctrl(unsigned char m, int g);
short halARBClientReserved(int id);
int halARBGetRegBase(int miu);
void halARBLoadGroupSetting(struct miu_arb_grp_reg *greg, int base, int g);
BOOL halARBIsFlowCTLEnable(struct miu_arb_grp_reg *greg, int mbr, char *period);
BOOL halARBEnableFlowCTL(struct miu_arb_grp_reg *greg, int mbr, bool enable, char period);
void halARBLoadPolicy(int miu, int idx);
int halARBGetOGCCFG(int base);
void halARBGroupBurstStore(struct miu_arb_grp_reg *greg, int base, int g, int burst);
void halARBGroupMemberBurstStore(struct miu_arb_grp_reg *greg, int base, int g, int burst);
void halARBClientNolimitStore(struct miu_arb_grp_reg *greg, int base, int c, int nolimit);
void halARBClientPrioprityStore(struct miu_arb_grp_reg *greg, int base, int g);
void halARBClientFlowctrl(struct miu_arb_grp_reg *greg, int base, int g);
void halARBResume(void);
#endif
