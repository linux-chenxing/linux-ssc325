#
# Makefile for MStar KEYPAD device drivers.
#

CONFIG_SSTAR_CHIP_NAME := $(subst ",,$(CONFIG_SSTAR_CHIP_NAME))
# general options
# EXTRA_CFLAGS      +=  -Idrivers/sstar/common
EXTRA_CFLAGS      +=  -Idrivers/sstar/include
EXTRA_CFLAGS      +=  -Idrivers/sstar/include/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/keypad
EXTRA_CFLAGS      +=  -Idrivers/sstar/keypad/$(CONFIG_SSTAR_CHIP_NAME)
EXTRA_CFLAGS      +=  -Idrivers/sstar/gpio
# specific options
EXTRA_CFLAGS      +=  -DTITANIA
EXTRA_CFLAGS      +=  -DMSOS_TYPE_LINUX

# files

obj-$(CONFIG_SS_KEYPAD) += ms_keypad.o
ms_keypad-y             += mdrv_keypad.o  $(CONFIG_SSTAR_CHIP_NAME)/mhal_keypad.o

# export header files
#EXPORT_H_FILES    += mdrv_keypad.h
