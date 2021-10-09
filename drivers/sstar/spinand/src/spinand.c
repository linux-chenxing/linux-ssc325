/*
* spinand.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: raul.wang <raul.wang@sigmastar.com.tw>
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
#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/types.h>
#include <ms_platform.h>
#include "mtdcore.h"
#include <mdrvSNI.h>
#include <drvSPINAND.h>
#include <drvFSP_QSPI.h>

static const u8 au8_magicData[] = {0x4D, 0x53, 0x54, 0x41, 0x52, 0x53, 0x45, 0x4D, 0x49, 0x55, 0x53, 0x46, 0x44, 0x43, 0x49, 0x53};
static const u8 au8_extMagicData[] = {'S', 'S', 'T', 'A', 'R', 'S', 'E', 'M', 'I', 'S', 'N', 'I', 'V', '0', '0', '0'};
static u8 u8_csFlg = 0;
static u32 g_u32_sni_address = 0;
static SPINAND_INFO_t *g_pst_spinand_info = NULL;
static SPINAND_EXT_INFO_t *g_pst_spinand_ext_info = NULL;

#define DRIVER_NAME "ss-spinand"

#define FLASH_LEAGCY_SNI_SIZE           (sizeof(au8_magicData) + sizeof(SPINAND_INFO_t))

#define FLASH_SNI_HEADER_SIZE           sizeof(au8_magicData)
#define FLASH_EXT_SNI_HEADER_SIZE       sizeof(au8_extMagicData)
#define FLASH_MAX_SNI_SIZE              512
#define FLASH_PAGE_SIZE_MASK            (g_pst_spinand_info->u16_PageByteCnt - 1)
#define FLASH_BLOCK_MASK                (g_pst_spinand_info->u16_BlkPageCnt - 1)
#define FLASH_PAGE_SIZE                 g_pst_spinand_info->u16_PageByteCnt
#define FLASH_BLOCK_SIZE                (FLASH_PAGE_SIZE * g_pst_spinand_info->u16_BlkPageCnt)
#define FLASH_BLOCK_SIZE_MASK           (FLASH_BLOCK_SIZE - 1)
#define FLASH_BLOCK_COUNT               g_pst_spinand_info->u16_BlkCnt
#define FLASH_PAGES_PER_BLOCK           g_pst_spinand_info->u16_BlkPageCnt
#define FLASH_SNI_BLOCKS_SIZE           10
#define FLASH_SNI_TABLE_SIZE            0x200

#define FLASH_2K_PAGE_SIZE              (2 << 10)
#define FLASH_2K_BLOCK_SIZE             (FLASH_2K_PAGE_SIZE * 64)
#define FLASH_4K_PAGE_SIZE              (4 << 10)
#define FLASH_4K_BLOCK_SIZE             (FLASH_4K_PAGE_SIZE * 64)

#define FLASH_GOOD_BLOCK_MAKER           0xFF

#if 1
#define spi_nand_msg(fmt, ...) printk(KERN_NOTICE "%s: " fmt "\n", __func__, ##__VA_ARGS__)
#define spi_nand_debug(fmt, ...)
#else
#define spi_nand_msg(fmt, ...)
#define spi_nand_debug(fmt, ...) printk(KERN_NOTICE "%s: " fmt "\n", __func__, ##__VA_ARGS__)
#endif

u8 _flash_count_bits(u32 u32_x)
{
    u8 u8_i = 0;

    while (u32_x)
    {
        u8_i++;
        u32_x >>= 1;
    }

    return (u8_i - 1);
}

static u8 _flash_is_sni_header(const u8 *u8_header, u8 u8_size, u8 *pu8_data)
{
    return !(memcmp(u8_header, pu8_data, u8_size));
}

static int _flash_set_quad_mode(u8 u8_enabled)
{
    SPINAND_ACCESS_CONFIG_t *pst_access;
    u32 u32_qe_status;

    pst_access = &g_pst_spinand_ext_info->st_profile.st_access;
    u32_qe_status = 0;

    if(ERR_SPINAND_SUCCESS == DRV_SPINAND_check_status())
    {
        if (ERR_SPINAND_SUCCESS != DRV_SPINAND_get_features(pst_access->st_qeStatus.u32_address, (u8*)&u32_qe_status, pst_access->st_qeStatus.u16_dataBytes))
        {
            spi_nand_msg("[FLASH_ERR]get features failed\r\n");
            return 0;
        }

        if (u8_enabled)
        {
            u32_qe_status |= pst_access->st_qeStatus.u16_value;
        }
        else
        {
            u32_qe_status &= ~pst_access->st_qeStatus.u16_value;
        }

        if (ERR_SPINAND_SUCCESS != DRV_SPINAND_set_features(pst_access->st_qeStatus.u32_address, (u8*)&u32_qe_status, pst_access->st_qeStatus.u16_dataBytes))
        {
            spi_nand_msg("[FLASH_ERR] set features failed\r\n");
            return 0;
        }
    }
    else
    {
        return 0;
    }

    if(ERR_SPINAND_SUCCESS == DRV_SPINAND_check_status())
        return 1;

    return 0;
}

static void _flash_setup_by_sni(SPINAND_EXT_INFO_t *pst_ext_sni)
{
    SPINAND_EXT_PROFILE_t *pst_profile;
    SPINAND_ACCESS_CONFIG_t *pst_access;
    if(pst_ext_sni)
    {
        if (_flash_is_sni_header(au8_extMagicData, FLASH_EXT_SNI_HEADER_SIZE, pst_ext_sni->au8_magic))
        {
            pst_profile = &pst_ext_sni->st_profile;
            pst_access = &pst_profile->st_access;
            DRV_SPINAND_setup_access(pst_access->st_read.u8_command,
                                     pst_access->st_read.u8_dummy,
                                     pst_access->st_program.u8_command,
                                     pst_access->st_random.u8_command);
            if (pst_profile->u16_flags & SPINAND_NEED_QE)
            {
                if(!_flash_set_quad_mode(1))
                    spi_nand_msg("[FLASH_ERR] Enable quad mode failed\r\n");
            }
            DRV_SPINAND_setup_timeout(pst_profile->u32_maxWaitTime);
        }
    }
}

static u32 _flash_offset_to_page_address(u32 u32_bytes_offset, u32 u32_page_size)
{
    u32 u32_page_mask;
    u32_page_mask = u32_page_size - 1;
    return ((u32_bytes_offset & ~u32_page_mask) >> _flash_count_bits(u32_page_size));
}

static void _flash_wait_dev_ready(void)
{
    if (ERR_SPINAND_TIMEOUT == DRV_SPINAND_check_status())
    {
        printk("[FLASH_ERR] check status fail!\r\n");
    }
}

static u8 _flash_check_page_status(void)
{
    SPINAND_EXT_PROFILE_t *pst_profile;

    u8 u8_status;

    if (ERR_SPINAND_TIMEOUT != (u8_status = DRV_SPINAND_check_status()))
    {
        if (_flash_is_sni_header(au8_extMagicData, FLASH_EXT_SNI_HEADER_SIZE, g_pst_spinand_ext_info->au8_magic))
        {
            pst_profile = &g_pst_spinand_ext_info->st_profile;

            if (ERR_SPINAND_ECC_RESERVED == u8_status)
            {
                if (pst_profile->u16_flags & SPINAND_ECC_RESERVED_NONE_CORRECTED)
                {
                    return ERR_SPINAND_ECC_NOT_CORRECTED;
                }
                else if (pst_profile->u16_flags & SPINAND_ECC_RESERVED_CORRECTED)
                {
                    return ERR_SPINAND_ECC_CORRECTED;
                }
            }
        }
    }

    return u8_status;
}

static u16 _flash_offset_wrap(u32 u32_page_address)
{
    if (1 < g_pst_spinand_info->u8PlaneCnt)
    {
        if (0 < (u32_page_address & FLASH_PAGES_PER_BLOCK))
        {
            return (1 << 12);
        }
    }

    return 0;
}

static void _flash_die_sel(u32 u32_page_address)
{
    u32 u32_which_die;
    u32 u32_limit_page_address;

    if (g_pst_spinand_ext_info->st_profile.u16_flags & SPINAND_MULTI_DIES)
    {
        u32_which_die = 0;
        u32_limit_page_address = g_pst_spinand_ext_info->st_profile.st_extConfig.st_dieConfig.u32_dieSize / FLASH_PAGE_SIZE;

        u32_which_die = g_pst_spinand_ext_info->st_profile.st_extConfig.st_dieConfig.st_dieCode.u8_command;

        if (u32_limit_page_address <= u32_page_address)
        {
            u32_which_die |= (1 << 8);
        }

        if (ERR_SPINAND_SUCCESS != DRV_SPINAND_complete_transmission((u8*)&u32_which_die, g_pst_spinand_ext_info->st_profile.st_extConfig.st_dieConfig.st_dieCode.u16_dataBytes + 1))
        {
            spi_nand_msg("[FLASH_ERR] Change die fail!");
        }
    }
}

static int _flash_read_from_cache(u32 u32_offset, u32 u32_address, u32 u32_size)
{
    if (ERR_SPINAND_SUCCESS != DRV_SPINAND_read_from_cache(u32_offset, (u8*)u32_address, u32_size))
    {
        printk("[FLASH_ERR] Read data from page fail!\r\n");
        return 0;
    }

    return u32_size;
}

static u32 _flash_read_oob(u32 u32_page_address, u16 u16_oob_offset, u32 u32_address, u32 u32_size)
{
    if (ERR_SPINAND_SUCCESS != DRV_SPINAND_page_read(u32_page_address))
    {
        printk("[FLASH_ERR] Page read fail!\r\n");
    }

    _flash_wait_dev_ready();

    return _flash_read_from_cache(u16_oob_offset, u32_address, u32_size);;
}

static u8 _flash_is_bad(u32 u32_page_address, u16 u16_oob_offset)
{
    u8 u8_bad;

    u8_bad = 0x00;

    if (1 == _flash_read_oob(u32_page_address, u16_oob_offset, (u32)&u8_bad, 1))
    {
        if (0xFF == u8_bad)
        {
            return 0;
        }

        printk("[FLASH] bad block @ page%d, oob_offset = 0x%x, oob = 0x%x.\r\n", u32_page_address, u16_oob_offset, u8_bad);
    }

    return 1;
}

static int _flash_check_sni_available(u32 u32_address)
{
    u8 u8_i;
    u32 u32_page_size;
    u32 u32_block_size;
    u32 u32_page_address;

    u32_page_size = FLASH_2K_PAGE_SIZE;
    u32_block_size = FLASH_2K_BLOCK_SIZE;

reLoadSNI:
    for (u8_i = 0; FLASH_SNI_BLOCKS_SIZE > u8_i; u8_i += 2)
    {
        u32_page_address = _flash_offset_to_page_address(u32_block_size * u8_i, u32_page_size);

        if (!_flash_is_bad(u32_page_address, u32_page_size))
        {
            if (ERR_SPINAND_ECC_NOT_CORRECTED != _flash_check_page_status())
            {
                _flash_read_from_cache(0, u32_address, FLASH_SNI_TABLE_SIZE);

                if (_flash_is_sni_header(au8_magicData, FLASH_SNI_HEADER_SIZE, (u8*)u32_address))
                {
                    printk("[FLASH] Found SNI in block %d.\r\n", u8_i);
                    return 1;
                }
            }
        }

    }

    if (FLASH_4K_PAGE_SIZE != u32_page_size)
    {
        u32_page_size = FLASH_4K_PAGE_SIZE;
        u32_block_size = FLASH_4K_BLOCK_SIZE;
        goto reLoadSNI;
    }

    printk("[FLASH] Not found SNI\r\n");

    return 0;
}

uint8_t	spi_nand_read_byte(struct mtd_info *mtd)
{
    u8 u8_byte;
    DRV_SPINAND_receive_data(&u8_byte, 1);
    return u8_byte;
}

u16 spi_nand_read_word(struct mtd_info *mtd)
{
    u16 u16_word;
    DRV_SPINAND_receive_data((u8*)&u16_word, 2);
    return u16_word;
}

void spi_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
    DRV_SPINAND_receive_data(buf, len);
}

void spi_nand_select_chip(struct mtd_info *mtd, int chip)
{
    spi_nand_debug("spi_nand_select_chip  Not support\r\n");
}

void spi_nand_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
    spi_nand_debug("spi_nand_cmd_ctrl Not support\r\n");
}

int spi_nand_dev_ready(struct mtd_info *mtd)
{
    spi_nand_debug("spi_nand_dev_ready Not support\r\n");

    return 1;
}


void spi_nand_cmdfunc(struct mtd_info *mtd, unsigned command, int column, int page_addr)
{
    struct nand_chip *chip;
    chip = (struct nand_chip*)mtd->priv;
    if(u8_csFlg)
    {
        DRV_QSPI_pull_cs(1);
        u8_csFlg = 0;
    }
    switch (command)
    {
        case NAND_CMD_STATUS:
            spi_nand_debug("NAND_CMD_STATUS");
            DRV_QSPI_pull_cs(0);
            u8_csFlg = 1;
            DRV_SPINAND_cmd_read_status_register();
            break;

        case NAND_CMD_PAGEPROG:
            break;

        case NAND_CMD_READOOB:
            spi_nand_debug("NAND_CMD_READOOB %d", column);
            _flash_die_sel(page_addr);
            if (ERR_SPINAND_TIMEOUT != DRV_SPINAND_page_read_with_status(page_addr))
            {
                DRV_QSPI_pull_cs(0);
                u8_csFlg = 1;
                DRV_SPINAND_cmd_normal_read_from_cache(_flash_offset_wrap(page_addr) | (mtd->writesize + column));
            }
            break;

        case NAND_CMD_READID:
            spi_nand_debug("NAND_CMD_READID");
            DRV_QSPI_pull_cs(0);
            u8_csFlg = 1;
            DRV_SPINAND_cmd_read_id();
            break;

        case NAND_CMD_ERASE2:
            spi_nand_debug("NAND_CMD_ERASE2");
            break;

        case NAND_CMD_ERASE1:
            spi_nand_debug("NAND_CMD_ERASE1, page_addr: 0x%x", page_addr);
            _flash_die_sel(page_addr);
            DRV_SPINAND_block_erase(page_addr);
            break;

        case NAND_CMD_READ0:
            break;

        case NAND_CMD_SEQIN:
            break;

        case NAND_CMD_RESET:
            spi_nand_debug("NAND_CMD_RESET");
            DRV_SPINAND_reset_status();
            break;

        default:
            printk("unsupported command %02Xh", command);
            break;
    }
    return;
}

int spi_nand_waitfunc(struct mtd_info *mtd, struct nand_chip *this)
{
    u8 u8_status;
    SPINAND_EXT_INFO_t *pst_spinand_ext_info = (SPINAND_EXT_INFO_t *)this->priv;

    DRV_SPINAND_setup_timeout(400000);

    u8_status = DRV_SPINAND_check_status();

    DRV_SPINAND_setup_timeout(pst_spinand_ext_info->st_profile.u32_maxWaitTime);

    return (ERR_SPINAND_E_FAIL > u8_status)? NAND_STATUS_READY : NAND_STATUS_FAIL;
}

void spi_nand_ecc_hwctl(struct mtd_info *mtd, int mode)
{
    spi_nand_debug(" spi_nand_ecc_hwctl Not support");
}

int spi_nand_ecc_calculate(struct mtd_info *mtd, const uint8_t *dat, uint8_t *ecc_code)
{
    spi_nand_debug("spi_nand_ecc_calculate Not support");
    return 0;
}

int spi_nand_ecc_correct(struct mtd_info *mtd, uint8_t *dat, uint8_t *read_ecc, uint8_t *calc_ecc)
{
    spi_nand_debug(" spi_nand_ecc_correct Not support");
    return 0;
}

int spi_nand_ecc_read_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
    uint8_t *buf, int oob_required, int page)
{
    do
    {
        _flash_die_sel(page);
        if(ERR_SPINAND_SUCCESS != DRV_SPINAND_page_read(page))
        {
            break;
        }
        _flash_wait_dev_ready();
        if(mtd->writesize != _flash_read_from_cache((0 | _flash_offset_wrap(page)), (u32)buf, mtd->writesize))
        {
            break;
        }

        if (oob_required)
        {
            _flash_wait_dev_ready();
            if(mtd->oobsize != _flash_read_from_cache((mtd->writesize | _flash_offset_wrap(page)), (u32)chip->oob_poi, mtd->oobsize))
            {
                break;
            }
        }

        return 0;
    }while(0);
    return -EIO;
}

int spi_nand_ecc_write_page_raw(struct mtd_info *mtd, struct nand_chip *chip, const uint8_t *buf, int oob_required, int page)
{
    do
    {
        if (ERR_SPINAND_SUCCESS != DRV_SPINAND_write_enable())
        {
            break;
        }
        _flash_wait_dev_ready();
        if(ERR_SPINAND_SUCCESS != DRV_SPINAND_program_load(0x0 | _flash_offset_wrap(page),(u8 *)buf, mtd->writesize))
        {
            break;
        }

        if (oob_required)
        {
            _flash_wait_dev_ready();
            if(ERR_SPINAND_SUCCESS != DRV_SPINAND_random_program_load(mtd->writesize | _flash_offset_wrap(page), chip->oob_poi, mtd->oobsize))
            {
                break;
            }
        }
        _flash_wait_dev_ready();
        if(ERR_SPINAND_SUCCESS != DRV_SPINAND_program_execute(page))
        {
            break;
        }

        if(NAND_STATUS_FAIL == spi_nand_waitfunc(mtd, chip))
        {
            break;
        }

        return 0;
    }while (0);
    return -EIO;
}


int spi_nand_ecc_read_page(struct mtd_info *mtd, struct nand_chip *chip,
    uint8_t *buf, int oob_required, int page)
{
    u8 u8_status;
    do
    {
        _flash_die_sel(page);
        if (ERR_SPINAND_SUCCESS != DRV_SPINAND_page_read(page))
        {
            break;
        }

        if(ERR_SPINAND_ECC_NOT_CORRECTED == (u8_status = _flash_check_page_status()))
        {
            mtd->ecc_stats.failed++;
            spi_nand_msg("ecc failed");
        }

        if (mtd->writesize != _flash_read_from_cache((0 | _flash_offset_wrap(page)), (u32)buf, mtd->writesize))
        {
            break;
        }

        if (ERR_SPINAND_ECC_CORRECTED == u8_status)
        {
            mtd->ecc_stats.corrected++;
        }

        if (oob_required)
        {
            _flash_wait_dev_ready();
            if (mtd->oobsize != _flash_read_from_cache((mtd->writesize | _flash_offset_wrap(page)), (u32)chip->oob_poi, mtd->oobsize))
            {
                break;
            }
        }
        return 0;
    }while(0);

    return -EIO;
}

int spi_nand_ecc_read_subpage(struct mtd_info *mtd, struct nand_chip *chip,
    uint32_t offs, uint32_t len, uint8_t *buf, int page)
{
    u8 u8_status;
    spi_nand_debug("page = 0x%x, offs = 0x%x, len = 0x%x", page ,offs, len);
    do
    {
        _flash_die_sel(page);
        if (ERR_SPINAND_SUCCESS != DRV_SPINAND_page_read(page))
        {
            break;
        }

        if(ERR_SPINAND_ECC_NOT_CORRECTED == (u8_status = _flash_check_page_status()))
        {
            mtd->ecc_stats.failed++;
            spi_nand_msg("ecc failed");
        }
        if (len != _flash_read_from_cache((offs | _flash_offset_wrap(page)), (u32)buf + offs, len))
        {
            break;
        }

        if (ERR_SPINAND_ECC_CORRECTED == u8_status)
        {
            mtd->ecc_stats.corrected++;
        }

        return 0;
    }while (0);

    return -EIO;
}

int spi_nand_ecc_write_subpage(struct mtd_info *mtd, struct nand_chip *chip,
                               uint32_t offset, uint32_t data_len,
                               const uint8_t *data_buf, int oob_required, int page)
{
    do
    {
        if (ERR_SPINAND_SUCCESS != DRV_SPINAND_write_enable())
        {
            break;
        }
        _flash_wait_dev_ready();
        if(ERR_SPINAND_SUCCESS != DRV_SPINAND_program_load(0x0 | _flash_offset_wrap(page), (u8*)data_buf, 0))
        {
            break;
        }
        _flash_wait_dev_ready();
        if (ERR_SPINAND_SUCCESS != DRV_SPINAND_random_program_load(offset | _flash_offset_wrap(page), (u8*)data_buf, data_len))
        {
            break;
        }
        if (oob_required)
        {
            _flash_wait_dev_ready();
            if (ERR_SPINAND_SUCCESS != DRV_SPINAND_random_program_load((mtd->writesize | _flash_offset_wrap(page)), chip->oob_poi, mtd->oobsize))
            {
                break;
            }
        }
        _flash_wait_dev_ready();
        if(ERR_SPINAND_SUCCESS != DRV_SPINAND_program_execute(page))
        {
            break;
        }

        if(NAND_STATUS_FAIL == spi_nand_waitfunc(mtd, chip))
        {
            break;
        }

        return 0;
    } while (0);

    return -EIO;
}

int spi_nand_ecc_write_page(struct mtd_info *mtd, struct nand_chip *chip, const uint8_t *buf, int oob_required, int page)
{
    spi_nand_debug("spi_nand_ecc_write_page\r\n");
    do
    {
        if (ERR_SPINAND_SUCCESS != DRV_SPINAND_write_enable())
        {
            break;
        }
        _flash_wait_dev_ready();
        if(ERR_SPINAND_SUCCESS != DRV_SPINAND_program_load(0x0 | _flash_offset_wrap(page),(u8 *)buf, mtd->writesize))
        {
            break;
        }

        if (oob_required)
        {
            _flash_wait_dev_ready();
            if (ERR_SPINAND_SUCCESS != DRV_SPINAND_random_program_load((mtd->writesize | _flash_offset_wrap(page)), chip->oob_poi, mtd->oobsize))
            {
                break;
            }
        }
        _flash_wait_dev_ready();
        if(ERR_SPINAND_SUCCESS != DRV_SPINAND_program_execute(page))
        {
            break;
        }

        if(NAND_STATUS_FAIL == spi_nand_waitfunc(mtd, chip))
        {
            break;
        }
        return 0;
    } while (0);
    return -EIO;
}

static int spi_nand_read_oob_std(struct mtd_info *mtd, struct nand_chip *chip,int page)
{
    _flash_die_sel(page);
    if (ERR_SPINAND_TIMEOUT != DRV_SPINAND_page_read_with_status(page))
    {
        DRV_QSPI_pull_cs(0);
        DRV_SPINAND_cmd_normal_read_from_cache(_flash_offset_wrap(page) | mtd->writesize);
        DRV_SPINAND_receive_data(chip->oob_poi, mtd->oobsize);
        DRV_QSPI_pull_cs(1);
        return 0;
    }
    return -EIO;
}

 static int spi_nand_write_oob_std(struct mtd_info *mtd, struct nand_chip *chip,
                   int page)
 {
    do
    {
        if (ERR_SPINAND_SUCCESS != DRV_SPINAND_write_enable())
        {
            break;
        }
        _flash_wait_dev_ready();
        if (ERR_SPINAND_SUCCESS != DRV_SPINAND_random_program_load(mtd->writesize | _flash_offset_wrap(page), chip->oob_poi, mtd->oobsize))
        {
            break;
        }
        _flash_wait_dev_ready();
        if(ERR_SPINAND_SUCCESS != DRV_SPINAND_program_execute(page))
        {
            break;
        }
        if(NAND_STATUS_FAIL == spi_nand_waitfunc(mtd, chip))
        {
            break;
        }

        return 0;
    }while(0);
     return -EIO ;
 }

static int sstar_spinand_probe(struct platform_device *pdev)
{
    u8 u8_dev_id = 0;
    u32 u32_flash_size = 0;

    struct nand_flash_dev *type = NULL;
    struct nand_chip *nand = NULL;
    struct mtd_info *mtd = NULL;

    do
    {
        if (ERR_SPINAND_SUCCESS != DRV_SPINAND_init())
        {
            printk("[FLASH_ERR] init fail!\r\n");
            break;
        }

        if (0 == g_u32_sni_address)
        {
            if (0 == (g_u32_sni_address = (u32)kzalloc(FLASH_SNI_TABLE_SIZE, GFP_KERNEL)))
            {
                break;
            }
        }

        if (NULL == (nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL)))
        {
            break;
        }

        mtd = &nand->mtd;
        mtd->priv = nand;

        g_pst_spinand_info = (SPINAND_INFO_t*)(g_u32_sni_address + sizeof(au8_magicData));
        g_pst_spinand_ext_info = (SPINAND_EXT_INFO_t*)(g_u32_sni_address + FLASH_LEAGCY_SNI_SIZE);

        if (!_flash_is_sni_header(au8_magicData, FLASH_SNI_HEADER_SIZE, (u8*)g_u32_sni_address))
        {
            if (!_flash_check_sni_available((u32)g_u32_sni_address))
            {
                break;
            }

            u32_flash_size = g_pst_spinand_info->u16_BlkPageCnt * g_pst_spinand_info->u16_PageByteCnt * g_pst_spinand_info->u16_BlkCnt;

            u8_dev_id = 0xEE;

            for (type = nand_flash_ids; type->name != NULL; type++)
            {
		        if (u8_dev_id == type->dev_id)
                {
                    printk("[FLASH] dev_id = 0x%x\r\n", type->dev_id);
                    type->mfr_id = g_pst_spinand_info->au8_ID[0];
                    type->dev_id = g_pst_spinand_info->au8_ID[1];
                    type->id_len = g_pst_spinand_info->u8_IDByteCnt;
                    strncpy(type->id, g_pst_spinand_info->au8_ID, g_pst_spinand_info->u8_IDByteCnt);
                    type->chipsize = u32_flash_size >> 20;
                    type->pagesize = g_pst_spinand_info->u16_PageByteCnt;
                    type->oobsize = g_pst_spinand_info->u16_SpareByteCnt;
                    type->erasesize = g_pst_spinand_info->u16_BlkPageCnt * g_pst_spinand_info->u16_PageByteCnt;
                    type->ecc.strength_ds = g_pst_spinand_info->u16_SectorByteCnt;
                    type->ecc.step_ds = g_pst_spinand_info->u16_PageByteCnt / g_pst_spinand_info->u16_SectorByteCnt;
                    DRV_SPINAND_alloc_bdma_buffer(type->pagesize + type->oobsize);
                    printk("[FLASH] mfr_id = 0x%x, dev_id= 0x%x id_len = 0x%x\r\n", type->id[0], type->id[1], type->id_len);
                    break;
                }
            }
            _flash_setup_by_sni(g_pst_spinand_ext_info);

            if (0 == g_pst_spinand_info->U8RIURead)
            {
                printk("[FLASH] Use BDMA.\r\n");
                DRV_SPINAND_use_bdma(1);
            }

            nand->options = NAND_BROKEN_XD | NAND_SKIP_BBTSCAN | NAND_SUBPAGE_READ | NAND_NO_SUBPAGE_WRITE;
            nand->read_byte = spi_nand_read_byte;
            nand->read_word = spi_nand_read_word;
            nand->read_buf = spi_nand_read_buf;

            nand->select_chip = spi_nand_select_chip;
            nand->cmd_ctrl = spi_nand_cmd_ctrl;
            nand->dev_ready = spi_nand_dev_ready;
            nand->cmdfunc = spi_nand_cmdfunc;
            nand->waitfunc = spi_nand_waitfunc;

            nand->bits_per_cell = 1;
            nand->chip_delay = 0;
            nand->bbt_options = NAND_BBT_USE_FLASH;
            nand->ecc.mode = NAND_ECC_HW;
            nand->ecc.size = g_pst_spinand_info->u16_SectorByteCnt;
            nand->ecc.steps = g_pst_spinand_info->u16_PageByteCnt / nand->ecc.size;
            nand->ecc.strength = nand->ecc.steps;
            nand->ecc.hwctl = spi_nand_ecc_hwctl;
            nand->ecc.calculate = spi_nand_ecc_calculate;
            nand->ecc.correct = spi_nand_ecc_correct;
            nand->ecc.read_page_raw = spi_nand_ecc_read_page_raw;
            nand->ecc.write_page_raw = spi_nand_ecc_write_page_raw;
            nand->ecc.read_page = spi_nand_ecc_read_page;
            nand->ecc.read_subpage = spi_nand_ecc_read_subpage;
            nand->ecc.write_page = spi_nand_ecc_write_page;
            nand->ecc.write_subpage = spi_nand_ecc_write_subpage;
            nand->ecc.read_oob = spi_nand_read_oob_std;
            nand->ecc.write_oob = spi_nand_write_oob_std;
            nand->priv = g_pst_spinand_ext_info;
            mtd->name = "nand0";
            mtd->owner = THIS_MODULE;

            if (0 != nand_scan(mtd, 1))
            {
                break;
            }

            nand->bits_per_cell = 1;
            nand_scan_tail(mtd);
            mtd->ooblayout = NULL;
            platform_set_drvdata(pdev, mtd);
        }
        return mtd_device_register(mtd, NULL, 0);
    } while (0);

    if (g_u32_sni_address)
    {
        kfree((void*)g_u32_sni_address);
    }

    if (nand)
    {
        kfree(nand);
    }

    return -1;

}

static int sstar_spinand_remove(struct platform_device *pdev)
{
    struct nand_chip *nand;
    struct mtd_info *mtd;

    mtd = platform_get_drvdata(pdev);
    nand = mtd->priv;

    if(mtd)
    {
        DRV_SPINAND_free_bdma_buffer(mtd->erasesize+mtd->oobsize);
    }

    if (g_u32_sni_address)
    {
        kfree((void*)g_u32_sni_address);
    }

    if (nand)
    {
        kfree(nand);
    }

    platform_set_drvdata(pdev, NULL);
    return 0;
}

#ifdef CONFIG_PM
static int sstar_spinand_suspend(struct platform_device *pdev, pm_message_t state)
{
    spi_nand_debug("%s:%d enter \n",__func__, __LINE__);
    return 0;
}

static int sstar_spinand_resume(struct platform_device *pdev)
{
    spi_nand_debug("%s:%d enter \n",__func__, __LINE__);
    DRV_FSP_QSPI_init();
    return 0;
}
#endif


static const struct of_device_id spinand_of_dt_ids[] =
{
	{ .compatible = "ss-spinand" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, spinand_of_dt_ids);

static struct platform_driver sstar_spinand_driver = {
    .probe 		= sstar_spinand_probe,
    .remove		= sstar_spinand_remove,
#ifdef CONFIG_PM
    .suspend    = sstar_spinand_suspend,
    .resume     = sstar_spinand_resume,
#endif
    .driver = {
        .name 	= DRIVER_NAME,
        .owner	= THIS_MODULE,
        .of_match_table = (spinand_of_dt_ids),
    },
};
module_platform_driver(sstar_spinand_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("Sstar MTD SPI NAND driver");

