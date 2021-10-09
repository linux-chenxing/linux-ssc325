/*
* mhal_pwm.c- Sigmastar
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
#include "mhal_pwm.h"
#include "gpio.h"
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of_irq.h>
#include <linux/time.h>
#include <linux/of_irq.h>
#include "padmux.h"
//------------------------------------------------------------------------------
//  Variables
//------------------------------------------------------------------------------
#define BASE_REG_NULL 0xFFFFFFFF

typedef struct
{
    u32         u32Adr;
    u32         u32Val;
    u32         u32Msk;
} regSet_t;

typedef struct
{
    U32    u32PadId;
    U32    u32Mode;
} pwmPadTbl_t;

static pwmPadTbl_t padTbl_0[] =
{
    {PAD_SR_IO00,     PINMUX_FOR_PWM0_MODE_1},
    {PAD_SR_IO04,     PINMUX_FOR_PWM0_MODE_2},
    {PAD_SR_IO08,     PINMUX_FOR_PWM0_MODE_3},
    {PAD_TTL4,        PINMUX_FOR_PWM0_MODE_4},
    {PAD_TTL14,       PINMUX_FOR_PWM0_MODE_5},
    {PAD_TTL18,       PINMUX_FOR_PWM0_MODE_6},
    {PAD_KEY0,      PINMUX_FOR_PWM0_MODE_7},
    {PAD_KEY10,     PINMUX_FOR_PWM0_MODE_8},
    {PAD_SD_CMD,    PINMUX_FOR_PWM0_MODE_9},
    {PAD_GPIO1,     PINMUX_FOR_PWM0_MODE_10},
    {PAD_UNKNOWN, BASE_REG_NULL},
    {PAD_UNKNOWN, BASE_REG_NULL},
};

static pwmPadTbl_t padTbl_1[] =
{
    {PAD_SR_IO01,     PINMUX_FOR_PWM1_MODE_1},
    {PAD_SR_IO05,     PINMUX_FOR_PWM1_MODE_2},
    {PAD_SR_IO09,     PINMUX_FOR_PWM1_MODE_3},
    {PAD_TTL5,        PINMUX_FOR_PWM1_MODE_4},
    {PAD_TTL15,       PINMUX_FOR_PWM1_MODE_5},
    {PAD_TTL19,     PINMUX_FOR_PWM1_MODE_6},
    {PAD_KEY1,      PINMUX_FOR_PWM1_MODE_7},
    {PAD_KEY11,     PINMUX_FOR_PWM1_MODE_8},
    {PAD_SD_D3,     PINMUX_FOR_PWM1_MODE_9},
    {PAD_GPIO2,     PINMUX_FOR_PWM1_MODE_10},
    {PAD_UNKNOWN, BASE_REG_NULL},
    {PAD_UNKNOWN, BASE_REG_NULL},
};

static pwmPadTbl_t padTbl_2[] =
{
    {PAD_SR_IO02,     PINMUX_FOR_PWM2_MODE_1},
    {PAD_SR_IO06,     PINMUX_FOR_PWM2_MODE_2},
    {PAD_SR_IO10,     PINMUX_FOR_PWM2_MODE_3},
    {PAD_TTL6,        PINMUX_FOR_PWM2_MODE_4},
    {PAD_TTL16,       PINMUX_FOR_PWM2_MODE_5},
    {PAD_TTL20,     PINMUX_FOR_PWM2_MODE_6},
    {PAD_KEY2,      PINMUX_FOR_PWM2_MODE_7},
    {PAD_KEY12,     PINMUX_FOR_PWM2_MODE_8},
    {PAD_SD_D2,     PINMUX_FOR_PWM2_MODE_9},
    {PAD_GPIO3,     PINMUX_FOR_PWM2_MODE_10},
    {PAD_UNKNOWN, BASE_REG_NULL},
    {PAD_UNKNOWN, BASE_REG_NULL},

};

static pwmPadTbl_t padTbl_3[] =
{
    {PAD_SR_IO03,     PINMUX_FOR_PWM3_MODE_1},
    {PAD_SR_IO07,     PINMUX_FOR_PWM3_MODE_2},
    {PAD_SR_IO11,     PINMUX_FOR_PWM3_MODE_3},
    {PAD_TTL7,        PINMUX_FOR_PWM3_MODE_4},
    {PAD_TTL17,       PINMUX_FOR_PWM3_MODE_5},
    {PAD_TTL21,     PINMUX_FOR_PWM3_MODE_6},
    {PAD_KEY3,      PINMUX_FOR_PWM3_MODE_7},
    {PAD_KEY13,     PINMUX_FOR_PWM3_MODE_8},
    {PAD_SD_GPIO0,  PINMUX_FOR_PWM3_MODE_9},
    {PAD_GPIO4,    PINMUX_FOR_PWM3_MODE_10},
    {PAD_UNKNOWN, BASE_REG_NULL},
    {PAD_UNKNOWN, BASE_REG_NULL},
};
static pwmPadTbl_t* padTbl[] =
{
    padTbl_0,
    padTbl_1,
    padTbl_2,
    padTbl_3,
};

static U8 _pwmEnSatus[PWM_NUM] = { 0 };
static U32 _pwmPeriod[PWM_NUM] = { 0 };
static U8 _pwmPolarity[PWM_NUM] = { 0 };
static U32 _pwmDuty[PWM_NUM][4] = {{0}}; //end  ( hardware support 4 set of duty )
static U8 _pwmDutyId[PWM_NUM] = { 0 }; 
static U32 _pwmShft[PWM_NUM][4] = {{0}}; //begin ( hardware support 4 set of shift )
static U8 _pwmShftId[PWM_NUM] = { 0 }; 
static U32 _pwmFreq[PWM_NUM] = { 0 };
static U32 _pwmStopFreq[PWM_NUM] = { 0 };
static bool isSync=1; // isSync=0 --> need to sync register data from mem 
#ifdef CONFIG_PWM_NEW
static U32 _pwmDiv[PWM_NUM] = { 0 };
static U32 _pwmPeriod_ns[PWM_NUM] = { 0 };
static U32 _pwmDuty_ns[PWM_NUM] = { 0 };
static U16 clk_pwm_div[7] = {1, 2, 4, 8, 32, 64, 128};
#endif

//------------------------------------------------------------------------------
//  Local Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  External Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  Global Functions
//------------------------------------------------------------------------------

//matt
#define  PWM_GROUP0_HOLD_INT        (1<<0)
#define  PWM_GROUP1_HOLD_INT        (1<<2)
#define  PWM_GROUP2_HOLD_INT        (1<<4)
#define  PWM_GROUP0_ROUND_INT       (1<<1)
#define  PWM_GROUP1_ROUND_INT       (1<<3)
#define  PWM_GROUP2_ROUND_INT       (1<<5)


int DrvPWMSetEndToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8DutyId, U32 u32Duty[3][4]);
int DrvPWMSetBeginToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8ShftId, U32 u32Shft[3][4]);
int DrvPWMSetPolarityExToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val);
void DrvPWMSetPeriodExToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 u32Period);

unsigned long long timeStart[PWM_GROUP_NUM]={ 0 },timeStop,timeVal;
static int totalRounds[PWM_NUM]={0};

static unsigned long long  getCurNs(void)
{
    struct timespec64 tv;
    unsigned long long curNs;
    
    getnstimeofday64(&tv);
    //printk("tv_sec= %lld\n",tv.tv_sec);
    //printk("tv_nsec= %ld\n",tv.tv_nsec);
    //printk("curNs = %lld\n",curNs);
    //printk("curNs = %lld\n",curNs);
    curNs = tv.tv_nsec;
    curNs += tv.tv_sec * 1000000000;
    return curNs;
}
    
    
void showBit(void *dev)
{
    struct mstar_pwm_chip *ms_chip = (struct mstar_pwm_chip*) dev;
    U16 test;
    test=INREG16(ms_chip->base +u16REG_PWM_INT )&0x3;
    (test&(1<<1))?(MS_PWM_DBG(" [ round int=1 ]\n")):(MS_PWM_DBG(" [ round int=0 ]\n"));
    (test&(1<<0))?(MS_PWM_DBG(" [ hold int=1 ]\n\n")):(MS_PWM_DBG(" [ hold int=0 ]\n\n"));
}
void MDEV_PWM_MemToReg(struct mstar_pwm_chip* ms_chip, U8 u8Id)
{
    U8 u8GroupId=u8Id,pwmId;
    int idx;
    for(idx=0;idx<4;idx++)
    {
        pwmId=(u8GroupId<<2)+idx; //idx+(groupid*4)
        if(pwmId<PWM_NUM)
        {
            DrvPWMSetPolarityExToReg(ms_chip, pwmId ,_pwmPolarity[pwmId]);
            DrvPWMSetPeriodExToReg(ms_chip, pwmId , _pwmPeriod[pwmId]);
            DrvPWMSetBeginToReg(ms_chip, pwmId,_pwmShftId[pwmId],_pwmShft);
            DrvPWMSetEndToReg(ms_chip, pwmId ,_pwmDutyId[pwmId],_pwmDuty);	
        }
    }
}
void MDev_PWM_isr_round_Act(void *dev,U16 u16Id)
{
    struct mstar_pwm_chip *ms_chip = (struct mstar_pwm_chip*) dev;
    U16 group_round_id=u16Id;
    U32 u32Reg;
    U8 i=0;
    U8 u8pwmID;

    timeStop=getCurNs(); //ns
    
    if(isSync) //no new data
    {
        CLRREG16(ms_chip->base + REG_GROUP_ENABLE, (1 << (group_round_id+ REG_GROUP_ENABLE_SHFT))); 
        u32Reg = (group_round_id << 0x7) + 0x40; // round bit

        for(i = 0; i < 4 ;i++){
            u8pwmID = (group_round_id * PWM_PER_GROUP) + i; //Fix coverity CID 541490 OVERRUN

            if(_pwmFreq[u8pwmID]){
                totalRounds[u8pwmID] += INREG16(ms_chip->base + u32Reg) & 0xFFFF;
            }
        }
        OUTREG16(ms_chip->base + u32Reg, 0); // set round=0
        MS_PWM_DBG("\nNONE of ROUND ! \n");
        showBit(ms_chip);
    }
    else //have new data
    {
        MS_PWM_DBG("Create New Round  !\n");
        CLRREG16(ms_chip->base + REG_GROUP_ENABLE, (1 << (group_round_id+ REG_GROUP_ENABLE_SHFT)));
        MDEV_PWM_MemToReg(ms_chip, group_round_id);
        /*
        DrvPWMSetPolarityExToReg(ms_chip,group_round_id,_pwmPolarity[group_round_id]);
        DrvPWMSetPeriodExToReg(ms_chip, group_round_id, _pwmPeriod[group_round_id]);
        DrvPWMSetBeginToReg(ms_chip, group_round_id,_pwmShftId[group_round_id],_pwmShft);
        DrvPWMSetEndToReg(ms_chip,group_round_id,_pwmDutyId[group_round_id],_pwmDuty);	
        */
        SETREG16(ms_chip->base + REG_GROUP_ENABLE, (1 << (group_round_id + REG_GROUP_ENABLE_SHFT)));
        isSync=1;
        MS_PWM_DBG("You can set Parameter now !\n");

        
        timeStart[group_round_id]=getCurNs(); //new start round ms
    }
}

void MDev_PWM_isr_hold_Act(void *dev,U16 val,U16 u16Id)
{ 
    U16 group_hold_id=u16Id;
    U16 group_hold_mode=val;
    struct mstar_pwm_chip *ms_chip = (struct mstar_pwm_chip*) dev;


    if(group_hold_mode==0)
    {
    		// (3.1) if mode 0
    		//     function, group_round_status --> return group_round_status[group_id];
    		// I6b0 --> period/begin/end/polar --> arguement in memory to HW register
        MDEV_PWM_MemToReg(ms_chip, group_hold_id);
        MS_PWM_DBG("3.1 done\n");
    }
    else if (group_hold_mode==1)
    {
    		// (3.2) if mode 1, do nothing and return
        MS_PWM_DBG("3.2 done\n");
    }
}

irqreturn_t PWM_IRQ(int irq, void *dev)
{
    struct mstar_pwm_chip *ms_chip = (struct mstar_pwm_chip*) dev;
    U16 group_id,group_hold_mode;

        // (1) read 0x75 --> reg_pwm_int, group_id = which group meet; both hold+round
    group_id = INREG16(ms_chip->base + u16REG_PWM_INT)&0x3F;
        // (2) read 0x77 --> hold mode 0 or 1, (?) each group should have its own mode, not share one mode (check with Ray and report to Oliver)	
    group_hold_mode = INREG16(ms_chip->base + u16REG_PWM_HOLD_MODE1) & BIT0;

    // 3 types of isr below.
    ///////////////////////////////////////////////////////
    // Part 1: use group_enable without sync. 
    /*
    if(group_id&PWM_GROUP0_HOLD_INT && group_id&PWM_GROUP0_ROUND_INT)
    {
        if (!(INREG16(ms_chip->base + REG_GROUP_JOIN)&0xF))
        {
            printk("Please sync first !");
            return IRQ_HANDLED;
        }
    }
    if(group_id&PWM_GROUP1_HOLD_INT && group_id&PWM_GROUP1_ROUND_INT)
    {
        if (!(INREG16(ms_chip->base + REG_GROUP_JOIN)&0xF0))
        {
            printk("Please sync first !");
            return IRQ_HANDLED;
        }
    }
    if(group_id&PWM_GROUP2_HOLD_INT && group_id&PWM_GROUP2_ROUND_INT)
    {
        if (!(INREG16(ms_chip->base + REG_GROUP_JOIN)&0x700)
        {
            printk("Please sync first !");
            return IRQ_HANDLED;
        }
    }
    */
    ///////////////////////////////////////////////////////
    // Part 2 : Hold interrupt isr
    
    if(group_id&PWM_GROUP0_HOLD_INT)
    {
        // (3)
        MDev_PWM_isr_hold_Act(ms_chip,group_hold_mode,0);
        // (4) 0x71 --> hold release
        CLRREG16(ms_chip->base + REG_GROUP_HOLD, (1 << (0+ REG_GROUP_HOLD_SHFT)));
        MS_PWM_DBG("Release hold mode done 1-0 !\n");
        showBit(ms_chip);
        isSync=1;
    }
//    if(group_id&PWM_GROUP1_HOLD_INT)
//    {
//        MDev_PWM_isr_hold_Act(ms_chip,group_hold_mode,1);
//        CLRREG16(ms_chip->base + REG_GROUP_HOLD, (1 << (1+ REG_GROUP_HOLD_SHFT)));
//        MS_PWM_DBG("Release hold mode done 1-1 !\n");
//        showBit(ms_chip);
//        isSync=1;
//    }
//    if(group_id&PWM_GROUP2_HOLD_INT)
//    {
//        MDev_PWM_isr_hold_Act(ms_chip,group_hold_mode,2);
//        CLRREG16(ms_chip->base + REG_GROUP_HOLD, (1 << (2+ REG_GROUP_HOLD_SHFT)));
//        MS_PWM_DBG("Release hold mode done 1-2 !\n");
//        showBit(ms_chip);
//        isSync=1;
//    }    

    ///////////////////////////////////////////////////////
    // Part 3 : Round interrupt isr
    
    if(group_id&PWM_GROUP0_ROUND_INT)
    {
        MS_PWM_DBG("2-0 done \n");
        MDev_PWM_isr_round_Act(ms_chip,0);   
    }
//    if(group_id&PWM_GROUP1_ROUND_INT)
//    {
//        MS_PWM_DBG("2-1 done \n");
//        MDev_PWM_isr_round_Act(ms_chip,1);
//    }
//    if(group_id&PWM_GROUP2_ROUND_INT)
//    {
//        MS_PWM_DBG("2-2 done \n");
//        MDev_PWM_isr_round_Act(ms_chip,2);
//    }

    return IRQ_HANDLED;
        
}

int DrvGroupEnable(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U8 u8Val)
{
    U16 u32JoinMask = 0X0000;
    if (PWM_GROUP_NUM <= u8GroupId)
        return 0;
    u32JoinMask |= 1 << (u8GroupId + PWM_NUM);
    
    if (u8Val)
    {
        SETREG16(ms_chip->base + REG_GROUP_ENABLE, (1 << (u8GroupId + REG_GROUP_ENABLE_SHFT)));
        CLRREG16(ms_chip->base + u16REG_SW_RESET, u32JoinMask);
        //MDEV_PWM_SetSyncFlag(1); //dont need to sync until new data in
    }
    else
    {
        CLRREG16(ms_chip->base + REG_GROUP_ENABLE, (1 << (u8GroupId + REG_GROUP_ENABLE_SHFT)));
        SETREG16(ms_chip->base + u16REG_SW_RESET, u32JoinMask);
    }
    return 1;
}

//+++[Only4I6e]
void MDEV_PWM_AllGrpEnable(struct mstar_pwm_chip *ms_chip)
{
  
}
//---[Only4I6e]

void DrvPWMInit(struct mstar_pwm_chip *ms_chip, U8 u8Id)
{
    U32 reset, u32Period;


//    if (!u8Id) {
//        U8 i;
//        for (i = 0; i < PWM_GROUP_NUM; i++) {
//            //[interrupt function]
//            //Each group bit0 must enable for interrupt function
//            //please see sync mode description for detail
//            //SW owner default need to enable h'74 bit0, bit4, bit8
//            DrvPWMGroupJoin(ms_chip, (i*PWM_PER_GROUP), 1);
//        }
//        printk(KERN_NOTICE "[NOTICE]Each grp bit0 must be enabled!\r\n");
//    }

    if (PWM_NUM <= u8Id)
        return;

    reset = INREG16(ms_chip->base + u16REG_SW_RESET) & (BIT0<<u8Id);
#ifdef CONFIG_PWM_NEW
    DrvPWMGetConfig(ms_chip, u8Id, NULL ,&u32Period);
    DrvPWMDutyQE0(ms_chip, 0, 0);
#else
    DrvPWMGetPeriod(ms_chip, u8Id, &u32Period);
#endif

    if ((0 == reset) && (u32Period))
    {
        _pwmEnSatus[u8Id] = 1;
    }
    else
    {
        DrvPWMEnable(ms_chip, u8Id, 0);
    }
}

//------------------------------------------------------------------------------
//
//  Function:   DrvPWMSetDuty
//
//  Description
//      Set Duty value
//
//  Parameters
//      u8Id:    [in] PWM ID
//      u16Val:  [in] Duty value (percentage)
//
//  Return Value
//      None
//
#ifdef CONFIG_PWM_NEW
void DrvPWMSetConfig(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 duty,U32 period)
{
    U8 i;
    U16 u16Div = 0;
    U32 common = 0;
    U32 pwmclk = 0;
    U32 periodmax = 0;
    U32 u32Period = 0x00000000;
    U32 u32Duty = 0x00000000;

    if(u8Id >= PWM_NUM)
        return;

//    pwmclk = (U32)(clk_get_rate(ms_chip->clk));
    pwmclk = 12000000;
    switch(pwmclk)
    {
        case 12000000:
            pwmclk = 3;
            common = 250;
            break;
        default:
            pwmclk = 3;
            common = 250;
    }

    /*      select   div       */
    for(i = 0;i<(sizeof(clk_pwm_div)/sizeof(U16));i++){
        periodmax = (clk_pwm_div[i] * 262144 / pwmclk) * common;
        if(period < periodmax)
        {
            u16Div = clk_pwm_div[i];
            _pwmDiv[u8Id] = clk_pwm_div[i];
            break;
        }
    }

    /*      select   period       */
    if(period < (0xFFFFFFFF / pwmclk))
    {
        u32Period= (pwmclk * period) / (u16Div * common);
        if(((pwmclk * period) % (u16Div * common)) > (u16Div * common / 2))
        {
            u32Period++;
        }
        _pwmPeriod_ns[u8Id] = (u32Period * u16Div * common) / pwmclk;
    }
    else
    {
        u32Period= (period / u16Div) * pwmclk / common;
        u32Period++;
        _pwmPeriod_ns[u8Id] = (u32Period * common / pwmclk) * u16Div;
    }
    /*      select   duty       */
    if(duty == 0)
    {
        if(_pwmEnSatus[u8Id])
        {
            SETREG16(ms_chip->base + u16REG_SW_RESET,BIT0<<((u8Id==10)?0:u8Id));
        }
    }
    else
    {
        if(_pwmEnSatus[u8Id])
        {
            CLRREG16(ms_chip->base + u16REG_SW_RESET, BIT0<<((u8Id==10)?0:u8Id));
        }
    }

    if(duty < (0xFFFFFFFF / pwmclk))
    {
        u32Duty= (pwmclk * duty) / (u16Div * common);
        if((((pwmclk * duty) % (u16Div * common)) > (u16Div * common / 2)) || (u32Duty == 0))
        {
            u32Duty++;
        }
        _pwmDuty_ns[u8Id] = (u32Duty * u16Div * common) / pwmclk;
    }
    else
    {
        u32Duty= (duty / u16Div) * pwmclk / common;
        u32Duty++;
        _pwmPeriod_ns[u8Id] = (u32Duty * common / pwmclk) * u16Div;
    }

    /*      set  div period duty       */
    u16Div--;
    u32Period--;
    u32Duty--;
    pr_err("clk=12M, u16Div=%d u32Duty=0x%x u32Period=0x%x\n", u16Div, u32Duty, u32Period);
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DIV, (u16Div & 0xFFFF));
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L, (u32Period&0xFFFF));
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_H, ((u32Period>>16)&0x3));
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L, (u32Duty&0xFFFF));
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_H, ((u32Duty>>16)&0x3));
}

void DrvPWMGetConfig(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32* Duty,U32* Period)
{
    U16 u16Div = 0;
    U32 u32Duty = 0;
    U32 u32Period = 0;
    U32 pwmclk = 0;
    U32 common = 0;

    if(u8Id >= PWM_NUM)
        return;

    pwmclk = (U32)(clk_get_rate(ms_chip->clk));

    switch(pwmclk)
    {
        case 12000000:
            pwmclk = 3;
            common = 250;
            break;
        default:
            pwmclk = 3;
            common = 250;
    }

    u16Div = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DIV);
    u16Div++;

    if(Period != NULL)
    {
        if(_pwmPeriod_ns[u8Id] == 0)
        {
            u32Period = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L) | ((INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_H) & 0x3) << 16);
            if(u32Period)
            {
                u32Period++;
            }
            _pwmPeriod_ns[u8Id] = (u32Period * u16Div * common) / pwmclk;
        }
        *Period = _pwmPeriod_ns[u8Id];
    }

    if(Duty != NULL)
    {
        if(_pwmDuty_ns[u8Id] == 0)
        {
            u32Duty = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L) | ((INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_H) & 0x3) << 16);
            if(u32Duty)
            {
                u32Duty++;
            }
            _pwmDuty_ns[u8Id] = (u32Duty * u16Div * common) / pwmclk;
        }
        *Duty = _pwmDuty_ns[u8Id];
    }

}

#else
void DrvPWMSetDuty(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 u32Val)
{
    U32 u32Period = 0x00000000;
    U32 u32Duty = 0x00000000;

    if (PWM_NUM <= u8Id)
        return;

    if (_pwmEnSatus[u8Id])
    {
        if (0 == u32Val)
            OUTREGMSK16(ms_chip->base + u16REG_SW_RESET, BIT0<<u8Id, BIT0<<u8Id);
    }

    u32Period = _pwmPeriod[u8Id];
    u32Duty = ((u32Period * u32Val) / 100);

    pr_err("reg=0x%08X clk=%d, u32Duty=0x%x\n", (U32)(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L), (U32)(clk_get_rate(ms_chip->clk)), u32Duty);
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L, (u32Duty&0xFFFF));
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_H, ((u32Duty>>16)&0x3));

    if (_pwmEnSatus[u8Id])
    {
        U32 reset = INREG16(ms_chip->base + u16REG_SW_RESET) & (BIT0<<u8Id);
        if (u32Val && reset)
            CLRREG16(ms_chip->base + u16REG_SW_RESET, 1<<u8Id);
    }
}

void DrvPWMGetDuty(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32* pu32Val)
{
    U32 u32Duty;

    *pu32Val = 0;
    if (PWM_NUM <= u8Id)
        return;
    u32Duty = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L) | ((INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_H) & 0x3) << 16);
    if (u32Duty)
    {
        U32 u32Period = _pwmPeriod[u8Id];
        // DrvPWMGetPeriod(ms_chip, u8Id, &u32Period);
        if (u32Period)
        {
            *pu32Val = (u32Duty * 100)/u32Period;
        }
    }
}

//------------------------------------------------------------------------------
//
//  Function:   DrvPWMSetPeriod
//
//  Description
//      Set Period value
//
//  Parameters
//      u8Id:    [in] PWM ID
//      u16Val:  [in] Period value
//
//  Return Value
//      None
//
void DrvPWMSetPeriod(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 u32Val)
{
    U32 u32Period = 0x00000000;
    U16 u16Div = 0;

    u32Period=(U32)(clk_get_rate(ms_chip->clk))/u32Val;

    //[APN] range 2<=Period<=262144
    if(u32Period < 2)
        u32Period = 2;
    if(u32Period > 262144){
        u16Div = u32Period/262144;
        u16Div--;
        u32Period = 262144;
    }
    //[APN] PWM _PERIOD= (REG_PERIOD+1)
    u32Period--;

    pr_err("reg=0x%08X clk=%d, period=0x%x\n", (U32)(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L), (U32)(clk_get_rate(ms_chip->clk)), u32Period);

    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L, (u32Period&0xFFFF));
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_H, ((u32Period>>16)&0x3));
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DIV, (u16Div & 0xFFFF));

    _pwmPeriod[u8Id] = u32Period;
}

void DrvPWMGetPeriod(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32* pu32Val)
{
    U32 u32Period;
    U16 u16Div;

    u32Period = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L) | ((INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_H) & 0x3) << 16);
    if ((0 == _pwmPeriod[u8Id]) && (u32Period))
    {
        _pwmPeriod[u8Id] = u32Period;
    }
    u16Div = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DIV) + 1;

    *pu32Val = 0;
    if (u32Period)
    {
        *pu32Val = (U32)(clk_get_rate(ms_chip->clk))/(u32Period+1) * u16Div;
    }
}
#endif
//------------------------------------------------------------------------------
//
//  Function:   DrvPWMSetPolarity
//
//  Description
//      Set Polarity value
//
//  Parameters
//      u8Id:   [in] PWM ID
//      u8Val:  [in] Polarity value
//
//  Return Value
//      None
//
void DrvPWMSetPolarity(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    OUTREGMSK16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (u8Val<<POLARITY_BIT), (0x1<<POLARITY_BIT));
}

void DrvPWMGetPolarity(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8* pu8Val)
{
    *pu8Val = (INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL) & (0x1<<POLARITY_BIT)) ? 1 : 0;
}

#if 0
void DrvPWMSetFreqDiv( U8 u8Id, U8 u8Val )
{
    if( PWM0 == u8Id )
    {
        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM0_DIV, u8Val, PWM_CTRL_DIV_MSAK );
    }
    else if( PWM1 == u8Id )
    {
        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM1_DIV, u8Val, PWM_CTRL_DIV_MSAK );
    }
    else if( PWM2 == u8Id )
    {
        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM2_DIV, u8Val, PWM_CTRL_DIV_MSAK );
    }
    else if( PWM3 == u8Id )
    {
        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM3_DIV, u8Val, PWM_CTRL_DIV_MSAK );
    }
    else
    {
        printk(KERN_ERR "void DrvPWMSetDiv error!!!! (%x, %x)\r\n", u8Id, u8Val);
    }

}
#endif

//------------------------------------------------------------------------------
//
//  Function:   DrvPWMSetDben
//
//  Description
//      Enable/Disable Dben function
//
//  Parameters
//      u8Id:   [in] PWM ID
//      u8Val:  [in] On/Off value
//
//  Return Value
//      None
//

void DrvPWMSetDben(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    OUTREGMSK16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (u8Val<<DBEN_BIT), (0x1<<DBEN_BIT));
    OUTREGMSK16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (u8Val<<VDBEN_SW_BIT), (0x1<<VDBEN_SW_BIT));
}

void DrvPWMEnable(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    if (PWM_NUM <= u8Id)
        return;
    DrvPWMSetDben(ms_chip, u8Id, 1);

    if(u8Val)
    {
#ifdef CONFIG_PWM_NEW
        CLRREG16(ms_chip->base + u16REG_SW_RESET, 1<<u8Id);
#else
        U32 u32DutyL = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L);
        U32 u32DutyH = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_H);
        if (u32DutyL || u32DutyH)
        {
            CLRREG16(ms_chip->base + u16REG_SW_RESET, 1<<u8Id);
        }
        else
        {
            SETREG16(ms_chip->base + u16REG_SW_RESET, 1<<u8Id);
        }
#endif
    }
    else
    {
        SETREG16(ms_chip->base + u16REG_SW_RESET, 1<<u8Id);
    }
    _pwmEnSatus[u8Id] = u8Val;
}

void DrvPWMEnableGet(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8* pu8Val)
{
    *pu8Val = 0;
    if (PWM_NUM <= u8Id)
        return;
    *pu8Val = _pwmEnSatus[u8Id];
}

/*
void DrvPWMPad_dump(void)
{
    int i;
    for (i = 0; i < pwmNum; i++)
    {
        pwmPadTbl_t* pTbl = padTbl[i];
        printk("[%s][%d] %d ------------------------------\n", __FUNCTION__, __LINE__, i);
        while (1)
        {
            regSet_t* pRegSet = pTbl->regSet;
            printk("[%s][%d]     ******************************\n", __FUNCTION__, __LINE__);
            printk("[%s][%d]         pad Id = %d\n", __FUNCTION__, __LINE__, pTbl->u32PadId);
            printk("[%s][%d]         (reg, val, msk) = (0x%08x, 0x%08x, 0x%08x)\n", __FUNCTION__, __LINE__, pRegSet[0].u32Adr, pRegSet[0].u32Val, pRegSet[0].u32Msk);
            printk("[%s][%d]         (reg, val, msk) = (0x%08x, 0x%08x, 0x%08x)\n", __FUNCTION__, __LINE__, pRegSet[1].u32Adr, pRegSet[1].u32Val, pRegSet[1].u32Msk);
            if (PAD_UNKNOWN == pTbl->u32PadId)
            {
                break;
            }
            pTbl++;
        }
    }
}
*/// example
//    DrvPWMPadSet(0, PAD_GPIO1);
//    DrvPWMPadSet(1, PAD_GPIO2);
//    DrvPWMPadSet(2, PAD_GPIO3);
//    DrvPWMPadSet(3, PAD_GPIO4);

extern S32 HalPadSetVal(U32 u32PadID, U32 u32Mode);
void DrvPWMPadSet(U8 u8Id, U8 u8Val)
{
    pwmPadTbl_t* pTbl = NULL;
//    U16 u16Temp;

    if (PWM_NUM <= u8Id) //Coverity: Fix Out of bounds read(OVERRUN)
    {
        return;
    }

    printk("[%s][%d] (pwmId, padId) = (%d, %d)\n", __FUNCTION__, __LINE__, u8Id, u8Val);

    pTbl = padTbl[u8Id];
    while (1)
    {
        if (u8Val == pTbl->u32PadId)
        {
            if (BASE_REG_NULL != pTbl->u32Mode)
            {
                HalPadSetVal(u8Val, pTbl->u32Mode);
            }
            break;
        }
        if (PAD_UNKNOWN == pTbl->u32PadId)
        {
            printk("[%s][%d] void DrvPWMEnable error!!!! (%x, %x)\r\n", __FUNCTION__, __LINE__, u8Id, u8Val);
            break;
        }
        pTbl++;
    }
}


int DrvPWMGroupCap(void)
{
    return (PWM_GROUP_NUM) ? 1 : 0;
}

int DrvPWMGroupJoin(struct mstar_pwm_chip* ms_chip, U8 u8PWMId, U8 u8Val)
{
    if (PWM_NUM <= u8PWMId)
        return 0;
    if(u8Val)
        SETREG16(ms_chip->base + REG_GROUP_JOIN, (1 << (u8PWMId + REG_GROUP_JOIN_SHFT)));
    else
        CLRREG16(ms_chip->base + REG_GROUP_JOIN, (1 << (u8PWMId + REG_GROUP_JOIN_SHFT)));
    return 1;
}

int DrvPWMGroupEnable(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U8 u8Val)
{
    U32 u32JoinMask;

    if (PWM_GROUP_NUM <= u8GroupId)
        return 0;
   
    if(!isSync)
    {
        MDEV_PWM_MemToReg(ms_chip, u8GroupId);
        MS_PWM_DBG("MEM to REG done !\n");
    }

    u32JoinMask = 0xF << ((u8GroupId << 2) + REG_GROUP_JOIN_SHFT);
    u32JoinMask = INREG16(ms_chip->base + REG_GROUP_JOIN) & u32JoinMask;
    u32JoinMask |= 1 << (u8GroupId + PWM_NUM);

    if (u8Val)
    {
        SETREG16(ms_chip->base + REG_GROUP_ENABLE, (1 << (u8GroupId + REG_GROUP_ENABLE_SHFT)));
        CLRREG16(ms_chip->base + u16REG_SW_RESET, u32JoinMask);
        isSync=1; //dont need to sync until new data in
    }
    else
    {
        CLRREG16(ms_chip->base + REG_GROUP_ENABLE, (1 << (u8GroupId + REG_GROUP_ENABLE_SHFT))); //ori 
        SETREG16(ms_chip->base + u16REG_SW_RESET, u32JoinMask);
        
    }

    timeStart[u8GroupId]=getCurNs(); // start round ns

    return 1;
}

int DrvPWMGroupIsEnable(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U8* pu8Val)
{
    *pu8Val = 0;
    if (PWM_GROUP_NUM <= u8GroupId)
        return 0;
    *pu8Val = (INREG16(ms_chip->base + REG_GROUP_ENABLE) >> (u8GroupId + REG_GROUP_ENABLE_SHFT)) & 0x1;
    return 1;
}

int DrvPWMGroupGetRoundNum(struct mstar_pwm_chip* ms_chip, U8 u8PwmId, U16* u16Val)
{
    U32 u32GroupRound = 0;
    timeStop=getCurNs(); //ns	
    
    if (PWM_NUM <= u8PwmId)
        return 0;

    printk("freq=%d \n",_pwmFreq[u8PwmId]);
    timeVal=(timeStop-timeStart[u8PwmId/4])*_pwmFreq[u8PwmId];
    timeVal/=1000000000;
    //do_div(timeVal,1000000000);
    u32GroupRound = INREG16(ms_chip->base + (((u8PwmId>>2) << 0x7) + 0x40)) & 0xFFFF;

    if(u32GroupRound){
        timeVal+=totalRounds[u8PwmId];
        }
    else{
        printk("Please set round!");
        timeVal=totalRounds[u8PwmId];
    }
    *u16Val = timeVal;
    printk("round take %lld times \n",timeVal);

    return 1;
}

int DrvPWMGroupSetRound(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U16 u16Val)
{
    U32 u32Reg;

    if (PWM_GROUP_NUM <= u8GroupId)
        return 0;
    u32Reg = (u8GroupId << 0x7) + 0x40; //(GroupId * 0x20<<2) + 0x10<<2
    OUTREG16(ms_chip->base + u32Reg, u16Val);

    //isEmptyRnd=0;
    return 1;
}

int DrvPWMGroupStop(struct mstar_pwm_chip *ms_chip, U8 u8GroupId, U8 u8Val)
{
    int idx;
    U32 u32JoinMask;

    if (PWM_GROUP_NUM <= u8GroupId)
        return 0;

    u32JoinMask = 0xF << ((u8GroupId << 2) + REG_GROUP_JOIN_SHFT);
    u32JoinMask = INREG16(ms_chip->base + REG_GROUP_JOIN) & u32JoinMask;
    u32JoinMask |= 1 << (u8GroupId + PWM_NUM);

    if (u8Val)
    {
        SETREG16(ms_chip->base + REG_GROUP_STOP, (1 << (u8GroupId + REG_GROUP_STOP_SHFT)));
        SETREG16(ms_chip->base + u16REG_SW_RESET, u32JoinMask);
    }
    else
    {
        CLRREG16(ms_chip->base + REG_GROUP_STOP, (1 << (u8GroupId + REG_GROUP_STOP_SHFT)));
        CLRREG16(ms_chip->base + u16REG_SW_RESET, u32JoinMask);
    }

    for (idx = (u8GroupId * PWM_PER_GROUP); idx < ((u8GroupId+1) * PWM_PER_GROUP); idx++)
    {
        if ((u8Val) && (idx < PWM_NUM))
        { 
            totalRounds[idx] = 0;
//            _pwmStopFreq[idx]= _pwmFreq[idx];
            _pwmFreq[idx] = 0;
        }
        else{
            timeStart[u8GroupId]=getCurNs();
            _pwmFreq[idx]= _pwmStopFreq[idx];// recover freq
            }
    }
    return 1;
}

int DrvPWMGroupHold(struct mstar_pwm_chip *ms_chip, U8 u8GroupId, U8 u8Val)
{
    if (PWM_GROUP_NUM <= u8GroupId)
        return 0;

    if (u8Val)
        SETREG16(ms_chip->base + REG_GROUP_HOLD, (1 << (u8GroupId + REG_GROUP_HOLD_SHFT)));
    else
        CLRREG16(ms_chip->base + REG_GROUP_HOLD, (1 << (u8GroupId + REG_GROUP_HOLD_SHFT)));

    return 1;
}
//+++[Only4I6e]
int DrvPWMGroupGetHoldM1(struct mstar_pwm_chip *ms_chip)
{
#if 0
    return INREG16(ms_chip->base + REG_GROUP_HOLD_MODE1);
#else
    //printk("\n[WARN][%s L%d] Only4i6e\n", __FUNCTION__, __LINE__);
    return 1;
#endif
}

int DrvPWMGroupHoldM1(struct mstar_pwm_chip *ms_chip, U8 u8Val)
{
#if 0
    if (u8Val) {
        SETREG16(ms_chip->base + REG_GROUP_HOLD_MODE1, 1);
        printk("[%s L%d] hold mode1 en!(keep low)\n", __FUNCTION__, __LINE__);
    }
    else {
        CLRREG16(ms_chip->base + REG_GROUP_HOLD_MODE1, 0);
        printk("[%s L%d] hold mode1 dis!\n", __FUNCTION__, __LINE__);
    }
#else
    //printk("\n[WARN][%s L%d] Only4i6e\n", __FUNCTION__, __LINE__);
#endif
    return 1;
}

int DrvPWMDutyQE0(struct mstar_pwm_chip *ms_chip, U8 u8GroupId, U8 u8Val)
{
#if 1
    if (PWM_GROUP_NUM <= u8GroupId)
        return 0;

    printk("[%s L%d] grp:%d x%x(%d)\n", __FUNCTION__, __LINE__, u8GroupId, u8Val, u8Val);
    if (u8Val)
        SETREG16(ms_chip->base + REG_PWM_DUTY_QE0, (1 << (u8GroupId + REG_PWM_DUTY_QE0_SHFT)));
    else
        CLRREG16(ms_chip->base + REG_PWM_DUTY_QE0, (1 << (u8GroupId + REG_PWM_DUTY_QE0_SHFT)));
#else
    //printk("\n[WARN][%s L%d] Only4i6e id:%d\n", __FUNCTION__, __LINE__, u8GroupId);
#endif
    return 1;
}

int DrvPWMGetOutput(struct mstar_pwm_chip *ms_chip, U8* pu8Output)
{
#if 0
    *pu8Output = INREG16(ms_chip->base + REG_PWM_OUT);
    printk("[%s L%d] output:x%x\n", __FUNCTION__, __LINE__, *pu8Output);
#else
    //printk("\n[WARN][%s L%d] Only4i6e\n", __FUNCTION__, __LINE__);
#endif
    return 1;
}
//---[Only4I6e]
// I6b0 --> end --> arguement in memory
int DrvPWMSetEnd(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8DutyId, U32 u32Val)
{
    U32 u32Period;
    U32 u32Duty;

    if (PWM_NUM <= u8Id)
        return 0;

    // u32Period = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L) + ((INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_H)<<16));
    u32Period = _pwmPeriod[u8Id];
    u32Duty = ((u32Period * u32Val) / 1000);
    //matt
    _pwmDutyId[u8Id]=u8DutyId;
    _pwmDuty[u8Id][u8DutyId]=u32Duty;
    isSync=0;
    
    if (u32Duty & 0xFFFC0000)
    {
        printk("[%s][%d] too large duty 0x%08x (18 bits in max)\n", __FUNCTION__, __LINE__, u32Duty);
    }
    return 1;
}

int DrvPWMSetEndToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8DutyId, U32 u32Duty[3][4])
{
    int id;                     
    for(id=0; id<=u8DutyId;id++){	
        if (0 == id)
        {
            OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L, (u32Duty[u8Id][id]&0xFFFF));
            OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_H, ((u32Duty[u8Id][id]>> 16)&0x0003));
        }
        else
        {
            OUTREG16(ms_chip->base + (u8Id*0x80) + (id << 3) + 28, (u32Duty[u8Id][id]&0xFFFF));
        }
        return 1;
    }
return 1;
}

// I6b0 --> begin --> arguement in memory
int DrvPWMSetBegin(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8ShftId, U32 u32Val)
{
    U32 u32Period;
    U32 u32Shft;

    if (PWM_NUM <= u8Id)
        return 0;

    u32Period = _pwmPeriod[u8Id];
    u32Shft = ((u32Period * u32Val) / 1000);
//matt
    _pwmShftId[u8Id]=u8ShftId;
    _pwmShft[u8Id][u8ShftId]=u32Shft;
    isSync=0;
    
    if (u32Shft & 0xFFFC0000)
    {
        printk("[%s][%d] too large shift 0x%08x (18 bits in max)\n", __FUNCTION__, __LINE__, u32Shft);
    }

    return 1;
}

int DrvPWMSetBeginToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8ShftId, U32 u32Shft[3][4])
{
    int id;                         
    for(id=0; id<=u8ShftId;id++){
        if (0 == id)
        {
            OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_SHIFT_L, (u32Shft[u8Id][id]&0xFFFF));
            OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_SHIFT_H, ((u32Shft[u8Id][id]>> 16)&0x0003));
        }
        else
        {
            OUTREG16(ms_chip->base + (u8Id*0x80) + (id << 3) + 24, (u32Shft[u8Id][id]&0xFFFF));
        }
        return 1;		
    }
    return 1;	
}

// I6b0 --> polarity --> arguement in memory
int DrvPWMSetPolarityEx(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    if (PWM_NUM<= u8Id)
        return 0;
    //matt
    _pwmPolarity[u8Id]=u8Val;
    isSync=0;
	
    return 1;
}

int DrvPWMSetPolarityExToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    if (u8Val)
        SETREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (0x1<<POLARITY_BIT));
    else
        CLRREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (0x1<<POLARITY_BIT));
    return 1;
}

// I6b0 --> period --> arguement in memory
void DrvPWMSetPeriodEx(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 u32Val)
{
    U32 u32Period;
    U32 u32Div;

    u32Div = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DIV); // workaround

    u32Period=(U32)(clk_get_rate(ms_chip->clk))/u32Val;
    u32Period /= (u32Div + 1); // workaround

    //[APN] range 2<=Period<=262144
    if(u32Period < 2)
        u32Period = 2;
    if(u32Period > 262144)
        u32Period = 262144;

    //[APN] PWM _PERIOD= (REG_PERIOD+1)
    u32Period--;
	
    //matt
    _pwmPeriod[u8Id] = u32Period;
    //_pwmFreq[u8Id] = (U32)(clk_get_rate(ms_chip->clk))/(_pwmPeriod[u8Id] * (u32Div+1));//for round number
    _pwmFreq[u8Id]=u32Val;
    _pwmStopFreq[u8Id]=u32Val;
    isSync=0;

    pr_err("reg=0x%08X clk=%d, period=0x%x\n", (U32)(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L), (U32)(clk_get_rate(ms_chip->clk)), u32Period);

}

void DrvPWMSetPeriodExToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 u32Period)
{
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L, (u32Period&0xFFFF));
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_H, ((u32Period>>16)&0x3));
}

#if 0
int DrvPWMSetMPluse(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    if (PWM_NUM <= u8Id)
        return 0;
    if (u8Val)
        SETREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (0x1<<DIFF_P_EN_BIT));
    else
        CLRREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (0x1<<DIFF_P_EN_BIT));
    return 1;
}
#endif

int DrvPWMDiv(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    if (PWM_NUM <= u8Id)
        return 0;
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DIV, u8Val);
    MS_PWM_DBG("mhal  DrvPWMDiv done !\n");
        
    return 1;
}

int DrvPWMGroupShowRoundNum(struct mstar_pwm_chip *ms_chip, char* buf_start, char* buf_end)
{
    char *str = buf_start;
    char *end = buf_end;
    int i;
    U32 u32GroupRound;

    timeStop=getCurNs(); //ns

    for (i = 0; i < PWM_NUM; i++)
    {
        u32GroupRound = INREG16(ms_chip->base + (((i/4) << 0x7) + 0x40)) & 0xFFFF;
        timeVal = (timeStop-timeStart[i/4])*_pwmFreq[i];
        timeVal /= 1000000000;
        timeVal = (timeVal > u32GroupRound) ? u32GroupRound : timeVal;
        timeVal+=totalRounds[i];
        str += scnprintf(str, end - str, "PWM%d\t Round = \t%lld\n", i, timeVal);
    }

    return (str - buf_start);
}

int DrvPWMGroupInfo(struct mstar_pwm_chip *ms_chip, char* buf_start, char* buf_end)
{
    char *str = buf_start;
    char *end = buf_end;
    int i;
    // U32 tmp;
    U32 u32Period, u32Polarity; // , u32MPluse;
    // U32 u32Shft0, u32Shft1, u32Shft2, u32Shft3;
    U32 u32Shft0;
    // U32 u32Duty0, u32Duty1, u32Duty2, u32Duty3;
    U32 u32Duty0;
    U32 u32SyncStatus;
    U32 u32ResetStatus;
    U32 u32GroupEnable, u32GroupReset, u32GroupHold, u32GroupStop, u32GroupRound;
    U32 clk = (U32)clk_get_rate(ms_chip->clk);
    U32 u32Div;

    if (0 == DrvPWMGroupCap())
    {
        str += scnprintf(str, end - str, "This chip does not support motor interface\n");
        return (str - buf_start);
    }

    str += scnprintf(str, end - str, "================================================\n");
    for (i = 0; i < PWM_GROUP_NUM; i++)
    {
        U32 pwmIdx;
        U32 j;

        // group enable
        u32GroupEnable = (INREG16(ms_chip->base + REG_GROUP_ENABLE) >> i) & 0x1;
        // group reset 
        u32GroupReset = (INREG16(ms_chip->base + u16REG_SW_RESET) >> (i + PWM_NUM)) & 0x1;
        // hold
        u32GroupHold = (INREG16(ms_chip->base + REG_GROUP_HOLD) >> (i + REG_GROUP_HOLD_SHFT)) & 0x1;
        // stop
        u32GroupStop = (INREG16(ms_chip->base + REG_GROUP_STOP) >> (i + REG_GROUP_STOP_SHFT)) & 0x1;
        // round
        u32GroupRound = INREG16(ms_chip->base + ((i << 0x7) + 0x40)) & 0xFFFF;

        str += scnprintf(str, end - str, "Group %d\n", i);
        pwmIdx = (i << 2);
        str += scnprintf(str, end - str, "\tmember\t\t");
        for (j = pwmIdx; j < pwmIdx + 4; j++)
        {
            if (j < PWM_NUM)
            {
                str += scnprintf(str, end - str, "%d ", j);
            }
        }
        str += scnprintf(str, end - str, "\n");
        str += scnprintf(str, end - str, "\tenable status\t%d\n", u32GroupEnable);
        str += scnprintf(str, end - str, "\tReset status\t%d\n", u32GroupReset);
        str += scnprintf(str, end - str, "\tHold\t\t%d\n", u32GroupHold);
        str += scnprintf(str, end - str, "\tStop\t\t%d\n", u32GroupStop);
        str += scnprintf(str, end - str, "\tRound\t\t%d\n", u32GroupRound);
    }

    str += scnprintf(str, end - str, "================================================\n");
    for (i = 0; i < PWM_NUM; i++)
    {
        // Polarity
        u32Polarity = (INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_CTRL) >> POLARITY_BIT) & 0x1;
        // u32MPluse = (INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_CTRL) >> DIFF_P_EN_BIT) & 0x1;
        // Period
#if 0
        if ((tmp = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_PERIOD_H)))
            printk("[%s][%d] pwmId %d period_h is not zero (0x%08x)\n", __FUNCTION__, __LINE__, i, tmp);
#endif
        u32Period = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_PERIOD_L);
        // Shift
#if 0
        if ((tmp = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_SHIFT_H)))
            printk("[%s][%d] pwmId %d shift_h is not zero (0x%08x)\n", __FUNCTION__, __LINE__, i, tmp);
#endif
        u32Shft0 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_SHIFT_L);
        // u32Shft1 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_SHIFT2);
        // u32Shft2 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_SHIFT3);
        // u32Shft3 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_SHIFT4);
        // Duty
#if 0
        if ((tmp = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_DUTY_H)))
            printk("[%s][%d] pwmId %d duty_h is not zero (0x%08x)\n", __FUNCTION__, __LINE__, i, tmp);
#endif
        u32Duty0 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_DUTY_L);
        // u32Duty1 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_DUTY2);
        // u32Duty2 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_DUTY3);
        // u32Duty3 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_DUTY4);
        // sync mode status
        u32SyncStatus = (INREG16(ms_chip->base + REG_GROUP_JOIN) >> (i + REG_GROUP_JOIN_SHFT)) & 0x1;
        // rest status
        u32ResetStatus = (INREG16(ms_chip->base + u16REG_SW_RESET) >> i) & 0x1;
        u32Div = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_DIV); // workaround


        // output to buffer
        str += scnprintf(str, end - str, "Pwm %d\n", i);
        str += scnprintf(str, end - str, "\tPad\t\t0x%08x\n", ms_chip->pad_ctrl[i]);
        str += scnprintf(str, end - str, "\tSync status\t%d\n", u32SyncStatus);
        str += scnprintf(str, end - str, "\tReset status\t%d\n", u32ResetStatus);
        str += scnprintf(str, end - str, "\tPolarity\t%d\n", u32Polarity);
#if 0
        str += scnprintf(str, end - str, "\tPeriod\t\t0x%08x\n", u32Period);
        str += scnprintf(str, end - str, "\tBegin\t\t0x%08x\n", u32Shft0);
        str += scnprintf(str, end - str, "\tEnd\t\t0x%08x\n", u32Duty0);
#endif
        u32Period++;
        u32Shft0++;
        u32Duty0++;
        u32Shft0 = (1000 * u32Shft0)/u32Period;
        u32Duty0 = (1000 * u32Duty0)/u32Period;
        // u32Period = ((u32Div+1)*clk)/u32Period;
        u32Period = clk/u32Period/(u32Div+1);

        str += scnprintf(str, end - str, "\tPeriod\t\t%d\n", u32Period);
        // str += scnprintf(str, end - str, "\tBegin\t\t0x%08x 0x%08x 0x%08x 0x%08x\n", u32Shft0, u32Shft1, u32Shft2, u32Shft3);
        // str += scnprintf(str, end - str, "\tEnd\t\t0x%08x 0x%08x 0x%08x 0x%08x\n", u32Duty0, u32Duty1, u32Duty2, u32Duty3);
        str += scnprintf(str, end - str, "\tBegin\t\t%d\n", u32Shft0);
        str += scnprintf(str, end - str, "\tEnd\t\t%d\n", u32Duty0);
    }
    // str += scnprintf(str, end - str, "This is a test\n");
    return (str - buf_start);
}
