CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

obj-$(CONFIG_MS_FLASH_ISP)           := kdrv_spinor.o

# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/flash_isp/inc
EXTRA_CFLAGS      +=  -Idrivers/sstar/flash_isp/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/bdma/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/fsp_qspi/drv/inc
EXTRA_CFLAGS      +=  -Idrivers/sstar/fsp_qspi/hal/inc
# specific options
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
kdrv_spinor-objs += src/drvSPINOR.o
kdrv_spinor-objs += src/mtd_serflash.o
