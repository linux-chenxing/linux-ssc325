/*
* pm.c- Sigmastar
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
#include <linux/suspend.h>
#include <linux/io.h>
#include <asm/suspend.h>
#include <asm/fncpy.h>
#include <asm/cacheflush.h>
#include "ms_platform.h"

#define FIN     printk(KERN_ERR"[%s]+++\n",__FUNCTION__)
#define FOUT    printk(KERN_ERR"[%s]---\n",__FUNCTION__)
#define HERE    printk(KERN_ERR"%s: %d\n",__FILE__,__LINE__)
#define SUSPEND_WAKEUP 0
#define SUSPEND_SLEEP  1
#define STR_PASSWORD   0x5A5A55AA
#define IN_SRAM 1


#if IN_SRAM
#define SUSPENDINFO_ADDRESS 0xA000B000
#else
#define SUSPENDINFO_ADDRESS 0x20000000
#endif
//----0xA0000000~0xA000A000---IPL.rom
//----0xA000C000~0xA000D000---IPL.ram
//----0xA0001000~0xA0002000---suspend code

typedef struct {
    char magic[8];
    unsigned int resume_entry;
    unsigned int count;
    unsigned int status;
    unsigned int password;
#ifdef CONFIG_SS_STRDEBUG
    unsigned int checksum;
#endif
} suspend_keep_info;

extern void sram_suspend_imi(void);
static void (*mstar_suspend_imi_fn)(void);
static void __iomem *suspend_imi_vbase;
suspend_keep_info *pStr_info;
int suspend_status = SUSPEND_WAKEUP;
extern void recode_timestamp(int mark, const char* name);

#ifdef CONFIG_SS_STRDEBUG
#include <asm-generic/sections.h>

int calc_checksum(void *buf, int size)
{
    int i;
    int sum = 0;

    for (i = 0; size > i; i += 4)
        sum += *(volatile int*)(buf + i);
    return sum;
}
#endif

static int mstar_suspend_ready(unsigned long ret)
{
//    long long gpi_irq =0;
    *(unsigned short volatile *) 0xFD200800 = 0x2222;
    mstar_suspend_imi_fn = fncpy(suspend_imi_vbase, (void*)&sram_suspend_imi, 0x1000);
    suspend_status = SUSPEND_SLEEP;

    //resume info
    if (pStr_info) {
        pStr_info->count++;
        pStr_info->status = SUSPEND_SLEEP;
        pStr_info->password = STR_PASSWORD;
    }
    //flush cache to ensure memory is updated before self-refresh
    __cpuc_flush_kern_all();
    //flush L3 cache
    Chip_Flush_MIU_Pipe();
    //flush tlb to ensure following translation is all in tlb
    local_flush_tlb_all();
#ifdef CONFIG_SS_STRDEBUG
    pStr_info->checksum = calc_checksum(_text, __init_begin-1-_text);
#endif
    #if 0
    for(i=PAD_GPIO2; i<=PAD_GPIO12; i++){
        MDrv_GPIO_PadVal_Set(i, 0x0);
        MDrv_GPIO_Set_Low(i);
    }

    for(i=PAD_GPIO14; i<=PAD_SATA_GPIO; i++){
        MDrv_GPIO_PadVal_Set(i, 0x0);
        MDrv_GPIO_Set_Low(i);
    }

        /*clear gpi*/
    for( gpi_irq = 0 ; gpi_irq < GPI_FIQ_END ; gpi_irq++)
    {
        SETREG16( (BASE_REG_GPI_INT_PA + REG_ID_0A + (gpi_irq/16)*4 ) , (1 << (gpi_irq%16)) );
    }
    #endif
    *(unsigned short volatile *) 0xFD200800 = 0x2224;
    mstar_suspend_imi_fn();
    return 0;
}

static int mstar_suspend_enter(suspend_state_t state)
{
    FIN;
    switch (state) 
    {
        case PM_SUSPEND_MEM:
            printk(KERN_INFO "state = PM_SUSPEND_MEM\n");
            cpu_suspend(0, mstar_suspend_ready);
            break;
        default:
            return -EINVAL;
    }

    return 0;
}

struct platform_suspend_ops mstar_suspend_ops = {
    .enter    = mstar_suspend_enter,
    .valid    = suspend_valid_only_mem,
};


int __init mstar_pm_init(void)
{
    unsigned int resume_pbase = virt_to_phys(cpu_resume);
    static void __iomem * suspend_info_vbase = (void *)SUSPENDINFO_ADDRESS;
    suspend_imi_vbase = __arm_ioremap_exec(0xA0001000, 0x1000, false);  //put suspend code at IMI offset 64K;
    suspend_info_vbase =  __arm_ioremap_exec(SUSPENDINFO_ADDRESS, 0x1000, false);
    pStr_info = (suspend_keep_info *)(suspend_info_vbase);
    memset(pStr_info, 0, sizeof(suspend_keep_info));
    strcpy(pStr_info->magic, "SIG_STR");
    pStr_info->resume_entry = resume_pbase;

    suspend_set_ops(&mstar_suspend_ops);

    printk(KERN_INFO "[%s] resume_pbase=0x%08X, suspend_imi_vbase=0x%08X\n", __func__, (unsigned int)resume_pbase, (unsigned int)suspend_imi_vbase);

    return 0;
}

