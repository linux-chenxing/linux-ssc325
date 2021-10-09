/*
 *#############################################################################
 * SigmaStar trade secret
 * Copyright (c) 2006-2011 SigmaStar Technology Corp.
 * All rights reserved.
 *
 * Unless otherwise stipulated in writing, any and all information contained
 * herein regardless in any format shall remain the sole proprietary of
 * SigmaStar Technology Corp. and be kept in strict confidence
 * (SigmaStar Confidential Information) by the recipient.
 * Any unauthorized act including without limitation unauthorized disclosure,
 * copying, use, reproduction, sale, distribution, modification, disassembling,
 * reverse engineering and compiling of the contents of SigmaStar Confidential
 * Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
 * rights to any and all damages, losses, costs and expenses resulting therefrom.
 *
 *#############################################################################
 */
#include <drvSPINOR.h>
#include <drvFSP_QSPI.h>
#include <linux/types.h>
#include <linux/mtd/concat.h>
#include <linux/delay.h>
#include <ms_msys.h>
#include <hal_bdma.h>

static u8 (*_DRV_SPINOR_read)(u32, u8*, u32);
static u8 (*_DRV_SPINOR_program_load_data)(u8*, u32);

DRV_SPINNOR_INFO_t st_spinor_drv_info;

static struct bdma_alloc_dmem
{
    dma_addr_t  bdma_phy_addr ;
    const char* DMEM_BDMA_INPUT;
    u8 *bdma_vir_addr;
}ALLOC_DMEM = {0, "BDMA", 0};

static void* alloc_dmem(const char* name, unsigned int size, dma_addr_t *addr)
{
    MSYS_DMEM_INFO dmem;
    memcpy(dmem.name,name,strlen(name)+1);
    dmem.length=size;
    if(0!=msys_request_dmem(&dmem)){
        return NULL;
    }
    *addr=dmem.phys;
    return (void *)((uintptr_t)dmem.kvirt);
}

static void free_dmem(const char* name, unsigned int size, void *virt, dma_addr_t addr )
{
    MSYS_DMEM_INFO dmem;
    memcpy(dmem.name,name,strlen(name)+1);
    dmem.length=size;
    dmem.kvirt=(unsigned long long)((uintptr_t)virt);
    dmem.phys=(unsigned long long)((uintptr_t)addr);
    msys_release_dmem(&dmem);
}

void DRV_SPINOR_alloc_bdma_buffer(u32 u32DataSize)
{
    if (!(ALLOC_DMEM.bdma_vir_addr = alloc_dmem(ALLOC_DMEM.DMEM_BDMA_INPUT,
                                             u32DataSize,
                                             &ALLOC_DMEM.bdma_phy_addr))){
        printk(KERN_ERR "[SPINAND] unable to allocate bmda buffer\n");
    }
    memset(ALLOC_DMEM.bdma_vir_addr, 0, u32DataSize);
}

void DRV_SPINOR_free_bdma_buffer(u32 u32DataSize)
{
    if(ALLOC_DMEM.bdma_vir_addr != 0)
    {
        free_dmem(ALLOC_DMEM.DMEM_BDMA_INPUT, u32DataSize, ALLOC_DMEM.bdma_vir_addr, ALLOC_DMEM.bdma_phy_addr);
        ALLOC_DMEM.bdma_vir_addr = 0;
    }
}

static u8 DRV_SPINOR_is_quad_cmd(u8 u8_cmd)
{
    switch (u8_cmd)
    {
        case SPI_NOR_CMD_QP:
        case SPI_NOR_CMD_4PP:
        case SPI_NOR_CMD_QR_6B:
        case SPI_NOR_CMD_QR_EB:
            return 1;
        default:
            return 0;
    }
}

static u8 _DRV_SPINOR_return_status(u8 u8_status)
{
    if (ERR_SPINOR_INVALID <= u8_status)
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    return u8_status;
}
u8 DRV_SPINOR_receive_data(u8* pu8_buf, u32 u32_size)
{
    u8 u8_rd_size;

    DRV_FSP_init(FSP_USE_SINGLE_CMD);

    while (0 != u32_size)
    {
        DRV_FSP_set_which_cmd_size(1, 0);
        u8_rd_size = DRV_FSP_set_rbf_size_after_which_cmd(1, u32_size);

        DRV_FSP_trigger();

        if (!DRV_FSP_is_done())
        {
            break;
        }

        u32_size -= DRV_FSP_read_rbf(pu8_buf, u8_rd_size);
        pu8_buf += u8_rd_size;

        DRV_FSP_clear_trigger();
    }

    if (0 != u32_size)
    {
        return ERR_SPINOR_TIMEOUT;
    }

    return ERR_SPINOR_SUCCESS;
}

static u8 _DRV_SPINOR_simple_transmission(u8* pu8_buf, u32 u32_size)
{
    u8 u8_wr_size;

    DRV_FSP_init(FSP_USE_SINGLE_CMD);
    while (0 != u32_size)
    {
        u8_wr_size = DRV_FSP_write_wbf(pu8_buf, u32_size);
        u32_size -= u8_wr_size;
        pu8_buf += u8_wr_size;
        DRV_FSP_set_which_cmd_size(1, u8_wr_size);

        DRV_FSP_trigger();

        if (!DRV_FSP_is_done())
        {
            break;
        }

        DRV_FSP_clear_trigger();
    }

    if (0 != u32_size)
    {
        return ERR_SPINOR_TIMEOUT;
    }

    return ERR_SPINOR_SUCCESS;
}

u8 DRV_SPINOR_complete_transmission(u8* pu8_buf, u32 u32_size)
{
    u8 u8_status;

    DRV_QSPI_pull_cs(0);
    
    u8_status = _DRV_SPINOR_simple_transmission(pu8_buf, u32_size);

    DRV_QSPI_pull_cs(1);
    
    return _DRV_SPINOR_return_status(u8_status);
    
}
u8 DRV_SPINOR_check_status(void)
{
    u32 u32_left_time;
    u32 u32_status;

    u32_left_time = st_spinor_drv_info.u32_time_wait;

    do
    {
        if (0 == u32_left_time)
        {
            printk("[SPINOR] check timeout! @0x%x us\r\n",st_spinor_drv_info.u32_time_wait);
            return ERR_SPINOR_TIMEOUT;
        }

        if (ERR_SPINOR_SUCCESS != DRV_SPINOR_read_status1((u8*)&u32_status, 1))
        {
            printk("[SPINOR] DRV_SPINOR_read_status1 timeout!\r\n");
            return ERR_SPINOR_TIMEOUT;
        }

        if (0 == (u32_status & SPI_NOR_BUSY))
        {
            break;
        }

        udelay(10);

        if (10 < u32_left_time)
        {
            u32_left_time -= 10;
        }
        else
        {
            u32_left_time = 0;
        }

    } while (1);

    return ERR_SPINOR_SUCCESS;
}

u8 DRV_SPINOR_write_enable(void)
{
    u8 u8_cmd;

    u8_cmd = SPI_NOR_CMD_WREN;

    return DRV_SPINOR_complete_transmission(&u8_cmd, 1);
}

static u8 _DRV_SPINOR_reset(void)
{
    u8 u8_cmd;
    u8 u8_status;

    u8_cmd = SPI_NOR_CMD_EN_RESET;

    if (ERR_SPINOR_SUCCESS == (u8_status = DRV_SPINOR_complete_transmission(&u8_cmd, 1)))
    {
        u8_cmd = SPI_NOR_CMD_RESET;
        u8_status = DRV_SPINOR_complete_transmission(&u8_cmd, 1);
    }

    if (ERR_SPINOR_SUCCESS == u8_status)
    {
        u8_status = DRV_SPINOR_check_status();
    }

    return _DRV_SPINOR_return_status(u8_status);

}

static u8 _DRV_SPINOR_reset_status(void)
{
    if (ERR_SPINOR_SUCCESS != _DRV_SPINOR_reset())
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    return DRV_SPINOR_check_status();
}

u8 _DRV_SPINOR_use_ext_address(u8 u8_ext)
{
    u8 au8_cmd[2];

    au8_cmd[0] = SPI_NOR_CMD_WREAR;
    au8_cmd[1] = u8_ext;

    if (ERR_SPINOR_SUCCESS != DRV_SPINOR_write_enable())
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    return DRV_SPINOR_complete_transmission(au8_cmd, 2);
}

u8 DRV_SPINOR_read_status(u8 u8_cmd, u8 *pu8_status, u8 u8_size)
{
    u8 u8_status;

    DRV_QSPI_pull_cs(0);
    
    if (ERR_SPINOR_SUCCESS == (u8_status = _DRV_SPINOR_simple_transmission(&u8_cmd, 1)))
    {
        u8_status = DRV_SPINOR_receive_data(pu8_status, u8_size);
    }

    DRV_QSPI_pull_cs(1);
    return _DRV_SPINOR_return_status(u8_status);
}

u8 DRV_SPINOR_read_status1(u8 *pu8_status, u8 u8_size)
{
    return DRV_SPINOR_read_status(SPI_NOR_CMD_RDSR, pu8_status, u8_size);
}

u8 DRV_SPINOR_read_status2(u8 *pu8_status, u8 u8_size)
{
    return DRV_SPINOR_read_status(SPI_NOR_CMD_RDSR2, pu8_status, u8_size);
}

u8 DRV_SPINOR_read_status3(u8 *pu8_status, u8 u8_size)
{
    return DRV_SPINOR_read_status(SPI_NOR_CMD_RDSR3, pu8_status, u8_size);
}

u8 DRV_SPINOR_write_status(u8 u8_cmd, u8 *pu8_status, u8 u8_size)
{
    u8 u8_status;

    if (ERR_SPINOR_SUCCESS != DRV_SPINOR_write_enable())
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    DRV_QSPI_pull_cs(0);
    
    if (ERR_SPINOR_SUCCESS == (u8_status = _DRV_SPINOR_simple_transmission(&u8_cmd, 1)))
    {
        u8_status = _DRV_SPINOR_simple_transmission(pu8_status, u8_size);
    }

    DRV_QSPI_pull_cs(1);

    if (ERR_SPINOR_SUCCESS == u8_status)
    {
        u8_status = DRV_SPINOR_check_status();
    }

    return _DRV_SPINOR_return_status(u8_status);
}

u8 DRV_SPINOR_write_status1(u8 *pu8_status, u8 u8_size)
{
    return DRV_SPINOR_write_status(SPI_NOR_CMD_WRSR, pu8_status, u8_size);
}

u8 DRV_SPINOR_write_status2(u8 *pu8_status, u8 u8_size)
{
    return DRV_SPINOR_write_status(SPI_NOR_CMD_WRSR2, pu8_status, u8_size);
}

u8 DRV_SPINOR_write_status3(u8 *pu8_status, u8 u8_size)
{
    return DRV_SPINOR_write_status(SPI_NOR_CMD_WRSR3, pu8_status, u8_size);
}

static u8 _DRV_SPINOR_program_load_data_by_bdma(u8 *pu8_data, u32 u32_size)
{
    u8 u8_status = ERR_SPINOR_SUCCESS;
    HalBdmaParam_t    tBdmaParam;

    DRV_FSP_init(FSP_USE_SINGLE_CMD);
    DRV_FSP_set_which_cmd_size(1, 0);
    DRV_FSP_enable_outside_wbf(0, 0, u32_size);
    do
    {
        memset(&tBdmaParam, 0, sizeof(HalBdmaParam_t));
        tBdmaParam.bIntMode 		 = 0; //0:use polling mode
        tBdmaParam.ePathSel 		 = HAL_BDMA_MIU0_TO_SPI;
        tBdmaParam.eSrcDataWidth	 = HAL_BDMA_DATA_BYTE_16;
        tBdmaParam.eDstDataWidth	 = HAL_BDMA_DATA_BYTE_8;
        tBdmaParam.eDstAddrMode 	 = HAL_BDMA_ADDR_INC; //address increase
        tBdmaParam.u32TxCount		 = u32_size;
        tBdmaParam.u32Pattern		 = 0;
        tBdmaParam.pSrcAddr 		 = (void*)(u32)(Chip_Phys_to_MIU(ALLOC_DMEM.bdma_phy_addr));
        tBdmaParam.pDstAddr 		 = 0;
        tBdmaParam.pfTxCbFunc		 = NULL;

        DRV_FSP_trigger();
        memcpy((void*)ALLOC_DMEM.bdma_vir_addr,(const void *)pu8_data,u32_size);
        Chip_Flush_MIU_Pipe();
        if (HAL_BDMA_PROC_DONE != HalBdma_DoTransfer(HAL_BDMA_CH0, &tBdmaParam))
        {
            u8_status = ERR_SPINOR_BDMA_FAILURE;
            break;
        }

        if (!DRV_FSP_is_done())
        {
            u8_status = ERR_SPINOR_TIMEOUT;
        }
    } while (0);

    DRV_FSP_clear_trigger();
    DRV_FSP_disable_outside_wbf();

    return _DRV_SPINOR_return_status(u8_status);
}

static u8 _DRV_SPINOR_program_page(u8 u8_cmd, u32 u32_address, u8 *pu8_data, u32 u32_size)
{
    u8 u8_status;
    u8 au8_cmd[3];
    u32 u32_bytes_to_load;

    au8_cmd[0] = (u32_address >> 16) & 0xff;
    au8_cmd[1] = (u32_address >> 8) & 0xff;
    au8_cmd[2] = u32_address & 0xff;

    if (ERR_SPINOR_SUCCESS != DRV_SPINOR_write_enable())
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    DRV_QSPI_pull_cs(0);
    do
    {
        if (ERR_SPINOR_SUCCESS != (u8_status = _DRV_SPINOR_simple_transmission(&u8_cmd, 1)))
        {
            break;
        }

        if (SPI_NOR_CMD_4PP == u8_cmd)
        {
            DRV_FSP_use_quad_mode(1);
        }

        if (ERR_SPINOR_SUCCESS != (u8_status = _DRV_SPINOR_simple_transmission(au8_cmd, 3)))
        {
            break;
        }

        DRV_FSP_use_quad_mode(DRV_SPINOR_is_quad_cmd(u8_cmd));

        while (0 != u32_size)
        {
            u32_bytes_to_load = u32_size;
            /*align to 64,for cache*/
            if (0 != ((u32)pu8_data % 64))
            {
                u32_bytes_to_load = 64 - ((u32)pu8_data % 64);

                if (u32_size < u32_bytes_to_load)
                {
                    u32_bytes_to_load = u32_size;
                }
            }
            else if (0 != (u32_size % 64))/*align to 64,for cache*/
            {
                u32_bytes_to_load = u32_size & ~0x3f;

                if (0 == u32_bytes_to_load)
                {
                    u32_bytes_to_load = u32_size;
                }
            }
            /*align to 64,for cache*/
            if (0 != (u32_bytes_to_load % 64))
            {
                u8_status = _DRV_SPINOR_simple_transmission(pu8_data, u32_bytes_to_load);
            }
            else
            {
                u8_status = _DRV_SPINOR_program_load_data(pu8_data, u32_bytes_to_load);
            }

            if (ERR_SPINOR_SUCCESS != u8_status)
            {
                break;
            }
            
            pu8_data += u32_bytes_to_load;
            u32_size -= u32_bytes_to_load;
        }
            
        DRV_FSP_use_quad_mode(0);
    } while (0);

    DRV_QSPI_pull_cs(1);

    if (ERR_SPINOR_SUCCESS == u8_status)
    {
        u8_status = DRV_SPINOR_check_status();
    }

    return _DRV_SPINOR_return_status(u8_status);

}

u8 DRV_SPINOR_program_page(u32 u32_address, u8 *pu8_data, u32 u32_size)
{
    if (0 != (u32_address & ~SPI_NOR_16MB_MASK))
    {
        if (ERR_SPINOR_SUCCESS != _DRV_SPINOR_use_ext_address(1))
        {
            return ERR_SPINOR_DEVICE_FAILURE;
        }
    }

    if (ERR_SPINOR_SUCCESS != _DRV_SPINOR_program_page(st_spinor_drv_info.u8_pageProgram, (u32_address & SPI_NOR_16MB_MASK), pu8_data, u32_size))
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    if (0 != (u32_address & ~SPI_NOR_16MB_MASK))
    {
        if (ERR_SPINOR_SUCCESS != _DRV_SPINOR_use_ext_address(0))
        {
            return ERR_SPINOR_DEVICE_FAILURE;
        }
    }

    return ERR_SPINOR_SUCCESS;

}

static u8 _DRV_SPINOR_read_data_by_riu(u32 u32_address, u8 *pu8_data, u32 u32_size)
{
    u8 u8_status;
    u8 au8_cmd[3];
    u8 u8_dummy = 0;
    u8 u8_wr_size = 0;

    DRV_QSPI_pull_cs(0);

    do
    {
        if (ERR_SPINOR_SUCCESS != (u8_status = _DRV_SPINOR_simple_transmission(&st_spinor_drv_info.u8_readData, 1)))
        {
            break;
        }

        au8_cmd[0] = (u32_address >> 16) & 0xff;
        au8_cmd[1] = (u32_address >> 8) & 0xff;
        au8_cmd[2] = u32_address & 0xff;

        u8_dummy = (st_spinor_drv_info.u8_dummy * 1) / 8;

        if (0xEB == st_spinor_drv_info.u8_readData)
        {
            u8_dummy = (st_spinor_drv_info.u8_dummy * 4) / 8;
            DRV_FSP_use_quad_mode(1);
        }

        do
        {
            if (ERR_SPINOR_SUCCESS != (u8_status = _DRV_SPINOR_simple_transmission(au8_cmd, 3)))
            {
                break;
            }

            while (0 != u8_dummy)
            {
                au8_cmd[u8_wr_size++] = 0x00;
                u8_dummy--;

                if (0 == u8_dummy || u8_wr_size == sizeof(au8_cmd))
                {
                    if (ERR_SPINOR_SUCCESS != (u8_status = _DRV_SPINOR_simple_transmission(au8_cmd, u8_wr_size)))
                    {
                        break;
                    }

                    u8_wr_size = 0;
                }
            }

            DRV_FSP_use_quad_mode(DRV_SPINOR_is_quad_cmd(st_spinor_drv_info.u8_readData));

            if (ERR_SPINOR_SUCCESS == u8_status)
            {
                u8_status = DRV_SPINOR_receive_data(pu8_data, u32_size);
            }
        } while (0);

        DRV_FSP_use_quad_mode(0);
    } while (0);

    DRV_QSPI_pull_cs(1);

    return _DRV_SPINOR_return_status(u8_status);

}

static u8 _DRV_SPINOR_read_data_by_bdma(u32 u32_address, u8 *pu8_data, u32 u32_size)
{
    u8 u8_status = ERR_SPINOR_SUCCESS;
    HalBdmaParam_t    tBdmaParam;
    DRV_QSPI_pull_cs(0);
    DRV_QSPI_use_3bytes_address_mode(DRV_QSPI_cmd_to_mode(st_spinor_drv_info.u8_readData), st_spinor_drv_info.u8_dummy);
    memset(&tBdmaParam, 0, sizeof(HalBdmaParam_t));
    tBdmaParam.bIntMode          = 0; //0:use polling mode
    tBdmaParam.ePathSel          = HAL_BDMA_SPI_TO_MIU0;
    tBdmaParam.eSrcDataWidth     = HAL_BDMA_DATA_BYTE_8;
    tBdmaParam.eDstDataWidth     = HAL_BDMA_DATA_BYTE_16;
    tBdmaParam.eDstAddrMode      = HAL_BDMA_ADDR_INC; //address increase
    tBdmaParam.u32TxCount        = u32_size;
    tBdmaParam.u32Pattern        = 0;
    tBdmaParam.pSrcAddr          = (void*)u32_address;
    tBdmaParam.pDstAddr          = (void*)(u32)(Chip_Phys_to_MIU(ALLOC_DMEM.bdma_phy_addr));
    tBdmaParam.pfTxCbFunc        = NULL;

    if (HAL_BDMA_PROC_DONE != HalBdma_DoTransfer(HAL_BDMA_CH0, &tBdmaParam))
    {
        printk(KERN_ERR "[SPINOR] bdma read fail\r\n");
        u8_status = ERR_SPINOR_BDMA_FAILURE;
    }

    DRV_QSPI_use_3bytes_address_mode(DRV_QSPI_cmd_to_mode(SPI_NOR_CMD_FASTREAD), 8);
    memcpy((void *)pu8_data,(const void*)ALLOC_DMEM.bdma_vir_addr, u32_size);
    DRV_QSPI_pull_cs(1);
    return _DRV_SPINOR_return_status(u8_status);
}

u8 DRV_SPINOR_read(u32 u32_address, u8 *pu8_data, u32 u32_size)
{
    u8 u8_ext;
    u8 u8_status;
    u32 u32_read_size;
    u32 u32_bytes_to_read;

    while (0 != u32_size)
    {
        u32_read_size = SPI_NOR_16MB - (u32_address & SPI_NOR_16MB_MASK);
        if (u32_size < u32_read_size)
        {
            u32_read_size = u32_size;
        }

        u8_ext = 0;
        if (u32_address & ~SPI_NOR_16MB_MASK)
        {
            u8_ext = 1;
        }

        if (ERR_SPINOR_SUCCESS != (u8_status = _DRV_SPINOR_use_ext_address(u8_ext)))
        {
            break;
        }

        while (0 != u32_read_size)
        {
            u32_bytes_to_read = (u32_read_size & ~0x3f);;
            /*align to 16,patch for bdma*/
            if (0 != (u32_address % 16))
            {
                u32_bytes_to_read = 16 - (u32_address % 16);
            }
            else
            {
                /*align to 64,for cache*/
                if (0 != ((u32)pu8_data % 64))
                {
                    u32_bytes_to_read = 64 - ((u32)pu8_data % 64);
                }

                if (0 == u32_bytes_to_read)
                {
                    u32_bytes_to_read = u32_read_size & 0x3f;;
                }
            }

            if (u32_read_size < u32_bytes_to_read)
            {
                u32_bytes_to_read = u32_read_size;
            }
            /*align to 64,for cache*/
            if (0 != (u32_bytes_to_read % 64))
            {
                u8_status = _DRV_SPINOR_read_data_by_riu(u32_address & SPI_NOR_16MB_MASK, pu8_data, u32_bytes_to_read);
            }
            else
            {
                u8_status = _DRV_SPINOR_read(u32_address & SPI_NOR_16MB_MASK, pu8_data, u32_bytes_to_read);
            }

            if (ERR_SPINOR_SUCCESS != u8_status)
            {
                break;
            }

            pu8_data += u32_bytes_to_read;
            u32_address += u32_bytes_to_read;
            u32_read_size -= u32_bytes_to_read;
            u32_size -= u32_bytes_to_read;
        }
        
        if (ERR_SPINOR_SUCCESS != u8_status)
        {
            break;
        }
    }

    return _DRV_SPINOR_return_status(u8_status);
}

static u8 _DRV_SPINOR_erase_status(u8 u8_command, u32 u32_address)
{
    u8 au8_cmd[4];
    u8 u8_wr_size;

    u8_wr_size = 0;
    au8_cmd[u8_wr_size++] = u8_command;
    au8_cmd[u8_wr_size++] = (u32_address >> 16)& 0xff;
    au8_cmd[u8_wr_size++] = (u32_address >> 8)& 0xff;
    au8_cmd[u8_wr_size++] = u32_address & 0xff;

    if (ERR_SPINOR_SUCCESS != DRV_SPINOR_write_enable() || ERR_SPINOR_SUCCESS != DRV_SPINOR_complete_transmission(au8_cmd, u8_wr_size))
    {
        return ERR_SPINOR_DEVICE_FAILURE;
    }

    return DRV_SPINOR_check_status();
}

static u8 _DRV_SPINOR_erase(u8 u8_command, u32 u32_address)
{
    u8 u8_status;

    do
    {
        if (0 != (u32_address & ~SPI_NOR_16MB_MASK))
        {
            if (ERR_SPINOR_SUCCESS != (u8_status = _DRV_SPINOR_use_ext_address(1)))
            {
                break;
            }
        }

        if (ERR_SPINOR_SUCCESS != (u8_status = _DRV_SPINOR_erase_status(u8_command, u32_address & SPI_NOR_16MB_MASK)))
        {
            return ERR_SPINOR_DEVICE_FAILURE;
        }
        
        if (0 != (u32_address & ~SPI_NOR_16MB_MASK))
        {
            if (ERR_SPINOR_SUCCESS != (u8_status = _DRV_SPINOR_use_ext_address(0)))
            {
                break;
            }
        }
    } while (0);

    return _DRV_SPINOR_return_status(u8_status);

}

u8 DRV_SPINOR_block_erase(u32 u32_address)
{
    return _DRV_SPINOR_erase(SPI_NOR_CMD_64BE, u32_address);
}

u8 DRV_SPINOR_sector_erase(u32 u32_address)
{
    return _DRV_SPINOR_erase(SPI_NOR_CMD_SE, u32_address);
}

u8 DRV_SPINOR_read_id(u8 *pu8_ids, u8 u8_bytes)
{
    u8 u8_status;
    u8 u8_cmd;

    u8_cmd = SPI_NOR_CMD_RDID;
    DRV_QSPI_pull_cs(0);
    if (ERR_SPINOR_SUCCESS == (u8_status = _DRV_SPINOR_simple_transmission((u8*)&u8_cmd, 1)))
    {
        u8_status = DRV_SPINOR_receive_data(pu8_ids, u8_bytes);
    }
    DRV_QSPI_pull_cs(1);
    return _DRV_SPINOR_return_status(u8_status);
}

void DRV_SPINOR_use_bdma(u8 u8_enabled)
{
    if (u8_enabled)
    {
        _DRV_SPINOR_program_load_data = _DRV_SPINOR_program_load_data_by_bdma;
        _DRV_SPINOR_read = _DRV_SPINOR_read_data_by_bdma;
    }
    else
    {
        _DRV_SPINOR_program_load_data = _DRV_SPINOR_simple_transmission;
        _DRV_SPINOR_read = _DRV_SPINOR_read_data_by_riu;
    }
}

void DRV_SPINOR_setup_time_wait(u32 u32_time_wait)
{
    st_spinor_drv_info.u32_time_wait = u32_time_wait;
}

void DRV_SPINOR_setup_access(u8 u8_read, u8 u8_dummy, u8 u8_program)
{
    st_spinor_drv_info.u8_readData = u8_read;
    st_spinor_drv_info.u8_dummy = u8_dummy;
    st_spinor_drv_info.u8_pageProgram = u8_program;

    printk("[SPINOR] ReadData(0x%x),Dummy(0x%x),pageProgram(0x%x)\r\n",st_spinor_drv_info.u8_readData,st_spinor_drv_info.u8_dummy,st_spinor_drv_info.u8_pageProgram);

    return;
}

u8 DRV_SPINOR_init(void)
{
    u8 u8_status = 0;

    DRV_FSP_QSPI_init();
    st_spinor_drv_info.u8_readData = SPI_NOR_CMD_FASTREAD;
    st_spinor_drv_info.u8_dummy = 0x08;
    st_spinor_drv_info.u8_pageProgram = SPI_NOR_CMD_PP;
    st_spinor_drv_info.u32_time_wait = -1;

    DRV_QSPI_use_3bytes_address_mode(DRV_QSPI_cmd_to_mode(SPI_NOR_CMD_FASTREAD), 0x08);

    _DRV_SPINOR_read = _DRV_SPINOR_read_data_by_riu;
    _DRV_SPINOR_program_load_data = _DRV_SPINOR_simple_transmission;

    u8_status = _DRV_SPINOR_reset_status();
    return u8_status;
}

