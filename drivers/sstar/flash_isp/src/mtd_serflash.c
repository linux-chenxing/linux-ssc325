/*
* mtd_serflash.c- Sigmastar
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
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <ms_platform.h> //for reading strapping
#include <drvSPINOR.h>
#include <mdrvSNI.h>
#include <mtd_serflash.h>
#include <drvFSP_QSPI.h>

static u32 g_u32_sni_address = 0;
static SPINOR_INFO_t *g_pst_spinor_info;

#define FLASH_SNI_TABLE_SIZE                0x200
#define FLASH_MAX_SNI_SIZE                  512
#define FLASH_FLASH_SIZE                    g_pst_spinor_info->u32_Capacity
#define FLASH_SECTOR_SIZE                   g_pst_spinor_info->u16_SectorByteCnt
#define FLASH_BLOCK_SIZE                    g_pst_spinor_info->u32_BlkBytesCnt
#define FLASH_SECTOR_SIZE_MASK              (g_pst_spinor_info->u16_SectorByteCnt - 1)
#define FLASH_BLOCK_SIZE_MASK               (g_pst_spinor_info->u32_BlkBytesCnt - 1)
#define FLASH_PAGE_SIZE                     g_pst_spinor_info->u16_PageByteCnt
#define FLASH_PAGE_SIZE_MASK                (g_pst_spinor_info->u16_PageByteCnt - 1)
#define FLASH_SNI_HEADER_SIZE               sizeof(au8_basicMagicData)
const u8 au8_basicMagicData[] = {0x4D, 0x53, 0x54, 0x41, 0x52, 0x53, 0x45, 0x4D, 0x49, 0x55, 0x53, 0x46, 0x44, 0x43, 0x49, 0x53};

extern int parse_mtd_partitions(struct mtd_info *master, const char *const *types,
			 struct mtd_partitions *pparts,
			 struct mtd_part_parser_data *data);

struct serflash
{
    struct mutex        lock;
    struct mtd_info     mtd;
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
    u8 u8_status = 0;
    u32 u32_bytes_left = 0;
    u32 u32_erase_size;

    /* sanity checks */
    if (!instr->len)
    {
        return 0;
    }
    /* range and alignment check */
    if (instr->addr + instr->len > mtd->size || instr->len % FLASH_SECTOR_SIZE || instr->addr % FLASH_SECTOR_SIZE)
    {
        return -EINVAL;
    }

    addr_temp = instr->addr;
    len_temp = instr->len;
    if ((do_div(addr_temp , mtd->erasesize) != 0) ||(do_div(len_temp, mtd->erasesize) != 0))
    {
        return -EINVAL;
    }

    mutex_lock(&flash->lock);

    u32_bytes_left = instr->len;
    while (0 != u32_bytes_left)
    {
        if (0 != (~FLASH_BLOCK_SIZE_MASK & u32_bytes_left))
        {
            if (ERR_SPINOR_SUCCESS != DRV_SPINOR_block_erase(instr->addr))
            {
                printk(KERN_ERR "block erase failed!\n");
                instr->state = MTD_ERASE_FAILED;
                mutex_unlock(&flash->lock);
                return (-EIO);
            }
            u32_erase_size = FLASH_BLOCK_SIZE;
        }
        else
        {
            if (ERR_SPINOR_SUCCESS != DRV_SPINOR_sector_erase(instr->addr))
            {
                printk(KERN_ERR "sector erase failed!\n");
                instr->state = MTD_ERASE_FAILED;
                mutex_unlock(&flash->lock);
                return (-EIO);
            }
            u32_erase_size = FLASH_SECTOR_SIZE;
        }
        instr->addr += u32_erase_size;
        u32_bytes_left -= u32_erase_size;

        if (ERR_SPINOR_INVALID == u8_status)
        {
            instr->state = MTD_ERASE_FAILED;
            mutex_unlock(&flash->lock);
            return (-EIO);
        }
    }

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
    u8 u8_status = 0;
    u32 u32_read_size;
    /* sanity checks */
    if (!len)
    {
        return 0;
    }
    if (from + len > flash->mtd.size)
    {
        return -EINVAL;
    }

    mutex_lock(&flash->lock);
    *retlen = len;
    while(0 != len)
    {
        u32_read_size = BLOCK_ERASE_SIZE;
        if(len < u32_read_size)
            u32_read_size = len;
        if (ERR_SPINOR_SUCCESS != (u8_status = DRV_SPINOR_read(from, (u8*)buf, u32_read_size)))
        {
            *retlen = 0;
            mutex_unlock(&flash->lock);
            return (-EIO);
        }
        len -= u32_read_size;
        buf += u32_read_size;
        from += u32_read_size;
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
    u16 u16_write_size = 0;

    if (retlen)
        *retlen = 0;

    /* sanity checks */
    if (!len)
    {
        return(0);
    }
    if (to + len > flash->mtd.size)
    {
        return -EINVAL;
    }

    mutex_lock(&flash->lock);

    *retlen = len;    //if success,return the input length

    while (0 < len)
    {
        u16_write_size = FLASH_PAGE_SIZE - (FLASH_PAGE_SIZE_MASK & to);
        if (u16_write_size > len)
        {
            u16_write_size = len;
        }
        if (ERR_SPINOR_SUCCESS != DRV_SPINOR_program_page(to, (u8 *)buf, u16_write_size))
        {
            printk(KERN_ERR "[FLASH_ERR] Program page fail\r\n");
            *retlen = 0;
            mutex_unlock(&flash->lock);
            return (-EIO);
        }

        to += u16_write_size;
        buf += u16_write_size;
        len -= u16_write_size;
    }
    mutex_unlock(&flash->lock);
    return 0;
}

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

static u8 _serflash_is_sni_header(u8 *pu8_data)
{
    return !(memcmp(au8_basicMagicData, pu8_data, sizeof(au8_basicMagicData)));
}

static void _spi_flash_switch_config(FLASH_CMD_SET_t *pt_cmt, u8 u8_enabled)
{
    u8 u8_fn;
    u8 u8_status;
    u8 u8_status_bytes;
    u8 (*read_status[3])(u8*, u8) = {DRV_SPINOR_read_status1, DRV_SPINOR_read_status2, DRV_SPINOR_read_status3};
    u32 u32_status;

    u32_status = 0;
    u8_status_bytes = 0;
    for (u8_fn = 0; sizeof(read_status)/sizeof(void*) > u8_fn && pt_cmt->u16_dataBytes > u8_status_bytes; u8_fn++)
    {
        if (0x01 & (pt_cmt->u32_address >> u8_fn))
        {
            if (ERR_SPINOR_SUCCESS != read_status[u8_fn](&u8_status, 1))
            {
                printk("[FLASH_ERR] Read status registers fail!\r\n");
            }
            u32_status |= (u8_status << (u8_status_bytes * 8));
            u8_status_bytes++;
            printk("[FLASH] status%d = 0x%x\r\n",u8_fn + 1,u8_status);
        }
    }

    if (u8_enabled)
    {
        u32_status |= pt_cmt->u16_value;
    }
    else
    {
        u32_status &= ~pt_cmt->u16_value;
    }

    if (ERR_SPINOR_SUCCESS != DRV_SPINOR_write_status(pt_cmt->u8_command, (u8*)&u32_status, pt_cmt->u16_dataBytes))
    {
        printk("[FLASH_ERR] DRV_SPINOR_write_status fail!\r\n");
    }
}

static void _serflash_unlock_whole_flash(u32 u32_sni_address)
{
    SPINOR_INFO_t *pst_spinor_info;
    SPINOR_PROTECT_STATUS_t *pst_w_protectStatus;
    FLASH_CMD_SET_t *pt_w_cmt;

    pst_spinor_info = (SPINOR_INFO_t*)u32_sni_address;
    pst_w_protectStatus = &pst_spinor_info->st_wProtectStatus;
    pt_w_cmt = &pst_w_protectStatus->st_blockStatus.st_blocks;

    if (0 < pt_w_cmt->u16_dataBytes)
    {
        pst_w_protectStatus = &pst_spinor_info->st_wProtectStatus;
        pt_w_cmt = &pst_w_protectStatus->st_blockStatus.st_blocks;
        _spi_flash_switch_config(pt_w_cmt, 0);
    }

}

static void _serflash_show_protect_status(u32 u32_sni_address)
{
    u8 u8_status_name_index;

    u32 u32_status_size;
    u32 u32_status;

    SPINOR_INFO_t *pst_spinor_info;
    SPINOR_PROTECT_STATUS_t *pst_protectStatus;
    FLASH_CMD_SET_t *pt_cmt;

    const u8 *pau8_status_name[] = {(const u8*)"complement", (const u8*)"top/buttom", (const u8*)"blocks", (const u8*)"SRP0", (const u8*)"SRP1"};

    pst_spinor_info = (SPINOR_INFO_t*)u32_sni_address;
    pst_protectStatus = &pst_spinor_info->st_rProtectStatus;

    u32_status_size = sizeof(SPINOR_PROTECT_STATUS_t);
    pt_cmt = (FLASH_CMD_SET_t*) &pst_protectStatus->st_blockStatus.st_complement;
    u8_status_name_index = 0;

    while (0 != u32_status_size)
    {
        if (0 < pt_cmt->u16_dataBytes)
        {
            u32_status = 0;

            if (ERR_SPINOR_SUCCESS != DRV_SPINOR_read_status(pt_cmt->u8_command, (u8*)&u32_status, pt_cmt->u16_dataBytes))
            {
                printk("[FLASH_ERR] DRV_SPINOR_read_status fail!\r\n");
            }
            printk("[FLASH] %s = 0x%x!\r\n",pau8_status_name[u8_status_name_index],u32_status & pt_cmt->u16_value);
        }

        pt_cmt++;
        u32_status_size -= sizeof(FLASH_CMD_SET_t);
        u8_status_name_index++;
    }
}

static void _serflash_setup_by_sni(SPINOR_INFO_t *pst_spinor_info)
{
    DRV_SPINOR_setup_access(pst_spinor_info->st_readData.u8_command,
                            pst_spinor_info->st_readData.u8_dummy,
                            pst_spinor_info->st_program.u8_command);
    DRV_SPINOR_setup_time_wait(pst_spinor_info->u32_MaxWaitTime);

    if (0 == (pst_spinor_info->au8_reserved[0] & 0x01))
    {
        DRV_SPINOR_use_bdma(1);
        printk("[FLASH] BDMA mode\r\n");
    }
    else
    {
        DRV_SPINOR_use_bdma(0);
        printk("[FLASH] RIU mode\r\n");
    }

    return;
}

static void _serflash_load_sni(u32 u32_address)
{
    u32 u32_sni_address;

    u32_sni_address = 0;

    if (ERR_SPINOR_SUCCESS != DRV_SPINOR_read(FLASH_CIS_LOAD_OFFSET, (u8*)&u32_sni_address, 2))
    {
        printk("[FLASH_ERR] SNI offset of satrting fail!\r\n");
        return;
    }
    u32_sni_address = (u32_sni_address + 0xfff) & ~0xfff;

    for (; FLASH_SEARCH_END > u32_sni_address; u32_sni_address += 0x1000)
    {
        if (ERR_SPINOR_SUCCESS != DRV_SPINOR_read(u32_sni_address, (u8*)u32_address, FLASH_SNI_HEADER_SIZE))
        {
            printk("[FLASH_ERR] Loading SNI header fail!\r\n");
            return;
        }

        if (_serflash_is_sni_header((u8*)u32_address))
        {
            break;
        }
    }

    if (ERR_SPINOR_SUCCESS != DRV_SPINOR_read(u32_sni_address, (u8*)u32_address, FLASH_SNI_TABLE_SIZE))
    {
        printk("[FLASH_ERR] Loading SNI fail!\r\n");
        return;
    }

    printk("[FLASH] Load SNI from 0x%x @ 0x%x\r\n",u32_sni_address,u32_address);
}

static void _serflash_info(FLASH_INFO_t *pst_flash_info)
{
    pst_flash_info->u32_eraseSize = g_pst_spinor_info->u16_SectorByteCnt;
    pst_flash_info->u32_writeSize = g_pst_spinor_info->u16_PageByteCnt;
    pst_flash_info->u32_capacity = g_pst_spinor_info->u32_Capacity;
}

static u8 _serflash_init(u32 u32_address)
{
    SPINOR_QUAD_CFG_t *pst_quad_enabled;

    g_u32_sni_address = u32_address;
    g_pst_spinor_info = (SPINOR_INFO_t*)(u32_address + FLASH_SNI_HEADER_SIZE);
    pst_quad_enabled = &g_pst_spinor_info->st_QE;
    if (!_serflash_is_sni_header((u8*)u32_address))
    {
        if (ERR_SPINOR_SUCCESS != DRV_SPINOR_init())
        {
            printk("[FLASH_ERR] init fail!\r\n");
            return (-EIO);
        }

        _serflash_load_sni(u32_address);
        if (pst_quad_enabled->u8_needQE)
        {
            _spi_flash_switch_config(&pst_quad_enabled->st_wQuadEnabled, 1);
        }
        _serflash_unlock_whole_flash((u32)g_pst_spinor_info);
    }
    _serflash_setup_by_sni(g_pst_spinor_info);
    _serflash_show_protect_status((u32)g_pst_spinor_info);
    printk("[FLASH] End flash init.\r\n");
    return 0;
}

/*
 * board specific setup should have ensured the SPI clock used here
 * matches what the READ command supports, at least until this driver
 * understands FAST_READ (for clocks over 25 MHz).
 */
static int serflash_probe(struct platform_device *pdev)
{
    struct serflash			*flash;
    FLASH_INFO_t st_flash_info;

#ifndef CONFIG_CAM_CLK
    int num_parents;
    struct clk **spi_clks;
    int i;
#endif
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
    // jedec_probe() will read id, so initialize hardware first
    if (0 == g_u32_sni_address)
    {
        g_u32_sni_address = (u32)kzalloc(FLASH_SNI_TABLE_SIZE,GFP_KERNEL);

        if (!g_u32_sni_address) {
            printk("[serflash_probe]: Failed to allocate memory!\n");
            return -ENOMEM;
        }
        // jedec_probe() will read id, so initialize hardware first
        if(_serflash_init(g_u32_sni_address))
        {
            printk("[serflash_probe] flash init failed!\n");
        }
    }
    _serflash_info(&st_flash_info);

    DRV_SPINOR_alloc_bdma_buffer(BLOCK_ERASE_SIZE);//for bdma read

    if (NULL == (flash = kzalloc(sizeof *flash, GFP_KERNEL)))
    {
        goto out;
    }

    mutex_init(&flash->lock);

    flash->mtd.priv = flash;
    flash->mtd.name ="nor0";
    flash->mtd.type = MTD_NORFLASH;
    flash->mtd.writesize = 1;
    flash->mtd.writebufsize = flash->mtd.writesize;
    flash->mtd.flags = MTD_CAP_NORFLASH;
    flash->mtd.size = st_flash_info.u32_capacity;
    flash->mtd._erase = serflash_erase;
    flash->mtd._read = serflash_read;
    flash->mtd._write = serflash_write;
    flash->mtd.erasesize = st_flash_info.u32_eraseSize;
    printk(KERN_DEBUG
           "mtd .name = %s, .size = 0x%.8x (%uMiB)\n"
           " .erasesize = 0x%.8x .numeraseregions = %d\n",
           flash->mtd.name,
           (unsigned int)flash->mtd.size, (unsigned int)flash->mtd.size / (1024*1024),
           (unsigned int)flash->mtd.erasesize,
           flash->mtd.numeraseregions);
    platform_set_drvdata(pdev, &flash->mtd);
    return mtd_device_register(&flash->mtd,NULL,0);
out:
    if (flash)
    {
        kfree((void*)flash);
    }

    if (g_u32_sni_address)
    {
        kfree((void*)g_u32_sni_address);
    }

    return -ENOMEM;
}


static int serflash_remove(struct platform_device *pdev)
{
    struct mtd_info *mtd;
    struct serflash	*flash;
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
    mtd = platform_get_drvdata(pdev);
    flash = mtd->priv;
    if(flash)
    {
        DRV_SPINOR_free_bdma_buffer(BLOCK_ERASE_SIZE);
        if(flash->mtd.numeraseregions)
            kfree(flash->mtd.eraseregions);
        kfree(flash);
    }
    platform_set_drvdata(pdev, NULL);

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
    DRV_FSP_QSPI_init();
    return 0;
}
#endif

static struct platform_driver ms_flash_driver = {
	.probe		= serflash_probe,
	.remove		= serflash_remove,
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

