CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

obj-$(CONFIG_MS_FLASH_ISP)           := kdrv_spinor.o

# general options
EXTRA_CFLAGS      +=  -Idrivers/sstar/flash_isp
EXTRA_CFLAGS      +=  -Idrivers/sstar/flash_isp/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/flash_isp/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/bdma/$(CONFIG_SSTAR_CHIP_NAME)

# specific options
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files
kdrv_spinor-objs := drvDeviceInfo.o
kdrv_spinor-objs += part_mxp.o
kdrv_spinor-objs += mxp_flash.o
kdrv_spinor-objs += drvSERFLASH.o
kdrv_spinor-objs += $(CONFIG_SSTAR_CHIP_NAME)/halSERFLASH.o
kdrv_spinor-objs += mtd_serflash.o
