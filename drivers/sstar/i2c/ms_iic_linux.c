/*
* ms_iic.c- Sigmastar
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
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <asm/io.h>
#include <linux/clk-provider.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
//#include "mst_devid.h"
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/miscdevice.h>

#include "ms_iic.h"
#include "mhal_iic_reg.h"
#include "mhal_iic.h"
#include "ms_platform.h"
#include "cam_sysfs.h"
#include "drv_camclk_Api.h"
#include <ms_iic_io.h>
#include "mdrv_gpio.h"
#include "mdrv_padmux.h"

#define I2C_ACCESS_DUMMY_TIME   5//3
///////////////////////////////////////////&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&////////////////////////////////////////////////
///////////////////////////////////////////&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&////////////////////////////////////////////////
///////////////////////////////////////////&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&////////////////////////////////////////////////
// Local defines & local structures
////////////////////////////////////////////////////////////////////////////////
//define hwi2c ports
#define HWI2C_PORTS                   HAL_HWI2C_PORTS
#define HWI2C_PORT0                   HAL_HWI2C_PORT0
#define HWI2C_PORT1                   HAL_HWI2C_PORT1
#define HWI2C_PORT2                   HAL_HWI2C_PORT2
#define HWI2C_PORT3                   HAL_HWI2C_PORT3
#define HWI2C_STARTDLY                5 //us
#define HWI2C_STOPDLY                 5 //us
//define mutex

#define HWI2C_MUTEX_CREATE(_P_)          //g_s32HWI2CMutex[_P_] = MsOS_CreateMutex(E_MSOS_FIFO, (char*)gu8HWI2CMutexName[_P_] , MSOS_PROCESS_SHARED)
#define HWI2C_MUTEX_LOCK(_P_)            //OS_OBTAIN_MUTEX(g_s32HWI2CMutex[_P_],MSOS_WAIT_FOREVER)
#define HWI2C_MUTEX_UNLOCK(_P_)          //OS_RELEASE_MUTEX(g_s32HWI2CMutex[_P_])
#define HWI2C_MUTEX_DELETE(_P_)          //OS_DELETE_MUTEX(g_s32HWI2CMutex[_P_])
#define MsOS_DelayTaskUs(x)				CamOsUsDelay(x)
#define MS_DEBUG_MSG(x)       x

#define I2CNAME "i2c-"

//static MS_S32 g_s32HWI2CMutex[HWI2C_PORTM] = {-1,-1,-1,-1};
//static char gu8HWI2CMutexName[HWI2C_PORTM][13] = {"HWI2CMUTEXP0","HWI2CMUTEXP1","HWI2CMUTEXP2","HWI2CMUTEXP3"};

//#define EN_I2C_LOG
#ifdef EN_I2C_LOG
#define HWI2C_DBG_FUNC()               if (_geDbgLv >= E_HWI2C_DBGLV_ALL) \
                                            {MS_DEBUG_MSG(CamOsPrintf("\t====   %s   ====\n", __FUNCTION__);)}
#define HWI2C_DBG_INFO(x, args...)     if (_geDbgLv >= E_HWI2C_DBGLV_INFO ) \
                                            {MS_DEBUG_MSG(CamOsPrintf(x, ##args);)}
#define HWI2C_DBG_ERR(x, args...)      if (_geDbgLv >= E_HWI2C_DBGLV_ERR_ONLY) \
                                            {MS_DEBUG_MSG(CamOsPrintf(x, ##args);)}
#else
#define HWI2C_DBG_FUNC()               //CamOsPrintf("\t##########################   %s   ################################\n", __FUNCTION__)
#define HWI2C_DBG_INFO(x, args...)     //CamOsPrintf(x, ##args)
#define HWI2C_DBG_ERR(x, args...)      CamOsPrintf(x, ##args)
#endif


//#define I2C_ACCESS_DUMMY_TIME   50
////////////////////////////////////////////////////////////////////////////////
// Local & Global Variables
////////////////////////////////////////////////////////////////////////////////
struct mstar_i2c_dev {
    struct device *dev;
    struct i2c_adapter adapter;
    struct clk *div_clk;
    struct clk *fast_clk;
    struct reset_control *rst;
    void __iomem *base;
    void __iomem *chipbase;
    void __iomem *clkbase;
    int cont_id;
    bool irq_disabled;
    int is_dvc;
    struct completion msg_complete;
    int msg_err;
    u8 *msg_buf;
    size_t msg_buf_remaining;
    int msg_read;
    u32 bus_clk_rate;
    bool is_suspended;
    int i2cgroup;
    int i2cpadmux;
    int i2c_speed;
    int i2c_en_dma;
};

typedef struct _i2c_dev_data{
    u32 i2cirq;
    int i2cgroup;
    struct miscdevice i2c_miscdev;
}i2c_dev_data;

static int  i2c_open(struct inode *inode, struct file *fp);
static long i2c_ioctl(struct file *fp, unsigned int cmd, unsigned long arg);
int  i2c_set_srcclk(u8 u8Port, HWI2C_SRC_CLK i2c_src_clk);
int  i2c_remove_srcclk(u8 u8Port);

static const struct file_operations i2c_fops =
{
    .open     = i2c_open,
    .unlocked_ioctl = i2c_ioctl,
};

#ifdef EN_I2C_LOG
static HWI2C_DbgLv _geDbgLv = E_HWI2C_DBGLV_INFO; //E_HWI2C_DBGLV_ERR_ONLY;
#endif

static struct platform_device *g_pdev[HWI2C_PORTS];

#ifdef CONFIG_CAM_CLK
void **pvhandler;
#endif

static int i2c_open(struct inode *inode, struct file *fp)
{
    return 0;
}
static long i2c_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    //I2C_IOCTL_CMD I2cCmd;
    CamOsPrintf("=======================i2c_ioctl=======================\n");
    if (_IOC_TYPE(cmd) != I2C_IOCTL_MAGIC) return -ENOTTY;
    switch(cmd)
    {
        case IOCTL_I2C_SET_SPEED:
#if 0
        {
            mutex_lock(&i2cMutex);
            memset(&I2cCmd,0,sizeof(I2C_IOCTL_CMD));
            if(copy_from_user(&I2cCmd, (I2C_IOCTL_CMD *)arg, sizeof(I2C_IOCTL_CMD))){
	            mutex_unlock(&i2cMutex);
	            return -EFAULT;
            }

            ret = i2c_remove_srcclk(I2cCmd.Clk_Cfg.u8Port);
            if(ret){
	            mutex_unlock(&i2cMutex);
	            return -EFAULT;
            }

            ret = _MDrv_HWI2C_SetClk(I2cCmd.Clk_Cfg.u8Port, I2cCmd.Clk_Cfg.eClk);
            mutex_unlock(&i2cMutex);
            return ret;
        }
#endif
        break;
    }
    return ret;
}

void *MDrv_HWI2C_DMA_Alloc(I2C_DMA *dma, int i2cgroup)
{
    return dma_alloc_coherent(NULL, 4096, &dma[i2cgroup].i2c_dma_addr, GFP_KERNEL);
}

void MDrv_HWI2C_DMA_DeAlloc(I2C_DMA *dma, int i2cgroup)
{
    dma_free_coherent(NULL, 4096, dma[i2cgroup].i2c_virt_addr, dma[i2cgroup].i2c_dma_addr);
}

/*
+------------------------------------------------------------------------------
| FUNCTION    : ms_i2c_xfer
+------------------------------------------------------------------------------
| DESCRIPTION : This function will be called by i2c-core.c i2c-transfer()
|               i2c_master_send(), and i2c_master_recv()
|               We implement the I2C communication protocol here
|               Generic i2c master transfer entrypoint.
|
| RETURN      : When the operation is success, it return the number of message
|               requrested. Negative number when error occurs.
|
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| padap               | x |   | the adaptor which the communication will be
|                    |   |   | procceed
|--------------------+---+---+-------------------------------------------------
| pmsg               | x |   | the message buffer, the buffer with message to
|                    |   |   | be sent or used to fill data readed
|--------------------+---+---+-------------------------------------------------
| num                | x |   | number of message to be transfer
+------------------------------------------------------------------------------
*/
//static int
int
ms_i2c_xfer(struct i2c_adapter *padap, struct i2c_msg *pmsg, int length)
{
    s32 ret;
    ret = ms_i2c_xfer_common(padap->nr, pmsg, length);
    switch (ret) {
        case I2C_FAIL:
            ret = -ENOTTY;
        break;
        case I2C_TIMEOUT:
            ret = -ETIMEDOUT;
        break;
        default:
        break;
    }
    return ret;
}
EXPORT_SYMBOL(ms_i2c_xfer);

/*
+------------------------------------------------------------------------------
| FUNCTION    : ms_i2c_func
+------------------------------------------------------------------------------
| DESCRIPTION : This function is returned list of supported functionality.
|
| RETURN      : return list of supported functionality
|
| Variable    : no variable
+------------------------------------------------------------------------------
*/
static u32 ms_i2c_func(struct i2c_adapter *padapter)
{
    return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

/* implement the i2c transfer function in algorithm structure */
static struct i2c_algorithm sg_ms_i2c_algorithm =
{
    .master_xfer = ms_i2c_xfer,
    .functionality = ms_i2c_func,
};

/* Match table for of_platform binding */
static const struct of_device_id mstar_i2c_of_match[] = {
	{ .compatible = "sstar,i2c", 0},
	{},
};

int i2c_set_srcclk(u8 u8Port, HWI2C_SRC_CLK i2c_src_clk)
{
    int ret = 0;
    int num_parents;
    int i;
    int clk_speed;
    struct platform_device *pdev = g_pdev[u8Port];

#ifndef CONFIG_CAM_CLK
    struct clk **iic_clks;
#else
    int *iic_clks;
    CAMCLK_Set_Attribute stSetCfg;
#endif

    switch (i2c_src_clk)
    {
        case E_HWI2C_SRC_CLK_12M:
            clk_speed = 12000000;
        break;
        case E_HWI2C_SRC_CLK_54M:
            clk_speed = 54000000;
        break;
        case E_HWI2C_SRC_CLK_72M:
            clk_speed = 72000000;
        break;
        default:
            CamOsPrintf( "[%s] Fail to get clk!\n", __func__);
            return FALSE;
        break;
    }
#ifndef CONFIG_CAM_CLK
    num_parents = of_clk_get_parent_count(pdev->dev.of_node);
    if(num_parents < 0)
    {
        CamOsPrintf( "[%s] Fail to get parent count! Error Number : %d\n", __func__, num_parents);
        ret = -ENOENT ;
        goto out;
    }

    iic_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);
    if(!iic_clks){
        ret = -ENOMEM;
        goto out;
    }
    //enable all clk
    for(i = 0; i < num_parents; i++)
    {
        iic_clks[i] = of_clk_get(pdev->dev.of_node, i);
        if (IS_ERR(iic_clks[i]))
        {
            CamOsPrintf( "[%s] Fail to get clk!\n", __func__);
            ret = -ENOENT;
        }
        else
        {
            clk_prepare_enable(iic_clks[i]);
            if(i == 0)
                clk_set_rate(iic_clks[i], clk_speed);
            clk_put(iic_clks[i]);
        }
    }
    kfree(iic_clks);
    if(ret){
        goto out;
    }
#else
    if(of_find_property(pdev->dev.of_node,"camclk",&num_parents))
    {
        num_parents /= sizeof(int);
        //CamOsPrintf( "[%s] Number : %d\n", __func__, num_parents);
        if(num_parents < 0)
        {
            CamOsPrintf( "[%s] Fail to get parent count! Error Number : %d\n", __func__, num_parents);
            ret = -ENOENT ;
            goto out;
        }
        iic_clks = kzalloc((sizeof(int) * num_parents), GFP_KERNEL);
        pvhandler = kzalloc((sizeof(void *) * num_parents), GFP_KERNEL);
        if(!iic_clks){
            ret = -ENOMEM;
            goto out;
        }
        for(i = 0; i < num_parents; i++)
        {
            iic_clks[i] = 0;
            of_property_read_u32_index(pdev->dev.of_node,"camclk", i,&iic_clks[i]);
            if (!iic_clks[i])
            {
                CamOsPrintf( "[%s] Fail to get clk!\n", __func__);
                ret = -ENOENT;
            }
            else
            {
                CamClkRegister("iic",iic_clks[i],&pvhandler[i]);
                CamClkSetOnOff(pvhandler[i],1);
                if(i == 0)
                {
	                CAMCLK_SETRATE_ROUNDUP(stSetCfg,clk_speed);
	                CamClkAttrSet(pvhandler[i],&stSetCfg);
                }
                //CamClkUnregister(pvhandler[i]);
            }
        }
        kfree(iic_clks);
        //kfree(pvhandler);
        if(ret){
            goto out;
        }
    }
    else
    {
        CamOsPrintf( "[%s] W/O Camclk \n", __func__);
    }
#endif
out:
	if(ret)
		return FALSE;
	else
		return TRUE;
}

int i2c_remove_srcclk(u8 u8Port)
{
    int ret = 0;
    int num_parents, i;
    struct platform_device *pdev = g_pdev[u8Port];
#ifndef CONFIG_CAM_CLK
    struct clk **iic_clks;
    struct clk *parent;
#else
    int *iic_clks;
#endif

#ifndef CONFIG_CAM_CLK
    num_parents = of_clk_get_parent_count(pdev->dev.of_node);
    iic_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);

    //disable all clk
    for(i = 0; i < num_parents; i++)
    {
        iic_clks[i] = of_clk_get(pdev->dev.of_node, i);
        if (IS_ERR(iic_clks[i]))
        {
            CamOsPrintf( "[iic_clks] Fail to get clk!\n" );
            kfree(iic_clks);
            return -1;
        }
        else
        {
            //force clock parent to 0, otherwise set rate to 12MHz in resume will not take effect
            parent = clk_hw_get_parent_by_index(__clk_get_hw(iic_clks[i]), 0)->clk;
            //pr_err("%s parent 0 clk: %ld\n", pdev->name, clk_get_rate(parent));
            clk_set_parent(iic_clks[i], parent);
            clk_disable_unprepare(iic_clks[i]);
            clk_put(iic_clks[i]);
        }
    }
    kfree(iic_clks);
#else
    if(of_find_property(pdev->dev.of_node,"camclk",&num_parents))
    {
        num_parents /= sizeof(int);
        //CamOsPrintf( "[%s] Number : %d\n", __func__, num_parents);
        if(num_parents < 0)
        {
            CamOsPrintf( "[%s] Fail to get parent count! Error Number : %d\n", __func__, num_parents);
            goto out;
        }
        iic_clks = kzalloc((sizeof(int) * num_parents), GFP_KERNEL);
        if(!iic_clks){
            goto out;
        }
        for(i = 0; i < num_parents; i++)
        {
            of_property_read_u32_index(pdev->dev.of_node,"camclk", i,&iic_clks[i]);
            if (!iic_clks[i])
            {
                CamOsPrintf( "[%s] Fail to get clk!\n", __func__);
            }
            else
            {
                CamClkSetOnOff(pvhandler[i],0);
                CamClkUnregister(pvhandler[i]);
            }
        }
        kfree(iic_clks);
        kfree(pvhandler);
        pvhandler = NULL;
    }
    else
    {
        CamOsPrintf( "[%s] W/O Camclk \n", __func__);
    }
    out:
#endif
    return ret;
}

static int mstar_i2c_probe(struct platform_device *pdev)
{
    struct mstar_i2c_dev *i2c_dev;
    struct resource *res;
    void __iomem *base;
    void __iomem *chipbase;
    void __iomem *clkbase;
    struct device_node	*node = pdev->dev.of_node;
    int ret = 0;
    int i2cgroup = 0;
    int i2cpadmux = 1;
    int i2c_speed = E_HWI2C_CLK_400KHZ;
    int i2c_en_dma = -1;
    int  IsUseMdrvPadmux = 0;
    //char i2cname[8] = {0};

    i2c_dev_data *data = NULL;

    data = kzalloc(sizeof(i2c_dev_data), GFP_KERNEL);
    if(!data){
        ret = -ENOMEM;
        goto out;
    }
    HWI2C_DBG_INFO(" mstar_i2c_probe\n");
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(!res)
	{
		ret = -ENOENT;
		goto out;
	}
    base = (void *)(IO_ADDRESS(res->start));

    res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    if(!res)
    {
    	ret = -ENOENT;
		goto out;
    }
    chipbase = (void *)(IO_ADDRESS(res->start));

    res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
    if(!res)
    {
    	ret = -ENOENT;
		goto out;
    }
    clkbase = (void *)(IO_ADDRESS(res->start));

    i2c_dev = devm_kzalloc(&pdev->dev, sizeof(*i2c_dev), GFP_KERNEL);
    if (!i2c_dev)
    {
        ret = -ENOMEM;
        goto out;
    }
    i2c_dev->base = base;
    i2c_dev->chipbase = chipbase;
    i2c_dev->adapter.algo = &sg_ms_i2c_algorithm;
    i2c_dev->cont_id = pdev->id;
    i2c_dev->dev = &pdev->dev;
    i2c_dev->clkbase = clkbase;

    of_property_read_u32(node, "i2c-group", &i2cgroup);
    //i2cgroup = of_alias_get_id(pdev->dev.of_node, "iic");
    HWI2C_DBG_INFO("i2cgroup=%d\n",i2cgroup);
    i2c_dev->i2cgroup = i2cgroup;

    of_property_read_u32(node, "i2c-padmux", &i2cpadmux);
    HWI2C_DBG_INFO("i2cpadmux=%d\n",i2cpadmux);
    i2c_dev->i2cpadmux = i2cpadmux;

    of_property_read_u32(node, "i2c-speed", &i2c_speed);
    HWI2C_DBG_INFO("i2c_speed=%d\n",i2c_speed);
    i2c_dev->i2c_speed = i2c_speed;

    of_property_read_u32(node, "i2c-en-dma", &i2c_en_dma);
    HWI2C_DBG_INFO("i2c_en_dma=%d\n",i2c_en_dma);
    i2c_dev->i2c_en_dma = i2c_en_dma;

    if (pdev->dev.of_node) {
        const struct of_device_id *match;
        match = of_match_device(mstar_i2c_of_match, &pdev->dev);
        //i2c_dev->is_dvc = of_device_is_compatible(pdev->dev.of_node,
        //				"sstar,cedric-i2c-dvc");
    } else if (pdev->id == 3) {
        i2c_dev->is_dvc = 1;
    }

#ifdef CONFIG_MS_I2C_INT_ISR
#if 0 //tmp cancel request ISR
    //init isr
    data->i2cgroup = i2c_dev->i2cgroup;
    data->i2cirq = CamIrqOfParseAndMap(node, 0);
    HAL_HWI2C_IrqRequest(data->i2cirq, i2c_dev->i2cgroup, (void*)pdev);
    //init tcond
    HAL_HWI2C_DMA_TsemInit((u8)i2c_dev->i2cgroup);
    //HWI2C_DBG_ERR("1mstar_i2c_probe i2cirq %d\n", data->i2cirq);
#endif
#endif
    
    init_completion(&i2c_dev->msg_complete);

    platform_set_drvdata(pdev, i2c_dev);

    g_pdev[(u8)i2cgroup] = pdev;
    #if defined(CONFIG_MS_PADMUX)
    if (1 == mdrv_padmux_active() ||
            TRUE == MDrv_HWI2C_IsPadSet()){
            IsUseMdrvPadmux = 1;
        }
    #endif
    MDrv_HW_IIC_Init(i2c_dev->base,i2c_dev->chipbase,i2cgroup,clkbase, i2cpadmux,IsUseMdrvPadmux, i2c_speed, i2c_en_dma);

    //i2c_set_adapdata(&i2c_dev->adapter, i2c_dev);
    i2c_dev->adapter.owner = THIS_MODULE;
    i2c_dev->adapter.class = I2C_CLASS_DEPRECATED;
    //strlcpy(i2c_dev->adapter.name, "Sstar I2C adapter",
    //	sizeof(i2c_dev->adapter.name));
    scnprintf(i2c_dev->adapter.name, sizeof(i2c_dev->adapter.name),
		 "Sstar I2C adapter %d", i2cgroup);
    i2c_dev->adapter.algo = &sg_ms_i2c_algorithm;

    i2c_dev->adapter.dev.parent = &pdev->dev;
    i2c_dev->adapter.nr = i2cgroup;
    i2c_dev->adapter.dev.of_node = pdev->dev.of_node;

    pdev->dev.platform_data = (void*)data;

    HWI2C_DBG_INFO(" i2c_dev->adapter.nr=%d\n",i2c_dev->adapter.nr);
    i2c_set_adapdata(&i2c_dev->adapter, i2c_dev);

    ret = i2c_add_numbered_adapter(&i2c_dev->adapter);
    if (ret) {
		dev_err(&pdev->dev, "Failed to add I2C adapter\n");
		goto out;
    }
    #if 0
    //dev register
    sprintf(i2cname, "%s%d%s", I2CNAME, i2c_dev->i2cgroup, "\0");
    data->i2c_miscdev.minor = MISC_DYNAMIC_MINOR;
    data->i2c_miscdev.name = i2cname;
    data->i2c_miscdev.fops = &i2c_fops;
    data->i2c_miscdev.parent = &pdev->dev;
    if(misc_register(&data->i2c_miscdev)){
        CamOsPrintf("regist NG: %x\n", data->i2c_miscdev.minor);
    }
    else{
        CamOsPrintf("regist ok: %x\n", data->i2c_miscdev.minor);
    }
    #endif
    return 0;
//err return
out:
    if(data){
        kfree(data);
    }
    //clk_unprepare(i2c_dev->div_clk);
    return ret;
}

static int mstar_i2c_remove(struct platform_device *pdev)
{
    struct mstar_i2c_dev *i2c_dev = platform_get_drvdata(pdev);
    i2c_dev_data *data;

    data = (i2c_dev_data*)pdev->dev.platform_data;

    MDrv_HW_IIC_DeInit(i2c_dev->adapter.nr);
#ifdef CONFIG_MS_I2C_INT_ISR
    //free isr and uninit I2c DMA
    HAL_HWI2C_IrqFree(data->i2cirq);
    HAL_HWI2C_DMA_TsemDeinit(data->i2cgroup);
#endif

    i2c_remove_srcclk((u8)i2c_dev->adapter.nr);
    kfree(data);
    i2c_del_adapter(&i2c_dev->adapter);

    return 0;
}

#ifdef CONFIG_PM_SLEEP
static int mstar_i2c_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct mstar_i2c_dev *i2c_dev = platform_get_drvdata(pdev);

    i2c_remove_srcclk((u8)i2c_dev->adapter.nr);
    MDrv_HW_IIC_DeInit(i2c_dev->adapter.nr);
    i2c_lock_adapter(&i2c_dev->adapter);
    i2c_dev->is_suspended = TRUE;
    i2c_unlock_adapter(&i2c_dev->adapter);

    return 0;
}

static int mstar_i2c_resume(struct platform_device *pdev)
{
    struct mstar_i2c_dev *i2c_dev = platform_get_drvdata(pdev);
    int IsUseMdrvPadmux = 0;
    
    #if defined(CONFIG_MS_PADMUX)
    if (1 == mdrv_padmux_active() ||
        TRUE == MDrv_HWI2C_IsPadSet()){
        IsUseMdrvPadmux = 1;
    }
    #endif
    
    i2c_lock_adapter(&i2c_dev->adapter);
    MDrv_HW_IIC_Init(i2c_dev->base,i2c_dev->chipbase,i2c_dev->adapter.nr,i2c_dev->clkbase, i2c_dev->i2cpadmux,IsUseMdrvPadmux, i2c_dev->i2c_speed, i2c_dev->i2c_en_dma);
    i2c_dev->is_suspended = FALSE;
    i2c_unlock_adapter(&i2c_dev->adapter);

    return 0;
}
#endif
MODULE_DEVICE_TABLE(of, mstar_i2c_of_match);


static struct platform_driver mstar_i2c_driver = {
    .probe   = mstar_i2c_probe,
    .remove  = mstar_i2c_remove,
#ifdef CONFIG_PM_SLEEP
    .suspend = mstar_i2c_suspend,
    .resume = mstar_i2c_resume,
#endif
    .driver  = {
        .name  = "mstar-i2c",
        .owner = THIS_MODULE,
        .of_match_table = mstar_i2c_of_match,
    },
};

static int __init mstar_i2c_init_driver(void)
{
    return platform_driver_register(&mstar_i2c_driver);
}

static void __exit mstar_i2c_exit_driver(void)
{
    platform_driver_unregister(&mstar_i2c_driver);
}

EXPORT_SYMBOL(MDrv_HW_IIC_Init);
EXPORT_SYMBOL(MDrv_HWI2C_WriteBytes);
EXPORT_SYMBOL(MDrv_HWI2C_ReadBytes);

subsys_initcall(mstar_i2c_init_driver);
module_exit(mstar_i2c_exit_driver);

MODULE_DESCRIPTION("Sstar I2C Bus Controller driver");
MODULE_AUTHOR("SSTAR");
MODULE_LICENSE("GPL");
