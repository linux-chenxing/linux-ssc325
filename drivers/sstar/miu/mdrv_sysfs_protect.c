/*
* mdrv_sysfs_protect.c- Sigmastar
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
#include <asm/uaccess.h>  /* for get_fs*/
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>      /* for dma_alloc_coherent */
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <asm/cacheflush.h>
#include "registers.h"
#include "mdrv_miu.h"
#if 0
 #define mkstr(s) #s
 
 // common for all MIU
 struct miu_client {    
// char* name;    
 short bw_client_id;    
 short bw_rsvd;    
 short bw_enabled;    
 short bw_dump_en;    
 short bw_filter_en;    
 short bw_max;    
 short bw_avg;    
 short bw_min;    
 short effi_avg;    
 short effi_min;    
 short effi_max;    
 short bw_max_div_effi;    
 short bw_avg_div_effi;
 }miuT_client[123]=
 {
 {0x00,0,1,0,0},
 {0x01,0,0,0,0},
 {0x02,0,0,0,0},
//    #define INTI_CLIENT(id)   {mkstr(id), id, 0, 1, 0, 0}    
//    INTI_CLIENT(MIU_CLIENT_VEN_R),
//    INTI_CLIENT(MIU_CLIENT_VEN_R),
//    INTI_CLIENT(MIU_CLIENT_VEN_W),
 };
//const char* halClientIDTNameTest(int id)
//{
//    int i = 0;
//    for (i = 0; i < MIU0_CLIENT_NUM; i++) {
//        if (miuT_client[i].bw_client_id == id) {
//            return eMIUClientID (i);
//        }
//    }
//    return NULL;
//}
// 
#endif
static struct miu_device protect_dev[1];
static ssize_t clientID_entry(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
// printk("id: %d na: %s \r\n",MIU_CLIENT_MIIC0_RW, mkstr(MIU_CLIENT_MIIC0_RW));    
// printk("id: %d name: %s \r\n",MIU_CLIENT_VEN_R, miuT_client[MIU_CLIENT_VEN_R].name);    
// printk("id: %d name: %d \r\n",MIU_CLIENT_MIIC0_RW, miuT_client[MIU_CLIENT_MIIC0_RW].bw_client_id);
    int i =0;
    char name[40]; 
    for(i=0;i<=30;i++){
            clientId_KernelProtectToName(i, name);
            printk("id: %d name: %s \r\n",i,  name);
    }
    return n;
}
static ssize_t clientID_entry_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    int n;

    n = sprintf(str, "[%s][%d]: OK!\n", __FUNCTION__, __LINE__);
    return n;
}


DEVICE_ATTR(client_ID, 0644, clientID_entry_show, clientID_entry);

static ssize_t miu_protect_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    int n;

    n = sprintf(str, "[%s][%d]: OK!\n", __FUNCTION__, __LINE__);
    return n;
}

static ssize_t miu_protect_entry(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    unsigned char Result = FALSE;
    unsigned char u8Blockx;
    unsigned short u8ProtectId[16] = {0};
    unsigned long long u64BusStart;
    unsigned long long u64BusEnd;
    unsigned char  bSetFlag;

    unsigned char token[16];

    sscanf(buf, "%s", token);
    if (0 == strcasecmp(token, "set"))
    {
        sscanf(buf, "%s %hhu %hu %llx %llx %hhu", token, &u8Blockx, &u8ProtectId[0], &u64BusStart, &u64BusEnd, &bSetFlag);

        printk("%s(%d) INPUT: (u8Blockx,u8ProtectId,u64BusStart,u64BusEnd,bSetFlag)=(%hhu,%hu,0x%08llx,0x%08llx,%hhu)\n", __FUNCTION__, __LINE__, u8Blockx, u8ProtectId[0], u64BusStart, u64BusEnd, bSetFlag);

        Result = MDrv_MIU_Protect(u8Blockx, &u8ProtectId[0], u64BusStart, u64BusEnd, bSetFlag);

        if(Result == FALSE ) {
            printk("ERR: Result = %d", Result);
        }
    }
    else if (0 == strcasecmp(token, "test"))
    {
        unsigned int* _va;
        sscanf(buf, "%s %llx", token, &u64BusStart);

        _va = ioremap((unsigned int)u64BusStart, 0x1000);
        printk("%s(%d) Write: MIU @ 0x%08llx, VA @ 0x%08x\n", __FUNCTION__, __LINE__, u64BusStart, (unsigned int)_va);

        *(_va) = 0xDEADBEEF;
    }
    else if (0 == strcasecmp(token, "test1"))
    {
        sscanf(buf, "%s %llx", token, &u64BusStart);

        printk("%s(%d) Write: MIU @ 0x%08llx\n", __FUNCTION__, __LINE__, u64BusStart);

        *(unsigned int*)((void*)(unsigned int)u64BusStart) = 0xDEADBEEF;
    }
    else
    {
         printk("%s(%d) Wrong parameter:\n", __FUNCTION__, __LINE__);
         printk("%s(%d) Usage: echo [CMD] [miublk] [client id] [start addr] [end addr] [enable] > miu_protect\n", __FUNCTION__, __LINE__);
         printk("%s(%d) Ex: echo set 0 1 0x20000000 0x20100000 1 > miu_protect\n", __FUNCTION__, __LINE__);
    }

    return n;
}
DEVICE_ATTR(miu_protect, 0644, miu_protect_show, miu_protect_entry);

void create_miu_protect_node(struct bus_type *miu_subsys)
{
    int ret;
    protect_dev[0].index = 0;
    protect_dev[0].dev.kobj.name = "miu_protect";
    protect_dev[0].dev.bus = miu_subsys;

    ret = device_register(&protect_dev[0].dev);
    if (ret) {
        printk(KERN_ERR "Failed to register %s device!! %d\n", protect_dev[0].dev.kobj.name, ret);
        return;
    }
    device_create_file(&protect_dev[0].dev, &dev_attr_miu_protect);
    device_create_file(&protect_dev[0].dev, &dev_attr_client_ID);
}
