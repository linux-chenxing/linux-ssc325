/*
* mhal_arb.c- Sigmastar
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
#include <linux/printk.h>
#include "mdrv_types.h"
#include "registers.h"
#include "mhal_arb.h"
#include "mhal_miu_client.h"
#include "mdrv_miu.h"
/*=============================================================*/
// Local variable
/*=============================================================*/

//static struct miu_device miu_arb_dev[MIU_NUM];
static struct miu_arb arb_policy[MIU_NUM];
const static struct miu_burst burst_map[MIU_ARB_BURST_OPT] =
{
    // keep the last option for no limit, length 0 means no limit
    {8, 0}, {16, 1}, {32, 2}, {64, 3}, {0, MIU_ARB_BURST_NOLIM}
};
static int _cur_policy = 0;
const static struct miu_policy policy_map[MIU_ARB_POLICY_NUM] =
{
    {"round-robin", MIU_ARB_POLICY_RR},
    {"real-time",   MIU_ARB_POLICY_RT},
};
const static struct miu_reg_addr reg_tbl[MIU_GRP_NUM] =
{
    {(REG_ID_0A),(REG_ID_0B),(REG_ID_0C),(REG_ID_0D),(REG_ID_0E),(REG_ID_12),(REG_ID_13),(REG_ID_18),(REG_ID_1C)},
    {(REG_ID_2A),(REG_ID_2B),(REG_ID_2C),(REG_ID_2D),(REG_ID_2E),(REG_ID_32),(REG_ID_33),(REG_ID_38),(REG_ID_3C)},
    {(REG_ID_4A),(REG_ID_4B),(REG_ID_4C),(REG_ID_4D),(REG_ID_4E),(REG_ID_52),(REG_ID_53),(REG_ID_58),(REG_ID_5C)},
    {(REG_ID_6A),(REG_ID_6B),(REG_ID_6C),(REG_ID_6D),(REG_ID_6E),(REG_ID_72),(REG_ID_73),(REG_ID_78),(REG_ID_7C)},
};
// round-robin policy
static struct miu_policy_tbl policy_rr =
{
    {
    // miu0
    {
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
    },
    // miu1
    {
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
    },
    }
};
// real-time path policy
static struct miu_policy_tbl policy_rt =
{
    {
    // miu0
    {
        {0x8021FF99, 0xA0AEA2BE, 0xAAA2AA82, 0x3020, 0xC305, 0xFDF9},
        {0x0000FFCC, 0xABEAA8A2, 0xA82AAAAE, 0x0002, 0xC105, 0xE7FF},
        {0x00000000, 0xAAB80A0A, 0xAA8ABAFA, 0x01CC, 0xC005, 0xFBFF},
        {0x10324800, 0xAAAAAAF3, 0xAAAAAA0C, 0x0002, 0xC305, 0xFFF0},
    },
    // miu1
    {
        {0x8021FF99, 0xA0AEA2BE, 0xAAA2AA82, 0x3020, 0xC305, 0xFDF9},
        {0x0000FFCC, 0xABEAA8A2, 0xA82AAAAE, 0x0002, 0xC105, 0xE7FF},
        {0x00000000, 0xAAB80A0A, 0xAA8ABAFA, 0x01CC, 0xC005, 0xFBFF},
        {0x10324800, 0xAAAAAAF3, 0xAAAAAA0C, 0x0002, 0xC305, 0xFFF0},
    },
    }
};

static struct miu_policy_tbl *arb_setting[MIU_ARB_POLICY_NUM] =
{
    &policy_rr, &policy_rt
};

static struct miu_policy_tbl policy_cur;

static void _set_priority(unsigned char miu, int client, char pri)
{
    int base;
    int g, c, ofst;
    struct miu_reg_val *val;

    // update reg
    g = client / MIU_GRP_CLIENT_NUM;
    c = client % MIU_GRP_CLIENT_NUM;
    ofst = c << 1;
    val = &(policy_cur.val[miu][g]);
    val->priority = (val->priority & ~(0x3 << ofst)) | (pri << ofst);

    base = (miu == 0) ? BASE_REG_MIU_ARB_E_PA : BASE_REG_MIU1_ARB_E_PA;
    OUTREG16((base+reg_tbl[g].priority0), val->priority & 0xFFFF);
    OUTREG16((base+reg_tbl[g].priority1), val->priority >> 16);
}

static void _set_burst(unsigned char miu, int client, char burst)
{
    int base;
    int g, c, ofst, i = 0, idx = 0;
    struct miu_reg_val *val;

    do {
        if (burst_map[i].len == burst)
        {
            idx = burst_map[i].idx;
            break;
        }
        i++;
    } while(i < MIU_ARB_BURST_OPT);

    // update reg
    g = client / MIU_GRP_CLIENT_NUM;
    c = client % MIU_GRP_CLIENT_NUM;
    ofst = c << 1;
    val = &(policy_cur.val[miu][g]);
    if (idx == MIU_ARB_BURST_NOLIM)
    {
        val->burst = (val->burst & ~(0x3 << ofst));
        val->nolimit |= (1 << c);
    }
    else {
        val->burst = (val->burst & ~(0x3 << ofst)) | (idx << ofst);
        val->nolimit &= ~(1 << c);
    }

    base = (miu == 0) ? BASE_REG_MIU_ARB_E_PA : BASE_REG_MIU1_ARB_E_PA;
    OUTREG16((base+reg_tbl[g].burst0), val->burst & 0xFFFF);
    OUTREG16((base+reg_tbl[g].burst1), val->burst >> 16);
    OUTREG16((base+reg_tbl[g].nolimit), val->nolimit);
}

static void _set_promote(unsigned char miu, int client, char promote)
{
    int base;
    int g, c;
    struct miu_reg_val *val;

    // update reg
    g = client / MIU_GRP_CLIENT_NUM;
    c = client % MIU_GRP_CLIENT_NUM;
    val = &(policy_cur.val[miu][g]);
    if (promote)
    {
        val->promote |= (1 << c);
    }
    else {
        val->promote &= ~(1 << c);
    }

    base = (miu == 0) ? BASE_REG_MIU_ARB_E_PA : BASE_REG_MIU1_ARB_E_PA;
    OUTREG16((base+reg_tbl[g].promote), val->promote);
}

static void _set_flowctrl(unsigned char miu, int grp, struct arb_flowctrl *fctrl)
{
    int base;
    struct miu_reg_val *val;

    val = &(policy_cur.val[miu][grp]);

    // if only ID1 enabled, swap ID0 & ID1
    if (MIU_ARB_GET_CNT_EN(fctrl->cnt0_enable))
    {
        if (!MIU_ARB_GET_CNT_ID0_EN(fctrl->cnt0_enable))
        {
            MIU_ARB_SET_CNT_ID0(fctrl->cnt0_id, MIU_ARB_GET_CNT_ID1(fctrl->cnt0_id));
            MIU_ARB_SET_CNT_ID0_EN(fctrl->cnt0_enable, 1);
            MIU_ARB_SET_CNT_ID1_EN(fctrl->cnt0_enable, 0);
        }
        if (!MIU_ARB_GET_CNT_ID1_EN(fctrl->cnt0_enable))
        {
            // only ID0 enabled, set ID1 as ID0
            MIU_ARB_SET_CNT_ID1(fctrl->cnt0_id, MIU_ARB_GET_CNT_ID0(fctrl->cnt0_id));
        }
        val->flowctrl = (val->flowctrl & ~(0xFFFF)) | fctrl->cnt0_id | (fctrl->cnt0_period << 8);
    }
    if (MIU_ARB_GET_CNT_EN(fctrl->cnt1_enable))
    {
        if (!MIU_ARB_GET_CNT_ID0_EN(fctrl->cnt1_enable))
        {
            MIU_ARB_SET_CNT_ID0(fctrl->cnt1_id, MIU_ARB_GET_CNT_ID1(fctrl->cnt1_id));
            MIU_ARB_SET_CNT_ID0_EN(fctrl->cnt1_enable, 1);
            MIU_ARB_SET_CNT_ID1_EN(fctrl->cnt1_enable, 0);
        }
        if (!MIU_ARB_GET_CNT_ID1_EN(fctrl->cnt1_enable))
        {
            // only ID0 enabled, set ID1 as ID0
            MIU_ARB_SET_CNT_ID1(fctrl->cnt1_id, MIU_ARB_GET_CNT_ID0(fctrl->cnt1_id));
        }
        val->flowctrl = (val->flowctrl & ~(0xFFFF0000)) | (fctrl->cnt1_id << 16) | (fctrl->cnt1_period << 24);
    }

    // update reg
    base = (miu == 0) ? BASE_REG_MIU_ARB_E_PA : BASE_REG_MIU1_ARB_E_PA;
    OUTREG16((base+reg_tbl[grp].flowctrl0), val->flowctrl & 0xFFFF);
    OUTREG16((base+reg_tbl[grp].flowctrl1), val->flowctrl >> 16);
    val->ctrl &= ~(0x0300);
    if (MIU_ARB_GET_CNT_EN(fctrl->cnt0_enable))
    {
        val->ctrl |= 0x0100;
    }
    if (MIU_ARB_GET_CNT_EN(fctrl->cnt1_enable))
    {
        val->ctrl |= 0x0200;
    }
    OUTREG16((base+reg_tbl[grp].ctrl), val->ctrl);
}

static void _load_miu_arb(int miu, int grp, struct miu_reg_val *val)
{
    int c, i;
    struct arb_flowctrl *f;

    c = grp * MIU_GRP_CLIENT_NUM;
    for(i = 0; i < MIU_GRP_CLIENT_NUM; i++)
    {
        arb_policy[miu].priority[c] = (val->priority >> (i<<1)) & 0x3;
        if (val->nolimit & (1 << i))
        {
            arb_policy[miu].burst[c] = 0;
        }
        else {
            arb_policy[miu].burst[c] = burst_map[(val->burst >> (i<<1)) & 0x3].len;
        }
        if (val->promote & (1 << i))
        {
            arb_policy[miu].promote[c] = 1;
        }
        else {
            arb_policy[miu].promote[c] = 0;
        }
        c++;
    }
    f = &arb_policy[miu].fctrl[grp];
    if (val->ctrl & 0x0100)
    {
        f->cnt0_id = val->flowctrl & 0xFF;
        f->cnt0_period = (val->flowctrl >> 8) & 0xFF;
        MIU_ARB_SET_CNT_ID0_EN(f->cnt0_enable, 1);
        // ID1 is identical with ID0
        if ((MIU_ARB_GET_CNT_ID0(f->cnt0_id)) == (MIU_ARB_GET_CNT_ID1(f->cnt0_id)))
        {
            MIU_ARB_SET_CNT_ID1_EN(f->cnt0_enable, 0);
        }
        else {
            MIU_ARB_SET_CNT_ID1_EN(f->cnt0_enable, 1);
        }
    }
    else {
        MIU_ARB_SET_CNT_ID0(f->cnt0_id, 0);
        MIU_ARB_SET_CNT_ID1(f->cnt0_id, 0);
        MIU_ARB_SET_CNT_ID0_EN(f->cnt0_enable, 0);
        MIU_ARB_SET_CNT_ID1_EN(f->cnt0_enable, 0);
        f->cnt0_period = 0x00;
    }
    if (val->ctrl & 0x0200)
    {
        f->cnt1_id = (val->flowctrl >> 16) & 0xFF;
        f->cnt1_period = (val->flowctrl >> 24) & 0xFF;
        MIU_ARB_SET_CNT_ID0_EN(f->cnt1_enable, 1);
        if ((MIU_ARB_GET_CNT_ID0(f->cnt1_id)) == (MIU_ARB_GET_CNT_ID1(f->cnt1_id)))
        {
            MIU_ARB_SET_CNT_ID1_EN(f->cnt1_enable, 0);
        }
        else {
            MIU_ARB_SET_CNT_ID1_EN(f->cnt1_enable, 1);
        }
    }
    else {
        MIU_ARB_SET_CNT_ID0(f->cnt1_id, 0);
        MIU_ARB_SET_CNT_ID1(f->cnt1_id, 0);
        MIU_ARB_SET_CNT_ID0_EN(f->cnt1_enable, 0);
        MIU_ARB_SET_CNT_ID1_EN(f->cnt1_enable, 0);
        f->cnt1_period = 0x00;
    }
}

int halARBGetCurPolicy(void)
{
    return _cur_policy;

}

void _load_policy(int idx)
{
    int base;
    int m, g;
    struct miu_reg_val *val;

    if (idx >= MIU_ARB_POLICY_NUM)
    {
        return;
    }
    memcpy(&policy_cur, arb_setting[idx], sizeof(struct miu_policy_tbl));

    for(m = 0; m < MIU_NUM; m++)
    {
        base = (m == 0) ? BASE_REG_MIU_ARB_E_PA : BASE_REG_MIU1_ARB_E_PA;
        for(g = 0; g < MIU_GRP_NUM; g++)
        {
            // register settings
            val = &(policy_cur.val[m][g]);
            OUTREG16((base+reg_tbl[g].flowctrl0), val->flowctrl & 0xFFFF);
            OUTREG16((base+reg_tbl[g].flowctrl1), val->flowctrl >> 16);
            OUTREG16((base+reg_tbl[g].priority0), val->priority & 0xFFFF);
            OUTREG16((base+reg_tbl[g].priority1), val->priority >> 16);
            OUTREG16((base+reg_tbl[g].burst0), val->burst & 0xFFFF);
            OUTREG16((base+reg_tbl[g].burst1), val->burst >> 16);
            OUTREG16((base+reg_tbl[g].nolimit), val->nolimit);
            OUTREG16((base+reg_tbl[g].ctrl), val->ctrl);
            OUTREG16((base+reg_tbl[g].promote), val->promote);
            // update readable info
            _load_miu_arb(m, g, val);
        }
    }
    _cur_policy = idx;
}

bool halARBFlowctrl_is_enable(unsigned char miu, short client, unsigned char *period)
{
    int g, i;
    struct arb_flowctrl *f;

    g = client / MIU_GRP_CLIENT_NUM;
    i = client % MIU_GRP_CLIENT_NUM;
    f = &arb_policy[miu].fctrl[g];

    *period = 0;
    if (MIU_ARB_GET_CNT_ID0_EN(f->cnt0_enable) && (MIU_ARB_GET_CNT_ID0(f->cnt0_id) == i))
    {
        *period = f->cnt0_period;
        return 1;
    }
    else if (MIU_ARB_GET_CNT_ID1_EN(f->cnt0_enable) && (MIU_ARB_GET_CNT_ID1(f->cnt0_id) == i))
    {
        *period = f->cnt0_period;
        return 1;
    }
    else if (MIU_ARB_GET_CNT_ID0_EN(f->cnt1_enable) && (MIU_ARB_GET_CNT_ID0(f->cnt1_id) == i))
    {
        *period = f->cnt1_period;
        return 1;
    }
    else if (MIU_ARB_GET_CNT_ID1_EN(f->cnt1_enable) && (MIU_ARB_GET_CNT_ID1(f->cnt1_id) == i))
    {
        *period = f->cnt1_period;
        return 1;
    }
    return 0;
}

bool halARBFlowctrl_enable(unsigned char miu, short client, bool enable, char period)
{
    int g, i;
    struct arb_flowctrl *f;

    g = client / MIU_GRP_CLIENT_NUM;
    i = client % MIU_GRP_CLIENT_NUM;
    f = &arb_policy[miu].fctrl[g];

    if (enable)
    {
        do {
            // enabled group flow control cnt0 has the same period
            if (MIU_ARB_GET_CNT_EN(f->cnt0_enable) && (f->cnt0_period == period))
            {
                if (!MIU_ARB_GET_CNT_ID0_EN(f->cnt0_enable))
                {
                    //id0 not used
                    MIU_ARB_SET_CNT_ID0_EN(f->cnt0_enable, 1);
                    MIU_ARB_SET_CNT_ID0(f->cnt0_id, i);
                    break;
                }
                else if (!MIU_ARB_GET_CNT_ID1_EN(f->cnt0_enable))
                {
                    //id1 not used
                    MIU_ARB_SET_CNT_ID1_EN(f->cnt0_enable, 1);
                    MIU_ARB_SET_CNT_ID1(f->cnt0_id, i);
                    break;
                }
            }
            // enabled group flow control cnt1 has the same period
            if (MIU_ARB_GET_CNT_EN(f->cnt1_enable) && (f->cnt1_period == period))
            {
                if (!MIU_ARB_GET_CNT_ID0_EN(f->cnt1_enable))
                {
                    //id0 not used
                    MIU_ARB_SET_CNT_ID0_EN(f->cnt1_enable, 1);
                    MIU_ARB_SET_CNT_ID0(f->cnt1_id, i);
                    break;
                }
                else if (!MIU_ARB_GET_CNT_ID1_EN(f->cnt1_enable))
                {
                    //id1 not used
                    MIU_ARB_SET_CNT_ID1_EN(f->cnt1_enable, 1);
                    MIU_ARB_SET_CNT_ID1(f->cnt1_id, i);
                    break;
                }
            }
            if (!MIU_ARB_GET_CNT_EN(f->cnt0_enable))
            {
                MIU_ARB_SET_CNT_ID0_EN(f->cnt0_enable, 1);
                f->cnt0_period = period;
                MIU_ARB_SET_CNT_ID0(f->cnt0_id, i);
                break;
            }
            if (!MIU_ARB_GET_CNT_EN(f->cnt1_enable))
            {
                MIU_ARB_SET_CNT_ID0_EN(f->cnt1_enable, 1);
                f->cnt1_period = period;
                MIU_ARB_SET_CNT_ID0(f->cnt1_id, i);
                break;
            }
            return 1; // not free one, failed

        } while(1);
    }
    else
    {
        // disable client flow control
        if (MIU_ARB_GET_CNT_ID0_EN(f->cnt0_enable) && (MIU_ARB_GET_CNT_ID0(f->cnt0_id) == i))
        {
            MIU_ARB_SET_CNT_ID0_EN(f->cnt0_enable, 0);
        }
        else if (MIU_ARB_GET_CNT_ID1_EN(f->cnt0_enable) && (MIU_ARB_GET_CNT_ID1(f->cnt0_id) == i))
        {
            MIU_ARB_SET_CNT_ID1_EN(f->cnt0_enable, 0);
        }
        else if (MIU_ARB_GET_CNT_ID0_EN(f->cnt1_enable) && (MIU_ARB_GET_CNT_ID0(f->cnt1_id) == i))
        {
            MIU_ARB_SET_CNT_ID0_EN(f->cnt1_enable, 0);
        }
        else if (MIU_ARB_GET_CNT_ID1_EN(f->cnt1_enable) && (MIU_ARB_GET_CNT_ID1(f->cnt1_id) == i))
        {
            MIU_ARB_SET_CNT_ID1_EN(f->cnt1_enable, 0);
        }
        else
        {
            //nothing changed
            return 0;
        }
    }
    _set_flowctrl(miu, g, f);

    return 0;
}
short halARBClientReserved(int id)
{
    if (id < MIU0_CLIENT_NUM)
    {
        return miu0_clients[id].bw_rsvd;
    }
    else if (id == CPU_CLIENT_ID)
    {
        return miu0_clients[MIU0_CLIENT_NUM - 2].bw_rsvd;
    }
    #if MIU_NUM > 1
    if (id < MIU1_CLIENT_NUM)
    {
        return miu1_clients[id].bw_rsvd;
    }
    #endif
    return 1;
}

void halARBPriority_store(u32 input, U8 m)
{
    int c;

   if (input > MIU_ARB_PRIO_4TH)
    {
        printk(KERN_ERR "Invalid priority %d\n", input);
        return;
    }
    c = arb_policy[m].client_selected;
    if (c != MIU_ARB_CLIENT_ALL)
    {
        if (!halARBClientReserved(c))
        {
            arb_policy[m].priority[c] = input;
            _set_priority(m, c, input);
        }
    }
    else
    {
        // all clients set to the same priority
        for (c = 0; c < MIU_ARB_CLIENT_NUM; c++)
        {
            arb_policy[m].priority[c] = input;
            _set_priority(m, c, input);
        }
    }

}

int halARBPolicySelect(unsigned char m)
{
    int c;
    c = arb_policy[m].client_selected;
    return c;
}

int halARBPriorityShow(unsigned char m, int c)
{
    return arb_policy[m].priority[c];
}

void halARBBurstStore(unsigned char m)
{
    int c, i = 0, burst = 0xFFFF;
    u32 input = 0;

    do {
        if (burst_map[i].len == input)
        {
            burst = input;
            break;
        }
    } while(++i < MIU_ARB_BURST_OPT);

    if (burst == 0xFFFF)
    {
        printk(KERN_ERR "Invalid burst %d\n", input);
        return ;
    }

    c = arb_policy[m].client_selected;
    printk(KERN_ERR "set client %d burst %d\n", c, input);
    if (c != MIU_ARB_CLIENT_ALL)
    {
        if (!halARBClientReserved(c))
        {
            arb_policy[m].burst[c] = burst;
            _set_burst(m, c, burst);
        }
    }
    else
    {
        // all clients set to the same burst
        for (c = 0; c < MIU_ARB_CLIENT_NUM; c++)
        {
            arb_policy[m].burst[c] = burst;
            _set_burst(m, c, burst);
        }
    }
}
int halARBGetBurstMapLen(int i)
{
    return burst_map[i].len;

}

int halARBGetArbPolicyBurst(unsigned char m, int c)
{
    return arb_policy[m].burst[c];

}

void halARBSetPromote(unsigned char m, int c,int input)
{
    arb_policy[m].promote[c] = input;
    _set_promote(m, c, input);
}

int halARBGetPromote(unsigned char m, int c)
{
    return arb_policy[m].promote[c];
}

int halARBGetPolicyIdx(int i)
{
    return policy_map[i].idx;
}

char* halARBGetPolicyName(int i)
{
    return policy_map[i].name;
}

void* halARBGetFctrl(unsigned char m, int g)
{
    return &arb_policy[m].fctrl[g];
}

int halARBGetRegBase(int miu)
{
    return 0;
}

void halARBLoadGroupSetting(struct miu_arb_grp_reg *greg, int base, int g)
{
}

BOOL halARBIsFlowCTLEnable(struct miu_arb_grp_reg *greg, int mbr, char *period)
{
    return 0;
}

BOOL halARBEnableFlowCTL(struct miu_arb_grp_reg *greg, int mbr, bool enable, char period)
{
    return 0;
}

void halARBLoadPolicy(int miu, int idx)
{
}

int halARBGetOGCCFG(int base)
{
    return  INREG16(base+REG_OGRP_CFG);
}

void halARBGroupBurstStore(struct miu_arb_grp_reg *greg, int base, int g, int burst)
{
}

void halARBGroupMemberBurstStore(struct miu_arb_grp_reg *greg, int base, int g, int burst)
{
}

void halARBClientNolimitStore(struct miu_arb_grp_reg *greg, int base, int c, int nolimit)
{
}

void halARBClientPrioprityStore(struct miu_arb_grp_reg *greg, int base, int g)
{
}

void halARBClientFlowctrl(struct miu_arb_grp_reg *greg, int base, int g)
{
}


void halARBResume(void)
{
}
