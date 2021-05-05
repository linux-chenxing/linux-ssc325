/*
 * Input driver for resistor ladder connected on ADC
 *
 * Copyright (c) 2016 Alexandre Belloni
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/err.h>
//#include <linux/iio/consumer.h>
//#include <linux/iio/types.h>
#include <linux/input.h>
#include <linux/input-polldev.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>

#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/of_platform.h>
#include <linux/delay.h>
#include "reg_rtcpwc.h"
#include "ms_types.h"
#include <linux/reboot.h>

struct adc_keys_button {
	u32 voltage;
	u32 keycode;
};

struct ms_rtc_info {
    struct platform_device *pdev;
    struct rtc_device *rtc_dev;
    void __iomem *rtc_base;
    u32 default_base;
    spinlock_t mutex;
#ifdef CONFIG_PM_SLEEP
    u32 sw0;
#ifdef CONFIG_RTCPWC_INNER_EHHE
    u32 sw1;
#endif
#endif
};
static struct ms_rtc_info *_pinfo = NULL;

struct adc_keys_state {
	//struct iio_channel *channel;
	int chan;
	u32 num_keys;
	u32 last_key;
	u32 keyup_voltage;
    void __iomem *reg_base;
	const struct adc_keys_button *map;
};

#define ISO_S0						0x00
#define ISO_S1						0x01
#define ISO_S2						0x03
#define ISO_S3						0x07
#define ISO_S4						0x05
#define ISO_S5						0x01
#define ISO_ACK_RETRY_TIME			20
#define RTC_CHECK_STATUS_DELAY_TIME_US  100

extern int ms_sar_get(int ch);
extern void ms_sar_hw_init(void);

static bool ms_RtcPwr_ISOCTL_EX(void)
{
    u8 ubCheck = ISO_ACK_RETRY_TIME;
    u16 reg = 0 ;
    // Input ISO ctrl sequence ,  3'b000(S0) -> 3'b001(S1)  -> 3'b011(S2)  -> 3'b111(S3)  -> 3'b101(S4)  -> 3'b001(S5)  -> 3'b000(S0)
    // Following notes is from MV2
    // The switch of state needs delay, 1ms at least according to designer,
    // but in our test, set to 3ms will still causes incorrect data read.
    // And the sequence should be finished within 1 sec.

    reg = readw(_pinfo->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    writew(reg & ISO_S0, _pinfo->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    reg = readw(_pinfo->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
    reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;

    while((reg ) && (--ubCheck)) {
        // mdelay(RTC_CHECK_STATUS_DELAY_TIME_MS);
        udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
        reg = readw(_pinfo->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
        reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;
    }
    if(ubCheck == 0)
        return FALSE;

    ubCheck = ISO_ACK_RETRY_TIME;
    reg = readw(_pinfo->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    writew(reg | ISO_S1, _pinfo->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    reg = readw(_pinfo->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
    reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;

    while((reg != RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT)&& (--ubCheck)) {
        // mdelay(RTC_CHECK_STATUS_DELAY_TIME_MS);
        udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
        reg = readw(_pinfo->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
        reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;
    }
    if(ubCheck == 0)
        return FALSE;


    ubCheck = ISO_ACK_RETRY_TIME;
    reg = readw(_pinfo->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    writew(reg | ISO_S2, _pinfo->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    reg = readw(_pinfo->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
    reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;

    while((reg )&& (--ubCheck)) {
        // mdelay(RTC_CHECK_STATUS_DELAY_TIME_MS);
        udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
        reg = readw(_pinfo->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
        reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;
    }
    if(ubCheck == 0)
        return FALSE;

    ubCheck = ISO_ACK_RETRY_TIME;
    reg = readw(_pinfo->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    writew(reg | ISO_S3, _pinfo->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    reg = readw(_pinfo->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
    reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;

    while((reg != RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT) && (--ubCheck)) {
        // mdelay(RTC_CHECK_STATUS_DELAY_TIME_MS);
        udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
        reg = readw(_pinfo->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
        reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;
    }
    if(ubCheck == 0)
        return FALSE;

    ubCheck = ISO_ACK_RETRY_TIME;
    reg = readw(_pinfo->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    writew(reg & ISO_S4, _pinfo->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    reg = readw(_pinfo->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
    reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;

    while((reg )&& (--ubCheck)) {
        // mdelay(RTC_CHECK_STATUS_DELAY_TIME_MS);
        udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
        reg = readw(_pinfo->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
        reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;
    }
    if(ubCheck == 0)
        return FALSE;

    ubCheck = ISO_ACK_RETRY_TIME;
    reg = readw(_pinfo->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    writew(reg & ISO_S5, _pinfo->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    reg = readw(_pinfo->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
    reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;

    while((reg != RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT )&& (--ubCheck)) {
        // mdelay(RTC_CHECK_STATUS_DELAY_TIME_MS);
        udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
        reg = readw(_pinfo->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
        reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;
    }
    if(ubCheck == 0)
        return FALSE;

    ubCheck = ISO_ACK_RETRY_TIME;
    reg = readw(_pinfo->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);
    writew(reg & ISO_S0, _pinfo->rtc_base + RTCPWC_DIG2RTC_ISO_CTRL);

    reg = readw(_pinfo->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
    reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;

    while((reg )&& (--ubCheck)) {
        // mdelay(RTC_CHECK_STATUS_DELAY_TIME_MS);
        udelay(RTC_CHECK_STATUS_DELAY_TIME_US);
        reg = readw(_pinfo->rtc_base + RTCPWC_RTC2DIG_ISO_CTRL_ACK);
        reg = reg & RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT;
    }
    if(ubCheck == 0)
        return FALSE;

    ubCheck = 22;
    do
    {
        reg = readw(_pinfo->rtc_base + RTCPWC_DIG2PWC_RTC_TESTBUS);
        if (reg & RTCPWC_ISO_EN)
        {
            break;
        }
        udelay(100);
        ubCheck--;
    }
    while (ubCheck);

    if(ubCheck == 0)
        return FALSE;

    // [from designer Belon.Chen] wait 2 ms is must since read/write base/counter/SW0/SW1 is valid after iso state complete
    mdelay(2);
    return TRUE;
}

#if defined(SYS_POWEROFF_DIRECT_BY_DRIVER)
static void ms_poweroff_Notify(void)
{
    int ret = -1;
    char path[] = "/usr/bin/killall";
    char *argv[] = {path,"cardv",NULL};
    char *envp[] = {"HOME=/", "PATH=/sbin:/bin:/user/bin", NULL};

    printk(KERN_CRIT "-------%s-------\n",__func__);
    ret = call_usermodehelper(path, argv, envp,UMH_WAIT_PROC);
    ssleep(3);
    kernel_power_off();
}
#endif

static void ms_poweroff_enter(void)
{
    if(!IS_ERR(_pinfo->rtc_base)){
        writew(0x00, (_pinfo->rtc_base + (0x0f<<2)));
        printk(KERN_CRIT "********%s*******\n",__func__);
        ms_RtcPwr_ISOCTL_EX();
    }
}

static void adc_keys_poll(struct input_polled_dev *dev)
{
	struct adc_keys_state *st = dev->private;
	int i, value;
	u32 diff, closest = 0xffffffff;
	int keycode = 0;

	//ret = iio_read_channel_processed(st->channel, &value);
	value = ms_sar_get(st->chan);
    value = (value * 325) / 100;
	if (unlikely(value < 0)) {
		/* Forcibly release key if any was pressed */
		value = st->keyup_voltage;
	} else {
		for (i = 0; i < st->num_keys; i++) {
			diff = abs(st->map[i].voltage - value);
			if (diff < closest) {
				closest = diff;
				keycode = st->map[i].keycode;
			}
		}
	}
    
	if (abs(st->keyup_voltage - value) < closest)
		keycode = 0;

    if (keycode == 0 && !IS_ERR(st->reg_base)){
        u16 reg = 0;
        reg = readw(st->reg_base + 0x48);
        keycode = (reg & 0x04)? KEY_POWER:0;
        if(keycode == KEY_POWER){
            //printk(KERN_CRIT "keycode = %d  value=%d  \n", keycode,reg);
            #if defined(SYS_POWEROFF_DIRECT_BY_DRIVER)
            ms_poweroff_Notify();
            #endif
        }
    }

	if (st->last_key && st->last_key != keycode)
		input_report_key(dev->input, st->last_key, 0);
    
	if (keycode){
        //printk("keycode = %d  value=%d  \n", keycode,value);
		input_report_key(dev->input, keycode, 1);
    }
    
	input_sync(dev->input);
	st->last_key = keycode;
}

static int adc_keys_load_keymap(struct device *dev, struct adc_keys_state *st)
{
	struct adc_keys_button *map;
	struct fwnode_handle *child;
	int i;

	st->num_keys = device_get_child_node_count(dev);
	if (st->num_keys == 0) {
		dev_err(dev, "keymap is missing\n");
		return -EINVAL;
	}

	map = devm_kmalloc_array(dev, st->num_keys, sizeof(*map), GFP_KERNEL);
	if (!map)
		return -ENOMEM;

	i = 0;
	device_for_each_child_node(dev, child) {
		if (fwnode_property_read_u32(child, "press-threshold-microvolt",
					     &map[i].voltage)) {
			dev_err(dev, "Key with invalid or missing voltage\n");
			fwnode_handle_put(child);
			return -EINVAL;
		}
		map[i].voltage /= 1000;

		if (fwnode_property_read_u32(child, "linux,code",
					     &map[i].keycode)) {
			dev_err(dev, "Key with invalid or missing linux,code\n");
			fwnode_handle_put(child);
			return -EINVAL;
		}

		i++;
	}

	st->map = map;
	return 0;
}

static int adc_keys_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct adc_keys_state *st;
    struct input_polled_dev *poll_dev;
    struct input_dev *input;
    int i, value,chan;
    int error;
    char compatible_name[64];
    struct device_node *dev_node;
    struct platform_device *pRtcdev;
    
    st = devm_kzalloc(dev, sizeof(*st), GFP_KERNEL);
    if (!st){
        return -ENOMEM;
    }

    sprintf(compatible_name, "sstar,infinity-rtcpwc");
    dev_node = of_find_compatible_node(NULL, NULL, compatible_name);

    if (!dev_node) {
        dev_err(dev, "[%s]: failed to find of_find_compatible_node\n", __func__);
        //return 0;
    }else{
        pRtcdev = of_find_device_by_node(dev_node);
        _pinfo = platform_get_drvdata(pRtcdev);
        if (!_pinfo) {
            dev_err(dev, "[%s]: failed to get platform_get_drvdata\n", __func__);
            st->reg_base = NULL;
            //return 0;
        }else{
            st->reg_base = _pinfo->rtc_base;
            dev_err(dev, "adc_keys_probe reg_base=0x%p success\n",_pinfo->rtc_base);
        }
    }

    if (!device_property_read_u32(dev, "io-channels", &chan))
        st->chan = chan;
    else{
    //set default
        st->chan = 0;
    }

    if (device_property_read_u32(dev, "keyup-threshold-microvolt",
            &st->keyup_voltage)) {
        dev_err(dev, "Invalid or missing keyup voltage\n");
        return -EINVAL;
    }
    st->keyup_voltage /= 1000;

    error = adc_keys_load_keymap(dev, st);
    if (error)
        return error;

    platform_set_drvdata(pdev, st);

    poll_dev = devm_input_allocate_polled_device(dev);
    if (!poll_dev) {
        dev_err(dev, "failed to allocate input device\n");
        return -ENOMEM;
    }

    if (!device_property_read_u32(dev, "poll-interval", &value))
        poll_dev->poll_interval = value;

    poll_dev->poll = adc_keys_poll;
    poll_dev->private = st;

    input = poll_dev->input;

    input->name = pdev->name;
    input->phys = "adc-keys/input0";

    input->id.bustype = BUS_HOST;
    input->id.vendor = 0x0001;
    input->id.product = 0x0001;
    input->id.version = 0x0100;

    __set_bit(EV_KEY, input->evbit);
    for (i = 0; i < st->num_keys; i++)
        __set_bit(st->map[i].keycode, input->keybit);

    if (device_property_read_bool(dev, "autorepeat"))
        __set_bit(EV_REP, input->evbit);
    
    error = input_register_polled_device(poll_dev);
    if (error) {
        dev_err(dev, "Unable to register input device: %d\n", error);
        return error;
    }
    ms_sar_hw_init();

    pm_power_off = ms_poweroff_enter;
    return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id adc_keys_of_match[] = {
    { .compatible = "adc-keys", },
    { }
};
MODULE_DEVICE_TABLE(of, adc_keys_of_match);
#endif

static struct platform_driver __refdata adc_keys_driver = {
	.driver = {
		.name = "adc_keys",
		.of_match_table = of_match_ptr(adc_keys_of_match),
	},
	.probe = adc_keys_probe,
};
module_platform_driver(adc_keys_driver);

MODULE_AUTHOR("Alexandre Belloni <alexandre.belloni@free-electrons.com>");
MODULE_DESCRIPTION("Input driver for resistor ladder connected on ADC");
MODULE_LICENSE("GPL v2");
