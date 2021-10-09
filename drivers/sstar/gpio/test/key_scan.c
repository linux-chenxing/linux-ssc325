/*
* key_scan.c - Sigmastar
*
* Copyright (C) 2020 Sigmastar Technology Corp.
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/


///////////////////////////////////////////////////////////////////////////////
/// @file      key_scan.c
/// @brief     Key Scan Test Code for Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/moduleparam.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include "cam_os_wrapper.h"
#include <linux/gpio.h>
#include <linux/interrupt.h>

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("SStar Key Scan");
MODULE_LICENSE("GPL");

#define OUT_HIGH		(1)
#define OUT_LOW			(0)

#define KEY_READ_ECO0	39 //PAD_KEY0
#define KEY_READ_ECO1	40 //PAD_KEY1
#define KEY_READ_ECO2	41 //PAD_KEY2
#define KEY_READ_ECO3	42 //PAD_KEY3
#define KEY_READ_ECO4	43 //PAD_KEY4
#define KEY_READ_ECO5	44 //PAD_KEY5
#define KEY_READ_ECO6	45 //PAD_KEY6

#define KEY_SCAN_ECO0	46 //PAD_KEY7
#define KEY_SCAN_ECO1	47 //PAD_KEY8
#define KEY_SCAN_ECO2	48 //PAD_KEY9
#define KEY_SCAN_ECO3	49 //PAD_KEY10
#define KEY_SCAN_ECO4	50 //PAD_KEY11
#define KEY_SCAN_ECO5	51 //PAD_KEY12
#define KEY_SCAN_ECO6	52 //PAD_KEY13


static int key_read_pad[] = 
{
	KEY_READ_ECO0, KEY_READ_ECO1, KEY_READ_ECO2,
	KEY_READ_ECO3, KEY_READ_ECO4, KEY_READ_ECO5, KEY_READ_ECO6
};
static int key_scan_pad[] = 
{	
	KEY_SCAN_ECO0, KEY_SCAN_ECO1, KEY_SCAN_ECO2, 
	KEY_SCAN_ECO3, KEY_SCAN_ECO4, KEY_SCAN_ECO5, KEY_SCAN_ECO6
};

typedef enum _key_value{
	key_null = 0,
	key_s1,  key_s2,  key_s3,  key_s4,  key_s5,  key_s6,  key_s7,
	key_s8,  key_s9,  key_s10, key_s11, key_s12, key_s13, key_s14,
	key_s15, key_s16, key_s17, key_s18, key_s19, key_s20, key_s21,
	key_s22, key_s23, key_s24, key_s25, key_s26, key_s27, key_s28,
	key_s29, key_s30, key_s31, key_s32, key_s33, key_s34, key_s35,
	key_s36, key_s37, key_s38, key_s39, key_s40, key_s41, key_s42,
	key_s43, key_s44, key_s45, key_s46, key_s47, key_s48, key_s49
}Key_Value_T;

typedef struct _key_pad_struct
{
	int key_count;
	int key_array[49];
}Key_Pad_T;


static int key_scan(Key_Pad_T * pkey_buf)
{
	int i = -1;
	int j = -1;
	int k = -1;
	int total_key = 0;
	int key_pad[7][7] = {0};
	
	total_key=0;
	for(i=0; i<(sizeof(key_read_pad)/sizeof(int)); i++)
	{
		if(gpio_direction_input(key_read_pad[i]) < 0)
		{
			printk("set key_read_pad to input mode failed, Pad index: %d ...\n", key_read_pad[i]);
			return -1;
		}
	}


	for(i=0; i<(sizeof(key_scan_pad)/sizeof(int)); i++)
	{
		if(gpio_direction_output(key_scan_pad[i], OUT_HIGH) < 0)
		{
			printk("set key_read_pad to output mode failed, Pad index: %d ...\n", key_scan_pad[i]);
			return -1;
		}
	}	
	
	for(i=0; i<(sizeof(key_scan_pad)/sizeof(int)); i++)
	{
		for(k=0; k<(sizeof(key_scan_pad)/sizeof(int)); k++)
		{
			if(gpio_direction_output(key_scan_pad[k], OUT_HIGH) < 0)
			{
				printk("set key_read_pad to output HIGH failed, Pad index: %d ...\n", key_scan_pad[i]);
				return -1;
			}
		}
		
		if(gpio_direction_output(key_scan_pad[i], OUT_LOW) < 0)
		{
			printk("set key_read_pad to output LOW failed, Pad index: %d ...\n", key_scan_pad[i]);
			return -1;
		}
		
		mdelay(10);
		printk("set scan pad %d to 0 \n", i+1);
		for(j=0; j<(sizeof(key_read_pad)/sizeof(int)); j++)
		{
			if(gpio_get_value(key_read_pad[j]) == 0)
			{
				printk("read pad %d is 0\n", j+1);
				total_key ++;
				key_pad[i][j] = 1;
			}	
		}
	}

	mdelay(10);
///////////////////////////////////////////////////////////////////////////////////////////////
#if 1
	for(i=0; i<(sizeof(key_scan_pad)/sizeof(int)); i++)
	{
		if(gpio_direction_input(key_scan_pad[i]) < 0)
		{
			printk("set key_read_pad to input mode failed, Pad index: %d ...\n", key_read_pad[i]);
			return -1;
		}
	}


	for(i=0; i<(sizeof(key_read_pad)/sizeof(int)); i++)
	{
		if(gpio_direction_output(key_read_pad[i], OUT_HIGH) < 0)
		{
			printk("set key_read_pad to output mode failed, Pad index: %d ...\n", key_scan_pad[i]);
			return -1;
		}
	}		
	
	for(i=0; i<(sizeof(key_read_pad)/sizeof(int)); i++)
	{
		for(k=0; k<(sizeof(key_read_pad)/sizeof(int)); k++)
		{
			if(gpio_direction_output(key_read_pad[k], OUT_HIGH) < 0)
			{
				printk("set key_read_pad to output HIGH failed, Pad index: %d ...\n", key_scan_pad[i]);
				return -1;
			}
		}
		
		if(gpio_direction_output(key_read_pad[i], OUT_LOW) < 0)
		{
			printk("set key_read_pad to output LOW failed, Pad index: %d ...\n", key_scan_pad[i]);
			return -1;
		}
		
		mdelay(10);
		printk("set scan pad %d to 0 \n", i+1);
		for(j=0; j<(sizeof(key_scan_pad)/sizeof(int)); j++)
		{
			if(gpio_get_value(key_scan_pad[j]) == 0)
			{
				printk("read pad %d is 0\n", j+1);
				total_key ++;
				key_pad[j][i] = 1;
			}	
		}
	}	
#endif
///////////////////////////////////////////////////////////////////////////////////////////////
	
	for(i=0,k=0; i<(sizeof(key_scan_pad)/sizeof(int)); i++)
	{
		for(j=0; j<(sizeof(key_read_pad)/sizeof(int)); j++)
		{
			if(key_pad[i][j] == 1)
			{
				pkey_buf->key_array[k] = i*7 + (j+1);
				k += 1;
			}
		}
	}
	
	if(k != total_key)
	{
		printk("This time the key scan failed, k:%d, total_key:%d!\n", k, total_key);
		//kfree(pkey_buf->key_array);
		//pkey_buf->key_array = NULL;
		//pkey_buf->key_count = 0;
		//return -1;
	}
	
	pkey_buf->key_count = k;
	printk("This time the key scan successfully, total num of pressing key is : %d!\n", total_key);
	return 0;
}

static ssize_t key_scan_read(struct file *file, char __user *to, size_t count, loff_t *f_pos)
{
	unsigned long ret; 
	char *pddr = NULL;//这边使用虚拟地址
	Key_Pad_T key_buf;
	
	memset(&key_buf, 0, 200);
	
	if(key_scan(&key_buf) < 0)
	{
		return 0;
	}
	
	pddr = (char *)&key_buf;
	count = 200;
	ret = copy_to_user(to, pddr, count);
	
	
	return count;
}

static ssize_t key_scan_write(struct file *file, const char __user *from, size_t count, loff_t *f_pos)
{
	return 0;
}

static int key_scan_mmap(struct file *file, struct vm_area_struct *vma)
{
	return 0;
}

static int key_scan_open(struct inode *inode, struct file *file)
{
	printk("open the KeyScan device!\n");

	return 0;
}

static int KeyScantInit(void)
{
	int i = -1;
	
	for(i=0; i<(sizeof(key_read_pad)/sizeof(int)); i++)
	{
		if(gpio_request(key_read_pad[i], "key scan") < 0)
		{
			printk("request key_read_pad failed, Pad index: %d ...\n", key_read_pad[i]);
			return -1;
		}
		
		if(gpio_direction_input(key_read_pad[i]) < 0)
		{
			printk("set key_read_pad to input mode failed, Pad index: %d ...\n", key_read_pad[i]);
			return -1;
		}
	}


	for(i=0; i<(sizeof(key_scan_pad)/sizeof(int)); i++)
	{
		if(gpio_request(key_scan_pad[i], "key scan") < 0)
		{
			printk("request key_scan_pad failed, Pad index: %d ...\n", key_scan_pad[i]);
			return -1;
		}

		if(gpio_direction_output(key_scan_pad[i], OUT_HIGH) < 0)
		{
			printk("set key_read_pad to output mode failed, Pad index: %d ...\n", key_scan_pad[i]);
			return -1;
		}
	}

    return 0;
}

static void KeyScantExit(void)
{
	int i = -1;
	
	for(i=0; i<(sizeof(key_read_pad)/sizeof(int)); i++)
	{
		gpio_free(key_read_pad[i]);
	}


	for(i=0; i<(sizeof(key_scan_pad)/sizeof(int)); i++)
	{
		gpio_free(key_scan_pad[i]);
	}

}

static const struct file_operations key_scan_fops = {
	.owner = THIS_MODULE,
	.open = key_scan_open,
	.read = key_scan_read,
	.write = key_scan_write,
	.mmap = key_scan_mmap,
};

static struct miscdevice key_scan_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "key_scan",
	.fops = &key_scan_fops,
};

static int __init key_scan_init(void)
{
	int ret = 0;

	if(KeyScantInit() < 0 )
	{
		pr_err("failed to do KeyScantInit!\n");
		goto err;	
	}
	
	ret = misc_register(&key_scan_misc);
	if (unlikely(ret)) {
		pr_err("failed to register misc device!\n");
		goto err;
	}

	return 0;

err:
	return ret;
}

static void __exit key_scan_exit(void)
{
	KeyScantExit();
	misc_deregister(&key_scan_misc);
}

module_init(key_scan_init);
module_exit(key_scan_exit);
