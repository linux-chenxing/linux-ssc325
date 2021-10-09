CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))

obj-$(CONFIG_MS_SPINAND) += kdrv_spinand.o

# general options

EXTRA_CFLAGS      +=  -Idrivers/sstar/bdma/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/spinand/inc
EXTRA_CFLAGS      +=  -Idrivers/sstar/fsp_qspi/drv/inc
EXTRA_CFLAGS      +=  -Idrivers/mtd

# specific options
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

kdrv_spinand-objs := src/spinand.o \
                     src/drvSPINAND.o  \
                     src/spinand_bbt.o
