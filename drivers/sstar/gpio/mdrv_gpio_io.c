/*
* mdrv_gpio_io.c- Sigmastar
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
//#include "MsCommon.h"
//#include <linux/autoconf.h>
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
#include <linux/device.h>
#include <asm/io.h>
#include <linux/semaphore.h>


#include <linux/err.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pm.h>

//#include "mst_devid.h"

#include "mdrv_gpio_io.h"
#include "mhal_gpio.h"
#include "mdrv_gpio.h"
#include "ms_platform.h"
#include "gpio.h"


//#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define GPIO_DBG_ENABLE              0

#if GPIO_DBG_ENABLE
#define GPIO_DBG(_f)                 (_f)
#else
#define GPIO_DBG(_f)
#endif

#if 0
#define LINE_DBG()                  printf("GPIO %d\n", __LINE__)
#else
#define LINE_DBG()
#endif

#define GPIO_PRINT(fmt, args...)        //printk("\n[GPIO][%05d] " fmt, __LINE__, ## args)

typedef struct
{
    S32                          s32MajorGPIO;
    S32                          s32MinorGPIO;
    struct cdev                 cDevice;
    struct file_operations      GPIOFop;
    struct fasync_struct        *async_queue; /* asynchronous readers */
} GPIO_ModHandle_t;

#ifdef CONFIG_PM_SLEEP
typedef enum
{
    GPIO_INVAILD = 0,
    GPIO_OUT,
    GPIO_IN,
}GPIO_Dir_e;

typedef struct
{
    U8 isreq;
    U8 dir;
    U8 val;
    U8 driving;
}GPIO_State_t;
#endif


#define MOD_GPIO_DEVICE_COUNT         1
#define MOD_GPIO_NAME                 "ModGPIO"

#define MDRV_NAME_GPIO                  "gpio"
#define MDRV_MAJOR_GPIO                 0x9b
#define MDRV_MINOR_GPIO                 0x00

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

static struct device *dev;

#ifdef CONFIG_PM_SLEEP
static GPIO_State_t gpio_state_table[GPIO_NR];
#endif

//static struct class *gpio_class;

int camdriver_gpio_request(struct gpio_chip *chip, unsigned offset)
{
    #ifdef CONFIG_PM_SLEEP
    gpio_state_table[offset].isreq = TRUE;
    #endif
    MDrv_GPIO_Pad_Set(offset);
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_request offset=%d\n",offset);
    return 0;
}

void camdriver_gpio_free(struct gpio_chip *chip, unsigned offset)
{
    #ifdef CONFIG_PM_SLEEP
    gpio_state_table[offset].isreq = FALSE;
    #endif
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_free\n");
}

void camdriver_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
    #ifdef CONFIG_PM_SLEEP
    gpio_state_table[offset].val = value;
    #endif
    if(value==0)
        MDrv_GPIO_Pull_Low(offset);
    else
        MDrv_GPIO_Pull_High(offset);
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_set\n");
}

int camdriver_gpio_get(struct gpio_chip *chip, unsigned offset)
{
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_get\n");
    return MDrv_GPIO_Pad_Read(offset);
}

int camdriver_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
    #ifdef CONFIG_PM_SLEEP
    gpio_state_table[offset].dir = GPIO_IN;
    #endif
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_direction_input\n");
    MDrv_GPIO_Pad_Odn(offset);
    return 0;
}

int camdriver_gpio_direction_output(struct gpio_chip *chip, unsigned offset,
                    int value)
{
    #ifdef CONFIG_PM_SLEEP
    gpio_state_table[offset].val = value;
    gpio_state_table[offset].dir = GPIO_OUT;
    #endif
    MDrv_GPIO_Pad_Oen(offset);
    if(value==0)
        MDrv_GPIO_Pull_Low(offset);
    else
        MDrv_GPIO_Pull_High(offset);
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_direction_output\n");
    return 0;
}

int camdriver_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
    int virq;

    virq = MDrv_GPIO_To_Irq(offset);
    if (virq < 0)
        return -ENXIO;

    GPIO_PRINT("%s virq:%d \n", __FUNCTION__, virq);
    return virq;
}
__attribute__((weak)) void MDrv_GPIO_Set_Driving(U8 u8IndexGPIO, U8 setHigh) {
   // "not support
}
void camdriver_gpio_set_driving(struct gpio_chip *chip, unsigned offset, int value)
{
    #ifdef CONFIG_PM_SLEEP
    gpio_state_table[offset].driving = value;
    #endif
    MDrv_GPIO_Set_Driving(offset, value);
}

static struct gpio_chip camdriver_gpio_chip = {
    .label          = "gpio",
    .request        = camdriver_gpio_request,
    .free           = camdriver_gpio_free,
    .direction_input    = camdriver_gpio_direction_input,
    .get            = camdriver_gpio_get,
    .direction_output   = camdriver_gpio_direction_output,
    .set            = camdriver_gpio_set,
    .to_irq         = camdriver_gpio_to_irq,
    .base           = 0,
};


static const struct of_device_id camdriver_gpio_of_match[] = {
    { .compatible = "sstar,gpio" },
    { },
};

static int camdriver_gpio_probe(struct platform_device *pdev)
{
    const struct of_device_id *match;
    int ret;

    dev = &pdev->dev;
    GPIO_PRINT("\n++[camdriver-gpio]camdriver_gpio_probe start\n");
    match = of_match_device(camdriver_gpio_of_match, &pdev->dev);

    if (!match) {
        printk("Err:[gpio] No dev found\n");
        return -ENODEV;
    }
    camdriver_gpio_chip.ngpio = GPIO_NR;
    camdriver_gpio_chip.of_node = pdev->dev.of_node;
    ret = gpiochip_add(&camdriver_gpio_chip);

    if (ret < 0) {
        printk("[gpio] add err\n");
        return ret;
    }

    GPIO_PRINT("--[camdriver-gpio]camdriver_gpio_probe end\n");
    MDrv_GPIO_Init();
    printk(KERN_WARNING"GPIO: probe end");
    return 0;
}

#ifdef CONFIG_PM_SLEEP
static int camdriver_gpio_suspend(struct device *dev)
{
    return 0;
}

static int camdriver_gpio_resume(struct device *dev)
{
    int i;

    for (i = 0; i < GPIO_NR; i++)
    {
        if(gpio_state_table[i].isreq == TRUE)
        {
            MDrv_GPIO_Pad_Set(i);
            if(gpio_state_table[i].dir == GPIO_IN)
            {
                MDrv_GPIO_Pad_Odn(i);
            }
            else if(gpio_state_table[i].dir == GPIO_OUT)
            {
                MDrv_GPIO_Pad_Oen(i);
                if(gpio_state_table[i].val == 0)
                {
                    MDrv_GPIO_Pull_Low(i);
                }
                else
                {
                    MDrv_GPIO_Pull_High(i);
                }
            }
            MDrv_GPIO_Set_Driving(i, gpio_state_table[i].driving);
        }
    }
    return 0;
}
#else
#define camdriver_gpio_suspend    NULL
#define camdriver_gpio_resume     NULL
#endif

static const struct dev_pm_ops camdriver_gpio_pm_ops = {
    .suspend = camdriver_gpio_suspend,
    .resume = camdriver_gpio_resume,
};

static struct platform_driver camdriver_gpio_driver = {
    .driver     = {
        .name   = "gpio",
        .owner  = THIS_MODULE,
        .of_match_table = camdriver_gpio_of_match,
        .pm = &camdriver_gpio_pm_ops,
    },
    .probe      = camdriver_gpio_probe,
};



void __mod_gpio_init(void)
{
    //GPIO chiptop initialization
    MDrv_GPIO_Init();
}


static int __init camdriver_gpio_init(void)
{
    return platform_driver_register(&camdriver_gpio_driver);
}
postcore_initcall(camdriver_gpio_init);

EXPORT_SYMBOL(camdriver_gpio_to_irq);
EXPORT_SYMBOL(camdriver_gpio_direction_output);
EXPORT_SYMBOL(camdriver_gpio_request);
EXPORT_SYMBOL(camdriver_gpio_free);
EXPORT_SYMBOL(camdriver_gpio_set);
EXPORT_SYMBOL(camdriver_gpio_get);
EXPORT_SYMBOL(camdriver_gpio_direction_input);
EXPORT_SYMBOL(camdriver_gpio_set_driving);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("GPIO driver");
MODULE_LICENSE("GPL");
