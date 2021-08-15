/*
* mtd_serflash.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: richard.guo <richard.guo@sigmastar.com.tw>
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
//*********************************************************************
//
//   MODULE NAME:
//       mtd_serflash.c
//
//   DESCRIPTION:
//      This file is an agent between mtd layer and spi flash driver
//
//  PUBLIC PROCEDURES:
//       Name                    Title
//       ----------------------- --------------------------------------
//       int xxx_proc           declare in its corresponding header file
//
//   LOCAL PROCEDURES:
//       Name                    Title
//       ----------------------- --------------------------------------
//       get_prnt_cnvs           the local procedure in the file
//
//  Written by Tao.Zhou@MSTAR Inc.
//---------------------------------------------------------------------
//
//********************************************************************
//--------------------------------------------------------------------
//                             GENERAL INCLUDE
//--------------------------------------------------------------------

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/version.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
#include <linux/slab.h>
#endif
#include "drvSERFLASH.h"
#include "drvDeviceInfo.h"
#ifdef CONFIG_MS_FLASH_ISP_MXP_PARTS
#include "part_mxp.h"
#endif
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <ms_platform.h> //for reading strapping

#define CONFIG_DISABLE_WRITE_PROTECT
#define CONFIG_MTD_PARTITIONS

/* Flash opcodes. */
#define	OPCODE_BE_4K		0x20	/* Erase 4KiB block */
#define	OPCODE_BE_32K		0x52	/* Erase 32KiB block */
//#define	OPCODE_CHIP_ERASE	0xc7	/* Erase whole flash chip */
#define	OPCODE_SE		0xd8	/* Sector erase (usually 64KiB) */
#define	OPCODE_RDID		0x9f	/* Read JEDEC ID */

/* Define max times to check status register before we give up. */
#define	MAX_READY_WAIT_COUNT	100000
#define	CMD_SIZE		4

//#ifdef CONFIG_MTD_PARTITIONS
#define	mtd_has_partitions()	(1)
//#else
//#define	mtd_has_partitions()	(0)
//#endif

//#define DEBUG(x...) printk(x...)

extern int parse_mtd_partitions(struct mtd_info *master, const char *const *types,
                                struct mtd_partition **pparts,
                                struct mtd_part_parser_data *data);

/****************************************************************************/

struct serflash
{
    struct mutex        lock;
    struct mtd_info     mtd;
    unsigned            partitioned:1;
    u8                  erase_opcode;
};

static inline struct serflash *mtd_to_serflash(struct mtd_info *mtd)
{
    return container_of(mtd, struct serflash, mtd);
}

/* Erase flash fully or part of it */
static int serflash_erase(struct mtd_info *mtd, struct erase_info *instr)
{

    struct serflash *flash = mtd_to_serflash(mtd);
    uint64_t addr_temp, len_temp;

    //printk(KERN_WARNING"%s: addr 0x%08x, len %ld\n", __func__, (u32)instr->addr, (long int)instr->len);

    /* sanity checks */
    if (!instr->len)
        return 0;

    /* range and alignment check */
    if (instr->addr + instr->len > mtd->size)
        return -EINVAL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
    /*  mod = do_div(x,y);
        result = x; */
    addr_temp = instr->addr;
    len_temp = instr->len;
    if ((do_div(addr_temp , mtd->erasesize) != 0) ||(do_div(len_temp, mtd->erasesize) != 0))
    {
        return -EINVAL;
    }
#else
    if ((instr->addr % mtd->erasesize) != 0 || (instr->len % mtd->erasesize) != 0)
        return -EINVAL;
#endif

    mutex_lock(&flash->lock);

#ifndef CONFIG_DISABLE_WRITE_PROTECT
    /*write protect false before erase*/
    if (!MDrv_SERFLASH_WriteProtect(0))
    {
        mutex_unlock(&flash->lock);
        return -EIO;
    }
#endif
    /* erase the whole chip */
    if (instr->len == mtd->size && !MDrv_SERFLASH_EraseChip())
    {
        instr->state = MTD_ERASE_FAILED;
#ifndef CONFIG_DISABLE_WRITE_PROTECT
        MDrv_SERFLASH_WriteProtect(1);
#endif
        mutex_unlock(&flash->lock);
        return -EIO;
    }
    else if (!MDrv_SERFLASH_AddressErase(instr->addr, instr->len, 1))
    {
        instr->state = MTD_ERASE_FAILED;
#ifndef CONFIG_DISABLE_WRITE_PROTECT
        MDrv_SERFLASH_WriteProtect(1);
#endif
        mutex_unlock(&flash->lock);
        return -EIO;
    }

#ifndef CONFIG_DISABLE_WRITE_PROTECT
    MDrv_SERFLASH_WriteProtect(1);
#endif
    mutex_unlock(&flash->lock);

    instr->state = MTD_ERASE_DONE;

    mtd_erase_callback(instr);

    return 0;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int serflash_read(struct mtd_info *mtd, loff_t from, size_t len,
                         size_t *retlen, u_char *buf)
{
    struct serflash *flash = mtd_to_serflash(mtd);

    //printk(KERN_WARNING "%s %s 0x%08x, len %zd\n",	__func__, "from", (u32)from, len);

    /* sanity checks */
    if (!len)
        return 0;

    if (from + len > flash->mtd.size)
        return -EINVAL;

    mutex_lock(&flash->lock);

#if 0
    /* Wait till previous write/erase is done. */
    if (wait_till_ready(flash))
    {
        /* REVISIT status return?? */
        mutex_unlock(&flash->lock);
        return 1;
    }
#endif

    if (MDrv_SERFLASH_Read(from, len, (unsigned char *)buf))
    {
        *retlen = len;
    }
    else
    {
        *retlen = 0;
        mutex_unlock(&flash->lock);
        return -EIO;
    }

    mutex_unlock(&flash->lock);

    return 0;
}

/*
 * Write an address range to the flash chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static int serflash_write(struct mtd_info *mtd, loff_t to, size_t len,
                          size_t *retlen, const u_char *buf)
{
    struct serflash *flash = mtd_to_serflash(mtd);

    //printk(KERN_WARNING "%s %s 0x%08x, len %zd\n",__func__, "to", (u32)to, len);

    if (retlen)
        *retlen = 0;

    /* sanity checks */
    if (!len)
        return(0);

    if (to + len > flash->mtd.size)
        return -EINVAL;

    mutex_lock(&flash->lock);

#if 0
    /* Wait until finished previous write command. */
    if (wait_till_ready(flash))
    {
        mutex_unlock(&flash->lock);
        return 1;
    }
#endif
#ifndef CONFIG_DISABLE_WRITE_PROTECT

    if (!MDrv_SERFLASH_WriteProtect(0))
    {
        mutex_unlock(&flash->lock);
        return -EIO;
    }
#endif
//modified by daniel.lee 2010/0514
    /*
           if (!MDrv_SERFLASH_BlockErase(erase_start, erase_end, TRUE))
           {
               mutex_unlock(&flash->lock);
    	    return -EIO;
           }
    */
    if (MDrv_SERFLASH_Write(to, len, (unsigned char *)buf))
    {
        if (retlen)
            *retlen = len;
    }
    else
    {
        if (retlen)
            *retlen = 0;
#ifndef CONFIG_DISABLE_WRITE_PROTECT
        MDrv_SERFLASH_WriteProtect(1);
#endif
        mutex_unlock(&flash->lock);
        return -EIO;
    }
#ifndef CONFIG_DISABLE_WRITE_PROTECT
    MDrv_SERFLASH_WriteProtect(1);
#endif
    mutex_unlock(&flash->lock);

    return 0;
}



#define MSTAR_SERFLASH_SIZE					(8 * 1024 * 1024)


#define SERFLASH_PART_PARTITION_0_OFFSET    0
#define SERFLASH_PART_PARTITION_0_SIZE		(32+512+32) * 1024

#define SERFLASH_PART_PARTITION_1_OFFSET	(SERFLASH_PART_PARTITION_0_OFFSET + SERFLASH_PART_PARTITION_0_SIZE)
#define SERFLASH_PART_PARTITION_1_SIZE		512 * 1024

#define SERFLASH_PART_PARTITION_2_OFFSET    (SERFLASH_PART_PARTITION_1_OFFSET + SERFLASH_PART_PARTITION_1_SIZE)
#define SERFLASH_PART_PARTITION_2_SIZE      512 * 1024

#define SERFLASH_PART_PARTITION_3_OFFSET    (SERFLASH_PART_PARTITION_2_OFFSET + SERFLASH_PART_PARTITION_2_SIZE)
#define SERFLASH_PART_PARTITION_3_SIZE      MSTAR_SERFLASH_SIZE - SERFLASH_PART_PARTITION_3_OFFSET

#if 0
#define SERFLASH_PART_PARTITION_TBL_OFFSET		0
#define SERFLASH_PART_PARTITION_TBL_SIZE		512 * 1024

#define SERFLASH_PART_LINUX_BOOT_PARAM_OFFSET	(SERFLASH_PART_PARTITION_TBL_OFFSET + SERFLASH_PART_PARTITION_TBL_SIZE)
#define SERFLASH_PART_LINUX_BOOT_PARAM_SIZE		512 * 1024

#define SERFLASH_PART_KERNEL_OFFSET				(SERFLASH_PART_LINUX_BOOT_PARAM_OFFSET + SERFLASH_PART_LINUX_BOOT_PARAM_SIZE)
#define SERFLASH_PART_KERNEL_SIZE				1536 * 1024

#define SERFLASH_PART_ROOTFS_OFFSET				(SERFLASH_PART_KERNEL_OFFSET + SERFLASH_PART_KERNEL_SIZE)
#define SERFLASH_PART_ROOTFS_SIZE				2560 * 1024

#define SERFLASH_PART_CONF_OFFSET				(SERFLASH_PART_ROOTFS_OFFSET + SERFLASH_PART_ROOTFS_SIZE)
#define SERFLASH_PART_CONF_SIZE					64 * 1024


#define SERFLASH_PART_CHAKRA_BOOT_PARAM_OFFSET	(NAND_PART_KERNEL_OFFSET + NAND_PART_KERNEL_SIZE)
#define SERFLASH_PART_CHAKRA_BOOT_PARAM_SIZE	SZ_512KB

#define SERFLASH_PART_CHAKRA_BIN_OFFSET			(NAND_PART_CHAKRA_BOOT_PARAM_OFFSET + NAND_PART_CHAKRA_BOOT_PARAM_SIZE)
#define SERFLASH_PART_CHAKRA_BIN_PARAM_SIZE		SZ_8MB

#define SERFLASH_PART_SUBSYSTEM_OFFSET			(NAND_PART_CONF_OFFSET + NAND_PART_CONF_SIZE)
#define SERFLASH_PART_SUBSYSTEM_SIZE			SZ_2MB

#define SERFLASH_PART_FONT_OFFSET				(NAND_PART_SUBSYSTEM_OFFSET + NAND_PART_SUBSYSTEM_SIZE)
#define SERFLASH_PART_FONT_SIZE					SZ_4MB

#define SERFLASH_PART_OPT_OFFSET				(NAND_PART_FONT_OFFSET + NAND_PART_FONT_SIZE)
#define SERFLASH_PART_OPT_SIZE					SZ_8MB

#define SERFLASH_PART_APPLICATION_OFFSET		(NAND_PART_OPT_OFFSET + NAND_PART_OPT_SIZE)
#define SERFLASH_PART_APPLICATION_SIZE			(MSTAR_NAND_SIZE - NAND_PART_APPLICATION_OFFSET)
#endif
//
//#if ( (NAND_PART_APPLICATION_OFFSET) >= MSTAR_SERFLASH_SIZE)
//    #error "Error: NAND partition is not correct!!!"
//#endif

static const struct mtd_partition serflash_partition_info[] =
{

    {
        .name       = "DATA",//"boot",
        .offset     = 0x00300000,//SERFLASH_PART_PARTITION_0_OFFSET,
        .size       = 0x00080000,//SERFLASH_PART_PARTITION_0_SIZE
        .mask_flags = 0
    },
    {
        .name       = "SYSTEM",//"boot",
        .offset     = 0x00380000,//SERFLASH_PART_PARTITION_0_OFFSET,
        .size       = 0x00400000,//SERFLASH_PART_PARTITION_0_SIZE
        .mask_flags = 0
    },
    {
        .name       = "EXT",//"boot",
        .offset     = 0x00780000,//SERFLASH_PART_PARTITION_0_OFFSET,
        .size       = 0x00880000,//SERFLASH_PART_PARTITION_0_SIZE
        .mask_flags = 0
    },

};

#define SERFLASH_NUM_PARTITIONS ARRAY_SIZE(serflash_partition_info)


extern int mxp_init_nor_flash(void);
extern hal_SERFLASH_t _hal_SERFLASH;
extern MS_BOOL bDetect;
extern MS_BOOL gQuadSupport;

#ifdef CONFIG_CAM_CLK
    #include "drv_camclk_Api.h"
    void **pvSerflashclk = NULL;
    u32 SerflashParentCnt = 1;


u8 serflash_ClkDisable(void)
{
    u32 u32clknum = 0;

    for(u32clknum = 0; u32clknum < SerflashParentCnt; u32clknum++)
    {
        if (pvSerflashclk[u32clknum])
        {
            CamClkSetOnOff(pvSerflashclk[u32clknum],0);
        }
    }
    return 1;
}
u8 serflash_ClkEnable(void)
{
    u32 u32clknum = 0;

    for(u32clknum = 0; u32clknum < SerflashParentCnt; u32clknum++)
    {
        if (pvSerflashclk[u32clknum])
        {
            CamClkSetOnOff(pvSerflashclk[u32clknum],1);
        }
    }

    return 1;
}
u8 serflash_ClkRegister( struct device *dev)
{
    u32 u32clknum;
    u32 SerFlashClk;
    u8 str[16];

    if(of_find_property(dev->of_node,"camclk",&SerflashParentCnt))
    {
        SerflashParentCnt /= sizeof(int);
        //printk( "[%s] Number : %d\n", __func__, num_parents);
        if(SerflashParentCnt < 0)
        {
            printk( "[%s] Fail to get parent count! Error Number : %d\n", __func__, SerflashParentCnt);
            return 0;
        }
        pvSerflashclk = kzalloc((sizeof(void *) * SerflashParentCnt), GFP_KERNEL);
        if(!pvSerflashclk){
            return 0;
        }
        for(u32clknum = 0; u32clknum < SerflashParentCnt; u32clknum++)
        {
            SerFlashClk = 0;
            of_property_read_u32_index(dev->of_node,"camclk", u32clknum,&(SerFlashClk));
            if (!SerFlashClk)
            {
                printk( "[%s] Fail to get clk!\n", __func__);
            }
            else
            {
                CamOsSnprintf(str, 16, "serflash_%d ",u32clknum);
                CamClkRegister(str,SerFlashClk,&(pvSerflashclk[u32clknum]));
            }
        }
    }
    else
    {
        printk( "[%s] W/O Camclk \n", __func__);
    }
    return 1;
}
u8 serflash_ClkUnregister(void)
{

    u32 u32clknum;

    for(u32clknum=0;u32clknum<SerflashParentCnt;u32clknum++)
    {
        if(pvSerflashclk[u32clknum])
        {
            printk(KERN_DEBUG "[%s] %p\n", __func__,pvSerflashclk[u32clknum]);
            CamClkUnregister(pvSerflashclk[u32clknum]);
            pvSerflashclk[u32clknum] = NULL;
        }
    }
    kfree(pvSerflashclk);

    return 1;
}
#endif

/*
 * board specific setup should have ensured the SPI clock used here
 * matches what the READ command supports, at least until this driver
 * understands FAST_READ (for clocks over 25 MHz).
 */
static int serflash_probe(struct platform_device *pdev)
{
    struct serflash			*flash;
#ifndef CONFIG_CAM_CLK
    int num_parents;
    struct clk **spi_clks;
#endif
    int i;
    MS_U32 u32Val;
    u32 u32Ret;

    if(Chip_Get_Storage_Type()!= MS_STORAGE_NOR)
        return 0;
#ifdef CONFIG_CAM_CLK
	serflash_ClkRegister(&pdev->dev);
	serflash_ClkEnable();
#else
    num_parents = of_clk_get_parent_count(pdev->dev.of_node);
    if(num_parents > 0)
    {
        spi_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);
        if(spi_clks==NULL)
        {
            printk( "[serflash_probe] kzalloc Fail!\n" );
            return -1;
        }
        //enable all clk
        for(i = 0; i < num_parents; i++)
        {
            spi_clks[i] = of_clk_get(pdev->dev.of_node, i);
            if (IS_ERR(spi_clks[i]))
            {
                printk( "[serflash_probe] Fail to get clk!\n" );
                kfree(spi_clks);
                return -1;
            }
            else
            {
                clk_prepare_enable(spi_clks[i]);
            }
            clk_put(spi_clks[i]);
        }
        kfree(spi_clks);
    }
#endif
    if(!of_property_read_u32(pdev->dev.of_node, "quadread", (u32 *)&u32Val))
    {
        gQuadSupport = u32Val;
    }else{
        gQuadSupport = 0;
        //printk( "[serflash_probe] not search quadread on DTS\n" );
    }

    //if(gQuadSupport)
        //printk( "[serflash_probe] DTS support Quad Read\n" );

    // jedec_probe() will read id, so initialize hardware first
    MDrv_SERFLASH_Init();

    /* Platform data helps sort out which chip type we have, as
     * well as how this board partitions it.  If we don't have
     * a chip ID, try the JEDEC id commands; they'll work for most
     * newer chips, even if we don't recognize the particular chip.
     */

    //bDetect is the global variable from halserflash.c
    if(bDetect == FALSE)
    {
         printk(KERN_WARNING"[FSP] found no flash_info!!");
         return -ENODEV;
    }


    flash = kzalloc(sizeof *flash, GFP_KERNEL);
    if (!flash)
        return -ENOMEM;

    mutex_init(&flash->lock);

    flash->mtd.priv = flash;
    flash->mtd.name ="NOR_FLASH";
    flash->mtd.type = MTD_NORFLASH;
    flash->mtd.writesize = 1;
    flash->mtd.writebufsize = flash->mtd.writesize;
    flash->mtd.flags = MTD_CAP_NORFLASH;
    MDrv_SERFLASH_DetectSize(&u32Val);
    flash->mtd.size = u32Val;
    flash->mtd._erase = serflash_erase;
    flash->mtd._read = serflash_read;
    flash->mtd._write = serflash_write;
    flash->erase_opcode = OPCODE_SE;
    flash->mtd.erasesize = (_hal_SERFLASH.u32SecSize);


#ifdef CONFIG_DISABLE_WRITE_PROTECT
    MDrv_SERFLASH_WriteProtect(0);
#endif

    //dev_info(&spi->dev, "%s (%d Kbytes)\n", info->name,
    //flash->mtd.size / 1024);

    printk(KERN_WARNING
           "mtd .name = %s, .size = 0x%.8x (%uMiB)\n"
           " .erasesize = 0x%.8x .numeraseregions = %d\n",
           flash->mtd.name,
           (unsigned int)flash->mtd.size, (unsigned int)flash->mtd.size / (1024*1024),
           (unsigned int)flash->mtd.erasesize,
           flash->mtd.numeraseregions);

    if (flash->mtd.numeraseregions)
        for (i = 0; i < flash->mtd.numeraseregions; i++)
            printk(KERN_WARNING
                   "mtd.eraseregions[%d] = { .offset = 0x%.8x, "
                   ".erasesize = 0x%.8x (%uKiB), "
                   ".numblocks = %d }\n",
                   i, (u32)flash->mtd.eraseregions[i].offset,
                   (unsigned int)flash->mtd.eraseregions[i].erasesize,
                   flash->mtd.eraseregions[i].erasesize / 1024,
                   (unsigned int)flash->mtd.eraseregions[i].numblocks);


    /* partitions should match sector boundaries; and it may be good to
     * use readonly partitions for writeprotected sectors (BP2..BP0).
     */
    if (mtd_has_partitions())
    {
        struct mtd_partition    *parts = NULL;
        int    nr_parts = 0;
#ifdef CONFIG_MTD_CMDLINE_PARTS
        static const char *part_probes[] = { "cmdlinepart", NULL, };
#endif

#ifdef CONFIG_MS_FLASH_ISP_MXP_PARTS
        printk(KERN_WARNING"MXP_PARTS!!\n");
        if(mxp_init_nor_flash()>=0)
        {
            int j=0;
            nr_parts=mxp_get_total_record_count();
            parts=kmalloc(sizeof(struct mtd_partition)*nr_parts,GFP_KERNEL);
            if(NULL==parts)BUG();

            for(i=0; i<nr_parts; i++)
            {
                char *nbuf;
                mxp_record rec;
                mxp_get_record_by_index(i,&rec);

                if(rec.type & MXP_PART_TYPE_MTD)
                {
                    nbuf=kmalloc(sizeof(rec.name),GFP_KERNEL);
                    if(NULL==nbuf)BUG();

                    memcpy(nbuf,rec.name,sizeof(rec.name));
                    parts[j].name=nbuf;
                    parts[j].offset=rec.start;
                    parts[j].size=rec.size;
                    parts[j].mask_flags=0;
                    j++;
                }
            }

            flash->partitioned = 1;
            if(j>0)
            {

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
                u32Ret = mtd_device_register(&flash->mtd, parts, j);
#else
                u32Ret = add_mtd_partitions(&flash->mtd, parts, j);
#endif
                if(!u32Ret)//success
                {
                    kfree(parts);
                    return u32Ret;
                }
            }
            kfree(parts);
            parts = NULL;
        }
        printk(KERN_WARNING"MXP NOT FOUND!!\n");
#endif //end CONFIG_MS_FLASH_ISP_MXP_PARTS


#ifdef CONFIG_MTD_CMDLINE_PARTS
        nr_parts = parse_mtd_partitions(&flash->mtd,part_probes, &parts, 0);
#endif

        if (nr_parts > 0)
        {
            for (i = 0; i < nr_parts; i++)
            {
                printk(KERN_WARNING "partitions[%d] = "
                       "{.name = %s, .offset = 0x%.8x, "
                       ".size = 0x%.8x (%uKiB) }\n",
                       i, parts[i].name,
                       (unsigned int)(parts[i].offset),
                       (unsigned int)(parts[i].size),
                       (unsigned int)(parts[i].size / 1024));
            }
            flash->partitioned = 1;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
            u32Ret = mtd_device_register(&flash->mtd, parts, nr_parts);
#else
            u32Ret = add_mtd_partitions(&flash->mtd, parts, nr_parts);
#endif
        }
        else
        {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
            u32Ret = mtd_device_register(&flash->mtd, serflash_partition_info, SERFLASH_NUM_PARTITIONS);
#else
            u32Ret = add_mtd_partitions(&flash->mtd, serflash_partition_info, SERFLASH_NUM_PARTITIONS);
#endif
        }
        if(!u32Ret) //success
        {
            return u32Ret;
        }

    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
    return mtd_device_register(&flash->mtd, NULL, NULL);
#else
    return add_mtd_device(&flash->mtd) == 1 ? -ENODEV : 0;
#endif
}


static int serflash_cleanup(struct platform_device *pdev)
{
#if 0
    struct serflash	*flash = mtd->priv;
    int		status;

    /* Clean up MTD stuff. */
    if (mtd_has_partitions() && flash->partitioned)
        status = del_mtd_partitions(&flash->mtd);
    else
        status = del_mtd_device(&flash->mtd);
    if (status == 0)
        kfree(flash);
#endif
#ifdef CONFIG_CAM_CLK
	serflash_ClkDisable();
	serflash_ClkUnregister();
#else
#if defined(CONFIG_OF)
    int num_parents, i;
    struct clk **spi_clks;

    num_parents = of_clk_get_parent_count(pdev->dev.of_node);
    if(num_parents > 0)
    {
        spi_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);
        if(spi_clks==NULL)
        {
            printk( "[serflash_probe] kzalloc Fail!\n" );
            return -1;
        }
        //disable all clk
        for(i = 0; i < num_parents; i++)
        {
            spi_clks[i] = of_clk_get(pdev->dev.of_node, i);
            if (IS_ERR(spi_clks[i]))
            {
                printk( "[serflash_cleanup] Fail to get clk!\n" );
                kfree(spi_clks);
                return -1;
            }
            else
            {
                clk_disable_unprepare(spi_clks[i]);
            }
            clk_put(spi_clks[i]);
        }
        kfree(spi_clks);
    }
#endif
#endif
    return 0;
}

#if defined (CONFIG_OF)
static struct of_device_id flashisp_of_device_ids[] = {
         {.compatible = "mtd-flashisp"},
         {},
};
#endif

#ifdef CONFIG_PM
static int serflash_suspend(struct platform_device *pdev, pm_message_t state)
{
#ifdef CONFIG_CAM_CLK
	serflash_ClkDisable();
#else
#if defined(CONFIG_OF)
    int num_parents, i;
    struct clk **spi_clks;

    num_parents = of_clk_get_parent_count(pdev->dev.of_node);
    if(num_parents)
    {
        spi_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);
        if(spi_clks==NULL)
        {
            printk( "[serflash_probe] kzalloc Fail!\n" );
            return -1;
        }

        //disable all clk
        for(i = 0; i < num_parents; i++)
        {
            spi_clks[i] = of_clk_get(pdev->dev.of_node, i);
            if (IS_ERR(spi_clks[i]))
            {
                printk( "[serflash_suspend] Fail to get clk!\n" );
                kfree(spi_clks);
                return -1;
            }
            else
            {
                clk_disable_unprepare(spi_clks[i]);
            }
            clk_put(spi_clks[i]);
        }
        kfree(spi_clks);
    }
#endif
#endif
    return 0;
}

static int serflash_resume(struct platform_device *pdev)
{
#ifdef CONFIG_CAM_CLK
	serflash_ClkEnable();
#else
#if defined(CONFIG_OF)
    int num_parents, i;
    struct clk **spi_clks;

    num_parents = of_clk_get_parent_count(pdev->dev.of_node);
    if(num_parents)
    {
        spi_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);
        if(spi_clks==NULL)
        {
            printk( "[serflash_probe] kzalloc Fail!\n" );
            return -1;
        }

        //enable all clk
        for(i = 0; i < num_parents; i++)
        {
            spi_clks[i] = of_clk_get(pdev->dev.of_node, i);
            if (IS_ERR(spi_clks[i]))
            {
                printk( "[serflash_cleanup] Fail to get clk!\n" );
                kfree(spi_clks);
                return -1;
            }
            else
            {
                clk_prepare_enable(spi_clks[i]);
            }
            clk_put(spi_clks[i]);
        }
        kfree(spi_clks);
    }
#endif
#endif
    return 0;
}
#endif

static struct platform_driver ms_flash_driver = {
	.probe		= serflash_probe,
	.remove		= serflash_cleanup,
#ifdef CONFIG_PM
    .suspend    = serflash_suspend,
    .resume     = serflash_resume,
#endif
	.driver		= {
		.name	= "mtd-flashisp",
#if defined(CONFIG_OF)
    .of_match_table = flashisp_of_device_ids,
#endif
	.owner	= THIS_MODULE,
	},
};


module_platform_driver(ms_flash_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tao.Zhou");
MODULE_DESCRIPTION("MTD Mstar driver for spi flash chips");

